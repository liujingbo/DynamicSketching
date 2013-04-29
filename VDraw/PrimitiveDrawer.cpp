#include "PrimitiveDrawer.h"

/**
 * Constructor
 */
PrimitiveDrawer::PrimitiveDrawer(): Drawer(true){
	this->isVertexColored = false;
	this->isFittingCylinderReady=false;
	this->isColoredPatchesDisplay = false;
	this->isPrimitivesDisplay=false;
	this->numOfDisplayPatch = 0;
}
void PrimitiveDrawer::init(){
	this->isVertexColored = false;
	this->isFittingCylinderReady=false;
	this->isColoredPatchesDisplay = false;
	this->isPrimitivesDisplay=false;
	this->numOfDisplayPatch = 0;

}
/**
 * Draw the given model, given the current camera position
 *
 * @param: Model* : the model to be drawn
 * @param: camera_position: the current camera position, in 3d-coordinates
 */
void PrimitiveDrawer::draw(Model* m, vec camera_position)
{
	if(!this->visible_) return;
	
	if( GetVertexColored() && GetPrimitivesDisplay() && GetFittingCylinderReady() ){
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
			glPushMatrix(); // 34151614331

			/****************************************************************
								cylinder
			****************************************************************/
			glTranslated(center_mov.X(),center_mov.Y(),center_mov.Z());
			glRotated(rotAngle, rotAxis.X(), rotAxis.Y(), rotAxis.Z());
			glTranslated(0,0,-height/2);

			GLUquadricObj* quad = gluNewQuadric();
			gluQuadricDrawStyle(quad, GLU_LINE);
			gluCylinder(quad, radius, radius, height, 30, 20); // height is not determined, tmp as the two extrems
		
			gluDeleteQuadric(quad);

			/****************************************************************
								
			****************************************************************/

	
			glPopMatrix();// 34151614331
		}
	}
}

/**
 * Draw the given mesh using the Triangle strips method (which is the fastest).
 *
 * @param: themesh: the mesh to be drawn.
 */
void PrimitiveDrawer::draw_tstrips(const TriMesh *themesh)
{
	const int *t = &themesh->tstrips[0];
	const int *end = t + themesh->tstrips.size();
	while (likely(t < end)) { // as long as we haven't reached the TSTRIPS-end
		int striplen = *t++;
		glDrawElements(GL_TRIANGLE_STRIP, striplen, GL_UNSIGNED_INT, t);
		t += striplen;
	}
}
