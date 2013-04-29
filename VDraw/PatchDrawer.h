
#ifndef PATCHDRAWER_H_
#define PATCHDRAWER_H_

#include "sc_cpu/Drawer.h"
#include "TriMesh.h"
#include "fitting\FittingCylinder.h"
#include <math.h>
//#include <time.h>// #include <dos.h> // for sleep  #include "windows.h"//
#include "..\ordering\HierarchyGraph.h"
#include <list>

class PatchDrawer: public Drawer
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
	PatchDrawer();
	virtual void init();
	virtual void draw(Model* m, vec camera_position);
	void SetVertexColored(bool iscolored_);
	const bool GetVertexColored();
	void SetFittingCylinderReady(bool isReady_);
	const bool GetFittingCylinderReady();
	void SetColoredPatchesDisplay(bool isDisplay_){isColoredPatchesDisplay = isDisplay_;}
	const bool GetColoredPatchesDisplay(){ return this->isColoredPatchesDisplay;}
	void SetPrimitivesDisplay(bool isDisplay_){ this->isPrimitivesDisplay = isDisplay_;}
	const bool GetPrimitivesDisplay(){ return this->isPrimitivesDisplay;}
	bool toggleCylinder();
	bool PrevStep();
	bool NextStep();
};

#endif /* PATCHDRAWER_H_ */

