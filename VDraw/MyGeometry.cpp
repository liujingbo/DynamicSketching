
#include "MyGeometry.h"

SymMatrix3d::SymMatrix3d()
{
	a = b = c = d = e = f = 0;
}

SymMatrix3d::SymMatrix3d(Vector3d v)
{
	double x = v.X();double y=v.Y(); double z=v.Z();
		a = x * x;
	b = x * y;
	c = x * z;
	d = y * y;
	e = y * z;
	f = z * z;
}

SymMatrix3d::SymMatrix3d(double x, double y, double z)
{
	a = x * x;
	b = x * y;
	c = x * z;
	d = y * y;
	e = y * z;
	f = z * z;
}

SymMatrix3d::SymMatrix3d(double a, double b, double c, double d, double e, double f)
{
	this->a = a;
	this->b = b;
	this->c = c;
	this->d = d;
	this->e = e;
	this->f = f;
}

void SymMatrix3d::Add(SymMatrix3d* other)
{
	this->a += other->a;
	this->b += other->b;
	this->c += other->c;
	this->d += other->d;
	this->e += other->e;
	this->f += other->f;
}

SymMatrix3d SymMatrix3d::operator + (SymMatrix3d other)
{
	return SymMatrix3d(this->a + other.a, this->b + other.b, this->c + other.c, this->d + other.d, this->e + other.e, this->f + other.f);
}

SymMatrix3d & SymMatrix3d::operator += (const SymMatrix3d &other){
	this->a += other.a; 
	this->b += other.b; 
	this->c += other.c; 
	this->d += other.d; 
	this->e += other.e; 
	this->f += other.f; 
	return *this;
}

SymMatrix3d SymMatrix3d::operator - (SymMatrix3d other)
{
	return SymMatrix3d(this->a - other.a, this->b - other.b, this->c - other.c, this->d - other.d, this->e - other.e, this->f - other.f);
}

SymMatrix3d SymMatrix3d::operator *(double other)
{
	return SymMatrix3d(this->a * other, this->b * other, this->c * other, this->d * other, this->e * other, this->f * other);
}

SymMatrix3d SymMatrix3d::operator / (double other)
{
	return SymMatrix3d(this->a / other, this->b / other, this->c / other, this->d / other, this->e / other, this->f / other);
}

Vector3d SymMatrix3d::operator *(Vector3d other)
{
	return Vector3d(this->a * other.X() + this->b * other.Y() + this->c * other.Z(), this->b * other.X() + this->d * other.Y() + this->e * other.Z(), this->c * other.X() + this->e * other.Y() + this->f * other.Z());
}
//Vector3d SymMatrix3d::GetEigenvalues()
//{
//	double a11 = this->a, a12 = this->b, a22 = this->d, a13 = this->c, a23 = this->e, a33 = this->f;
//	double c0 = (a11*a22*a33) + (2.0*a12*a13*a23) - (a11*a23*a23) - (a22*a13*a13) - (a33*a12*a12); // Mat. Determinant
//	double c1 = a11*a22 - a12*a12 + a11*a33 - a13*a13 + a22*a33 - a23*a23;
//	double c2 = a11 + a22 + a33;
//	double a = (3*c1 - c2*c2) / 3.0;
//	double b = (9*c1*c2 - 2*c2*c2*c2 - 27*c0) / 27.0;
//	double Q = ((b*b) / 4.0) + ((a*a*a) / 27.0);
//
//	if (Q > 1.0e-12) // Evecs = (1,0,0), (0,1,0), (0,0,1)
//	{
//		return Vector3d(a11, a11, a11);
//	}
//
//	double l1, l2, l3; // Eigenvalues to be computed
//
//	if (Q >= 0)
//	{
//		double p = (b > 0)?(pow(b / 2, 1.0 / 3.0)):(0);
//		l1 = l2 = ((c2 / 3.0) + p);
//		l3 = ((c2 / 3.0) - 2.0*p);
//	}
//	else
//	{
//		double t = atan2(sqrt(-Q), -b / 2.0) / 3.0, r = pow(((b*b) / 4.0) - Q, 1.0 / 6.0);
//		double cos_t = cos(t), sin_t = sin(t);
//		double sq3 = sqrt(3.0);
//		l1 = l2 = l3 = (c2 / 3.0);
//		l1 += (2*r*cos_t);
//		l2 -= r*(cos_t + sq3*sin_t);
//		l3 -= r*(cos_t - sq3*sin_t);
//	}
//
//	double L1 = 0, L2 = 0, L3 = 0;
//	if (l1 <= l2 && l1 <= l3)
//	{
//		L1 = l1;
//		L2 = (l2 < l3)?(l2):(l3);
//		L3 = (l2 < l3)?(l3):(l2);
//	}
//	else if (l2 <= l1 && l2 <= l3)
//	{
//		L1 = l2;
//		L2 = (l1 < l3)?(l1):(l3);
//		L3 = (l1 < l3)?(l3):(l1);
//	}
//	else
//	{
//		L1 = l3;
//		L2 = (l1 < l2)?(l1):(l2);
//		L3 = (l1 < l2)?(l2):(l1);
//	}
//	return Vector3d(L1, L2, L3);
//}
//
//Vector3d SymMatrix3d::GetMaxEigenvalues()
//{
//	double a11 = -this->a, a12 = -this->b, a22 = -this->d, a13 = -this->c, a23 = -this->e, a33 = -this->f;
//	double c0 = (a11 * a22 * a33) + (2.0 * a12 * a13 * a23) - (a11 * a23 * a23) - (a22 * a13 * a13) - (a33 * a12 * a12); // Mat. Determinant
//	double c1 = a11 * a22 - a12 * a12 + a11 * a33 - a13 * a13 + a22 * a33 - a23 * a23;
//	double c2 = a11 + a22 + a33;
//	double a = (3 * c1 - c2 * c2) / 3.0;
//	double b = (9 * c1 * c2 - 2 * c2 * c2 * c2 - 27 * c0) / 27.0;
//	double Q = ((b * b) / 4.0) + ((a * a * a) / 27.0);
//
//	if (Q > 1.0e-12) // Evecs = (1,0,0), (0,1,0), (0,0,1)
//	{
//		return Vector3d(a11, a11, a11);
//	}
//
//	double l1, l2, l3; // Eigenvalues to be computed
//
//	if (Q >= 0)
//	{
//		double p = (b > 0) ? (pow(b / 2, 1.0 / 3.0)) : (0);
//		l1 = l2 = ((c2 / 3.0) + p);
//		l3 = ((c2 / 3.0) - 2.0 * p);
//	}
//	else
//	{
//		double t = atan2(sqrt(-Q), -b / 2.0) / 3.0, r = pow(((b * b) / 4.0) - Q, 1.0 / 6.0);
//		double cos_t = cos(t), sin_t = sin(t);
//		double sq3 = sqrt(3.0);
//		l1 = l2 = l3 = (c2 / 3.0);
//		l1 += (2 * r * cos_t);
//		l2 -= r * (cos_t + sq3 * sin_t);
//		l3 -= r * (cos_t - sq3 * sin_t);
//	}
//
//	double L1 = 0, L2 = 0, L3 = 0;
//	if (l1 <= l2 && l1 <= l3)
//	{
//		L1 = l1;
//		L2 = (l2 < l3) ? (l2) : (l3);
//		L3 = (l2 < l3) ? (l3) : (l2);
//	}
//	else if (l2 <= l1 && l2 <= l3)
//	{
//		L1 = l2;
//		L2 = (l1 < l3) ? (l1) : (l3);
//		L3 = (l1 < l3) ? (l3) : (l1);
//	}
//	else
//	{
//		L1 = l3;
//		L2 = (l1 < l2) ? (l1) : (l2);
//		L3 = (l1 < l2) ? (l2) : (l1);
//	}
//	return Vector3d(L1, L2, L3);
//}
//
//Vector3d SymMatrix3d::GetMinEigenvector()
//{
//	double a11 = a, a12 = b, a22 = d, a13 = c, a23 = e, a33 = f;
//	double l, l1, l2, l3, c0, c1, c2;
//	Vector3d eig = GetEigenvalues();
//	l = eig.X();
//	l2 = eig.Y();
//	l3 = eig.Z();
//
//	if (l == l3 && l == l2)
//	{
//		return  Vector3d(1,0,0);
//	}
//
//	a11 -= l;
//	a22 -= l;
//	a33 -= l;
//	double u11 = a22*a33 - a23*a23, u12 = a13*a23 - a12*a33, u13 = a12*a23 - a13*a22;
//	double u22 = a11*a33 - a13*a13, u23 = a12*a13 - a23*a11, u33 = a11*a22 - a12*a12;
//	l1 = u11*u11 + u12*u12 + u13*u13;
//	l2 = u12*u12 + u22*u22 + u23*u23;
//	l3 = u13*u13 + u23*u23 + u33*u33;
//
//	if (l1 >= l2 && l1 >= l3)
//	{
//		c0 = u11;
//		c1 = u12;
//		c2 = u13;
//		l = l1;
//	}
//	else if (l2 >= l1 && l2 >= l3)
//	{
//		c0 = u12;
//		c1 = u22;
//		c2 = u23;
//		l = l2;
//	}
//	else
//	{
//		c0 = u13;
//		c1 = u23;
//		c2 = u33;
//		l = l3;
//	}
//
//	l = sqrt(l);
//	return Vector3d(c0 / l, c1 / l, c2 / l);
//}
//
//Vector3d SymMatrix3d::GetMaxEigenvector()
//{
//	double a11 = -a, a12 = -b, a22 = -d, a13 = -c, a23 = -e, a33 = -f;
//	double l, l1, l2, l3, c0, c1, c2;
//	Vector3d eig = this->GetMaxEigenvalues();
//	l = eig.X();
//	l2 = eig.Y();
//	l3 = eig.Z();
//
//	if (l == l3 && l == l2)
//	{
//		return Vector3d(1, 0, 0);
//	}
//
//	a11 -= l;
//	a22 -= l;
//	a33 -= l;
//	double u11 = a22 * a33 - a23 * a23, u12 = a13 * a23 - a12 * a33, u13 = a12 * a23 - a13 * a22;
//	double u22 = a11 * a33 - a13 * a13, u23 = a12 * a13 - a23 * a11, u33 = a11 * a22 - a12 * a12;
//	l1 = u11 * u11 + u12 * u12 + u13 * u13;
//	l2 = u12 * u12 + u22 * u22 + u23 * u23;
//	l3 = u13 * u13 + u23 * u23 + u33 * u33;
//
//	if (l1 >= l2 && l1 >= l3)
//	{
//		c0 = u11;
//		c1 = u12;
//		c2 = u13;
//		l = l1;
//	}
//	else if (l2 >= l1 && l2 >= l3)
//	{
//		c0 = u12;
//		c1 = u22;
//		c2 = u23;
//		l = l2;
//	}
//	else
//	{
//		c0 = u13;
//		c1 = u23;
//		c2 = u33;
//		l = l3;
//	}
//
//	l = sqrt(l);
//	return Vector3d(c0 / l, c1 / l, c2 / l);
//}

bool SymMatrix3d::Invert()
{
	double det, pos, neg, t;
	double ans[6];

	pos = neg = 0.0;
	t = a * d * f;
	if (t > 0)
		pos += t;
	else
		neg += t;
	t = b * e * c;
	if (t > 0)
		pos += t;
	else
		neg += t;
	t = c * b * e;
	if (t > 0)
		pos += t;
	else
		neg += t;
	t = -c * d * c;
	if (t > 0)
		pos += t;
	else
		neg += t;
	t = -b * b * f;
	if (t > 0)
		pos += t;
	else
		neg += t;
	t = -a * e * e;
	if (t > 0)
		pos += t;
	else
		neg += t;
	det = pos + neg;

	t = det / (pos - neg);
	if (abs(t) >= 1.0e-15)
	{
		ans[0] = (d * f - e * e) / det;
		ans[1] = -(b * f - e * c) / det;
		ans[2] = (a * f - c * c) / det;
		ans[3] = (b * e - d * c) / det;
		ans[4] = -(a * e - b * c) / det;
		ans[5] = (a * d - b * b) / det;
		a = ans[0];
		b = ans[1];
		d = ans[2];
		c = ans[3];
		e = ans[4];
		f = ans[5];
		return true;
	}

	return false;
}




SymMatrix4d::SymMatrix4d()
{
	a = b = c = d = e = f = g = h = i = j = 0;
}

SymMatrix4d::SymMatrix4d(Vector4d v){
	double x = v.x;double y = v.y;double z = v.z;double w = v.w;
	a = x * x;
	b = x * y;
	c = x * z;
	d = x * w;
	e = y * y;
	f = y * z;
	g = y * w;
	h = z * z;
	i = z * w;
	j = w * w;
}

SymMatrix4d::SymMatrix4d(double x, double y, double z, double w)
{
	a = x * x;
	b = x * y;
	c = x * z;
	d = x * w;
	e = y * y;
	f = y * z;
	g = y * w;
	h = z * z;
	i = z * w;
	j = w * w;
}

SymMatrix4d::SymMatrix4d(double a, double b, double c, double d, double e, double f, double g, double h, double i, double j)
{
	this->a = a;
	this->b = b;
	this->c = c;
	this->d = d;
	this->e = e;
	this->f = f;
	this->g = g;
	this->h = h;
	this->i = i;
	this->j = j;
}

void SymMatrix4d::Add(SymMatrix4d* other)
{
	this->a += other->a;
	this->b += other->b;
	this->c += other->c;
	this->d += other->d;
	this->e += other->e;
	this->f += other->f;
	this->g += other->g;
	this->h += other->h;
	this->i += other->i;
	this->j += other->j;
}

SymMatrix4d SymMatrix4d::operator + (SymMatrix4d other)
{
	return SymMatrix4d(this->a + other.a, this->b + other.b, this->c + other.c, this->d + other.d, this->e + other.e, this->f + other.f, this->g + other.g, this->h + other.h, this->i + other.i, this->j + other.j);
}

SymMatrix4d SymMatrix4d::operator *(double other)
{
	return SymMatrix4d(this->a * other, this->b * other, this->c * other, this->d * other, this->e * other, this->f * other, this->g * other, this->h * other, this->i * other, this->j * other);
}

Vector4d SymMatrix4d::operator *(Vector4d other)
{
	return Vector4d(this->a * other.x + this->b * other.y + this->c * other.z + this->d * other.w, this->b * other.x + this->e * other.y + this->f * other.z + this->g * other.w, this->c * other.x + this->f * other.y + this->h * other.z + this->i * other.w, this->d * other.x + this->g * other.y + this->i * other.z + this->j * other.w);
}

bool SymMatrix4d::Invert()
{
	if (this->a <= 0)
		return false;

	double d00 = 1.0 / this->a;
	double L10 = this->b;
	double l10 = this->b*d00;
	double L20 = this->c;
	double l20 = this->c*d00;
	double L30 = this->d;
	double l30 = this->d*d00;
	double d11 = this->e - (L10*l10);

	if (d11 <= 0)
		return false;
	else
		d11 = 1.0 / d11;

	double L21 = (this->f - (L10*l20));
	double l21 = L21*d11;
	double L31 = (this->g - (L10*l30));
	double l31 = L31*d11;
	double d22 = this->h - (L20*l20) - (L21*l21);

	if (d22 <= 0)
		return false;
	else
		d22 = 1.0 / d22;

	double L32 = (this->i - (L20*l30) - (L21*l31));
	double l32 = L32*d22;
	double d33 = this->j - (L30*l30) - (L31*l31) - (L32*l32);

	if (d33 <= 0)
		return false;
	else
		d33 = 1.0 / d33;

	L20 = l10*l21 - l20;
	L31 = l21*l32 - l31;
	L30 = l32*l20 - L31*l10 - l30;

	this->a = d00 + (-l10)*((-l10)*d11) + L20*(L20*d22) + L30*(L30*d33);
	this->b = ((-l10)*d11) + L20*((-l21)*d22) + L30*(L31*d33);
	this->e = d11 + (-l21)*((-l21)*d22) + L31*(L31*d33);
	this->c = (L20*d22) + L30*((-l32)*d33);
	this->f = ((-l21)*d22) + L31*((-l32)*d33);
	this->h = d22 + (-l32)*((-l32)*d33);
	this->d = (L30*d33);
	this->g = (L31*d33);
	this->i = ((-l32)*d33);
	this->j = d33;

	return true;
}


Vector4d::Vector4d()
{
	this->x = 0;
	this->y = 0;
	this->z = 0;
	this->w = 0;
}
Vector4d::Vector4d(double x, double y, double z, double w)
{
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;
}

Vector4d::Vector4d(Vector2d v)
{
	this->x = v.x;
	this->y = v.y;
	this->z = 0;
	this->w = 0;
}

Vector4d::Vector4d(Vector2d v, double z, double w)
{
	this->x = v.x;
	this->y = v.y;
	this->z = z;
	this->w = w;
}

Vector4d::Vector4d(Vector3d v)
{
	this->x = v.X();
	this->y = v.Y();
	this->z = v.Z();
	this->w = 0;
}

Vector4d::Vector4d(Vector3d v, double w)
{
	this->x = v.X();
	this->y = v.Y();
	this->z = v.Z();
	this->w = w;
}

Vector4d::Vector4d(double arr[], int index)
{
	x = arr[index];
	y = arr[index + 1];
	z = arr[index + 2];
	w = arr[index + 3];
}

double Vector4d::Dot(Vector4d v)
{
	return x * v.x + y * v.y + z * v.z + w * v.w;
}

double Vector4d::Length()
{
	return sqrt(x * x + y * y + z * z + w * w);
}

Vector4d Vector4d::Normalize()
{
	return *this / this->Length();
}

//Matrix4d Vector4d::OuterCross(Vector4d v)
//{
//	Matrix4d m;
//	m[0][0] = x * v.x;
//	m[0][1] = x * v.y;
//	m[0][2] = x * v.z;
//	m[0][3] = x * v.w;
//	m[1][0] = y * v.x;
//	m[1][1] = y * v.y;
//	m[1][2] = y * v.z;
//	m[1][3] = y * v.w;
//	m[2][0] = z * v.x;
//	m[2][1] = z * v.y;
//	m[2][2] = z * v.z;
//	m[2][3] = z * v.w;
//	m[3][0] = w * v.x;
//	m[3][1] = w * v.y;
//	m[3][2] = w * v.z;
//	m[3][3] = w * v.w;
//	return m;
//}

Vector3d Vector4d::XYZ()
{
	return Vector3d(x, y, z);
}

//std::string Vector4d::ToString()
//{
//	return StringConverterHelper::toString(x) + " " + StringConverterHelper::toString(y) + " " + StringConverterHelper::toString(z) + " " + StringConverterHelper::toString(w);
//}

Vector4d Vector4d::Max(Vector4d v1, Vector4d v2)
{
	return Vector4d((v1.x > v2.x) ? v1.x : v2.x, (v1.y > v2.y) ? v1.y : v2.y, (v1.z > v2.z) ? v1.z : v2.z, (v1.w > v2.w) ? v1.w : v2.w);
}

Vector4d Vector4d::Min(Vector4d v1, Vector4d v2)
{
	return Vector4d((v1.x < v2.x) ? v1.x : v2.x, (v1.y < v2.y) ? v1.y : v2.y, (v1.z < v2.z) ? v1.z : v2.z, (v1.w < v2.w) ? v1.w : v2.w);
}

Vector4d Vector4d::operator + (Vector4d v2)
{
	return Vector4d(this->x + v2.x, this->y + v2.y, this->z + v2.z, this->w + v2.w);
}

Vector4d Vector4d::operator - (Vector4d v2)
{
	return Vector4d(this->x - v2.x, this->y - v2.y, this->z - v2.z, this->w - v2.w);
}

Vector4d Vector4d::operator *(double s)
{
	return Vector4d(this->x * s, this->y * s, this->z * s, this->w * s);
}

Vector4d Vector4d::operator / (double s)
{
	return Vector4d(this->x / s, this->y / s, this->z / s, this->w / s);
}


Vector2d Vector2d::MinValue = Vector2d(-DBL_MAX, -DBL_MAX);
Vector2d Vector2d::MaxValue = Vector2d(DBL_MAX, DBL_MAX);

Vector2d::Vector2d(double x, double y)
{
	this->x = x;
	this->y = y;
}

Vector2d::Vector2d(double arr[], int index)
{
	x = arr[index];
	y = arr[index + 1];
}

double& Vector2d::operator [](int index)
{
	if (index == 0)
		return x;
	if (index == 1)
		return y;
	exit(1);
	return y;
	//throw ArgumentException();
}

//C# TO C++ CONVERTER TODO TASK: You cannot specify separate 'set' logic for indexers in native C++:
//void Vector2d::setdefault(const int& index, double value)
//{
//	if (index==0)
//		x = value;
//	if (index==1)
//		y = value;
//}

double Vector2d::Dot(Vector2d v)
{
	return x*v.x + y*v.y;
}

double Vector2d::Length()
{
	return sqrt(x*x + y*y);
}

Vector2d Vector2d::Normalize()
{
	return *this / this->Length();
}

//std::string Vector2d::ToString()
//{
//	return StringConverterHelper::toString(x) + " " + StringConverterHelper::toString(y);
//}

Vector2d Vector2d::Max(Vector2d v1, Vector2d v2)
{
	return Vector2d((v1.x > v2.x) ? v1.x : v2.x, (v1.y > v2.y) ? v1.y : v2.y);
}

Vector2d Vector2d::Min(Vector2d v1, Vector2d v2)
{
	return Vector2d((v1.x < v2.x) ? v1.x : v2.x, (v1.y < v2.y) ? v1.y : v2.y);
}

bool Vector2d::Equals(Vector2d v)
{
	return (this->x == v.x) && (this->y == v.y) ? true : false;
}

Vector2d Vector2d::operator + (Vector2d v2)
{
	return Vector2d(this->x + v2.x, this->y + v2.y);
}

Vector2d Vector2d::operator - (Vector2d v2)
{
	return Vector2d(this->x - v2.x, this->y - v2.y);
}

Vector2d Vector2d::operator *(double s)
{
	return Vector2d(this->x*s, this->y*s);
}

Vector2d Vector2d::operator / (double s)
{
	return Vector2d(this->x / s, this->y / s);
}