#ifndef MYGEOMETRY_H_201208031203
#define MYGEOMETRY_H_201208031203 

#include <cmath>
#include "vector3.h"


#include <string>
#include <cmath>
// #include "stringconverter.h"
class Vector2d
	{
	public:
		static Vector2d MinValue;
		static Vector2d MaxValue;

		double x, y;

		Vector2d(double x, double y);

		Vector2d(double arr[], int index);


		double& operator [](int index);
		double Dot(Vector2d v);
		double Length();

		Vector2d Normalize();

		// virtual std::string ToString();


		static Vector2d Max(Vector2d v1, Vector2d v2);
		static Vector2d Min(Vector2d v1, Vector2d v2);
		bool Equals(Vector2d v);

		Vector2d operator + (Vector2d v2);
		Vector2d operator - (Vector2d v2);
		Vector2d operator *(double s);
		Vector2d operator *(Vector2d v);
		Vector2d operator / (double s);

		//static public bool operator == (Vector2d v1, Vector2d v2)
		//{
		//    return (v1.x==v2.x) && (v1.y==v2.y);
		//}
		//static public bool operator !=(Vector2d v1, Vector2d v2)
		//{
		//    return !(v1 == v2);
		//}
	};

class Vector4d
{
	public:
		double x, y, z, w;

		Vector4d();
		Vector4d(double x, double y, double z, double w);
		Vector4d(Vector2d v);
		Vector4d(Vector2d v, double z, double w);
		Vector4d(Vector3d v);
		Vector4d(Vector3d v, double w);

		Vector4d(double arr[], int index);


		double Dot(Vector4d v);
		double Length();

		Vector4d Normalize();
		//Matrix4d OuterCross(Vector4d v);

		Vector3d XYZ();

		// virtual std::string ToString();


		static Vector4d Max(Vector4d v1, Vector4d v2);
		static Vector4d Min(Vector4d v1, Vector4d v2);

		Vector4d operator + (Vector4d v2);
		Vector4d operator - (Vector4d v2);
		Vector4d operator *(double s);
		Vector4d operator *(Vector4d v);
		Vector4d operator / (double s);
};



class SymMatrix3d
{
public:
	double a, b, c, d, e, f;

	SymMatrix3d();
	//C# TO C++ CONVERTER TODO TASK: Calls to same-class constructors are not supported in C++ prior to C++11:
	//ORIGINAL LINE: public SymMatrix3d(Vector3d v) : this(v.x, v.y, v.z)
	SymMatrix3d(Vector3d v);
	SymMatrix3d(double x, double y, double z);
	SymMatrix3d(double a, double b, double c, double d, double e, double f);

	void Add(SymMatrix3d* other);
	SymMatrix3d operator + (SymMatrix3d other);
	SymMatrix3d & operator += (const SymMatrix3d &other);
	SymMatrix3d operator - (SymMatrix3d other);
	SymMatrix3d operator *(double other);
	SymMatrix3d operator / (double other);
	Vector3d operator *(Vector3d other);
	//Vector3d GetEigenvalues(); // not stable, use eig3 instead
	//Vector3d GetMaxEigenvalues();
	//Vector3d GetMinEigenvector();
	//Vector3d GetMaxEigenvector();
	bool Invert();
};



class SymMatrix4d
{
public:
	double a, b, c, d, e, f, g, h, i, j;

	SymMatrix4d();
	//C# TO C++ CONVERTER TODO TASK: Calls to same-class constructors are not supported in C++ prior to C++11:
	//ORIGINAL LINE: public SymMatrix4d(Vector4d v) : this(v.x, v.y, v.z, v.w)
	SymMatrix4d(Vector4d v);
	SymMatrix4d(double x, double y, double z, double w);
	SymMatrix4d(double a, double b, double c, double d, double e, double f, double g, double h, double i, double j);

	void Add(SymMatrix4d* other);

	SymMatrix4d operator + (SymMatrix4d other);
	SymMatrix4d operator *(double other);
	Vector4d operator *(Vector4d other);
	bool Invert();


};

#endif