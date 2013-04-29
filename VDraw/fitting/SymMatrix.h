	class SymMatrix4d
	{
	public:
		double a, b, c, d, e, f, g, h, i, j;

		SymMatrix4d();
//C# TO C++ CONVERTER TODO TASK: Calls to same-class constructors are not supported in C++ prior to C++11:
//ORIGINAL LINE: public SymMatrix4d(Vector4d v) : this(v.x, v.y, v.z, v.w)
		SymMatrix4d(Vector4d* v);
		SymMatrix4d(double x, double y, double z, double w);
		SymMatrix4d(double a, double b, double c, double d, double e, double f, double g, double h, double i, double j);

		void Add(SymMatrix4d* other);

		SymMatrix4d* operator + (SymMatrix4d* other);
		SymMatrix4d* operator *(double other);
		Vector4d* operator *(Vector4d* other);
		bool Invert();


	};