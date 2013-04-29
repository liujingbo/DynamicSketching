#include "PatchDrawer.h"

/**
 * Constructor
 */
PatchDrawer::PatchDrawer(): Drawer(true){
	this->isColoredPatchesDisplay = false;
	this->isPrimitivesDisplay=false;
	init();
}

void PatchDrawer::init(){
	this->isVertexColored = false;
	this->isFittingCylinderReady=false;

	this->numOfDisplayPatch = 0;

}
/**
 * Draw the given model, given the current camera position
 *
 * @param: Model* : the model to be drawn
 * @param: camera_position: the current camera position, in 3d-coordinates
 */
void PatchDrawer::draw(Model* m, vec camera_position)
{
	if(!this->visible_) return;

	if( GetColoredPatchesDisplay() && GetVertexColored()){
		// use simple triangle drawing
		//FaceList fList = mesh.Faces();
		const vector<TriMesh::Face>& fList   = m->mesh_->faces;
		const vector<point>&         vList   = m->mesh_->vertices;
		const vector<vec>&           nList   = m->mesh_->normals;
		const vector<float>&         cList   = m->colorlist_;
		const vector<int>&           pcolors = m->pointcolors_;
		const vector<int>&         fcolors = m->facecolors_; // for patch color

		glShadeModel(GL_FLAT); 
		//glColor3f(0.4f, 0.4f, 1.0f);
		glBegin(GL_TRIANGLES) ;
		for (size_t i=0; i<fList.size(); i++) {

			TriMesh::Face f = fList[i];
			point v0 = vList[f.v[0]];
			point v1 = vList[f.v[1]];
			point v2 = vList[f.v[2]];

			int colorindex = fcolors[i];
			glColor3f(cList[colorindex*3], cList[colorindex*3+1], cList[colorindex*3+2]);
			glVertex3d(v0[0],v0[1],v0[2]);
			glVertex3d(v1[0],v1[1],v1[2]);
			glVertex3d(v2[0],v2[1],v2[2]);
		}
		glEnd();

		//// setup vertex and array pointers
		//glBindBufferARB(GL_ARRAY_BUFFER_ARB, m->vbo_positions_);
		//glEnableClientState(GL_VERTEX_ARRAY); // enable vertices
		//glVertexPointer(3, GL_FLOAT,0,0);
		//glBindBufferARB(GL_ARRAY_BUFFER_ARB, m->vbo_normals_);
		//glEnableClientState(GL_NORMAL_ARRAY); // enable vertices
		//glNormalPointer(GL_FLOAT,0,0);
		//glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);


		//// try without VBO for color
		//// ref with VBO: http://sdickinson.com/wordpress/?p=122
		//if ( GetVertexColored() ){
		//	glEnableClientState( GL_COLOR_ARRAY );
		//	glColorPointer(3, GL_FLOAT, 0, m->facecolors_.data());
		//}
		//// draw the mesh_ using triangle strips
		////glColor3f(1,1,1);
		//draw_tstrips(m->mesh_);

		//glDisableClientState(GL_COLOR_ARRAY); //http://openflipper.org/Documentation/OpenFlipper-1.0.1/Unsupported_2Streaming_2Client_2MeshViewerWidgetT_8cc_source.html
	}
	
	if( GetPrimitivesDisplay() && GetFittingCylinderReady() ){
		//int clk  = clock();
		//int num_of_display_patch = (int)(((float)clk)/CLOCKS_PER_SEC/0.5) % (m->fitting_cylinders.size()) + 1;
		//if(m->fitting_cylinders.size() == 0) num_of_display_patch=0;
		//fitting_cylinders is in the same order as segment #

		//vector<int> order=m->hierarchy->order;// simply for easier access
		

		for(int i=0; i<m->fitting_cylinders.size() && i< ((this->numOfDisplayPatch) % m->fitting_cylinders.size())+1 ;i++){
			int displayPatchNum = i;//order[i];
			FittingCylinder* cyl=m->fitting_cylinders[displayPatchNum];

			GLdouble radius = abs(cyl->cylinder_radius);
			GLdouble height = cyl->cylinder_height;
			Vector3d axis = cyl->cylinder_axis;
			Vector3d center = cyl->cylinder_center;
			const vector<float>&  cList = m->colorlist_;

			Vector3d ori_center = Vector3d(0,0,0);
			Vector3d ori_axis = Vector3d(0,0,1);
			Vector3d center_mov = center - ori_center;
			Vector3d rotAxis =( ori_axis.Cross( axis)).Normalize();

			double rotAngle = (acos(ori_axis.Dot(axis))  * 180.0) / 3.14159265;

			glShadeModel(GL_FLAT); 
			glDisable(GL_LIGHTING);
			glDisable(GL_BLEND);

			glColor3f(cList[displayPatchNum*3],cList[displayPatchNum*3+1],cList[displayPatchNum*3+2]);

			// transformation
			
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();

			// **************************************************************** //
							// cylinder
			// **************************************************************** //
			glTranslated(center_mov.X(),center_mov.Y(),center_mov.Z());
			glRotated(rotAngle, rotAxis.X(), rotAxis.Y(), rotAxis.Z());
			glTranslated(0,0,-height/2);

			GLUquadricObj* quad = gluNewQuadric();
			gluQuadricDrawStyle(quad, GLU_LINE);
			gluCylinder(quad, radius, radius, height, 30, 20); // height is not determined, tmp as the two extrems
		
			gluDeleteQuadric(quad);

			// **************************************************************** //
							// NPR
			// **************************************************************** //

	
			glPopMatrix();
		}
	}
}

/**
 * Draw the given mesh using the Triangle strips method (which is the fastest).
 *
 * @param: themesh: the mesh to be drawn.
 */
void PatchDrawer::draw_tstrips(const TriMesh *themesh)
{
	const int *t = &themesh->tstrips[0];
	const int *end = t + themesh->tstrips.size();
	while (likely(t < end)) { // as long as we haven't reached the TSTRIPS-end
		int striplen = *t++;
		glDrawElements(GL_TRIANGLE_STRIP, striplen, GL_UNSIGNED_INT, t);
		t += striplen;
	}
}


void PatchDrawer::SetVertexColored(bool iscolored_){
	this->isVertexColored = iscolored_;
}
const bool PatchDrawer::GetVertexColored(){

	return this->isVertexColored;
}

void PatchDrawer::SetFittingCylinderReady(bool isReady_){
	this->isFittingCylinderReady = isReady_;
}
const bool PatchDrawer::GetFittingCylinderReady(){
	return this->isFittingCylinderReady;
}

bool PatchDrawer::toggleCylinder(){
	isFittingCylinderReady = !isFittingCylinderReady;
	return isFittingCylinderReady;
}

bool PatchDrawer::PrevStep(){
	this->numOfDisplayPatch --;
	return true;
}
bool PatchDrawer::NextStep(){
	this->numOfDisplayPatch++;
	return true;
}