#ifndef VCURVE_20121031
#define VCURVE_20121031


#include <vector>
#include <assert.h>
#include <opencv/cv.h>
#include <opencv/cxcore.h>
#include <opencv/highgui.h>
#include "../include/Vec.h"

#include <iostream>
#include <fstream>

#include <math.h>
/*
image domain point (x, y)
*/
class vPoint2D{
private:
	int v[2];
	int unprojection; // index of the vertex from the mesh

	double t[2]; // tangent
	double c[2]; // concavity
public:
	
	// for rendering
	double thickness;
	double intensity;
	double velocity;

	vPoint2D(){v[0]=v[1]=0;init();}
	vPoint2D(int x, int y){v[0]=x;v[1]=y;init();}
	vPoint2D(int x, int y, int ind3d){init(); v[0]=x;v[1]=y; unprojection = ind3d;}
	vPoint2D(int x, int y, int seg, bool onsurface){init(); v[0]=x;v[1]=y; segment = seg;}
	void init();
	const int x(){return v[0];}
	const int y(){return v[1];}
	void setX(int _in){v[0] = _in;}
	void setY(int _in){v[1] = _in;}
	void setPosition(int _in1, int _in2){v[0] = _in1;v[1] = _in2;}
	void setTangent(const double in[2]){t[0] = in[0]; t[1]=in[1];}
	void setTangent(double in1, double in2){t[0] = in1; t[1]=in2;}
	void setConcavity(const double in[2]){c[0] = in[0]; c[1]=in[1];}
	void setConcavity(double in1, double in2){c[0] = in1; c[1]=in2;}
	const double (&tangent())[2]{return t;}
	const double (&concavity())[2]{return c;}

	double color[3]; // color of the point [0,255] => will be changed 
	void setColor(double in1, double in2, double in3){color[0]=in1; color[1]=in2; color[2] = in3;}
	vec2 toVec2(){ return vec2((float)v[0],(float)v[1]);}

	int segment; // the segmentation the point belongs to
	int get3DIndex(){return this->unprojection;}
};

/*
object domain point (x, y, z, w)
*/
class vPoint3D{
public:
	vec v;
	vec normal;
	vec curvature;
	vPoint2D projection;
	int segment;

	vPoint3D(){init();}
	vPoint3D(vec _v, int _seg){init(); v=_v;segment=_seg; }
	~vPoint3D(){}
	void init(){ segment =-1;}
};

/*
Data structure to hold the information about curves
extracted from 2D images & 3D mesh
*/
class vCurve{
	//enum curve_type{};
private:
	bool tangentReady;
	bool concavityReady;
	float defaultHeight;
	IplImage* texture;
public:
	vCurve* spline;

public:
	enum CurveType{OPEN, CLOSE, STRAIGHT, UNKNOWN };
	CurveType type;
	bool isInner; // to distinguish sc(true) and extended edges(false)
	std::vector<vPoint2D> curve2d; // 2D straight lines, ellipses from Houghtransform detection
	double length;
	static double fading_factor;

	vCurve(){
		iniMembers();
	}
	vCurve(std::vector<vPoint2D>& _inPts){
		curve2d = _inPts;
		iniMembers();
	}
	~vCurve();
	void iniMembers();

	// basic
	double Length(){
		double relen=0;
		for(int i=0;i<this->curve2d.size()-1;i++){
			vec2 p1 = this->curve2d[i].toVec2();
			vec2 p2 = this->curve2d[i+1].toVec2();
			relen += dist(p1,p2);
		}
		this->length = relen;

		return relen;
	}
	vec2 center();

	void addPoint(vPoint2D p){ curve2d.push_back(p);}
	const int size(){return curve2d.size(); }
	vPoint2D at(int ind){ // be careful when using this
		//assert(ind>=0 && ind< this->size());
		if(ind<0)
			return curve2d.front();
		else if(ind>=curve2d.size() )
			return curve2d.back();
		return curve2d[ind];
	}
	void computeTangentAndConcavity(){
		if (this->size()<2) {}
		else if(this->size() == 2){
			double t1 = curve2d[1].x() - curve2d[0].x();
			double t2 = curve2d[1].y() - curve2d[0].y();
			double t = sqrt(t1*t1+t2*t2);
			this->curve2d[0].setTangent( t1/t, t2/t );
			this->curve2d[1].setTangent( t1/t, t2/t );
		}
		else{

			// optional & can be improved: smooth the tangents 
			// TODO: may use LoG instead of all the following
			std::vector<vPoint2D> smoothed;
			// first one
			vPoint2D p0(this->curve2d[0].x(), this->curve2d[0].y());
			smoothed.push_back(p0);
			// second
			int a = 0.25* curve2d[0].x() + 0.25* curve2d[2].x() +0.5* curve2d[1].x();
			int b = 0.25* curve2d[0].y() + 0.25* curve2d[2].y() +0.5* curve2d[1].y();
			vPoint2D p1(a,b);
			smoothed.push_back(p1);
			if(curve2d.size()>=5){
				for(int i=2;i<this->size()-2;i++){
					a = 6.0/16*curve2d[i].x() + 4.0/16*curve2d[i-1].x() + 4.0/16*curve2d[i+1].x() + 1.0/16*curve2d[i-2].x()+ 1.0/16*curve2d[i+2].x();
					b = 6.0/16*curve2d[i].y() + 4.0/16*curve2d[i-1].y() + 4.0/16*curve2d[i+1].y() + 1.0/16*curve2d[i-2].y()+ 1.0/16*curve2d[i+2].y();
					vPoint2D p(a,b);
					smoothed.push_back(p);
				}
			}
			if(curve2d.size()>=4){ // the last second
				int i=this->curve2d.size()-2;
				a = 0.25* curve2d[i-1].x() + 0.25* curve2d[i+1].x() +0.5* curve2d[i].x();
				b = 0.25* curve2d[i-1].y() + 0.25* curve2d[i+1].y() +0.5* curve2d[i].y();
				vPoint2D p2(a,b);
				smoothed.push_back(p2);
			}
			// the last one
			vPoint2D p3(this->curve2d[curve2d.size()-1].x(), this->curve2d[curve2d.size()-1].y());
			smoothed.push_back(p3);

			assert(smoothed.size() == curve2d.size());

			double t1,t2,t,c1,c2,c;

			// tangent
			for(int i=1; i< this->size()-1; i++){
				// tangent
				t1 = smoothed[i+1].x() - smoothed[i-1].x();
				t2 = smoothed[i+1].y() - smoothed[i-1].y();
				t = sqrt(t1*t1+t2*t2);
				this->curve2d[i].setTangent( t1/t, t2/t );
			}
			// take care of the first one
			this->curve2d[0].setTangent( this->curve2d[1].tangent() );
			// take care of the last one
			curve2d[this->size()-1].setTangent(curve2d[this->size()-2].tangent());

			// concavity
			for(int i=1; i< this->size()-1; i++){
				//concavity
				c1 = curve2d[i+1].tangent()[0] - curve2d[i-1].tangent()[0];
				c2 = curve2d[i+1].tangent()[1] - curve2d[i-1].tangent()[1];
				this->curve2d[i].setConcavity(c1, c2);
			}
			this->curve2d[0].setConcavity( this->curve2d[1].concavity() );
			this->curve2d[this->size()-1].setConcavity( this->curve2d[this->size()-2].concavity() );
		}
		this->tangentReady = true;
		this->concavityReady = true;
	}
	void computeLoG(int gkernelsize=5, int lkernelsize=5){
		if (this->size()<2) {}
		else if(this->size() == 2){
			double t1 = curve2d[1].x() - curve2d[0].x();
			double t2 = curve2d[1].y() - curve2d[0].y();
			double t = sqrt(t1*t1+t2*t2);
			this->curve2d[0].setTangent( t1/t, t2/t );
			this->curve2d[1].setTangent( t1/t, t2/t );
		}
		else{
			CvMat* xCord0 = cvCreateMat( 1, curve2d.size(), CV_32FC1 );
			CvMat* yCord0 = cvCreateMat( 1, curve2d.size(), CV_32FC1 );
			for(int i=0; i<curve2d.size(); i++){
				cvmSet(xCord0,0,i,curve2d[i].x());
				cvmSet(yCord0,0,i,curve2d[i].y());
			}
			cv::Mat xCord(xCord0);
			cv::Mat yCord(yCord0);

			for(int i=0;i<20;i++){
				cv::GaussianBlur( xCord, xCord,cv::Size(gkernelsize,gkernelsize), 0, 0, cv::BORDER_DEFAULT );
				cv::GaussianBlur( yCord, yCord,cv::Size(gkernelsize,gkernelsize), 0, 0, cv::BORDER_DEFAULT );
			}
			/*for(int i=0; i< xCord.cols; i++){
				curve2d[i].setPosition(xCord.at<float>(0,i),yCord.at<float>(0,i));
			}*/

			cv::Mat dstx,dsty;

			cv::Laplacian( xCord, dstx, xCord.depth(), lkernelsize, 1, 0, cv::BORDER_DEFAULT );
			cv::Laplacian( yCord, dsty, yCord.depth(), lkernelsize, 1, 0, cv::BORDER_DEFAULT );

			assert(dsty.cols == this->curve2d.size() && dsty.cols == dstx.cols);

			for(int i=0; i< dstx.cols; i++){
				curve2d[i].setConcavity(dstx.at<float>(0,i),dsty.at<float>(0,i));
			}
		}
		this->concavityReady = true;
		return;
	}
	bool isTangentReady(){return this->tangentReady;}
	bool isConcavityReady(){return this->concavityReady;}

	double color[3]; // color of the point [0,255] => will be changed 
	void setColor(double in1, double in2, double in3){color[0]=in1; color[1]=in2; color[2] = in3;}

	// loader
	void loadTexture(char* fileName){
		this->texture = cvLoadImage(fileName);
		return;
	}
	void setTexture(IplImage* _texture){
		this->texture = _texture;
		return;
	}

	// drawers
	void draw(IplImage* img, CvScalar _color, int rate=4, int LOD=10, bool output = false);
	void drawWithTexture(IplImage* img, std::vector<vec2>* track, IplImage* _texture);// draw texture on img along track (xs,ys)

	// interpolation
	void removeRedundancy(); // remove cancatenated repeated points
	void getSplineCurve(vCurve &spc, int sampleRate, int lod);

	double *basis_matrix_overhauser_uni ( void)
		//****************************************************************************
		//
		//  Purpose:
		//
		//    BASIS_MATRIX_OVERHAUSER_UNI sets the uniform Overhauser spline basis matrix.
		//
		//  Discussion:
		//
		//    This basis matrix assumes that the data points P1, P2, P3 and
		//    P4 are uniformly spaced in T, and that P2 corresponds to T = 0,
		//    and P3 to T = 1.
		//
		//  Licensing:
		//
		//    This code is distributed under the GNU LGPL license. 
		//
		//  Modified:
		//
		//    14 February 2004
		//
		//  Author:
		//
		//    John Burkardt
		//
		//  Reference:
		//
		//    James Foley, Andries vanDam, Steven Feiner, John Hughes,
		//    Computer Graphics, Principles and Practice,
		//    Second Edition,
		//    Addison Wesley, 1995,
		//    ISBN: 0201848406,
		//    LC: T385.C5735.
		//
		//  Parameters:
		//
		//    Output, double BASIS_MATRIX_OVERHASUER_UNI[4*4], the basis matrix.
		//
	{
		double *mbasis;

		mbasis = new double[4*4];

		mbasis[0+0*4] = - 1.0 / 2.0;
		mbasis[0+1*4] =   3.0 / 2.0;
		mbasis[0+2*4] = - 3.0 / 2.0;
		mbasis[0+3*4] =   1.0 / 2.0;

		mbasis[1+0*4] =   2.0 / 2.0;
		mbasis[1+1*4] = - 5.0 / 2.0;
		mbasis[1+2*4] =   4.0 / 2.0;
		mbasis[1+3*4] = - 1.0 / 2.0;

		mbasis[2+0*4] = - 1.0 / 2.0;
		mbasis[2+1*4] =   0.0;
		mbasis[2+2*4] =   1.0 / 2.0;
		mbasis[2+3*4] =   0.0;

		mbasis[3+0*4] =   0.0;
		mbasis[3+1*4] =   2.0 / 2.0;
		mbasis[3+2*4] =   0.0;
		mbasis[3+3*4] =   0.0;

		return mbasis;
	}
	void OVERHAUSER_interpolate (int NDATA,double* tdata,double* ydata,int nsample,int left, double* tout, double* yout )
		//****************************************************************************
		//
		//  Purpose:
		//
		//    TEST07 tests BASIS_MATRIX_OVERHAUSER_UNI and BASIS_MATRIX_TMP.
		//
		//  Discussion:
		//
		//    YDATA(1:NDATA) = ( TDATA(1:NDATA) + 2 )**2 + 3
		//
		//  Licensing:
		//
		//    This code is distributed under the GNU LGPL license.
		//
		//  Modified:
		//
		//    13 January 2007
		//
		//  Author:
		//
		//    John Burkardt
		//
	{
# define N 4
//# define NDATA 4//

		int i;
		int j;
		int jhi;
		//int left;//
		double *mbasis;
		//int nsample = 4;//
		//double tdata[NDATA] = { -1.0, 0.0, 1.0, 2.0 };//
		double thi;
		double tlo;
		double tval;// out
		//double ydata[NDATA] = { 4.0, 7.0, 12.0, 19.0 };//
		double yval;// out

		mbasis = basis_matrix_overhauser_uni ( );
		left = 2;

		for ( i = 0; i <= NDATA; i++ )
		{
			if ( i == 0 ){
				tlo = tdata[0] - 0.5 * ( tdata[1] - tdata[0] );
				thi = tdata[0];}
			else if ( i < NDATA ){
				tlo = tdata[i-1];
				thi = tdata[i];}
			else if ( NDATA <= i ){
				tlo = tdata[NDATA-1];
				thi = tdata[NDATA-1] + 0.5 * ( tdata[NDATA-1] - tdata[NDATA-2] );}

			if ( i < NDATA ){
				jhi = nsample - 1;}
			else{
				jhi = nsample;}

			for ( j = 0; j <= jhi; j++ )
			{
				tval = ( ( double ) ( nsample - j ) * tlo
					+ ( double ) (           j ) * thi )
					/ ( double ) ( nsample     );

				yval = basis_matrix_tmp ( left, N, mbasis, NDATA, tdata, ydata, tval );

				tout[i*nsample + j] = tval;
				yout[i*nsample + j] = yval;
			}
		}

		delete [] mbasis;

		return;
# undef N
//# undef NDATA
	}
	double basis_matrix_tmp ( int left, int n, double mbasis[], int ndata, 
		double tdata[], double ydata[], double tval )
		//****************************************************************************80
		//
		//  Purpose:
		//
		//    BASIS_MATRIX_TMP computes Q = T * MBASIS * P
		//
		//  Discussion:
		//
		//    YDATA is a vector of data values, most frequently the values of some
		//    function sampled at uniformly spaced points.  MBASIS is the basis
		//    matrix for a particular kind of spline.  T is a vector of the
		//    powers of the normalized difference between TVAL and the left
		//    endpoint of the interval.
		//
		//  Licensing:
		//
		//    This code is distributed under the GNU LGPL license. 
		//
		//  Modified:
		//
		//    10 October 2012
		//
		//  Author:
		//
		//    John Burkardt
		//
		//  Parameters:
		//
		//    Input, int LEFT, indicats that TVAL is in the interval
		//    [ TDATA(LEFT), TDATA(LEFT+1) ], or that this is the "nearest"
		//    interval to TVAL.
		//    For TVAL < TDATA(1), use LEFT = 1.
		//    For TDATA(NDATA) < TVAL, use LEFT = NDATA - 1.
		//
		//    Input, int N, the order of the basis matrix.
		//
		//    Input, double MBASIS[N*N], the basis matrix.
		//
		//    Input, int NDATA, the dimension of the vectors TDATA and YDATA.
		//
		//    Input, double TDATA[NDATA], the abscissa values.  This routine
		//    assumes that the TDATA values are uniformly spaced, with an
		//    increment of 1.0.
		//
		//    Input, double YDATA[NDATA], the data values to be interpolated or
		//    approximated.
		//
		//    Input, double TVAL, the value of T at which the spline is to be
		//    evaluated.
		//
		//    Output, double BASIS_MATRIX_TMP, the value of the spline at TVAL.
		//
	{
		double arg;
		int first;
		int i;
		int j;
		double tm;
		double *tvec;
		double yval;

		tvec = new double[n];

		if ( left == 1 )
		{
			arg = 0.5 * ( tval - tdata[left-1] );
			first = left;
		}
		else if ( left < ndata - 1 )
		{
			arg = tval - tdata[left-1];
			first = left - 1;
		}
		else if ( left == ndata - 1 )
		{
			arg = 0.5 * ( 1.0 + tval - tdata[left-1] );
			first = left - 1;
		}
		//
		//  TVEC(I) = ARG**(N-I).
		//
		tvec[n-1] = 1.0;
		for ( i = n-2; 0 <= i; i-- )
		{
			tvec[i] = arg * tvec[i+1];
		}

		yval = 0.0;
		for ( j = 0; j < n; j++ )
		{
			tm = 0.0;
			for ( i = 0; i < n; i++ )
			{
				tm = tm + tvec[i] * mbasis[i+j*n];
			}
			yval = yval + tm * ydata[first - 1 + j];
		}

		delete [] tvec;

		return yval;
	}
};


#endif