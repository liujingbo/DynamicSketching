
#include <algorithm>
#include <map>
#include <vector>
#include <queue>
#include <float.h>
#include <cmath>
#include <list>
#include <unordered_set>
#include <limits>



#ifndef FITTINGCYLINDER_H_
#define FITTINGCYLINDER_H_
#include <assert.h>

//#include "stringconverter.h"
#include "MyGeometry.h"
//#include "ProxyFitter.h"
#include "../math/eig3.h"

#include <iostream>
#include <fstream>

#include "../sc_cpu/Model.h"
// for each give segment, output cylinder parameters {axis, radias, center}

class FittingPrimitive{
private:
	double cost;
public:
	class EdgeRecord
	{
	public:
		int index1;
		int index2;

		EdgeRecord(int e1, int e2){
			// make sure e1 < e2
			if (e1 > e2)
			{
				int tmp = e1;
				e1 = e2;
				e2 = tmp;
			}
			index1 = e1;
			index2 = e2;
		}
		const bool Equals(const EdgeRecord rec) const{
			return index1 == rec.index1 && index2 == rec.index2;
		}
		const int GetHashCode()const{
			return index1 + index2;
		}
		inline bool operator==(const EdgeRecord rec) const{ return (this->index1 == rec.index1) && (this->index2 == rec.index2); }
	};
	struct EdgeRecordHashFunction {
		size_t operator()(const EdgeRecord& er )  const { return er.GetHashCode(); }
	};
	struct EdgeRecordEqualFunction {
		bool operator()(const EdgeRecord& a, const EdgeRecord& b)  const { return a.Equals(b); }
	};
	class DualGraphNode //: public PriorityQueueElement
	{
	public:
		int id; // unique id of the node
		vector<int> vertices; // all the vertices within the cluster
		vector<int> faces; // all the triangles within the cluster
		std::unordered_set<EdgeRecord, EdgeRecordHashFunction,EdgeRecordEqualFunction> edges; // public HashSet<EdgeRecord> edges;
		Vector3d center; // weighted sum of barycenters;
		SymMatrix3d cov_v; // covariance matrix of cluster vertices
		SymMatrix3d cov_c; // covariance matrix of normal variation
		SymMatrix4d ATA; // for sphere fitting
		Vector4d ATb; // for sphere fitting
		double totArea; // total area of the cluster
		//double convexHullVolume;
		//std::set<DualGraphNode*> adjNodes; // public HashSet<DualGraphNode> adjNodes = new HashSet<DualGraphNode>();
		//DualGraphNode* collapseTo;
		//double collapseCost;
		//PrimitiveType collapseType;
		//PrimitiveTreeNode* treeNode;
		//int pqIndex;


		//output 
		Vector3d cylinder_axis;
		Vector3d cylinder_center;
		double cylinder_radias;
		double cylinder_height;

		// constructor
		DualGraphNode()
		{
			InitializeInstanceFields();
		}

		void expandByFaces(Model* mesh, vector<int>& faceindexlist);

	private:
		void InitializeInstanceFields(){
			//convexHullVolume = 0;
			//adjNodes = std::set<DualGraphNode*>();
			//delete collapseTo;
			//collapseCost = DBL_MAX;
			//collapseType = PrimitiveType::UNKNOWN;
			//delete treeNode;
			this->faces.clear();
			this->vertices.clear();
			this->edges.clear();
			totArea = 0;
		}
	};
	enum PrimitiveType{UNKNOWN,PLANE,SPHERE,CYLINDER};

	// ****************************************
	//              mesh & segment
	// ****************************************
	DualGraphNode node;
	Model* m;
	PrimitiveType type;

	FittingPrimitive(){
		init();
	}
	FittingPrimitive(Model* mesh, vector<int> * faceindexlist);

	void init(){
		type = UNKNOWN;
		cost = 0;
	}
	bool FitPrimitive(){
	
	}

	// ****************************************
	//              cylinder
	// ****************************************
	Vector3d cylinder_axis;
	double cylinder_radius;
	Vector3d cylinder_center;
	double cylinder_height;
	FittingPrimitive (Vector3d axis_, Vector3d center_, double radius_, double height_){
		type = CYLINDER;
		cylinder_axis = axis_;
		cylinder_center=center_;
		cylinder_radius=radius_;
		cylinder_height=height_;
	}
	double FitCylinder()
		{

			double totArea = this->node.totArea;
			Vector3d center = this->node.center / totArea;
			SymMatrix3d cov_c = this->node.cov_c;

			double symM[3][3];
			symM[0][0]=cov_c.a;
			symM[0][1]=symM[1][0]=cov_c.b;
			symM[0][2]=symM[2][0]=cov_c.c;
			symM[1][1]=cov_c.d;
			symM[1][2]=symM[2][1]=cov_c.e;
			symM[2][2]=cov_c.f;

			double V[3][3];
			double d[3];

			eigen_decomposition(symM, V, d);

			//Vector3d axis = cov_c.GetMaxEigenvector().Normalize();
			//Vector3d ce1 = cov_c.GetMinEigenvector().Normalize();
			//Vector3d ce2 = axis.Cross(ce1).Normalize(); // type overflow
			Vector3d ce2(V[0][0],V[0][1],V[0][2]); 
			Vector3d ce1(V[1][0],V[1][1],V[1][2]);
			Vector3d axis(V[2][0],V[2][1],V[2][2]);

			this->cylinder_axis = axis;
			

			SymMatrix3d ATA ;
			Vector3d ATb ;

			assert(this->m->mesh_->faces.size() == this->m->faceareas_.size() );
			assert(this->m->mesh_->faces.size() == this->m->facecenters_.size());
				

			for(int i=0; i< this->node.faces.size();i++)
			{ 
				int index = this->node.faces[i];
			//foreach (int index in n1.faces)
			//{
				double area = m->faceareas_[index];// faceArea[index];
				Vector3d v = Vector3d( m->facecenters_[index][0],m->facecenters_[index][1],m->facecenters_[index][2]) - center;
				double b1 = v.Dot(ce1);
				double b2 = v.Dot(ce2);
				double w2 = area * area;
				ATA.a += b1 * b1 * w2; ATA.b += b1 * b2 * w2; ATA.c += b1 * w2;
				ATA.d += b2 * b2 * w2; ATA.e += b2 * w2;
				ATA.f += w2;
				double xy2 = (b1 * b1 + b2 * b2) * w2;
				ATb[0] += b1 * xy2;
				ATb[1] += b2 * xy2;
				ATb[2] += xy2;
			}
			ATA.a *= 4; ATA.b *= 4; ATA.c *= 2;
			ATA.d *= 4; ATA.e *= 2;
			ATb[0] *= 2; ATb[1] *= 2;

			if (!ATA.Invert()) return numeric_limits<double>::max( );
			Vector3d ans = ATA * ATb;
			double x0 = ans[0];
			double y0 = ans[1];
			double r2 = ans[2] + x0 * x0 + y0 * y0;
			assert (r2 > 0) ;
			double radius = sqrt(r2);
			this->cylinder_radius = radius;
			//if (totArea / radius < 1.0e-9) return double.MaxValue;
			

			Vector3d cylCenter = center + ce1 * x0 + ce2 * y0;
			this->cylinder_center =cylCenter;
			
			double cost = 0;
			double top = -numeric_limits<double>::max( );
			double bottom = numeric_limits<double>::max( );
			for(int i=0; i< this->node.faces.size();i++)
			{ 
				int index = this->node.faces[i];

				double area = m->faceareas_[index];
				double d = (Vector3d( m->facecenters_[index][0],m->facecenters_[index][1],m->facecenters_[index][2]) - cylCenter).Cross(axis).L2Norm() - radius;
				cost += d * d * area;
				double face_height = (Vector3d( m->facecenters_[index][0],m->facecenters_[index][1],m->facecenters_[index][2]) - cylCenter).Dot(axis);
				//get height
				if(face_height >top) top = face_height;
				if(face_height <bottom ) bottom = face_height;
				
				/*Vector3d proj =  face_height * axis + cylCenter;
				Vector3d e = Vector3d( m->facenormals_[index][0],m->facenormals_[index][1],m->facenormals_[index][2]) - (Vector3d( m->facecenters_[index][0],m->facecenters_[index][1],m->facecenters_[index][2]) - proj).Normalize();
				double d = e.Dot(e);
				cost += d * area;*/
			}

			this->cylinder_height =top - bottom ;

			OutputCylinders();

			this->cost = cost/totArea;
			return cost/totArea;
		}
	void OutputCylinders(){
		ofstream myfile ("..\\testdata\\cylinders\\cylinders_396_4_v99.cyl", ios::out | ios::app | ios::binary);
		if (myfile.is_open())
		{
			myfile << "#type cylinder\n";
			myfile << "#axis\n";
			myfile << this->cylinder_axis<<endl;
			myfile << "#center\n";
			myfile << this->cylinder_center<<endl;
			myfile << "#radius\n";
			myfile << this->cylinder_radius<<endl;
			myfile << "#height\n";
			myfile << this->cylinder_height<<endl;

			myfile.close();
		}
		else cout << "Unable to open file";
	}

	// ****************************************
	//              sphere
	// ****************************************
	Vector3d sphere_center;
	double sphere_radius;
	FittingPrimitive(Vector3d _center, double _radius){
		type = SPHERE;
		this->sphere_center = _center;
		this->sphere_radius = _radius;
	}
	double FitSphere(){
		double totArea = this->node.totArea;
		SymMatrix4d ATA = this->node.ATA;
		Vector4d ATb = this->node.ATb;

		if (ATA.Invert() == false)
		{
			return DBL_MAX;
		}
		Vector4d sol = ATA * ATb;
		Vector3d center(sol.x, sol.y, sol.z);
		double r2 = sol.w + center.Dot(center);
		if (r2 < 0)
		{
			return DBL_MAX;
		}
		double radius = sqrt(r2);
		this->sphere_center = center;
		this->sphere_radius = radius;

		//if (totArea / radius < 1.0e-9) return double.MaxValue;

		double cost = 0;

		for(int i=0; i< this->node.faces.size();i++){
			int index = this->node.faces[i];
			double area = m->faceareas_[index];
			Vector3d e = Vector3d(m->facenormals_[index][0],m->facenormals_[index][1],m->facenormals_[index][2])- (Vector3d( m->facecenters_[index][0],m->facecenters_[index][1],m->facecenters_[index][2])-center).Normalize();
			double d=e.Dot(e);
			cost += d*area;
		}

		return cost;
	}

};

class FittingCylinder: public FittingPrimitive{

	// constructor
public: 

	FittingCylinder (Model* mesh, vector<int> * faceindexlist);

	FittingCylinder (Vector3d axis_, Vector3d center_, double radius_, double height_){
		cylinder_axis = axis_;
		cylinder_center=center_;
		cylinder_radius=radius_;
		cylinder_height=height_;
	}

};

class FittingSphere:public FittingPrimitive{

	public:

	FittingSphere(Model* mesh, vector<int> * faceindexlist);
	FittingSphere(Vector3d _center, double _radius)
	{
		this->sphere_center = _center;
		this->sphere_radius = _radius;
	}

	//double FitSphere(){
	//	double totArea = this->node.totArea;
	//	SymMatrix4d ATA = this->node.ATA;
	//	Vector4d ATb = this->node.ATb;

	//	if (ATA.Invert() == false)
	//	{
	//		return DBL_MAX;
	//	}
	//	Vector4d sol = ATA * ATb;
	//	Vector3d center(sol.x, sol.y, sol.z);
	//	double r2 = sol.w + center.Dot(center);
	//	if (r2 < 0)
	//	{
	//		return DBL_MAX;
	//	}
	//	double radius = sqrt(r2);
	//	this->sphere_center = center;
	//	this->sphere_radius = radius;

	//	//if (totArea / radius < 1.0e-9) return double.MaxValue;

	//	double cost = 0;

	//	for(int i=0; i< this->node.faces.size();i++){
	//		int index = this->node.faces[i];
	//		double area = m->faceareas_[index];
	//		Vector3d e = Vector3d(m->facenormals_[index][0],m->facenormals_[index][1],m->facenormals_[index][2])- (Vector3d( m->facecenters_[index][0],m->facecenters_[index][1],m->facecenters_[index][2])-center).Normalize();
	//		double d=e.Dot(e);
	//		cost += d*area;
	//	}

	//	return cost;
	//}

};

#endif