#ifndef PRIMITIVES_h_20130103
#define PRIMITIVES_h_20130103

#include <Vec.h>

class Primitives{

public:
	enum PRIMITIVE_TYPE{ PRISM, CONE, UNKNOWN};

	PRIMITIVE_TYPE type;
	Primitives(){type = UNKNOWN;}
	Primitives(PRIMITIVE_TYPE _t){type = _t;}
	~Primitives(){}

};


class Prism:public Primitives{

public:
	Prism():Primitives(PRISM){}
	~Prism(){}

	double planes[6][4];
	vec points[8];

	void addPlanes(double _planes[][4]){
		for(int i=0;i<6;i++){
			for(int j=0;j<4;j++){
				planes[i][j] = _planes[i][j];
			}
		}
		return;
	}

	void addPoints(vec* _points){
		for(int i=0;i<8;i++){
			points[i] = _points[i];
		}
	}

};
#endif