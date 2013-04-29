#include "vNode.h"
#include "vGraph.h"


float vNode::beta=1;
float vNode::fieldSize = 5;
float vNode::lamda = sqrt(2*beta)/fieldSize; // lamda < sqrt(2*beta)/(max_d)

vNode::vNode(){
	init();
}
vNode::vNode(int _segID){
	init();
	this->segmentID = _segID;
}
vNode::~vNode(){
	if(superNode != NULL) delete superNode;

}

void vNode::init(){
	this->segmentID = -1;
	this->flag_pose=false;
	vertices = NULL;
	superNode = NULL;
	stroke = NULL;
	this->isLeaf = true;
	entropy = 0;
	information =0;
	voting =0;
	visited = false;
	//lengthAlpha = 0.5;
	return;
}

void vNode::push_vertex_index(int _ind){
	v.push_back(_ind);
	return;
}

void vNode::setVertices(const std::vector<vec>* _vertices){
		vertices = _vertices;
}

void vNode::setCurve(vCurve* _curve){
	this->stroke = _curve;
	return;
}

vCurve* vNode::getCurve(){
	return this->stroke;
}

void vNode::computeCenter(){
	vec accum;
	for(int i=0;i<v.size();i++){
		accum += vertices->at(v[i]);
	}
	this->center = accum/(float)v.size();
}

void vNode::draw(IplImage* img, CvScalar color, int rate, int LOD, bool output){
	if(this->isLeaf){
		this->stroke->draw(img, color, rate, LOD, output);
	}
	else{
		_Graph::vertex_iterator vIt, vEnd;
		assert(this->superNode != NULL);
		for(boost::tie(vIt,vEnd)=boost::vertices(this->superNode->g);vIt != vEnd;vIt++){
			this->superNode->g[*vIt]->draw( img, color, rate, LOD, output);
		}
	}
	return;
}

double vNode::computeCurEntropy(const vGraph& ref){
	double e=0;
	const _Graph& g = ref.g;

	for(int i=0;i<this->coveredSegments.size();i++){
		using namespace boost;
		int seg = this->coveredSegments[i];

		// find the reference segment
		_Graph::vertex_iterator vIt, vEnd;
		vNode* refSeg = NULL;
		for(boost::tie(vIt,vEnd)=boost::vertices(g);vIt != vEnd ;vIt++){
			vNode* n= g[*vIt];
			if(!n->isLeaf && n->segmentID == seg){
				refSeg = n;
				break;
			}
		}
		//assert( refSeg != NULL); // this happens, some segments have no exsc in certain viewpoint, just skip this segment
		if(refSeg == NULL){
			continue;
		}

		//std::queue<vNode*> nq;
		// get entropy sum from this seg ref
		_Graph& g_seg = refSeg->superNode->g;
		for(boost::tie(vIt,vEnd)=boost::vertices(g_seg);vIt != vEnd ;vIt++){
			vNode* n= g_seg[*vIt];
			assert(n->isLeaf);
			double f = megaForce( this->stroke, n->getCurve());
			
			// the generated entropy can not be larger then the content reference's information
			double bound;
			if(n->information == 0){bound = n->computeInformation();}
			else{bound = n->information;}
			if(f>bound)
				f = bound;

			if(f>n->voting){ // count the voting only if it provides extra info
				e += (f-n->voting);
			}
		}
	}
	return e;
}


double vNode::computeInformation(){
	assert(this->stroke != NULL);
	double info=0;
	if(this->stroke->type ==vCurve::STRAIGHT){
		info = megaForce(this->stroke, this->stroke);//
		//info = 1/beta;
	}
	else if(this->stroke->type ==vCurve::OPEN){
		info = megaForce(this->stroke, this->stroke);
	}
	else {
		assert(0);
	}

	if(info == 0){ // possible, for loop, STRAIGHT
			//info = 1;
			//assert(0);
		// info = DBL_MAX;
	}

	this->information = info;
	return info;
}

// distance from point v to line segment [u1,u2]
float vNode::Dist(vec2 uc, vec2 u, vec2 v){
	float d;

	vec2 tanu = normalize(u);
	vec2 noru = vec2(tanu[1],-tanu[0]);
	float d_tan = abs( tanu DOT (v-uc));
	float d_nor = abs (noru DOT (v-uc));
	
	if(d_tan<= len(u)/2.0f) { // point v project onto the line segment [u]
		d = d_nor;
	}
	else{ // project outside the line segment
		vec2 u1 = uc+0.5f*u;
		vec2 u2 = uc-0.5f*u;
		float d1= dist(v, u1);
		float d2= dist(v, u2);
		d = (d1>d2)?d2:d1;
	}
	return d;
}

// approximated as a point to line segment distance
float vNode::Dist(vec2 uc, vec2 u, vec2 vc, vec2 v){

	float d[4];
	d[0] = Dist(uc,u, vc+0.5f*v);
	d[1] = Dist(uc,u, vc-0.5f*v);
	d[2] = Dist(vc,v, uc+0.5f*u);
	d[3] = Dist(vc,v, uc-0.5f*u);

	float min_dist=std::numeric_limits<float>::max();
	for(int i=0;i<4;i++){
		if(min_dist > d[i])
			min_dist = d[i];
	}

	return min_dist;
}

// TODO: consider mF(c1, c2) = mF(c2,c1) right?
double vNode::megaForce( vCurve* c1,  vCurve* c2){
	assert(c1 != NULL && c2!=NULL);
	float ff =0;
	
	if(c1->size()>1 && c2->size()>1){
		if(c1->type == vCurve::OPEN && c2->type == vCurve::OPEN){
			for(int i=0;i<c1->size()-1;i++){
				vec2 v = c1->curve2d[i+1].toVec2() - c1->curve2d[i].toVec2();
				vec2 vc = (c1->curve2d[i+1].toVec2() + c1->curve2d[i].toVec2())/2.0f;
				if(len(v) == 0) continue;
				for(int j=0;j<c2->size()-1;j++){
					vec2 u = c2->curve2d[j+1].toVec2() - c2->curve2d[j].toVec2();
					vec2 uc = (c2->curve2d[j+1].toVec2() + c2->curve2d[j].toVec2())/2.0f;

					if(len(u) == 0) continue;
					float d = dist(vc,uc);
					if(d < this->fieldSize){ // far lines do not provide entropy
						d *= lamda;
						ff += abs( (v DOT u)/(len(v)*len(u)*(d*d+beta)));
						//ff += abs( (v DOT u)/(d*d+beta));
					}
				}
			}
		}
		else if(c1->type == vCurve::STRAIGHT && c2->type == vCurve::STRAIGHT){
			vec2 v = c1->curve2d.back().toVec2() - c1->curve2d.front().toVec2();
			vec2 vc = (c1->curve2d.back().toVec2() + c1->curve2d.front().toVec2())/2.0f;
			if(len(v) == 0) return 0;

			vec2 u = c2->curve2d.back().toVec2() - c2->curve2d.front().toVec2();
			vec2 uc = (c2->curve2d.back().toVec2() + c2->curve2d.front().toVec2())/2.0f;
			
			if(len(u) == 0) return 0;
			float d = dist(vc,uc);
			if(d < this->fieldSize){ // far lines do not provide entropy
				d *= lamda;
				ff += abs( (v DOT u)/(len(v)*len(u)*(d*d+beta)));
				//ff += abs( (v DOT u)/(d*d+beta));
			}
		
		}
		else if(c1->type == vCurve::STRAIGHT && c2->type == vCurve::OPEN){
			vec2 v = c1->curve2d.back().toVec2() - c1->curve2d.front().toVec2();
			vec2 vc = (c1->curve2d.back().toVec2() + c1->curve2d.front().toVec2())/2.0f;
			
			if(len(v) != 0) {
				for(int j=0;j<c2->size()-1;j++){
					vec2 u = c2->curve2d[j+1].toVec2() - c2->curve2d[j].toVec2();
					vec2 uc = (c2->curve2d[j+1].toVec2() + c2->curve2d[j].toVec2())/2.0f;
					if(len(u) == 0) continue;

					float d= Dist(uc,u,vc,v);// dist(uc,vc);//this->Dist(uc,u,vc,u);

					if(d < this->fieldSize){ // far lines do not provide entropy
						d *= lamda;
						ff += abs( (v DOT u)/(len(v)*len(u)*(d*d+beta)));
						//ff += abs( (v DOT u)/(d*d+beta));
					}
				}
			}
		}
		else if(c2->type == vCurve::STRAIGHT && c1->type == vCurve::OPEN){
			ff = megaForce(c2,c1);
		}
		else{
			assert(0);
		}
	}

	return abs( ff);
}

double vNode::updateVoting( vGraph& ref){
	double e=0;
	_Graph& g = ref.g;

	for(int i=0;i<this->coveredSegments.size();i++){
		using namespace boost;
		int seg = this->coveredSegments[i];

		// find the reference segment
		_Graph::vertex_iterator vIt, vEnd;
		vNode* refSeg = NULL;
		for(boost::tie(vIt,vEnd)=boost::vertices(g);vIt != vEnd ;vIt++){
			vNode* n= g[*vIt];
			if(!n->isLeaf && n->segmentID == seg){
				refSeg = n;
				break;
			}
		}
		
		if(refSeg == NULL){
			continue;
		}

		// get entropy sum from this seg ref
		_Graph& g_seg = refSeg->superNode->g;
		for(boost::tie(vIt,vEnd)=boost::vertices(g_seg);vIt != vEnd ;vIt++){
			vNode* n= g_seg[*vIt];
			assert(n->isLeaf);
			double f = megaForce( this->stroke, n->getCurve());

			// the generated entropy can not be larger then the content reference's information
			double bound;
			if(n->information == 0){bound = n->computeInformation();}
			else{bound = n->information;}
			if(f>bound)
				f = bound;

			if(f>n->voting){ // count the voting only if it provides extro info
				e += (f-n->voting);
				n->voting = f;
			}
		}
	}

	this->entropy = e;
	return e;
}

void vNode::setH(int h){
	hsv_h = h;
}

int vNode::getH(){
	return hsv_h;
}
// void vNode::computeControlPoint(){
// 	if(!isLeaf){
// 		controlPoint=vec2(0, 0);
// 		_Graph &g=superNode->g;
// 		_Graph::vertex_iterator vi, vEnd;
// 		int n=0;
// 		for(boost::tie(vi, vEnd)=boost::vertices(g); vi!=vEnd; vi++){
// 			NodeID nid=*vi;
// 			g[nid]->computeControlPoint();
// 			n++;
// 			controlPoint+=g[nid]->controlPoint;
// 		}
// 		if(n!=0)
// 			controlPoint/=n;
// 		return;
// 	}
// 	double length=0;
// 	vCurve* curve=this->getCurve();
// 	length=curve->Length();
// 	double relen=0;
// 	//printf("length=%d\n", length);
// 	for(int i=0;i<curve->curve2d.size()-1;i++){
// 		vec2 p1 = curve->curve2d[i].toVec2();
// 		vec2 p2 = curve->curve2d[i+1].toVec2();
// 		relen += dist(p1,p2);
// 
// 		if(relen>=length*lengthAlpha){
// 			float alpha=(relen-length*lengthAlpha)/dist(p1,p2);
// 			controlPoint=p1*alpha+(1-alpha)*p2;
// 			controlPoint[0]+=0.5;
// 			controlPoint[1]+=0.5;
// 			break;
// 		}
// 	}
// }

