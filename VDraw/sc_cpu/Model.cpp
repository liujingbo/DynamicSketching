/*
 * Implementation of the Model class, a class representing a loaded mesh and its view-independent and view-dependent data.
 *
 * Author: Jeroen Baert
 */

#include "Model.h"
#include "mesh_info.h"
#include "vertex_info.h"
#include "Drawer.h"
#include "fitting\FittingCylinder.h"
/**
 * Constructor: construct a model
 * @param filename : the filesystem location of the file containing mesh_ data
 */
Model::Model(const char* filename)
{
	// read mesh_ from file
	TriMesh* mesh = TriMesh::read(filename);
	// TriMesh2 library can calculate some vertex-info for us
	mesh->need_across_edge();
	mesh->need_normals();
	mesh->need_tstrips();
	mesh->need_bsphere();
	mesh->need_curvatures();
	mesh->need_across_edge();
	mesh->need_dcurv();
	mesh->need_neighbors(); // for chaining
	mesh_ = mesh;
	computeViewIndependentData();
	setupVBOs();

	fitting_cylinders.clear();

	//convex hull
	this->viewport = new int[4];
	for(int i=0;i<4;i++) viewport[i]=0;

	this->modelview = new double[16];
	this->projection = new double[16];
	for(int i=0;i<16;i++){modelview[i]=projection[i]=0;}

	// 3d CG lines
	connectDistThreshold = 6;//3d:0.07;//0.00000618;
	connectDistThreshold_max = 10;
	connectDistThreshold_min = 0;
	connectAngThreshold = 0.565;
	connectAngThreshold_max = 1;
	connectAngThreshold_min = -0.5;

	// graph
	this->graph = NULL;
}

Model::~Model(){
	if(viewport) delete viewport;
	if(modelview) delete modelview;
	if(projection) delete projection;
	if(graph != NULL) delete graph;
}

/**
 * Draw a given model using it's current draw stack.
 *
 * @param camera_position: the current position of the camera, in 3d coordinates
 */
void Model::draw(vec camera_position){
	// clear all view-dependent buffers: drawers_ will fill them as necessary
	clearViewDependentData();
	// for every drawer in the draw stack, call draw function
	for(unsigned int i = 0; i<drawers_.size(); i++){
		drawers_[i]->draw(this, camera_position);
	}
}

/**
 * Push back a drawer into this model's drawing stack
 * @param: d : the drawer you want to push
 */
void Model::pushDrawer(Drawer* d){
	drawers_.push_back(d);
}

/**
 * Pop a drawer from this model's drawing stack
 */
void Model::popDrawer(){
	drawers_.pop_back();
}

/**
 * Compute NdotV for all vertices, given a camera position
 */
void Model::needNdotV(vec camera_position)
{
	if(ndotv_.empty()){
		compute_ndotv(mesh_,camera_position,ndotv_);
	}
}

/**
 * Compute radial curvature and the numerator/denominator of the directional curvature derivative
 * given a camera standpoint
 *
 * @param: camera_position : the camera standpoint
 * @param: sc_threshold : a filtering threshold for small curvatures
 */
void Model::needCurvDerivatives(vec camera_position, float sc_threshold)
{
	if(kr_.empty() || num_.empty() || den_.empty()){
		int n = mesh_->vertices.size();
		ndotv_.resize(n);
		kr_.resize(n);
		num_.resize(n);
		den_.resize(n);
		compute_CurvDerivatives(mesh_,camera_position,kr_,num_,den_,sc_threshold);
	}
}

/**
 * Compute the view independent data for this model.
 */
void Model::computeViewIndependentData(){
	// reserve memory chunks for per-vertex info
	int n = mesh_->vertices.size();
	ndotv_.resize(n);
	kr_.resize(n);
	num_.resize(n);
	den_.resize(n);
	// The other view-indipendent data, we have to compute ourselves
	std::cout<<"Computing face normals... ";
	computeFaceNormals(mesh_,facenormals_);
	std::cout<< "Done" << std::endl << "Computing feature size... ";
	feature_size_ = computeFeatureSize(mesh_);
	std::cout<< "Done" << std::endl;
}

/**
 * set patch number of each face for this model.
 	vector<int> facecolors_; // for patch color
	vector<vec> colorlist_;
 */
void postSetPatchNumber(){
	

}


/**
 * Clear the buffers containing the view dependent data
 */
void Model::clearViewDependentData(){
	ndotv_.clear();
	kr_.clear();
	num_.clear();
	den_.clear();
}

/**
 * Transfer vertex/normal info into GPU memory as STATIC_DRAW data in Vertex Buffer Objects (VBO's).
 */
void Model::setupVBOs(){
	int bufferSize;
	// load vertex positions into VBO buffer
	glGenBuffersARB(1, &vbo_positions_);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo_positions_);
	// static draw data, we're not going to change vertex information
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, mesh_->vertices.size()*sizeof(float)*3, &(mesh_->vertices[0]), GL_STATIC_DRAW_ARB);
	glGetBufferParameterivARB(GL_ARRAY_BUFFER_ARB, GL_BUFFER_SIZE_ARB, &bufferSize);
	std::cout << "Vertex array loaded in VBO: " << bufferSize << " bytes\n" << std::endl;
	// do the same for normals
	glGenBuffersARB(1, &vbo_normals_);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo_normals_);
	// static draw data, we're not going to change vertex information
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, mesh_->normals.size()*sizeof(float)*3, &(mesh_->normals[0]), GL_STATIC_DRAW_ARB);
	glGetBufferParameterivARB(GL_ARRAY_BUFFER_ARB, GL_BUFFER_SIZE_ARB, &bufferSize);
	std::cout << "Normal array loaded in VBO: " << bufferSize << " bytes\n" << std::endl;
	// unbind buffers to prevent fudging up pointer arithmetic
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
}
/********************************************************************
	                      Hierarchy 
*********************************************************************/	

//bool Model::setHierarchyGraph(HierarchyGraph* hg_){
//	if(hg_ != NULL){
//		this->hierarchy = hg_;
//		return true;
//	}
//	else {
//		return false;
//	}
//
//}

/********************************************************************
	                  Primitive  Fitting
*********************************************************************/

void Model::initPatches(){
	// init patches
	patches.clear();
	patches_vertex_index.clear();

	this->patches.resize(this->colorlist_.size()/3);
	for(int i=0; i<this->facecolors_.size();i++){
		patches[facecolors_[i]].push_back(i);
	}

	this->patches_vertex_index.resize(this->colorlist_.size()/3);
	for(int i=0;i<this->pointcolors_.size();i++){
		patches_vertex_index[pointcolors_[i]].push_back(i);
	}
}
double Model::ComputeFaceArea(int fIndex){
	TriMesh::Face f = this->mesh_->faces[fIndex];
	point v1 = this->mesh_->vertices[f[0]];
	point v2 = this->mesh_->vertices[f[1]];
	point v3 = this->mesh_->vertices[f[2]];
	point tmp = ((v2-v1) CROSS (v3-v1));
	return (tmp DOT tmp) / 2.0;
}
double Model::ComputeAllFaceAreaCenter(){		
	int fLen = this->mesh_->faces.size();
	this->faceareas_.clear();
	this->faceareas_.resize(fLen);
	this->facecenters_.clear();
	this->facecenters_.resize(fLen);
	for(int i=0;i<fLen;i++){
		int fIndex = i;
		TriMesh::Face f = this->mesh_->faces[fIndex];
		point v1 = this->mesh_->vertices[f[0]];
		point v2 = this->mesh_->vertices[f[1]];
		point v3 = this->mesh_->vertices[f[2]];
		point tmp = ((v2-v1) CROSS (v3-v1));


		this->faceareas_[i] = (tmp DOT tmp) / 2.0;
		this->facecenters_[i] = (v1+v2+v3) / 3.0f;
	}
	return 0;
}
double Model::fitCylinderPrimitive(){
	//if(fitting_cylinders) delete fitting_cylinders;

	///    fitting

	for(int i=0; i<patches.size() ;i++){
		vector<int>* tmp_fList=&(patches[i]);
		this->ComputeAllFaceAreaCenter();
		this->fitting_cylinders.push_back( new FittingCylinder(this, tmp_fList));
		double fitting_cylinder_cost = this->fitting_cylinders[i]->FitCylinder(); // need cosider
	}

	// **********  single patch test ****************
	//vector<int> tmp_fList;// just take all face for test
	//tmp_fList.resize(this->mesh_->faces.size());
	//for(int i=0;i<tmp_fList.size();i++){
	//	tmp_fList[i]=i;
	//}
	//this->ComputeAllFaceAreaCenter();
	//this->fitting_cylinders .push_back(new FittingCylinder(this, tmp_fList));
	//double fitting_cylinder_cost = this->fitting_cylinders[0]->FitCylinder();

	// ********* first patch test ******************
	//vector<int> tmp_fList=patches[0];
	//this->ComputeAllFaceAreaCenter();
	//this->fitting_cylinders.push_back( new FittingCylinder(this, tmp_fList));
	//double fitting_cylinder_cost = this->fitting_cylinders[0]->FitCylinder(); // need cosider

	return 0;
}

/*load computed cylinders from file*/
double Model::fitCylinderPrimitive(const char* st ){
	fitting_cylinders.clear();

	string line;
	ifstream myfile (st );
	if (myfile.is_open())
	{
		double radius,height;
		double x,y,z;
		Vector3d  axis,center;
		for(int i=0; i<this->colorlist_.size()/3 && myfile.good();i++){
			getline (myfile,line);// type
			myfile>>line;//#axis
			myfile>>x>>y>>z;
			axis = Vector3d(x,y,z);
			myfile>>line;//#center
			myfile>>x>>y>>z;
			center= Vector3d(x,y,z);
			myfile>>line;//#radius
			myfile>>radius;
			myfile>>line;//#height
			myfile>>height;
			getline (myfile,line);// skip the '\n' after height
			this->fitting_cylinders.push_back( new FittingCylinder(axis,center,radius,height));
		}
		myfile.close();
		
		return 1;
	}

	return 0;
}



/********************************************************************
	           Processing before pasing to 2D domain (edge contours, pose lines)
*********************************************************************/
/********************************************************************
	                         edge contours
*********************************************************************/
// TODO: may consider N-depth neighbours, for connecting N vertices breaks
bool Model::inRingNeighbour(int v1, int v2){
	std::vector<int> ring = this->mesh_->neighbors[v1];
	for(int j=0;j<ring.size();j++){
		if(ring[j] == v2)
			return true;
	}
	return false;
}

bool Model::in2RingNeighbour(int v1, int v2){
	std::vector<int> ring = this->mesh_->neighbors[v1];
	for(int j=0;j<ring.size();j++){
		if(ring[j] == v2)
			return true;
		std::vector<int> ring2 = this->mesh_->neighbors[ring[j]];
		for( int k=0;k<ring2.size(); k++){
			if(ring2[k] == v2)
				return true;
		}
	}
	return false;
}

// try merging close chains, ignore 'small' gap
// TODO: may consider the T-junctions here
std::vector<std::vector<int>>* Model::tryMergeChains(std::vector<std::vector<int>>* allChains){
	// test if the head/end is connected to another chain's head/end
	// if yes, connect them
	//std::vector<std::vector<vec>> mergedChains;
	for(int i=0; i< allChains->size();i++){
		int head = allChains->at(i).front(); // assume (indeed) the tail has no other source neighbors
		for(int j=i+1; j<allChains->size();j++){
			if(allChains->at(j).size()>1){
				int head2 = allChains->at(j).front(); 
				int tail2 = allChains->at(j).back();
				if(this->inRingNeighbour( head , head2)){ // in ring neighbour
					// reverse adjacent & add current chain to the adjacent chain
					// in this case, the head and tail of the current chain has no other source neighbors
					// but the tail of the adjacent chain may still have source neighbors
					// so put the tail of the adjacent chain as new head
					std::vector<int> mer;
					for(int k = allChains->at(j).size()-1; k>=0; k--){
						mer.push_back(allChains->at(j)[k]);
					}
					/*for(int k =0; k<allChains->at(i).size(); k++){
					mer.push_back(allChains->at(i)[k]);
					}*/
					mer.insert(mer.end(),allChains->at(i).begin(),allChains->at(i).end());
					allChains->at(i) = mer;

					// update head
					head = mer.front();

					// remove the adjacent chain
					allChains->erase(allChains->begin()+j);
					j--;
					continue;
				}

				else if(this->inRingNeighbour( head , tail2)){ // in ring neighbour
					// in this case, the head and tail of the current chain has no source neighbors
					// but the head of the adjacent chain may still have source neighbors
					// so put the head of the adjacent chain as new head
					std::vector<int> mer;
					/*for(int k = 0; k<allChains->at(j).size(); k++){
					mer.push_back(allChains->at(j)[k]);
					}
					for(int k =0; k<allChains->at(i).size(); k++){
					mer.push_back(allChains->at(i)[k]);
					}*/
					mer.insert(mer.end(),allChains->at(j).begin(),allChains->at(j).end());
					mer.insert(mer.end(),allChains->at(i).begin(),allChains->at(i).end());
					allChains->at(i) = mer;

					// update head
					head = mer.front();

					// remove the adjacent chain
					allChains->erase(allChains->begin()+j);
					j--;
					continue;
				}
			}
			else if(allChains->at(j).size() == 1){
				int head2 = allChains->at(j).front(); 
				if(this->inRingNeighbour( head , head2) ){
					std::vector<int> mer;
					mer.push_back(head2);
					mer.insert(mer.end(),allChains->at(i).begin(),allChains->at(i).end());
					allChains->at(i) = mer;

					// update head
					head = mer.front();

					// remove the adjacent chain
					allChains->erase(allChains->begin()+j);
					j--;
					continue;
				}
			}
			else {
				assert(0);
			}
		}
	}

	// check sum
	int chk =0 ;
	for(int i=0;i<allChains->size();i++){
		chk += allChains->at(i).size();
	}
	
	return allChains;
}
std::vector<std::vector<int>> Model::buildEdgeChains(){
	std::vector<std::vector<int>> allChains;
	numOfSources=0;
	int tmp_numOfSources =0;
	if((this->edgeVertices.size())>0)
	{
		int length = this->mesh_->vertices.size();
		this->edgeVerticesIndexed = new bool[length]; // true -> edge/source
		for(int i=0; i<length; i++){
			edgeVerticesIndexed[i] = false;
		}
		for(int i=0;i<this->edgeVertices.size();i++){
			this->edgeVerticesIndexed[edgeVertices[i]]=true;
		}
		for(int i=0; i<length; i++){
			if(this->edgeVerticesIndexed[i]) numOfSources++;
		}
		tmp_numOfSources = numOfSources;

		int pickSource=0;
		while(true){
			if(numOfSources == 0)
			{
				// check sum
				int chk =0 ;
				for(int i=0;i<allChains.size();i++){
					chk += allChains[i].size();
				}
				assert( tmp_numOfSources == chk);

				return allChains;
			}
			else if(numOfSources>0){
				// pick random (first) source
				for(int i=pickSource; i<length; i++ ){
					if(this->edgeVerticesIndexed[i]){
						this->edgeVerticesIndexed[i] = false;
						pickSource = i;
						break; // stop at first source
					}
				}
				std::vector<int> newC = buildSingleChain(pickSource);
				//if(newC.size()>5) // thersholding out small strokes == remove all isolated vertices
				allChains.push_back(newC);
			}else{ 
				abort();
			}// error case
		}
		delete edgeVerticesIndexed;
	}

	// check sum
	int chk =0 ;
	for(int i=0;i<allChains.size();i++){
		chk += allChains[i].size();
	}
	assert(numOfSources == chk);
	return allChains;
}
std::vector<int> Model::buildSingleChain(int s){
	std::vector<int> chain;

	chain.push_back(s);
	this->edgeVerticesIndexed[s]=false;
	this->numOfSources --;
	while(true){
		std::vector<int> ring = this->mesh_->neighbors[s];
		int nextS = -1;
		for(int i=0;i<ring.size();i++){
			if(this->edgeVerticesIndexed[ring[i]]){
				nextS = ring[i];
				chain.push_back(nextS);
				this->edgeVerticesIndexed[nextS] = false;
				this->numOfSources --;
				s=nextS;
				break;
			}
		}
		if (nextS == -1)
			return chain;
	}

	return chain;
}
std::vector<std::vector<int>>* Model::tryMergeChains_strict(std::vector<std::vector<int>>* allChains){
	// test if the head/end is connected to another chain's head/end
	// if yes, connect them
	//std::vector<std::vector<vec>> mergedChains;
	for(int i=0; i< allChains->size();i++){
		int head = allChains->at(i).front(); // assume (indeed) the tail has no other source neighbors
		for(int j=i+1; j<allChains->size();j++){
			if(allChains->at(j).size()>1){
				int head2 = allChains->at(j).front(); 
				int tail2 = allChains->at(j).back();
				if(head == head2){ // exact connecting
					// reverse adjacent & add current chain to the adjacent chain
					// in this case, the head and tail of the current chain has no other source neighbors
					// but the tail of the adjacent chain may still have source neighbors
					// so put the tail of the adjacent chain as new head
					std::vector<int> mer;
					for(int k = allChains->at(j).size()-1; k>=0; k--){
						mer.push_back(allChains->at(j)[k]);
					}
					/*for(int k =0; k<allChains->at(i).size(); k++){
					mer.push_back(allChains->at(i)[k]);
					}*/
					mer.insert(mer.end(),allChains->at(i).begin(),allChains->at(i).end());
					allChains->at(i) = mer;

					// update head
					head = mer.front();

					// remove the adjacent chain
					allChains->erase(allChains->begin()+j);
					j--;
					continue;
				}

				else if( head == tail2){ // strict connection
					// in this case, the head and tail of the current chain has no source neighbors
					// but the head of the adjacent chain may still have source neighbors
					// so put the head of the adjacent chain as new head
					std::vector<int> mer;
					/*for(int k = 0; k<allChains->at(j).size(); k++){
					mer.push_back(allChains->at(j)[k]);
					}
					for(int k =0; k<allChains->at(i).size(); k++){
					mer.push_back(allChains->at(i)[k]);
					}*/
					mer.insert(mer.end(),allChains->at(j).begin(),allChains->at(j).end());
					mer.insert(mer.end(),allChains->at(i).begin(),allChains->at(i).end());
					allChains->at(i) = mer;

					// update head
					head = mer.front();

					// remove the adjacent chain
					allChains->erase(allChains->begin()+j);
					j--;
					continue;
				}
			}
			else if(allChains->at(j).size() == 1){
				assert(0);
			}
		}
	}

	// check sum
	int chk =0 ;
	for(int i=0;i<allChains->size();i++){
		chk += allChains->at(i).size();
	}
	
	return allChains;
}
std::vector<std::vector<int>> Model::buildEdgeChains2(){
	std::vector<std::vector<int>> allChains;
	int len = this->edgeVertices.size();
	if(len>0)
	{
		// their are len/2 edges, pair up the vertices
		for(int i=0;i<len/2;i++){
			std::vector<int> oneEdge;
			oneEdge.push_back(this->edgeVertices[i*2]);
			oneEdge.push_back(this->edgeVertices[i*2+1]);
			allChains.push_back(oneEdge);
		}
	}

	return allChains;
}


/********************************************************************
	                         pose lines
*********************************************************************/
// try using segmentationSharedVertices data int[a][b][]
// TODO: direction and order of these curves
// maybe: chainning algo, and go to the path with min turning angle
//std::vector<std::vector<vec>> * Model::buildPoseLines(){
//	if(this->graph == NULL) return &(this->poselines);
//	this->poselines.clear();
//	_Graph& g=this->graph->g; 
//	_Graph::out_edge_iterator eIt, eEnd;
//	_Graph::vertex_iterator vIt, vEnd;
//	for( boost::tie(vIt,vEnd)=boost::vertices(g); vIt!=vEnd;vIt++ ){
//		vNode* s=g[*vIt];
//		vec center = s->center;
//
//		// complication case
//		// the idea is, find a pricipal direction, as an averaging of the connecting edges
//		// passing through the center
//		// the connecting edges partition into two clusters
//		// each cluster find a middle point
//		// if this point still has many connecting edges, recursive
//		// motivation: symmetric parts
//		if(boost::out_degree(*vIt,g)>2){
//
//			std::vector<vec> stella;
//			for(boost::tie(eIt, eEnd) = boost::out_edges(*vIt,g); eIt!=eEnd; ++eIt){
//				vec c= g[*eIt]->center;
//				stella.push_back(c);
//			}
//			std::vector<std::vector<vec>> re =poseSpliting(center, stella);
//			poselines.insert(poselines.end(),re.begin(),re.end());
//		}
//		else{
//			for(boost::tie(eIt, eEnd) = boost::out_edges(*vIt,g); eIt!=eEnd; ++eIt){
//				vec c= g[*eIt]->center;
//				std::vector<vec> newline;
//				newline.push_back(center);
//				newline.push_back(c);
//				poselines.push_back(newline);
//			}
//		}
//	}
//
//	return &(this->poselines);
//}
std::vector<std::vector<vPoint3D>> * Model::buildPoseLines(){
	if(this->graph == NULL) return &(this->poselines);
	this->poselines.clear();
	_Graph& g=this->graph->g; 
	_Graph::edge_iterator eIt, eEnd;
	_Graph::vertex_iterator v, vIt, vEnd;
	boost::tie(eIt,eEnd) = boost::edges(g);
	poselines = this->poseRecursiveSpan(*v,*eIt, true);

	// build branches, for sim Hull
	this->buildBranches( &poselines);
	return &(this->poselines);
}

/*
 All node has to be initialized with segmentID before using this functions.
 bone[0]: the line link to vEdge* from, start point is the connecting point
 bone[other]: other lines
*/
// TODO: may need vPoint3D to preserve the segmentation information
std::vector<std::vector<vPoint3D>> Model::poseRecursiveSpan(NodeID v, EdgeID from, bool root=false){
	std::vector<std::vector<vPoint3D>> bones;
	_Graph& g=this->graph->g;
	vNode* s= g[v];
	vec center = s->center;
	s->flag_pose = true; // visited,  for cyclic graph
	_Graph::out_edge_iterator eIt, eEnd;
	_Graph::adjacency_iterator  vIt, vEnd;

	if(!root){
		if(boost::out_degree(v,g)>2){
			// [001] create stella: make sure the order is the same as [002]
			std::vector<vec> stella;
			boost::tie(vIt, vEnd) = boost::adjacent_vertices( v,g);
			for(;vIt!=vEnd;vIt++){
				EdgeID e;
				bool exist=false;
				boost::tie(e,exist) = boost::edge(v, *vIt, g);
				assert(exist);
				vEdge* adjE = g[e];
				vec ncenter = adjE->center;
				stella.push_back(ncenter);
			}

			// follow the recursive splitting algorithm
			std::vector<vec> dirs;
			double max_len =0;
			double max_ind =-1;
			for(int i=0; i<stella.size(); i++){
				dirs.push_back(stella[i]-center);
				if(len(dirs[i])>max_len){
					max_len = len(dirs[i]);
					max_ind = i;
				}
			}
			assert(max_ind != -1);
			vec init = dirs[max_ind];

			// 2 clusters, refine the pricipal direction
			std::vector<vec> cluster1; // go with principal
			std::vector<vec> cluster2;
			std::vector<bool> dirlabel; // true for go with principal direction, false for opposite
			for(int i=0;i<dirs.size();i++){
				if(i == max_ind) { 
					dirlabel.push_back(true);
					cluster1.push_back(stella[i]);
					continue;
				}
				double vote = dirs[i] DOT init;
				if(vote >0){
					dirlabel.push_back(true);
					cluster1.push_back(stella[i]);
					init = init*(1.0f*(float)i) + dirs[i];
					init /=  (float)(i+1);
				}
				else{
					dirlabel.push_back(false);
					cluster2.push_back(stella[i]);
					init = init*(1.0f*(float)i) - dirs[i];
					init /=  (float)(i+1);
				}
			}


			// for each cluster contitue to do this, or return

			std::vector<std::vector<vec>> re1,re2;
			float magic = 1-0.618;
			vec nc; // center for dirlabel true
			if(cluster1.size()> 0){
				for(int i=0; i< cluster1.size();i++){nc += cluster1[i];}
				nc /= (float)cluster1.size();
				if(cluster1.size()> 1){nc -= ((nc-center) DOT init)/(len(init)) *magic* init/(len(init));}
				
				//re1 = poseSpliting(nc,cluster1);
			}
			else{nc = center;}

			vec nc2; // center for dirlabel false
			if(cluster2.size()>0){
				for(int i=0; i< cluster2.size();i++){nc2 += cluster2[i];}
				nc2 /= (float)cluster2.size();
				if(cluster2.size()>1){nc2 -= ((nc2-center) DOT init)/(len(init))*magic* init/(len(init));}
				
				//re2 = poseSpliting(nc2,cluster2);
			}
			else{nc2 = center;}


			//unify the result lines
			std::vector<vPoint3D> stretch;
			stretch.push_back(vPoint3D( nc, s->segmentID));
			stretch.push_back(vPoint3D( center, s->segmentID));
			stretch.push_back(vPoint3D( nc2, s->segmentID));
			bones.push_back(stretch);

			// do not link to where it is from
		
			EdgeID e;
			boost::tie(vIt, vEnd) = boost::adjacent_vertices( v,g);
			for(int i=0;vIt!=vEnd;vIt++,i++){
				
				bool exist=false;
				boost::tie(e,exist) = boost::edge(v, *vIt, g);
				assert(exist);

				vEdge* adjE = g[e];
				vec ncenter = adjE->center;
			
				if(e == from){ // to source, arrange it at first bones[0]
					
					assert(g[*vIt]->flag_pose);	// for cyclic graph
					std::vector<vPoint3D> bone;
					bone.push_back(vPoint3D(ncenter, s->segmentID));
					bone.push_back(vPoint3D((dirlabel[i])?nc:nc2, s->segmentID));

					bones.insert(bones.begin(),bone);
				}else{

					if(g[*vIt]->flag_pose)	continue;	// for cyclic graph

					std::vector<vPoint3D> bone;

					bone.push_back(vPoint3D(center,s->segmentID));
					bone.push_back(vPoint3D((dirlabel[i])?nc:nc2,s->segmentID));

					// recursively spanning
					std::vector<std::vector<vPoint3D>> rebones = this->poseRecursiveSpan(*vIt,e); 
					bone.insert(bone.end(), rebones[0].begin(),rebones[0].end());
					bones.push_back(bone);
					if(rebones.size()>1){
						bones.insert(bones.end(),rebones.begin()+1,rebones.end());
					}
				}
			}
		}
		else if(boost::out_degree(v,g)==2){
			
			EdgeID e;
			boost::tie(vIt, vEnd) = boost::adjacent_vertices( v,g);

			//std::vector<std::vector<vec>> span;
			std::vector<vPoint3D> toSource;
			for(;vIt!=vEnd;vIt++){
				
				bool exist=false;
				boost::tie(e,exist) = boost::edge(v, *vIt, g);
				assert(exist);

				vEdge* adjE = g[e];
				vec ncenter = adjE->center;
			
				if(e == from){ // to source, arrange it at first bones[0]
					
					assert(g[*vIt]->flag_pose);	// for cyclic graph
					//std::vector<vec> bone;
					toSource.push_back(vPoint3D(ncenter,s->segmentID));
					toSource.push_back(vPoint3D(center,s->segmentID));

					//twoNeighbourBone.insert(bones.begin(),bone);
				}else{

					if(g[*vIt]->flag_pose)	continue;	// for cyclic graph

					std::vector<vPoint3D> bone;

					bone.push_back(vPoint3D(center,s->segmentID));
					bone.push_back(vPoint3D(ncenter,s->segmentID));

					// recursively spanning
					std::vector<std::vector<vPoint3D>> rebones = this->poseRecursiveSpan(*vIt,e); 
					bone.insert(bone.end(), rebones[0].begin(),rebones[0].end());
					bones.push_back(bone);
					if(rebones.size()>1){
						bones.insert(bones.end(),rebones.begin()+1,rebones.end());
					}
				}
			}
			if(bones.size() <1)//TODO: have not carefully thought about how this happens, just a quick fix trial
			{
				bones.push_back(toSource);
			}
			else{
				bones[0].insert(bones[0].begin(), toSource.begin(),toSource.end());
			}
		}
		else if(boost::out_degree(v,g)==1){
			boost::tie(eIt, eEnd) = boost::out_edges(v,g);
			vec ncenter = g[*eIt]->center;

			assert(*eIt == from);
			std::vector<vPoint3D> bone;

			bone.push_back(vPoint3D(ncenter,s->segmentID));
			bone.push_back(vPoint3D(center,s->segmentID));
			bone.push_back(vPoint3D(2.0f*center-ncenter, s->segmentID));	// for edge segments

			bones.push_back(bone);
		}
		else{ //single isolated segment
			assert(0);
		}
	}
	else{
		int out_deg = boost::out_degree(v,g);
		if(out_deg >2){
			
			
			// [001] create stella: make sure the order is the same as [002]
			std::vector<vec> stella;
			boost::tie(vIt, vEnd) = boost::adjacent_vertices( v,g);
			for(;vIt!=vEnd;vIt++){
				EdgeID e;
				bool exist=false;
				boost::tie(e,exist) = boost::edge(v, *vIt, g);
				assert(exist);
				vEdge* adjE = g[e];
				vec ncenter = adjE->center;
				stella.push_back(ncenter);
			}

			// follow the recursive splitting algorithm
			std::vector<vec> dirs;
			double max_len =0;
			double max_ind =-1;
			for(int i=0; i<stella.size(); i++){
				dirs.push_back(stella[i]-center);
				if(len(dirs[i])>max_len){
					max_len = len(dirs[i]);
					max_ind = i;
				}
			}
			assert(max_ind != -1);
			vec init = dirs[max_ind];

			// 2 clusters, refine the pricipal direction
			std::vector<vec> cluster1; // go with principal
			std::vector<vec> cluster2;
			std::vector<bool> dirlabel; // true for go with principal direction, false for opposite
			for(int i=0;i<dirs.size();i++){
				if(i == max_ind) { 
					dirlabel.push_back(true);
					cluster1.push_back(stella[i]);
					continue;
				}
				double vote = dirs[i] DOT init;
				if(vote >0){
					dirlabel.push_back(true);
					cluster1.push_back(stella[i]);
					init = init*(1.0f*(float)i) + dirs[i];
					init /=  (float)(i+1);
				}
				else{
					dirlabel.push_back(false);
					cluster2.push_back(stella[i]);
					init = init*(1.0f*(float)i) - dirs[i];
					init /=  (float)(i+1);
				}
			}


			// for each cluster contitue to do this, or return

			std::vector<std::vector<vec>> re1,re2;
			float magic = 1-0.618;
			vec nc; // center for dirlabel true
			if(cluster1.size()> 0){
				for(int i=0; i< cluster1.size();i++){nc += cluster1[i];}
				nc /= (float)cluster1.size();
				if(cluster1.size()> 1){nc -= ((nc-center) DOT init)/(len(init)) *magic* init/(len(init));}
				
				//re1 = poseSpliting(nc,cluster1);
			}
			else{nc = center;}

			vec nc2; // center for dirlabel false
			if(cluster2.size()>0){
				for(int i=0; i< cluster2.size();i++){nc2 += cluster2[i];}
				nc2 /= (float)cluster2.size();
				if(cluster2.size()>1){nc2 -= ((nc2-center) DOT init)/(len(init))*magic* init/(len(init));}
				
				//re2 = poseSpliting(nc2,cluster2);
			}
			else{nc2 = center;}


			// "spine"
			std::vector<vPoint3D> stretch;
			stretch.push_back(vPoint3D(nc,s->segmentID));
			stretch.push_back(vPoint3D(center,s->segmentID));
			stretch.push_back(vPoint3D(nc2,s->segmentID));
			bones.push_back(stretch);

			// [002] original: make sure the order is the same as [001]
			boost::tie(vIt, vEnd) = boost::adjacent_vertices( v,g);
			for(int i=0;vIt!=vEnd;vIt++,i++){
				EdgeID e;
				bool exist=false;
				boost::tie(e,exist) = boost::edge(v, *vIt, g);
				assert(exist);

				vEdge* adjE = g[e];
				vec ncenter = adjE->center;
			

				std::vector<vPoint3D> bone;

				bone.push_back(vPoint3D((dirlabel[i])?nc:nc2, s->segmentID)); // [003]
				bone.push_back(vPoint3D(ncenter,s->segmentID));

				// recursively spanning
				std::vector<std::vector<vPoint3D>> rebones = this->poseRecursiveSpan(*vIt,e); 
				bone.insert(bone.end(), rebones[0].begin(),rebones[0].end());
				bones.push_back(bone);
				if(rebones.size()>1){
					bones.insert(bones.end(),rebones.begin()+1,rebones.end());
				}

			}
		}
		else if(out_deg ==2){
			boost::tie(vIt, vEnd) = boost::adjacent_vertices( v,g);
			EdgeID e;
			bool exist=false;

			// one bone
			boost::tie(e,exist) = boost::edge(v, *vIt, g);
			assert(exist);
			vEdge* adjE = g[e];
			vec ncenter = adjE->center;
			std::vector<vPoint3D> leftbone;
			leftbone.push_back(vPoint3D(center,s->segmentID));
			leftbone.push_back(vPoint3D(ncenter,s->segmentID));
			// recursively spanning
			std::vector<std::vector<vPoint3D>> rebonesleft = this->poseRecursiveSpan(*vIt,e); 
			leftbone.insert(leftbone.end(), rebonesleft[0].begin(),rebonesleft[0].end());
			

			// another bone
			vIt++;
			boost::tie(e,exist) = boost::edge(v, *vIt, g);
			assert(exist);
			adjE = g[e];
			ncenter = adjE->center;
			std::vector<vPoint3D> rightbone;
			rightbone.push_back(vPoint3D(center,s->segmentID));
			rightbone.push_back(vPoint3D(ncenter,s->segmentID));
			std::vector<std::vector<vPoint3D>> rebonesright = this->poseRecursiveSpan(*vIt,e);
			rightbone.insert(rightbone.end(), rebonesright[0].begin(), rebonesright[0].end());


			//combine left and right bones
			std::vector<vPoint3D> bone;
			for(int i=leftbone.size()-1;i>=0;i--){
				bone.push_back(leftbone[i]);
			}
			bone.insert(bone.end(),rightbone.begin(), rightbone.end());

			bones.push_back(bone);

			// spaned bones
			if(rebonesleft.size()>1){
				bones.insert(bones.end(),rebonesleft.begin()+1,rebonesleft.end());
			}
			if(rebonesright.size()>1){
				bones.insert(bones.end(),rebonesright.begin()+1,rebonesright.end());
			}
		}
		else if (out_deg == 1){
			EdgeID e;
			boost::tie(vIt, vEnd) = boost::adjacent_vertices( v,g);
			assert(!g[*vIt]->flag_pose);	// for cyclic graph, root case should have not visited any node
			
			bool exist=false;
			boost::tie(e,exist) = boost::edge(v, *vIt, g);
			assert(exist);

			vEdge* adjE = g[e];
			vec ncenter = adjE->center;
			
			std::vector<vPoint3D> bone;
			bone.push_back(vPoint3D(center,s->segmentID));
			bone.push_back(vPoint3D(ncenter,s->segmentID));
			
			std::vector<std::vector<vPoint3D>> rebones = poseRecursiveSpan( *vIt, e);
			
			bone.insert(bone.end(), rebones[0].begin(),rebones[0].end());
			bones.push_back(bone);
			if(rebones.size()>1){
				bones.insert(bones.end(),rebones.begin()+1,rebones.end());
			}
		}
		else{
			assert(0);
		}
	
	}


	return bones;
}
// center is the segment's center
// stella are the centers of adjacent edges (shared vertices)

std::vector<std::vector<vec>> Model::poseSpliting(vec center, std::vector<vec> stella){
	
	std::vector<std::vector<vec>> result;
	if(stella.size()>2){
		std::vector<vec> dirs;

		double max_len =0;
		double max_ind =-1;
		for(int i=0; i<stella.size(); i++){
			dirs.push_back(stella[i]-center);
			if(len(dirs[i])>max_len){
				max_len = len(dirs[i]);
				max_ind = i;
			}
		}
		assert(max_ind != -1);
		vec init = dirs[max_ind];

		// 2 clusters
		std::vector<vec> cluster1; // go with principal
		std::vector<vec> cluster2;
		std::vector<bool> dirlabel; // true for go with principal direction, false for opposite
		for(int i=0;i<dirs.size();i++){
			if(i == max_ind) { 
				dirlabel.push_back(true);
				cluster1.push_back(stella[i]);
				continue;
			}
			double vote = dirs[i] DOT init;
			if(vote >0){
				dirlabel.push_back(true);
				cluster1.push_back(stella[i]);
				init = init*(1.0f*(float)i) + dirs[i];
				init /=  (float)(i+1);
			}
			else{
				dirlabel.push_back(false);
				cluster2.push_back(stella[i]);
				init = init*(1.0f*(float)i) - dirs[i];
				init /=  (float)(i+1);
			}
		}

		// for each cluster contitue to do this, or return

		std::vector<std::vector<vec>> re1,re2;
		float magic = 1-0.618;
		vec nc;
		if(cluster1.size()> 0){
			for(int i=0; i< cluster1.size();i++){
				nc += cluster1[i];
			}
			nc /= (float)cluster1.size();

			//assert((nc DOT init)>0);
			if(cluster1.size()> 1){
				nc -= ((nc-center) DOT init)/(len(init)) *magic* init/(len(init));
			}
			re1 = poseSpliting(nc,cluster1);
		}
		else{
			nc = center;
		}

		vec nc2;

		if(cluster2.size()>0){
			for(int i=0; i< cluster2.size();i++){
				nc2 += cluster2[i];
			}
		
			nc2 /= (float)cluster2.size();
			//assert((nc DOT init)<0);
			if(cluster2.size()>1){
				nc2 -= ((nc2-center) DOT init)/(len(init))*magic* init/(len(init));
			}
			re2 = poseSpliting(nc2,cluster2);
		}
		else{
			nc2 = center;
		}

			std::vector<vec> stretch;
			stretch.push_back(nc);
			stretch.push_back(center);
			stretch.push_back(nc2);
			result.push_back(stretch);

		result.insert(result.end(),re1.begin(),re1.end());
		result.insert(result.end(),re2.begin(),re2.end());
	}
	else if (stella.size() == 2){
		std::vector<vec> duo;
		duo.push_back(stella[1]);
		duo.push_back(center);
		duo.push_back(stella[0]);
		result.push_back(duo);
	}
	else if (stella.size() == 1){
		std::vector<vec> solo;
		solo.push_back(center);
		solo.push_back(stella[0]);
		result.push_back(solo);
	}
	else {
	}
	
	return result;
}

// TODO: joint suggestion


/********************************************************************
	                          suggestive contours
*********************************************************************/
std::vector<int> Model::filterUnderDepthVertices(std::vector<int>* vs, GLfloat* depth_data , int w, int h){
	std::vector<int> filtered;
	for(int i=0; i<vs->size();i++){
		vec v=this->mesh_->vertices[vs->at(i)];
		vec projV = this->projVec3(v);
		double epsilon = 0.01;
		int tmp_ind = (int)projV[0]+(int)(projV[1])*w;
		double error = abs(projV[2] - depth_data[tmp_ind]);
		if (  error < epsilon){
			filtered.push_back(vs->at(i));
		}
	}

	return filtered;
}
// extremly long segments are not reasonable
std::vector<std::vector<vec>>* Model::filtlerLongSegments(std::vector<std::vector<vec>>* allChains){
	//assert(  ==2 );
	double avg=0;int _size = allChains->size();
	for(int i=0; i<allChains->size();i++){
		vec seg = allChains->at(i)[0]; // only one segment in each std::vector<vec>
		avg += len(seg);
		if (len(seg)> 1){
			allChains->erase(allChains->begin()+i);i--;
		}
	}
	avg /= _size;
	
	return allChains;
}
std::vector<std::vector<vec>>* Model::filterUnderDepthSegments(std::vector<std::vector<vec>>* allChains ,GLfloat* depth_data, int w, int h,std::vector<std::vector<vec>>*filtered){
	
	for(int i=0;i<allChains->size();i++){
		std::vector<vec>* c = &(allChains->at(i));
		bool _do_put = true;
		for(int j=0;j<c->size();j++){
			vec projV = this->projVec3(c->at(j));
			double epsilon = 0.1;
			int tmp_ind = (int)projV[0]+(int)(projV[1])*w;
			double error = abs(projV[2] - depth_data[tmp_ind]);
			if (  error > epsilon){
				_do_put = false;break;
			}
		}
		if(_do_put){
			filtered->push_back(allChains->at(i));
		}
	}

	return filtered;
}

std::vector<std::vector<vec>>* Model::tryMergeChains(std::vector<std::vector<vec>>* allChains){
	// test if the head/end is connected to another chain's head/end
	// if yes, connect them
	//std::vector<std::vector<vec>> mergedChains;

	double dist_threshold = 0.0001;
	for(int i=0; i< allChains->size();i++){
		vec head = allChains->at(i).front();
		vec tail = allChains->at(i).back();

		// looking for the nearest every time, until the nearest is too far away
		while(true){
			//double min_dist_hh = 10; // distance bigger then 10 is definitely not connected, head to head2
			//double min_dist_th = 10; // tail to head2
			//double min_dist_ht = 10; // head to tail2
			//double min_dist_tt = 10; // tail to tail2

			// get the nearest
			double min_dist[4]={10,10,10,10};
			int min_ind[4]={-1,-1,-1,-1};
			for(int j=i+1; j<allChains->size();j++){
				vec head2 = allChains->at(j).front(); 
				vec tail2 = allChains->at(j).back();
				double tmp_dist[4] = {10,10,10,10};
				tmp_dist[0]=dist(head,head2);
				tmp_dist[1]=dist(tail,head2);
				tmp_dist[2]=dist(head,tail2);
				tmp_dist[3]=dist(tail,tail2);

				// test angle
				vec tanHead = allChains->at(i)[0] - allChains->at(i)[1];
				if(allChains->at(i).size()>2)	tanHead += allChains->at(i)[0] - allChains->at(i)[2];
				vec tanTail = allChains->at(i).back() - allChains->at(i)[allChains->at(i).size()-2];
				if(allChains->at(i).size()>2)	tanTail += allChains->at(i).back() - allChains->at(i)[allChains->at(i).size()-3];
				vec tanHead2 = allChains->at(j)[0] - allChains->at(j)[1];
				if(allChains->at(j).size()>2)	tanHead2 += allChains->at(j)[0] - allChains->at(j)[2];
				vec tanTail2 = allChains->at(j).back() - allChains->at(j)[allChains->at(j).size()-2];
				if(allChains->at(j).size()>2)	tanTail2 += allChains->at(j).back() - allChains->at(j)[allChains->at(j).size()-3];

				vec tanhh = allChains->at(i)[0] - allChains->at(j)[0];
				vec tanth = allChains->at(i).back() - allChains->at(j)[0];
				vec tanht = allChains->at(i)[0] - allChains->at(j).back();
				vec tantt = allChains->at(i).back() - allChains->at(j).back();

				for(int k=0;k<4;k++){
					// test the angle constrain here
					double cose1=-1, cose2=-1;
					switch(k){
					case 0:cose1 = -(tanHead DOT tanHead2)/(len(tanHead)*len(tanHead2));cose2 = -(tanHead DOT tanhh)/(len(tanHead)*len(tanhh));break;
					case 1:cose1 = -(tanTail DOT tanHead2)/(len(tanTail)*len(tanHead2));cose2 = -(tanTail DOT tanth)/(len(tanTail)*len(tanth)); break;
					case 2:cose1 = -(tanHead DOT tanTail2)/(len(tanHead)*len(tanTail2));cose2 = -(tanHead DOT tanht)/(len(tanHead)*len(tanht));break;
					case 3:cose1 = -(tanTail DOT tanTail2)/(len(tanTail)*len(tanTail2));cose2 = -(tanTail DOT tantt)/(len(tanTail)*len(tantt));break;
					default:assert(0);break;
					}

					if(tmp_dist[k]<min_dist[k]){
						if(cose1 >0.2 ){ //cose2 doen't working well
							min_dist[k] = tmp_dist[k];
							min_ind[k] = j;
						}
					}
				}
			}
			double glob_min = 11;
			int glon_min_ind = -1; // nearest segment
			for(int k=0;k<4;k++){
				if(glob_min > min_dist[k]){
					glob_min = min_dist[k];
					glon_min_ind = k;
				}
			}

			if(glob_min< dist_threshold ){ // merge
					vec head2 = allChains->at(min_ind[glon_min_ind]).front(); 
					vec tail2 = allChains->at(min_ind[glon_min_ind]).back();
					std::vector<vec> mer;
					switch(glon_min_ind){
					case 0: // head to head2: min_dist[0], min_ind[0]
						for(int k = allChains->at(min_ind[glon_min_ind]).size()-1; k>=0; k--){
							mer.push_back(allChains->at(min_ind[glon_min_ind])[k]);
						}
						/*for(int k =0; k<allChains->at(i).size(); k++){
							mer.push_back(allChains->at(i)[k]);
						}*/
						mer.insert(mer.end(),allChains->at(i).begin(),allChains->at(i).end());
						allChains->at(i) = mer;
						// remove the adjacent chain
						allChains->erase(allChains->begin()+min_ind[glon_min_ind]);

						//update head and tail
						head = tail2;
						break;
					case 1: // tail connect to head 2
						/*for(int k =0; k<allChains->at(i).size(); k++){
							mer.push_back(allChains->at(i)[k]);
						}*/
						mer.insert(mer.end(),allChains->at(i).begin(),allChains->at(i).end());
						/*for(int k =0; k< allChains->at(min_ind[glon_min_ind]).size();  k++){
							mer.push_back(allChains->at(min_ind[glon_min_ind])[k]);
						}*/
						mer.insert(mer.end(),allChains->at(min_ind[glon_min_ind]).begin(),allChains->at(min_ind[glon_min_ind]).end());
						allChains->at(i) = mer;
						allChains->erase(allChains->begin()+min_ind[glon_min_ind]);

						tail = tail2;
						break;
					case 2: // head to tail2
						/*for(int k =0; k< allChains->at(min_ind[glon_min_ind]).size(); k++){
							mer.push_back(allChains->at(min_ind[glon_min_ind])[k]);
						}*/
						mer.insert(mer.end(),allChains->at(min_ind[glon_min_ind]).begin(),allChains->at(min_ind[glon_min_ind]).end());
						/*for(int k =0; k<allChains->at(i).size(); k++){
							mer.push_back(allChains->at(i)[k]);
						}*/
						mer.insert(mer.end(),allChains->at(i).begin(),allChains->at(i).end());
						allChains->at(i) = mer;
						allChains->erase(allChains->begin()+min_ind[glon_min_ind]);

						head = head2;
						break;
					case 3: //  tail to tail2
						/*for(int k =0; k<allChains->at(i).size(); k++){
							mer.push_back(allChains->at(i)[k]);
						}*/
						mer.insert(mer.end(),allChains->at(i).begin(),allChains->at(i).end());
						for(int k = allChains->at(min_ind[glon_min_ind]).size()-1; k>=0; k--){
							mer.push_back(allChains->at(min_ind[glon_min_ind])[k]);
						}
						allChains->at(i) = mer;
						allChains->erase(allChains->begin()+min_ind[glon_min_ind]);

						tail = head2;
						break;
					}
			}
			else{
				break; // the while loop
			}
		}
	}

	std::vector<std::vector<vec>> anotherChain;
	for(int i=0;i<allChains->size();i++){
		if(allChains->at(i).size()>10){
			anotherChain.push_back(allChains->at(i));
		}
	}
	*allChains = anotherChain;
	return allChains;
}

// give the para's value back
double Model::paraNameChanged_coonectDist(double & max, double & min ){
	max = connectDistThreshold_max;
	min = connectDistThreshold_min;
	return connectDistThreshold;
}
double Model::paraNameChanged_coonectAng(double & max, double & min ){
	max = connectAngThreshold_max;
	min = connectAngThreshold_min;
	return connectAngThreshold;
}
void Model::paraValChanged_coonectDist(int val){
	connectDistThreshold = val/100.0 * (connectDistThreshold_max-connectDistThreshold_min)+connectDistThreshold_min;
	return;
}
void Model::paraValChanged_coonectAng(int val){
	connectAngThreshold = val/100.0 * (connectAngThreshold_max-connectAngThreshold_min)+connectAngThreshold_min;
	return;
}
// when SC is an extension of contour, give priority to connect SC and contour
// the edge to be chosen is with minimum error of {distance, tangent direction}
// merged is now the extended sc, edges will be changed
// TODO: every extended sc, should be added into edges, as an edge, other sc can attach to it
// TODO: after extending the head, if tail's related edge is affected, needs handling
void Model::tryMergeEdgeAndSC(std::vector<std::vector<int>>* edge, std::vector<std::vector<vec>>* sc, std::vector<std::vector<vec>>* merged){

	double connectThres = connectDistThreshold; // 0.0000618

	// for each sc, check within the edges
	for(int i=0; i<sc->size();i++){
		if(sc->at(i).size()<10){
			continue;	
		}
		else{
			vec head = sc->at(i).front();
			vec headNeib[3], tailNeib[3]; //[min_dist, index of edge, index of splitting]*3
			for(int j=0;j<3;j++){
				headNeib[j][0]=10; // init min_dist
				tailNeib[j][0]=10;
			}
			double hMOM =10, tMOM = 10; // MaxOfMin_dist in headNeib
			int hMOMInd =0, tMOMInd = 0;

			// change of edge may affect the tail connection, so separately computed
			for(int j=0; j<edge->size();j++){
				std::vector<int>* e = &(edge->at(j));
				// check the head/tail of sc separately, because there are [4 cases] for the merge
				if(e->size() <5) continue;
				double error;
				int split_pos = contourConnecting(head, e, error);
				if(error < hMOM && error<connectThres){
					headNeib[hMOMInd] = vec(error,j,split_pos);// [dist,index of edge,index of split position]
					this->updateMOM(headNeib,hMOM,hMOMInd);
				}
			}

			// [exSC coding-p1] regard the extendedSC as edges too
			// important: the extendedSC edges noted as (edge->size()) + extendedSC_index, at the edge index term
			for(int j=0; j<merged->size() ;j++){
				std::vector<vec>* m = &(merged->at(j));
				if(m->size() <5) continue;
				double error;
				int split_pos = contourConnecting(head, m, error);
				if(error < hMOM && error<connectThres){
					headNeib[hMOMInd] = vec(error, edge->size()+j,split_pos);// extendedSC edges noted as (edge->size()) + extendedSC_index
					this->updateMOM(headNeib,hMOM,hMOMInd);
				}
			}

			std::vector<vec> extendedSC=sc->at(i);
			/*************************
			head
			**************************/
			// determine which edge to merge with -> weighting between dist and tangent connectivity
			// totE = dist * sin(a) ???
			double tote[3]={0};
			double dir[3]={0};
			double acos_val[3]={0};
			for(int j=0;j<3;j++){
				if(headNeib[j][0] == 10){dir[j] =0; tote[j] =10;continue;}
				double cos_val;
				if(headNeib[j][1]<edge->size()){
					dir[j] = TjunctionResolution_sc(&extendedSC, &(edge->at( (int)headNeib[j][1] )), (int)headNeib[j][2], cos_val,true);
				}
				else{ //[exSC coding-p2] recover exSC, cool, right?
					dir[j] = TjunctionResolution_sc(&extendedSC, &(merged->at( (int)(headNeib[j][1]-edge->size()) )), (int)headNeib[j][2], cos_val,true);
				}
				//tote[j] = headNeib[j][0]  * acos( cos_val);
				tote[j] =  acos( cos_val);
				acos_val[j] = acos( cos_val);
			}
			double min_val=10;
			int min_ind=-1;
			for(int j=0;j<3;j++){
				if(tote[j] < min_val){
					min_val = tote[j];
					min_ind = j;
				}
			}
			if(min_ind != -1){
				int chosenEdgeInd =headNeib[ min_ind][1];
				// [exSC coding-p3]
				bool useEXSC=false;
				std::vector<int>* e ;
				std::vector<vec>* m ;
				if(chosenEdgeInd < edge->size() )
					e= &(edge->at(chosenEdgeInd));
				else{
					useEXSC = true;
					chosenEdgeInd -=  edge->size();
					m = &(merged->at(chosenEdgeInd ));
				}
				int split_pos=headNeib[ min_ind][2];
				int chosen_dir = dir[min_ind];
				if(split_pos != -1){



					std::vector<vec> tmp = extendedSC; // for faster vector space allocation: donot insert except at the end
					if(!useEXSC){

						switch(chosen_dir){
						case 1:// towards the beginning
							extendedSC.clear();
							for(int k=0;k<=split_pos;k++){
								extendedSC.push_back(mesh_->vertices[e->at(k)]);
							}
							extendedSC.insert(extendedSC.end(),tmp.begin(),tmp.end());

							// modify the edge
							if(e->size()-split_pos>3)
								e->erase(e->begin(),e->begin()+split_pos);
							else
								edge->erase(edge->begin()+(int)(headNeib[min_ind][1]));
							break;
						case 2:// towards the end
							extendedSC.clear();
							//extendedSC.insert(extendedSC.begin(),e->begin()+split_pos,e->end());
							for(int k=e->size()-1;k>=split_pos;k--){
								extendedSC.push_back(mesh_->vertices[e->at(k)]);
							}
							extendedSC.insert(extendedSC.end(),tmp.begin(),tmp.end());

							// change of edge may affect the tail connection, handle this here
							if(split_pos>3)
								e->resize(split_pos);
							else
								edge->erase(edge->begin()+(int)(headNeib[min_ind][1]));
							break;
						default:
							break;
						}

					}

					else{// [exSC coding-p4]
						switch(chosen_dir){
						case 1:// towards the beginning
							extendedSC.clear();
							extendedSC.insert(extendedSC.end(),m->begin(),m->begin()+split_pos);
							extendedSC.insert(extendedSC.end(),tmp.begin(),tmp.end());

							// modify the edge
							if(m->size()-split_pos>3)
								m->erase(m->begin(),m->begin()+split_pos);
							else
								merged->erase(merged->begin()+chosenEdgeInd);
							break;
						case 2:// towards the end
							extendedSC.clear();
							for(int k=m->size()-1; k>=split_pos;k-- ){
								extendedSC.push_back(m->at(k));
							}
							extendedSC.insert(extendedSC.end(),tmp.begin(),tmp.end());

							// change of edge may affect the tail connection, handle this here
							if(split_pos>3)
								m->resize(split_pos);
							else
								merged->erase(merged->begin()+chosenEdgeInd);
							break;
						default:
							break;
						}
					}
				}
			}

			/*************************
					tail
			**************************/
			vec tail = extendedSC.back();
			// change of edge may affect the tail connection, so separately computed
			for(int j=0; j<edge->size();j++){
				std::vector<int>* e = &(edge->at(j));
				// check the head/tail of sc separately, because there are [4 cases] for the merge
				if(e->size() <5) continue;
				double error;
				int split_pos = contourConnecting(tail, e, error);
				if(error < tMOM && error<connectThres ){
					tailNeib[tMOMInd] = vec(error,j,split_pos);// [dist,index of edge,index of split position]
					this->updateMOM(tailNeib,tMOM,tMOMInd);
				}
			}
			for(int j=0; j<merged->size() ;j++){
				std::vector<vec>* m = &(merged->at(j));
				// check the head/tail of sc separately, because there are [4 cases] for the merge
				if(m->size() <5) continue;
				double error;
				int split_pos = contourConnecting(tail, m, error);
				if(error < tMOM && error<connectThres){
					tailNeib[tMOMInd] = vec(error, edge->size()+j,split_pos);// extendedSC edges noted as (edge->size()) + extendedSC_index
					this->updateMOM(tailNeib,tMOM,tMOMInd);
				}
			}

			tote[0]=tote[1]=tote[2]=0;
			dir[0]=dir[1]=dir[2]=0;
			for(int j=0;j<3;j++){
				if(tailNeib[j][0] == 10){dir[j] =0; tote[j] =10;continue;}
				double cos_val;
				if(tailNeib[j][1]<edge->size())
					dir[j] = TjunctionResolution_sc(&extendedSC, &(edge->at( (int)tailNeib[j][1] )), (int)tailNeib[j][2], cos_val,false); 
				else
					dir[j] = TjunctionResolution_sc(&extendedSC, &(merged->at( (int)(tailNeib[j][1]-edge->size()) )), (int)tailNeib[j][2], cos_val,false);
				//tote[j] = tailNeib[j][0]  * acos( cos_val);
				tote[j] =  acos( cos_val);
			}
			min_val=10;
			min_ind=-1;
			for(int j=0;j<3;j++){
				if(tote[j] < min_val){
					min_val = tote[j];
					min_ind = j;
				}
			}
			if(min_ind != -1){
				int chosenEdgeInd =tailNeib[ min_ind][1];
				// [exSC coding-p3]
				bool useEXSC=false;
				std::vector<int>* e =NULL;
				std::vector<vec>* m =NULL;
				if(chosenEdgeInd < edge->size() )
					e = &(edge->at(chosenEdgeInd));
				else{
					useEXSC = true;
					chosenEdgeInd -=  edge->size();
					m = &(merged->at(chosenEdgeInd));
				}
				int split_pos=tailNeib[ min_ind][2];
				int chosen_dir = dir[min_ind];
				if(split_pos != -1){
					vec tmp_tail = extendedSC.back();
					if(!useEXSC){
						switch(chosen_dir){
						case 1:// towards the beginning
							for(int k=0;k<=split_pos; k++){
								extendedSC.push_back(mesh_->vertices[e->at(split_pos-k)]);
							}
							if(e->size()-split_pos>3)
								e->erase(e->begin(),e->begin()+split_pos);
							else
								edge->erase(edge->begin()+(int)(tailNeib[min_ind][1]));
							break;
						case 2:// towards the end
							//extendedSC.insert(extendedSC.end(),e->begin()+split_pos,e->end());
							for(int k=split_pos;k<e->size(); k++){
								extendedSC.push_back(mesh_->vertices[e->at(k)]);
							}

							if(split_pos>3)
								e->resize(split_pos);
							else
								edge->erase(edge->begin()+(int)(tailNeib[min_ind][1]));
							break;
						default:
							break;
						}
					}
					else{// [exSC coding-p4]
						switch(chosen_dir){
						case 1:// towards the beginning
							for(int k=0;k<=split_pos; k++){
								extendedSC.push_back( m->at(split_pos-k) );
							}

							if(m->size()-split_pos>3)
								m->erase(m->begin(),m->begin()+split_pos);
							else
								merged->erase(merged->begin()+chosenEdgeInd);
							break;
						case 2:// towards the end
							extendedSC.insert(extendedSC.end(),m->begin()+split_pos,m->end());

							if(split_pos>3)
								m->resize(split_pos);
							else
								merged->erase(merged->begin()+chosenEdgeInd);
							break;
						default:
							break;
						}
					}
				}
			}
			//result
			merged->push_back(extendedSC);
		}
	}

	// merge edges into EXSC

	for(int i=0; i<edge->size(); i++){
		std::vector<int>* e = &(edge->at(i));
		if(e->size() <5) continue;
		std::vector<vec> mutate;
		for(int j=0;j<e->size();j++){
			mutate.push_back( this->mesh_->vertices[e->at(j)] );
		}
		merged->push_back(mutate);
	}
	return;
}
bool Model::inNearNeighbour(vec v1, vec v2){
	if(dist(v1,v2)<0.05) 
		return true;
	else
		return false;
}

int Model::contourConnecting(vec scEnd,std::vector<int>* edge, double& d){
	double min_dist=10;
	int min_ind=-1;
	for(int i=0;i<edge->size();i++){
		double cur_dist= dist( projVec3(scEnd), projVec3(this->mesh_->vertices[edge->at(i)]));   //dist(scEnd, this->mesh_->vertices[edge->at(i)]);
		if(cur_dist< min_dist ){
			min_dist = cur_dist;
			min_ind = i;
		}
	}
	d= min_dist;
	return min_ind;
}
int Model::contourConnecting(vec scEnd,std::vector<vec>* edge, double& d){
	double min_dist=10;
	int min_ind=-1;
	for(int i=0;i<edge->size();i++){
		double cur_dist=dist(projVec3(scEnd), projVec3(edge->at(i)));
		if(cur_dist< min_dist ){
			min_dist = cur_dist;
			min_ind = i;
		}
	}
	d= min_dist;
	return min_ind;
}
/*
	Angular error function
	return
	1: link sc to the edge begin direction
	2: link sc to the edge end direction
	0: do not link
*/
int Model::TjunctionResolution_sc(std::vector<vec>* extendedSC, std::vector<int>* e, int split_pos, double& cos_val, bool ishead){
	point sc_dir, sc_dir2;
	if(ishead){
		sc_dir=6.0f*extendedSC->at(0)-2.0f*extendedSC->at(1)-3.0f*extendedSC->at(2)-extendedSC->at(3);
		sc_dir2=6.0f*(mesh_->vertices[e->at(split_pos)])-2.0f*extendedSC->at(0)-3.0f*extendedSC->at(1)-extendedSC->at(2); 
	}
	else{
		int tmp_sc_size = extendedSC->size();
		sc_dir=6.0f*extendedSC->at(tmp_sc_size-1)-2.0f*extendedSC->at(tmp_sc_size-2)-3.0f*extendedSC->at(tmp_sc_size-3)-extendedSC->at(tmp_sc_size-4); 
		sc_dir2 = 6.0f*(mesh_->vertices[e->at(split_pos)])-2.0f*extendedSC->at(tmp_sc_size-1)-3.0f*extendedSC->at(tmp_sc_size-2)-extendedSC->at(tmp_sc_size-3);
	}
	sc_dir /= 12.0f;sc_dir2 /= 12.0f;
	vec e_begin, e_end;
	if(split_pos > 2)	e_begin = -6.0f*(mesh_->vertices[e->at(split_pos)])+2.0f*mesh_->vertices[e->at(split_pos-1)]+3.0f*mesh_->vertices[e->at(split_pos-2)]+mesh_->vertices[e->at(split_pos-3)];
	if(e->size()-split_pos>3)	e_end = -6.0f*mesh_->vertices[e->at(split_pos)]+2.0f*mesh_->vertices[e->at(split_pos+1)]+3.0f*mesh_->vertices[e->at(split_pos+2)]+mesh_->vertices[e->at(split_pos+3)];
	
	// check the tanget at the split point
	double cos_split =( e_begin DOT e_end)/(len(e_begin)* len(e_end));
	
	vec2 e_begin2d = projDir3(e_begin);
	vec2 e_end2d = projDir3(e_end);
	double cos_split2d =( e_begin2d DOT e_end2d)/(len(e_begin2d)* len(e_end2d));

	double cos_val2=0;
	int dir = TjunctionResolution(sc_dir, e_begin, e_end ,cos_val);
	int dir2 = TjunctionResolution(sc_dir2, e_begin, e_end ,cos_val2);

	return dir;
}
int Model::TjunctionResolution_sc(std::vector<vec>* extendedSC, std::vector<vec>* e, int split_pos, double& cos_val, bool ishead){
	point sc_dir, sc_dir2;
	if(ishead){
		sc_dir=6.0f*extendedSC->at(0)-2.0f*extendedSC->at(1)-3.0f*extendedSC->at(2)-extendedSC->at(3);
		sc_dir2 =6.0f*e->at(split_pos)-2.0f*extendedSC->at(0)-3.0f*extendedSC->at(1)-extendedSC->at(2); 
	}
	else{
		int tmp_sc_size = extendedSC->size();
		sc_dir=6.0f*extendedSC->at(tmp_sc_size-1)-2.0f*extendedSC->at(tmp_sc_size-2)-3.0f*extendedSC->at(tmp_sc_size-3)-extendedSC->at(tmp_sc_size-4); 
		sc_dir2 =6.0f*e->at(split_pos)-2.0f*extendedSC->at(tmp_sc_size-1)-3.0f*extendedSC->at(tmp_sc_size-2)-extendedSC->at(tmp_sc_size-3); 
	}
	sc_dir /= 12.0f;sc_dir2 /= 12.0f;
	vec e_begin, e_end;
	if(split_pos > 2)	e_begin = -6.0f*e->at(split_pos)+2.0f*e->at(split_pos-1)+3.0f*e->at(split_pos-2)+e->at(split_pos-3);
	if(e->size()-split_pos>3)	e_end = -6.0f*e->at(split_pos)+2.0f*e->at(split_pos+1)+3.0f*e->at(split_pos+2)+e->at(split_pos+3);
	
	// check the tanget at the split point
	double cos_split =( e_begin DOT e_end)/(len(e_begin)* len(e_end));

	vec2 e_begin2d = projDir3(e_begin);
	vec2 e_end2d = projDir3(e_end);
	double cos_split2d =( e_begin2d DOT e_end2d)/(len(e_begin2d)* len(e_end2d));

	double cos_val2=0;
	int dir = TjunctionResolution(sc_dir, e_begin, e_end ,cos_val);
	int dir2 = TjunctionResolution(sc_dir2, e_begin, e_end ,cos_val2);

	return dir;
}
int Model::TjunctionResolution(vec _sc, vec _e_begin, vec _e_end, double& cos_val){
	// make it into 2D
	vec2 sc=projDir3(_sc);
	vec2 e_begin= projDir3(_e_begin);
	vec2 e_end = projDir3(_e_end);

	double thres_cos = connectAngThreshold;
	if(len(e_begin)==0 && len(e_end) == 0) { 
		cos_val =1;  
		return 0;}
	if(len(sc) ==0 ) {
		cos_val =1; 
		// assert(0); 
		return 0;}
	if(len(e_begin)==0) { cos_val = (sc DOT e_end)/(len(sc)* len(e_end)); if(cos_val<thres_cos) return 0; else return 2;}
	if(len(e_end)==0) { cos_val = (sc DOT e_begin)/(len(sc) * len(e_begin)); if(cos_val<thres_cos) return 0; else return 1;}

	//cos
	double cos_begin = (sc DOT e_begin)/(len(sc) * len(e_begin));
	double cos_end = (sc DOT e_end)/(len(sc)* len(e_end));

	if(cos_begin > cos_end && cos_begin>thres_cos){
		cos_val = cos_begin;
		return 1;
	}
	else if(cos_begin < cos_end  && cos_end>thres_cos){ cos_val=cos_end; return 2;}
	else{ 
		cos_val = 1;
		return 0;
	}
}
void Model::updateMOM(vec neib[], double& mom, int & mom_ind){
	
	double max_val=-1;
	int max_ind=-1;
	for(int i=0;i<3;i++){
		if(neib[i][0]>max_val){
			max_val = neib[i][0];
			max_ind = i;
		}
	}
	mom = max_val;
	mom_ind = max_ind;
	return;
}
vec2 Model::projDir3(vec v){
	GLdouble pos3D_x, pos3D_y, pos3D_z;
	gluProject(v[0], v[1] , v[2],
					modelview, projection, viewport, 
					&pos3D_x, &pos3D_y, &pos3D_z);
	vec2 tmp = vec2(pos3D_x, pos3D_y);
	gluProject(0, 0 , 0,
					modelview, projection, viewport, 
					&pos3D_x, &pos3D_y, &pos3D_z);
	vec2 tmp2 = vec2(pos3D_x, pos3D_y);
	return tmp-tmp2;
}
vec Model::projVec3(vec v){
	GLdouble pos3D_x, pos3D_y, pos3D_z;
	gluProject(v[0], v[1] , v[2],
					modelview, projection, viewport, 
					&pos3D_x, &pos3D_y, &pos3D_z);
	return vec(pos3D_x, pos3D_y,pos3D_z);
}


/*
	face based sc curves
	input: indices of a collection of faces
	output: curves  std::vector<std::vector<int>>: indices of vertices
*/

std::vector<std::vector<int>> Model::buildFaceChains(std::vector<int>* faces){
	std::vector<std::vector<int>> chains;

	std::vector<bool> visited;
	visited.resize( this->mesh_->faces.size());

	for(int i=0;i<visited.size();i++){
		visited[i] = false;
	}

	for(int i=0;i<faces->size();i++){
		int faceInd = faces->at(i);
		if(!visited[faceInd]){ // span
			visited[faceInd]=true;
			std::vector<int> chain = this->buildSingleFaceChain(faceInd, faces, &visited);
			//if( chain.size()>1 )
			chains.push_back( chain);
		}
	}

	int chk_sum =0;
	std::vector<std::vector<int>> tmp;
	for(int i=0;i<chains.size();i++){
		chk_sum += chains[i].size();
		if(chains[i].size()>0)
			tmp.push_back(chains[i]);
	}
	assert( chk_sum == faces->size());
	
	chains = tmp;
	return chains;
}
std::vector<int> Model::buildSingleFaceChain(int source, std::vector<int>* faces, std::vector<bool>* visited){
	std::list<int> neighbours;
	std::vector<int> chain;
	const vector<TriMesh::Face> & across_edge = this->mesh_->across_edge;

	chain.push_back(source);
	int faceInd = source;
	while (true){
		bool isEmpty = true;

		// strict connection
		for(int i=0;i<3;i++){
			int strict_neighbour = across_edge[faceInd][i];
			if( !visited->at(strict_neighbour) && this->inFaceIndicesList(strict_neighbour, faces ) ){
				faceInd = strict_neighbour;
				visited->at(strict_neighbour) = true;
				chain.push_back(strict_neighbour);
				isEmpty = false;
			}
		}


		
		if(isEmpty)
			return chain;
	}

	return chain;
}
bool Model::inFaceIndicesList(int toFind, std::vector<int>* faces){
	bool isfound = false;
	for(int i=0;i<faces->size();i++){
		if(toFind == faces->at(i))
			return true;
	}
	return isfound;
}
std::vector<std::vector<int>> Model::tryMergeFaces(std::vector<std::vector<int>>* allChains){
	
	for(int i=0; i< allChains->size();i++){
		int head = allChains->at(i).front(); // assume (indeed) the tail has no other source neighbors
		for(int j=i+1; j<allChains->size();j++){
			if(allChains->at(j).size()>1){
				int head2 = allChains->at(j).front(); 
				int tail2 = allChains->at(j).back();
				if(this->inFaceNeighbour( head , head2)){ 
					// reverse adjacent & add current chain to the adjacent chain
					// in this case, the head and tail of the current chain has no other source neighbors
					// but the tail of the adjacent chain may still have source neighbors
					// so put the tail of the adjacent chain as new head
					std::vector<int> mer;
					for(int k = allChains->at(j).size()-1; k>=0; k--){
						mer.push_back(allChains->at(j)[k]);
					}
					mer.insert(mer.end(),allChains->at(i).begin(),allChains->at(i).end());
					allChains->at(i) = mer;
					// remove the adjacent chain
					allChains->erase(allChains->begin()+j);
					j--;
					continue;
				}

				else if(this->inFaceNeighbour( head , tail2)){ 
					// in this case, the head and tail of the current chain has no source neighbors
					// but the head of the adjacent chain may still have source neighbors
					// so put the head of the adjacent chain as new head
					std::vector<int> mer;
					mer.insert(mer.end(),allChains->at(j).begin(),allChains->at(j).end());
					mer.insert(mer.end(),allChains->at(i).begin(),allChains->at(i).end());
					allChains->at(i) = mer;
					// remove the adjacent chain
					allChains->erase(allChains->begin()+j);
					j--;
					continue;
				}
			}
			else if(allChains->at(j).size() == 1){
				int head2 = allChains->at(j).front(); 
				if(this->inFaceNeighbour( head , head2)){
					std::vector<int> mer;
					mer.push_back(head2);
					mer.insert(mer.end(),allChains->at(i).begin(),allChains->at(i).end());
					allChains->at(i) = mer;
					// remove the adjacent chain
					allChains->erase(allChains->begin()+j);
					j--;
					continue;
				}
			}
			else {
				assert(0);
			}
		}
	}
	return *allChains;
}
bool  Model::inFaceNeighbour( int f1,int f2){
	bool isIn =false;
	const vector<TriMesh::Face> & across_edge = this->mesh_->across_edge;

	// strict connection
	for(int i=0;i<3;i++){
		int strict_neighbour = across_edge[f1][i];
		if( strict_neighbour == f2 ){
			return true;
		}
	}

	// loose connection
	TriMesh::Face f = this->mesh_->faces[f1];

	int p1 = f[0];
	int p2 = f[1];
	int p3 = f[2];
	std::vector<int> ns1 = this->mesh_->adjacentfaces[p1];
	std::vector<int> ns2 = this->mesh_->adjacentfaces[p2];
	std::vector<int> ns3 = this->mesh_->adjacentfaces[p2];

	std::vector<int> ns;
	ns.insert( ns.end(),ns1.begin(),ns1.end());
	ns.insert( ns.end(),ns2.begin(),ns2.end());
	ns.insert( ns.end(),ns3.begin(),ns3.end());

	for(int j=0;j <ns.size();j++){
		if(ns[j] == f2){
			return true;
		}
	}


	return isIn;
}
// TODO: better stratege to chose vertex from face
std::vector<std::vector<int>>  Model::faceChain2VertexChain(std::vector<std::vector<int>>* faceChains){
	std::vector<std::vector<int>> vertexChains;
	for(int i=0;i<faceChains->size();i++){
		std::vector<int>& fChain = faceChains->at(i);
		if(fChain.size()<3) continue;
		std::vector<int> vChain;
		for(int j=0;j<fChain.size();j++){
			TriMesh::Face f = this->mesh_->faces[fChain[j]];
			vChain.push_back(f[0]);
		}
		vertexChains.push_back(vChain);
	}

	// remove redundant vertices (repeated vertices)
	std::vector<std::vector<int>> filteredChain;
	for(int i=0;i<vertexChains.size();i++){
		std::vector<int>& vChain = vertexChains[i];
		std::vector<int> filtered;
		int lastV=-1;
		for(int j=0;j<vChain.size();j++){
			if(vChain[j] != lastV){
				filtered.push_back(vChain[j]);
				lastV = vChain[j];
			}
		}
		if(filtered.size()<3) continue;
		filteredChain.push_back(filtered);
	}

	return filteredChain;
}

// return 1 for begin direction, 2 for end direction, 0 only if error;
int Model::TjunctionResolution_assert(vec _sc, vec _e_begin, vec _e_end, double& cos_val){
	// make it into 2D
	vec2 sc=projDir3(_sc);
	vec2 e_begin= projDir3(_e_begin);
	vec2 e_end = projDir3(_e_end);

	if(len(e_begin)==0 && len(e_end) == 0) { cos_val =1; return 0;}
	if(len(sc) ==0 ) {cos_val =1;assert(0);return 0;}
	if(len(e_begin)==0) { cos_val = (sc DOT e_end)/(len(sc)* len(e_end)); return 2;}
	if(len(e_end)==0) { cos_val = (sc DOT e_begin)/(len(sc) * len(e_begin)); return 1;}

	//cos
	double cos_begin = (sc DOT e_begin)/(len(sc) * len(e_begin));
	double cos_end = (sc DOT e_end)/(len(sc)* len(e_end));

	if(cos_begin > cos_end ){
		cos_val = cos_begin;
		return 1;
	}
	else{ cos_val=cos_end; return 2;}
}
// make sure sc.size()>3
int Model::TjunctionResolution_sc(std::vector<int>* sc, std::vector<int>* e, int split_pos, double& cos_val, bool ishead){
	point sc_dir, sc_dir2;
	const std::vector<vec>& vList = this->mesh_->vertices;

	// check distance, if too far, do not connect
	point sc_connector, dist_vec;
	if(ishead)
		sc_connector = vList[ sc->front()];
	else
		sc_connector = vList[ sc->back()];
	dist_vec = sc_connector - vList[e->at(split_pos)];
	if( len(  this->projVec3(dist_vec)) >7 ) return 0;


	if(ishead){
		sc_dir =6.0f* vList[ sc->at(0)]-2.0f*vList[sc->at(1)]-3.0f*vList[sc->at(2)]-vList[sc->at(3)];
		//sc_dir2=6.0f*(vList[e->at(split_pos)])-2.0f*vList[sc->at(0)]-3.0f*vList[sc->at(1)]-vList[sc->at(2)]; 
	}
	else{
		int tmp_sc_size = sc->size();
		sc_dir=6.0f*vList[sc->at(tmp_sc_size-1)]-2.0f*vList[sc->at(tmp_sc_size-2)]-3.0f*vList[sc->at(tmp_sc_size-3)]-vList[sc->at(tmp_sc_size-4)]; 
		//sc_dir2 = 6.0f*(mesh_->vertices[e->at(split_pos)])-2.0f*extendedSC->at(tmp_sc_size-1)-3.0f*extendedSC->at(tmp_sc_size-2)-extendedSC->at(tmp_sc_size-3);
	}
	sc_dir /= 12.0f;
	//sc_dir2 /= 12.0f;

	// return dir if only one side available
	/*if(split_pos < 3 || e->size()-split_pos<4 ){
		if(split_pos > e->size()-split_pos )
			return 1;
		else if(split_pos < e->size()-split_pos )
			return 2;
		else
			return 0;
	}*/

	vec e_begin, e_end;
	if(split_pos > 2)	e_begin = -6.0f*(mesh_->vertices[e->at(split_pos)])+2.0f*mesh_->vertices[e->at(split_pos-1)]+3.0f*mesh_->vertices[e->at(split_pos-2)]+mesh_->vertices[e->at(split_pos-3)];
	if(e->size()-split_pos>3)	e_end = -6.0f*mesh_->vertices[e->at(split_pos)]+2.0f*mesh_->vertices[e->at(split_pos+1)]+3.0f*mesh_->vertices[e->at(split_pos+2)]+mesh_->vertices[e->at(split_pos+3)];
	
	// check the tanget at the split point
	double cos_split =( e_begin DOT e_end)/(len(e_begin)* len(e_end));
	
	vec2 e_begin2d = projDir3(e_begin);
	vec2 e_end2d = projDir3(e_end);
	double cos_split2d =( e_begin2d DOT e_end2d)/(len(e_begin2d)* len(e_end2d));

	double cos_val2=0;
	int dir = TjunctionResolution(sc_dir, e_begin, e_end ,cos_val);
	// int dir2 = TjunctionResolution(sc_dir2, e_begin, e_end ,cos_val2);

	return dir;
}
// if any edge vertex is in 2-ring's neighbour of head/tail of sc
// split edge line, and connect one of them to sc, according to {tanget} values at that point
// exsc can be regarded as edge
std::vector<std::vector<int>> Model::buildEXSC(std::vector<std::vector<int>>* sc, std::vector<std::vector<int>>* edge){
	std::vector<std::vector<int>> exsc;
	const int through_away_threshold = 1;

	// assumption: no two sc edges can be connected: ensured by preprocessing of sc strokes
	for(int i=0;i<sc->size();i++){ // each sc line
		// if an sc line donesn't connect any edge, then it will not connect any extened edge
		std::vector<int>& scline = sc->at(i);
		if(scline.size()<4) continue;
		int head = scline.front();
		int tail = scline.back();

		// connect to edge
		bool connected = false;
		for(int j=0;j<edge->size() && (!connected);j++){
			std::vector<int>& e = edge->at(j);
			for(int k=0; k<e.size();k++){
				// head connection
				if(this->in2RingNeighbour( head, e[k]) && e[k]!=tail ){
					//determine connecting direction
					double error =0;
					int dir = this->TjunctionResolution_sc(&scline, &e, k,error, true);
					std::vector<int> newEXSC;
					//connect to the edge begin direction
					if(dir == 1){
						newEXSC.insert(newEXSC.end(),e.begin(),e.begin()+k);
						newEXSC.insert(newEXSC.end(),scline.begin(),scline.end());
						//exsc.push_back(newEXSC);

						// update e
						// modify the remaining edge
						if(e.size() -1- k>through_away_threshold){
							std::vector<int> tmp;
							tmp.insert(tmp.end(),e.begin()+k,e.end());
							e = tmp;
						}
						else{
							edge->erase(edge->begin()+j);
							j--;
						}
						// and put the new exteneded edge to the tail
						edge->push_back(newEXSC);

						connected = true;
						break;
					}
					//connect to the edge end diretion
					else if(dir ==2){
						for(int it= e.size()-1;it>=k;it--){
							newEXSC.push_back(e[it]);
						}
						newEXSC.insert(newEXSC.end(),scline.begin(),scline.end());
						//exsc.push_back(newEXSC);

						// update e
						if(k>through_away_threshold){
							std::vector<int> tmp;
							tmp.insert(tmp.end(),e.begin(),e.begin()+k);
							e = tmp;
						}
						else{
							edge->erase(edge->begin()+j);
							j--;
						}
						// and put the new exteneded edge to the tail
						edge->push_back(newEXSC);

						connected = true;
						break;
					}
					else{}
				}
				// tail connection
				else if(this->in2RingNeighbour( tail, e[k]) && e[k]!=head ){
					double error =0;
					int dir = this->TjunctionResolution_sc(&scline, &e, k,error, false);
					std::vector<int> newEXSC;
					if(dir ==1){ // to the e.begin direction
						newEXSC.insert(newEXSC.end(),e.begin(),e.begin()+k);
						for(int it=scline.size()-1; it>=0;it--){
							newEXSC.push_back(scline[it]);
						}
						//exsc.push_back(newEXSC);

						// update e
						if(e.size() -1- k>through_away_threshold){
							std::vector<int> tmp;
							tmp.insert(tmp.end(),e.begin()+k,e.end());
							e = tmp;
						}
						else{
							edge->erase(edge->begin()+j);
							j--;
						}
						// and put the new exteneded edge to the tail
						edge->push_back(newEXSC);

						connected = true;
						break;
					}
					else if(dir ==2){ // to the e.end direction
						// [e.end, e.k] + [sc.tail, sc.head]
						// reverse, get [sc.head, sc.tail] +[e.k+e.end]
						newEXSC.insert(newEXSC.end(),scline.begin(),scline.end());
						newEXSC.insert(newEXSC.end(),e.begin()+k,e.end());
						//exsc.push_back(newEXSC);

						// update e
						if(k>through_away_threshold){
							std::vector<int> tmp;
							tmp.insert(tmp.end(),e.begin(),e.begin()+k);
							e = tmp;
						}
						else{
							edge->erase(edge->begin()+j);
							j--;
						}

						// and put the new exteneded edge to the tail
						edge->push_back(newEXSC);
						connected = true;
						break;
					}
					else{}
				}
			}

			
		}
		if(!connected)
			exsc.push_back(scline);
		// connect to exsc
	}


	

	return exsc;
}

/********************************************************************
	                            covex hull
*********************************************************************/
coord2_t Model::cross(const Point &O, const Point &A, const Point &B)
{
	return (A.x - O.x) * (coord2_t)(B.y - O.y) - (A.y - O.y) * (coord2_t)(B.x - O.x);
}
vector<Point> Model::convex_hull_2d(vector<Point> P)
{
	int n = P.size(), k = 0;
	vector<Point> H(2*n);

	// Sort points lexicographically
	sort(P.begin(), P.end());

	// Build lower hull
	for (int i = 0; i < n; i++) {
		while (k >= 2 && cross(H[k-2], H[k-1], P[i]) <= 0) k--;
		H[k++] = P[i];
	}

	// Build upper hull
	for (int i = n-2, t = k+1; i >= 0; i--) {
		while (k >= t && cross(H[k-2], H[k-1], P[i]) <= 0) k--;
		H[k++] = P[i];
	}

	H.resize(k);
	return H;
}
bool Model::compute_convex_hull_2d(vector<Point> P, int h){


	vector<Point> hull = convex_hull_2d(P);
	vector<vec2> hull2;
	for(int i=0;i<hull.size();i++){
		hull2.push_back(vec2(hull[i].x,  h-hull[i].y));
	}
	this->convex_hulls_2d.push_back(hull2 );

	return true;


}
void Model::update_convex_hull_2d(int h, bool useSimHull = false){
	// 2d convex hull
	vector<Point> patch_points;
	this->convex_hulls_2d.clear();
	
	// get the global hull
	const std::vector<vec>& vList =this->mesh_->vertices; 
	for(int i=0;i<vList.size();i++){
		vec proj = this->projVec3( vList[i]);
		Point cvp;
		cvp.x = proj[0];
		cvp.y = proj[1];
		patch_points.push_back(cvp);
	}
	this->compute_convex_hull_2d(patch_points,h);

	
	if(useSimHull && this->branches.size()>0){
		for(int i=0;i<branches.size();i++){
			patch_points.clear();
			for(int j=0;j<branches[i].size();j++){
				int seg = branches[i][j];
			
				for(int k=0;k<patches_vertex_index[seg].size();k++){
					int verInd = patches_vertex_index[seg][k];
					point v = this->mesh_->vertices[verInd];

					GLdouble pos3D_x, pos3D_y, pos3D_z;
					gluProject(v[0], v[1], v[2],
						modelview, projection, viewport, 
						&pos3D_x, &pos3D_y, &pos3D_z);
					Point proj_p;
					proj_p.x = pos3D_x;
					proj_p.y = pos3D_y;

					patch_points.push_back(proj_p);
				}
			}
			this->compute_convex_hull_2d(patch_points,h);
		}
	}
	else{
		for(int i=0;i<patches_vertex_index.size() ;i++){
			patch_points.clear();
			for(int j=0;j<patches_vertex_index[i].size();j++){
				int verInd = patches_vertex_index[i][j];
				point v = this->mesh_->vertices[verInd];

				GLdouble pos3D_x, pos3D_y, pos3D_z;
				gluProject(v[0], v[1], v[2],
					modelview, projection, viewport, 
					&pos3D_x, &pos3D_y, &pos3D_z);
				Point proj_p;
				proj_p.x = pos3D_x;
				proj_p.y = pos3D_y;

				patch_points.push_back(proj_p);
			}
			this->compute_convex_hull_2d(patch_points,h);
		}
	}


	return;
}

void Model::setGraph(vGraph* _g, std::vector<NodeID> _nIDs){ 
	graph = _g;
	this->segment2NodeID = _nIDs;

	return;
}
void Model::buildBranches(std::vector<std::vector<vPoint3D>>* skeleton){ // call after get raw pose line, and do not change afterwards

	this->branches.clear();

	for(int i=0;i<skeleton->size();i++){
	
		std::vector<vPoint3D>& raw_b = skeleton->at(i);

		std::vector<int> coveredSeg;
		for(int j=0;j<raw_b.size();j++){
			int seg = raw_b[j].segment;
			bool _exist=false;
			for(int k=0;k<coveredSeg.size();k++){
				if(coveredSeg[k] == seg){
					_exist = true;
					break;
				}
			}
			if(! _exist){
				coveredSeg.push_back(seg);
			}
		}

		// type A: spine
		if(coveredSeg.size() == 1){
			// assert
			NodeID nID = this->segment2NodeID[coveredSeg[0]];
			int outDegree = boost::out_degree(nID, this->graph->g);
			assert(outDegree > 2);

			branches.push_back(coveredSeg);
		}
		else if(coveredSeg.size()>1){ // type B: branch, possibly with a bit of spine
			std::vector<int> removeSpine;
			for(int j=0;j<coveredSeg.size();j++){
				NodeID nID = this->segment2NodeID[coveredSeg[j]];
				int outDegree = boost::out_degree(nID, this->graph->g);
				assert(outDegree >0);
				if(outDegree <= 2){
					removeSpine.push_back(coveredSeg[j]);
				}
			}

			if(removeSpine.size()>0){
				branches.push_back(removeSpine);
			}
		}
	}

	return;
}
