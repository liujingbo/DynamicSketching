#include "vCurve.h"

void vPoint2D::init(){
		unprojection=-1;intensity=1;thickness=velocity =-1;t[0]=t[1]=c[0]=c[1]=0;
		color[0]=color[1]=color[2] = 0;
		segment = -1;
}

double vCurve::fading_factor = 1.0;

vCurve::~vCurve(){
	if(spline != NULL) delete spline;

}
void vCurve::iniMembers(){
		tangentReady = false;
		concavityReady = false;
		this->color[0]=color[1]=color[2]=0;
		this->type = UNKNOWN;
		length = -1;
		defaultHeight = 2;
		texture = NULL;
		spline = NULL;
		isInner = false;
		// this->loadTexture("..\\testdata\\textures\\charcoal_2.png");
}

void vCurve::draw(IplImage* img, CvScalar _color, int rate, int LOD, bool output )
{
	using namespace std;

	// output point format:
	// p x y intensity thickness velocity randomness
	ofstream myfile ("..\\testdata\\strokes\\1_1.strokes", ios::out | ios::app | ios::binary);
	if(output)
		assert (myfile.is_open());

	if(this->curve2d.size()<2) return;

	// determine drawing direction: bool: forward-true, reverse-false
	bool direction= true;
	vec2 p1 = this->curve2d.front().toVec2();
	vec2 p2 = this->curve2d.back().toVec2();
	vec2 p12 = p2 - p1;
	if(len(p12)>=0) {
		normalize(p12);
		if(p12[0]<=0 && p12[1]<=0) direction=false;
		else if(p12[0] >=0 && p12[1]<=0 && p12[0]<0.5) direction=false;
		else if(p12[0] <=0 && p12[1]>=0 && p12[0]<-0.5) direction=false;
	}
	else{
		int dummy = 0;
	}


	// +++++++++++++++++ initial settings ++++++++++++++
	// use texture or draw original track
	bool useTexture=false;

	// over write the default color
	CvScalar stroke_color; // BGR
	if(this->color[0]!= 0 || this->color[1]!= 0 || this->color[2] != 0){
		stroke_color.val[0] = this->color[0];
		stroke_color.val[1] = this->color[1];
		stroke_color.val[2] = this->color[2];
	}
	else{
		stroke_color.val[0] = 0;
		stroke_color.val[1] = 0;
		stroke_color.val[2] = 0;
	}
	// ++++++++++++++++++++++++++++++++++++++++++++++++++



	

	switch(this->type){
	case OPEN:{
		if(output){myfile << "# type OPEN\n";}
		// get spline
		if(spline == NULL){
			spline = new vCurve();
			this->getSplineCurve( *spline, rate, LOD);
		}

		// actural drawing
		std::vector<vec2> track;
		for(int i=0; i+1 <spline->size(); i++){
			CvPoint start,end;
			start.x = spline->at(i).x();
			start.y = spline->at(i).y();
			end.x = spline->at(i+1).x();
			end.y =spline->at(i+1).y();

			if(!useTexture){
				cvLine( img, start, end, stroke_color, 1);
			}

			vPoint2D p = spline->curve2d[i];
			track.push_back(vec2(p.x(),p.y()));
		}

		if(useTexture){
			this->drawWithTexture(img, &track, this->texture);
		}

		// output the curve's track
		if(output){
			int lastT=0;
			int lastY=0;
			for(int i=0; i<this->spline->size(); i++){
				int ind;
				if(direction){
					ind = i;
				}
				else{
					ind=spline->size()-1-i;
				}

				if((int)spline->at(ind).x() != lastT || spline->at(ind).y() != lastY){
					myfile<<"p "<<(int)spline->at(ind).x()<<" "<<(int)spline->at(ind).y()<<" "
						<<this->at(ind/LOD*rate).intensity*fading_factor<<" "<<this->at(ind/LOD*rate).thickness<<" "<<this->at(ind/LOD*rate).velocity<<" ";
					lastT = (int)spline->at(ind).x();
					lastY = (int)spline->at(ind).y();
				}
			}
			myfile<<"\n";
		}
		break;
			  }
	case CLOSE:{
		if(output){myfile << "# type CLOSE\n";}
		break;
			   }
	case STRAIGHT: {
		CvPoint start,end;
		start.x = curve2d.front().x();
		start.y = curve2d.front().y();
		end.x = curve2d.back().x();
		end.y = curve2d.back().y();

		int resolution = this->Length()/3;

		// restore to pass the center
		vec2 _cent = this->center();
		vec2 s(start.x,start.y);
		vec2 e(end.x,end.y);
		vec2 _tang = s-e;
		s = _cent + _tang/2.0f;
		e = _cent - _tang/2.0f;

		// actural drawing
		// cvLine( img, start, end, color, 1);
		float extend_percent = 0.1;
		if(resolution>1){
			std::vector<vec2> track;
			vec2 tmps=s;
			s = (1+extend_percent)*s - extend_percent* e; start.x = s[0]; start.y=s[1];
			e = (1+extend_percent)*e - extend_percent* tmps; end.x = e[0];end.y=e[1];

			// vec2 v = (e-s)/(float)resolution;
			/*for(int i=0;i<resolution;i++){
				vec2 interMediate = s + (float)i*v;
				track.push_back(interMediate);
			}*/
			track.push_back(s);
			track.push_back(e);

			if(!useTexture){
				cvLine( img, start, end, stroke_color, 1);
			}
			else{
				this->drawWithTexture(img, &track,this->texture);
			}
		}

		// output
		
		if(output && this->Length()>4){
			myfile << "# type STRAIGHT\n";
			if(!direction){ // reverse the direction
				vec2 tmp = s;
				s = e;
				e = tmp;
			}
			vec2 increment = ( e-s)/(float)resolution;
			int lastT=0;
			int lastY=0;
			int i=0;
			for(; i<resolution && len(increment)>0; i++){
				vec2 o = s + (float)i * increment;
				if((int)o[0] == lastT && (int)o[1] == lastY) continue;
				myfile<<"p "<<(int)o[0]<<" "<<(int)o[1]<<" "
					<<this->at(0).intensity*fading_factor<<" "<<this->at(0).thickness<<" "<<this->at(0).velocity<<" ";
				lastT= (int)o[0];
				lastY= (int)o[1];
			}
			if(i==1){
				double dummy=13412; 
			}
			myfile<<"\n";
		}

		break;
				   }
	case UNKNOWN:{
		if(output){myfile << "# type UNKNOWN\n";}
		for(int j=0;j+1<curve2d.size(); j++){
			CvPoint start,end;
			start.x = curve2d[j].x();
			start.y = curve2d[j].y();
			end.x = curve2d[j+1].x();
			end.y = curve2d[j+1].y();
			cvLine( img, start, end, stroke_color, 1);
		}
		break;
				 }
	default:break;
	}

	if(output)
		myfile.close();

	return;

}

void vCurve::drawWithTexture(IplImage* img, std::vector<vec2>* track, IplImage* _texture){
	
	if(img == NULL || _texture == NULL) {assert(0);return;}
	using namespace cv;
	float srcW = 0; // length of track, not length of curve >.<
	for(int i=0;i<track->size()-1;i++){
		srcW += dist( track->at(i),track->at(i+1));
	}
	float dstW = _texture->width;
	float srcH = this->defaultHeight;
	float dstH = _texture->height;

	// intensity [0,1] 
	float intense =0;
	for(int i=0;i<this->curve2d.size();i++){
		float tmp_intense = curve2d[i].intensity;
		if(tmp_intense == -1) tmp_intense=0.618;
		intense += tmp_intense *this->fading_factor;
	}
	intense/= curve2d.size(); 
	if(intense < 0.1) intense = 0.1;

	assert(intense>=0 && intense <=1);

	// parameterize the stroke into (u,v), u is in along the stroke direction
	float u = 0; // [0, srcW]

	for(int i=0;i<track->size()-1;i++){
		// [us, ut] from the texture
		vec2 tex_ps (u/srcW*dstW,dstH/2);

		// draw segment [track[i], track[i+1]]
		vec2 ps = track->at(i);
		vec2 pt = track->at(i+1);
		if(dist(ps,pt)<=0) continue;

		vec2 segTan = pt-ps; normalize(segTan);
		vec2 segNorm (segTan[1],-segTan[0]);

		Point p1=Point(ps[0],ps[1]);
		Point p2=Point(pt[0],pt[1]);

		// positions need to be filled with texture
		Mat srcMask = Mat::zeros(img->width,img->height,CV_8U);
		line( srcMask,p1,p2,1, srcH);

		int minx = min(ps[0],pt[0]) - srcH;
		int maxx = max(ps[0],pt[0]) + srcH;
		int miny = min(ps[1],pt[1]) - srcH;
		int maxy = max(ps[1],pt[1]) + srcH;

		for(int y= max(0,miny); y < maxy && y<srcMask.rows;y++){
			for(int x= max(0,minx);x< maxx && x<srcMask.cols;x++){
				if(srcMask.at<unsigned char>(y,x) != 0){
					 //find the corresponding position of (x,y) in texture
					vec2 vectorized = vec2(x,y) - ps;
					vec2 seg_cord1((vectorized DOT segTan), (vectorized DOT segNorm) );
					vec2 seg_cord2( seg_cord1[0]/srcW*dstW, seg_cord1[1]/srcH*dstH );
					vec2 tex_cord = tex_ps+seg_cord2;
					if((int)tex_cord[1] >=0 && (int)tex_cord[1]<_texture->height 
						&& (int)tex_cord[0]>=0 && (int)tex_cord[0]<_texture->width){
							
						//check overlapping
						CvScalar s = cvGet2D(_texture,(int)tex_cord[1], (int)tex_cord[0]);// row, then col
						CvScalar t = cvGet2D(img, y, x);
						double s_avg = (s.val[0]+s.val[1]+s.val[2]);

						double faded = 255-intense*(255-s_avg);
						
						double t_avg = (t.val[0]+t.val[1]+t.val[2]);
						s.val[0]=s.val[1]=s.val[2]=faded;
						if(faded < t_avg){ // texture is darker [0,0,0] is black
							cvSet2D(img, y, x, s );// row, then col
						}
					}
				}
			}
		}

		// update parameterization
		u += dist(track->at(i),track->at(i+1));
	}

	return;
}

void vCurve::getSplineCurve(vCurve &spc, int sampleRate, int lod){
		static double Spline_Matrix[4][4]={
			{ 1.0 / 6.0, 2.0 / 3.0, 1.0 / 6.0, 0 },
			{ 0,         2.0 / 3.0, 1.0 / 3.0, 0 }, 
			{ 0,         1.0 / 3.0, 2.0 / 3.0, 0 },
			{ 0,         1.0 / 6.0, 2.0 / 3.0, 1.0 / 6.0 } };
		static int C[4]={1, 3, 3, 1};
		vCurve vc;
		vPoint2D V[4];
		double x, y;
		double **b=new double*[lod];
		double u;
		vc.addPoint(vPoint2D(curve2d[0]));
		vc.addPoint(vPoint2D(curve2d[0]));
		vc.addPoint(vPoint2D(curve2d[0]));
		for(int i=sampleRate;i<size();i+=sampleRate){
			vc.addPoint(vPoint2D(curve2d[i]));
		}
		if(size()%sampleRate!=1){
			vc.addPoint(vPoint2D(curve2d.back()));
		}
		vc.addPoint(vPoint2D(vc.curve2d.back()));
		vc.addPoint(vPoint2D(vc.curve2d.back()));
		vc.addPoint(vPoint2D(vc.curve2d.back()));
		for(int i=0;i<lod;i++){
			b[i]=new double[4];
			u=(i+1)/(double)lod;
			for(int j=0;j<4;j++){
				b[i][j]=C[j];
			}
			for(int j=0;j<4;j++){
				for(int k=0;k<j;k++){
					b[i][k]*=1-u;
					b[i][3-k]*=u;
				}
			}
		}
		double v[4][2];
		for(int i=0;i<vc.size()-3;i++){
			for(int j=0;j<4;j++){
				v[j][0]=v[j][1]=0;
				for(int k=0;k<4;k++){
					v[j][0]+=Spline_Matrix[j][k]*vc.curve2d[i+k].x();
					v[j][1]+=Spline_Matrix[j][k]*vc.curve2d[i+k].y();
				}
			}
			for(int j=0;j<lod;j++){
				x=y=0;
				for(int k=0;k<4;k++){
					x+=v[k][0]*b[j][k];
					y+=v[k][1]*b[j][k];
				}
				spc.addPoint(vPoint2D((int)(x+0.5), (int)(y+0.5)));
			}
		}
		for(int i=0;i<lod;i++){
			delete[] b[i];
		}
		delete[] b;
	}


void vCurve::removeRedundancy(){
	std::vector<vPoint2D> filtered;

	int lastx = 0;
	int lasty = 0;
	for(int i=0;i<this->curve2d.size();i++){
		if(this->curve2d[i].x() == lastx && this->curve2d[i].y() == lasty){
		}
		else{
			filtered.push_back(this->curve2d[i]);
			lastx = this->curve2d[i].x();
			lasty = this->curve2d[i].y();
		}
	}

	this->curve2d = filtered;
	return;
}

vec2 vCurve::center(){
	vec2 _center;

	if(this->curve2d.size()<=0)
		return vec2(0,0);

	for(int i=0;i<this->curve2d.size();i++){
		_center += this->curve2d[i].toVec2();
	}

	_center /= this->curve2d.size();

	return _center;
}