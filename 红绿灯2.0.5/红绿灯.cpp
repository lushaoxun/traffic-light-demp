#include<cv.h>
#include<highgui.h>
#include<iostream>
#include<map>
#include<vector>
#include<Python.h>
#include "util.h"

using namespace std;

IplImage* source = 0;
IplImage* src = 0;
IplImage* hough_showimg = 0;
IplImage* showimg = 0;
IplImage* lastimg = 0;
IplImage* mid = 0;
IplImage* dst = 0;
int step = 1;
CvFont font;
int g_width = 0, g_height = 0;


//hough8，晚上是15，白天是0
int hough4 = 1, hough5 = 60, hough6 = 50, hough7 = 10, hough8 = 0, hough9 = 30;
int rs = 27, ri = 19, ys = 35, yi = 35, gs = 24, gi = 11;
int rhl = 40, rhu = 55;
int yhl = 7, yhu = 15;
int ghl = 40, ghu = 55;
int least_dots = 80;
int ratio_w_h = 70;

double t_intensity = 80.0;
bool is_night = false;

bool sortByArea( const CvRect &v1, const CvRect &v2)
{  
	//return v1.x + v1.width > v2.x + v2.width; 
	return v1.width * v1.height > v2.width * v2.height; 
}  
#define PI 3.141592653
inline double* rgb2hsi( double rgb[3] ){
	double* hsi = new double[3], theta;

	double r = rgb[0] / 255.0;
	double g = rgb[1] / 255.0;
	double b = rgb[2] / 255.0;

	if( r == 0 && g == 0 & b == 0 )
		hsi[2] = hsi[1] = hsi[0] = 0;
	else{
		theta = 0.5 * ((r-g) + (r-b)) / sqrt((r-g)*(r-g) + (r-b)*(g-b));
		theta = ((theta > 1) ? 1 : theta);
		theta = ((theta < -1) ? -1 : theta);
		if( b <= g )
			hsi[0] = acos( theta ) / (2 * PI);
		else
			hsi[0] = 1 - acos( theta ) / (2 * PI);
		hsi[1] = 1 - 3.0 * min( min(r,g), b ) / ( r + g + b );
		hsi[2] = ( r + g + b ) / 3.0;

		hsi[1] = ( (hsi[1] < 0) ? 0 : hsi[1] );
		hsi[0] = ( (hsi[1] == 0) ? 0 : hsi[0] );
		hsi[2] = ( (hsi[2] < 0) ? 0 : hsi[2] );
	}
	return hsi;
}


vector<CvRect> hough_circles_region;
void  drawCircle(IplImage* img, IplImage* showimg){
	cvCopyImage(img, showimg);
	IplImage* gray = cvCreateImage( cvGetSize(img), 8, 1 );
	CvMemStorage* storage = cvCreateMemStorage(0);
	cvCvtColor( img, gray, CV_BGR2GRAY );
	cvSmooth( gray, gray, CV_GAUSSIAN, 9, 9 ); // smooth it, otherwise a lot of false circles may be detected

	cvSaveImage("temp.jpg", gray);
	if(hough4 == 0 || hough5 == 0 || hough6 == 0 || hough7 == 0)
		return;

	vector<vector<int>> temp_circles;
	getImgCircles("temp.jpg", temp_circles, hough4, hough5, hough6, hough7, hough8, hough9);

	int i;
	if (hough_circles_region.size() != 0){
		hough_circles_region.clear();
	}
	for( i = 0; i < temp_circles.size(); i++ )
	{
		vector<int> p = temp_circles[i];
		CvRect rect = cvRect(p[0] - p[2], p[1] - p[2], 2 * p[2], 2 * p[2]);
		if(rect.width * rect.height < least_dots)
			continue;

		cvCircle( showimg, cvPoint(cvRound(p[0]),cvRound(p[1])), 3, CV_RGB(0,255,0), -1, 8, 0 );
		cvCircle( showimg, cvPoint(cvRound(p[0]),cvRound(p[1])), cvRound(p[2]), CV_RGB(255,0,0), 3, 8, 0 );

		hough_circles_region.push_back(rect);
	}
	cvReleaseImage(&gray);
	cvReleaseMemStorage(&storage);
}

bool in_circle(CvRect rect){
	bool incircle = false;
	for (int j = 0; j < hough_circles_region.size(); j++){
		CvRect hrect = hough_circles_region[j];

		int cx = rect.x + rect.width / 2;
		int cy = rect.y + rect.height / 2;
		if((cx >= hrect.x && cx <= hrect.x + hrect.width) && 
			(cy >= hrect.y && cy <= hrect.y + hrect.height)){
				incircle = true;
				break;
		}
	}
	bool inrect = false;
	inrect = (rect.y <= g_height / 2) ? true : false;
	return incircle && inrect;
}
IplImage* contourImg = 0;
map<string, vector<CvRect>> rgb_points;

void myDrawContours(IplImage* img, int color = 1){

	string rgb_flag = "RED";
	switch(color){
	case 1:
		rgb_flag = "RED";
		break;
	case 2:
		rgb_flag = "YELLOW";
		break;
	case 3:
		rgb_flag = "GREEN";
		break;
	default:
		break;
	}
	rgb_points.clear();

	cvCvtColor(img, contourImg, CV_BGR2GRAY);

	CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* contour = 0;
	// 提取轮廓
	int contour_num = cvFindContours(contourImg, storage, &contour, sizeof(CvContour), CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);

	//cvZero(dst);		// 清空数组
	//dst = src;
	CvSeq *_contour = contour; 
	double maxarea = 0;
	double minarea = 0;
	int m = 0;
	for( ; contour != 0; contour = contour->h_next )  
	{  
		double tmparea = fabs(cvContourArea(contour));

		double a = cvContourArea(contour);
		if(tmparea < minarea)
		{  
			cvSeqRemove(contour, 0); // 删除面积小于设定值的轮廓
			continue;
		}  
		CvRect aRect = cvBoundingRect( contour, 0 ); 

		double ratio1 = aRect.width/double(aRect.height);
		double ratio2 = aRect.height/double(aRect.width);
		double ratio = ratio_w_h / 100.0;
		if ((ratio1 <= 1 && ratio1 >= ratio) || (ratio2 <= 1 && ratio2 >= ratio))  
		{  
			if(rgb_points.find(rgb_flag) == rgb_points.end()){
				vector<CvRect> points;
				points.push_back(aRect);
				rgb_points.insert(pair <string, vector<CvRect>> (rgb_flag, points));
			}else{
				rgb_points.find(rgb_flag)->second.push_back(aRect);
			}
		}

		// 创建一个色彩值
		CvScalar color = CV_RGB( 0, 255, 255 );

		cvDrawContours(img, contour, color, color, -1, 1, 8);	//绘制外部和内部的轮廓
	}  

	cvReleaseMemStorage(&storage);
}

void processImgColor(int color = 1){

	int color1 = 0;
	int color2 = 0;
	int color3 = 0;
	int color4 = 0;

	string color_info = "0";
	switch (color)
	{
	case 1:
		color1 = 255;	color2 = color3 = color4 = 0;
		color_info = "RED";
		break;
	case 2:
		color2 = 255;	color1 = color3 = color4 = 0;
		color_info = "YELLOW";
		break;
	case 3:
		color3 = 255;	color2 = color1 = color4 = 0;
		color_info = "GREEN";
		break;
	default:
		break;
	}
	CvScalar newVal;
	CvScalar scalar;
	cvCopyImage(mid, dst);
	//cvShowImage("dst", dst);
	int area;
	CvPoint point;

	if (least_dots == 0){
		return;
	}
	for(int ii = 0; ii < dst->height; ii += step){
		for(int jj = 0; jj < dst->width; jj += step){
			scalar = cvGet2D(dst, ii, jj);
			int b = scalar.val[0];
			int g = scalar.val[1];
			int r = scalar.val[2];
			double rgb[3] = {r, g, b};
			double* hsi = rgb2hsi(rgb);
			double h = hsi[0];
			double s = hsi[1];
			double i = hsi[2];

			if ((h>=0.9 || (h<=0.07 && h>=0.0)) && (s > rs / 100.0) && (i > ri / 100.0)){
				newVal = cvScalar(color1, color1, color1);			//如果为红色就改为黑色
			}else if ((h>=yhl/100.0 && h<=yhu/100.0) && (s > ys / 100.0) && (i > yi / 100.0)){
				newVal = cvScalar(color2, color2, color2);	//如果为黄色就改为白色
			}else if ((h>=ghl/100.0 && h<=ghu/100.0) && (s > gs / 100.0) && (i > gi / 100.0)){
				newVal = cvScalar(color3, color3, color3);	//如果为绿色就改为灰色
			}else{
				newVal = cvScalar(color4, color4, color4);	//如果为绿色就改为其他色
			}
			point = cvPoint(jj, ii);

			cvSet2D(dst, ii, jj, newVal);
		}
	}


	//寻找霍夫圆
	int g_nStructElementSize = 2;
	IplConvKernel* element = cvCreateStructuringElementEx( 2*g_nStructElementSize+1,2*g_nStructElementSize+1,g_nStructElementSize,g_nStructElementSize, CV_SHAPE_ELLIPSE, NULL );//see=strel('disk',2);
	cvDilate(dst,dst, element);
	cvReleaseStructuringElement(&element);

	cvSmooth(dst, dst, CV_MEDIAN);
	drawCircle(dst, hough_showimg);

	//寻找边界
	myDrawContours(dst, color);


	for(map<string, vector<CvRect>>::iterator iter = rgb_points.begin(); iter != rgb_points.end(); iter++){
		string key = iter->first;
		const char* info = key.data();

		vector<CvRect> rects = iter->second;
		sort(rects.begin(), rects.end(), sortByArea);
		int max_area = rects[0].width * rects[0].height;
		CvRect rect = rects[0];
		//cvPutText(source, info , cvPoint(rect.x + rect.width, rect.y + rect.height), &font, CV_RGB(255,0,0));
		int this_area = max_area;
		int last_area = max_area + 1000;
		for (int i = 0; i < rects.size(); i++){

			//检测是否在Hough Circle中
			bool incircle = in_circle(rects[i]);

			this_area = rects[i].width * rects[i].height;
			if(incircle /*&& this_area < last_area*/ && this_area > least_dots){
				max_area = this_area;
				rect = rects[i];
				cvPutText(showimg, info , cvPoint(rect.x + rect.width, rect.y + rect.height), &font, CV_RGB(255,0,0));
				cvPutText(lastimg, info , cvPoint(rect.x + rect.width, rect.y + rect.height), &font, CV_RGB(255,0,0));
				cvDrawRect(hough_showimg, cvPoint(rect.x, rect.y), cvPoint(rect.x + rect.width, rect.y + rect.height), cvScalar(0, 0, 255), 1);
			}
			last_area = this_area;
		}
	}
}

void processImg(const char* path){

	cvCopyImage(source, showimg);
	cvCopyImage(src, lastimg);

	processImgColor(1);
	processImgColor(2);
	processImgColor(3);


//	cvShowImage(path, lastimg);
}

void judgeNight(char * path){
	if(calIntensity(path) >= t_intensity){
		is_night = false;
	}else{
		is_night = true;
	}
	if(is_night){
		hough4 = 1, hough5 = 60, hough6 = 50, hough7 = 10, hough8 = 13, hough9 = 30;
		rs = 27, ri = 13, ys = 35, yi = 22, gs = 23, gi = 27;
		rhl = 40, rhu = 55;
		yhl = 7, yhu = 15;
		ghl = 40, ghu = 54;
		least_dots = 80;
		ratio_w_h = 80;
	}
}

void getHLD(const char* path){
	source = cvLoadImage(path, CV_LOAD_IMAGE_UNCHANGED);
	src = cvLoadImage(path, CV_LOAD_IMAGE_UNCHANGED);

	//judgeNight(path);

	lastimg = cvCreateImage(cvGetSize(source), source->depth, source->nChannels);

	cvSetImageROI(source, cvRect(0, 0, source->width, source->height / 2));

	g_width = source->width;
	g_height = source->height;

	showimg = cvCreateImage(cvGetSize(source), source->depth, source->nChannels);
	cvCopyImage(source, showimg);
	contourImg = cvCreateImage(cvSize(source->width, source->height / 2), source->depth, 1);

	hough_showimg = cvCreateImage(cvGetSize(source), source->depth, source->nChannels);
	drawCircle(source, hough_showimg);

	mid = cvCreateImage(cvGetSize(source), source->depth, source->nChannels);

	cvSmooth(source, mid, CV_MEDIAN);

	dst = cvCreateImage(cvGetSize(source), source->depth, source->nChannels);

//	cvNamedWindow(path, 1);

	cvInitFont( &font,CV_FONT_HERSHEY_PLAIN,1, 1, 0, 1, 8);
	processImg(path);

	string p = path;
	string pp = p.substr(0, p.find_last_of("\\")) + "\\result\\" + p.substr(p.find_last_of("\\") + 1, p.size());
	cvSaveImage(pp.c_str(), lastimg);

	cvReleaseImage(&source);
	cvReleaseImage(&source);
	cvReleaseImage(&src);
	cvReleaseImage(&hough_showimg);
	cvReleaseImage(&showimg);
	cvReleaseImage(&lastimg);
	cvReleaseImage(&mid);
	cvReleaseImage(&dst);
}

void changeToNight(){
	hough4 = 1, hough5 = 60, hough6 = 50, hough7 = 10, hough8 = 13, hough9 = 30;
	rs = 27, ri = 13, ys = 35, yi = 22, gs = 23, gi = 27;
	rhl = 40, rhu = 55;
	yhl = 7, yhu = 15;
	ghl = 40, ghu = 54;
	least_dots = 80;
	ratio_w_h = 80;
}

void changeToDay(){
	hough4 = 1, hough5 = 60, hough6 = 50, hough7 = 10, hough8 = 0, hough9 = 30;
	rs = 27, ri = 19, ys = 35, yi = 35, gs = 24, gi = 11;
	rhl = 40, rhu = 55;
	yhl = 7, yhu = 15;
	ghl = 40, ghu = 55;
	least_dots = 80;
	ratio_w_h = 70;
}

int main( int argc, char** argv )
{
	
	//changeToNight();

	char * path = NULL;

	if (argc == 2){
		path = argv[1];
	}else{
		path = "E:\\桌面\\白天黑夜成功版\\day\\IMG_0239.JPG";
	}

	initPythonModule();

	changeToDay();
	//changeToNight();
	for(int i = 1; i < argc; i++){
		path = argv[i];
		getHLD(path);
	}

	destroyPythonMoudle();
	return 1;
}