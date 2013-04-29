
#ifndef PRIMITIVEDRAWER_H_
#define PRIMITIVEDRAWER_H_

#include "sc_cpu/Drawer.h"
#include "TriMesh.h"
#include "fitting\FittingCylinder.h"
#include <math.h>
//#include <time.h>// #include <dos.h> // for sleep  #include "windows.h"//
#include "..\ordering\HierarchyGraph.h"
#include <list>

class PrimitiveDrawer: public Drawer
{
private:
	bool isVertexColored;
	bool isFittingCylinderReady;
	bool isColoredPatchesDisplay;
	bool isPrimitivesDisplay;
	int numOfDisplayPatch;
private:
	void draw_tstrips(const TriMesh* mesh);
public:
	PrimitiveDrawer();
	virtual void init();
	virtual void draw(Model* m, vec camera_position);


	void SetVertexColored(bool iscolored_){this->isVertexColored = iscolored_;}
	const bool GetVertexColored(){return this->isVertexColored;}
	void SetFittingCylinderReady(bool isReady_){this->isFittingCylinderReady = isReady_;}
	const bool GetFittingCylinderReady(){return this->isFittingCylinderReady;}
	void SetColoredPatchesDisplay(bool isDisplay_){isColoredPatchesDisplay = isDisplay_;}
	const bool GetColoredPatchesDisplay(){ return this->isColoredPatchesDisplay;}
	void SetPrimitivesDisplay(bool isDisplay_){ this->isPrimitivesDisplay = isDisplay_;}
	const bool GetPrimitivesDisplay(){ return this->isPrimitivesDisplay;}
	bool toggleCylinder(){isFittingCylinderReady = !isFittingCylinderReady;return isFittingCylinderReady;}
	bool PrevStep(){
	this->numOfDisplayPatch --;return true;}
	bool NextStep(){this->numOfDisplayPatch++;return true;}
};

#endif /* PRIMITIVEDRAWER_H_ */

