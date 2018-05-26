#include <cv.h>
#include <highgui.h>
#include <ml.h>
#include <cxcore.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include "opencv2/core/core.hpp"  
#include "highgui.h"  
#include "opencv2/imgproc/imgproc.hpp"  
#include "opencv2/features2d/features2d.hpp"  
#include "opencv2/nonfree/nonfree.hpp"  
#include "level.h"
//#pragma comment(linker, "/STACK:10240000000,10240000000")//手动开大栈区

/*
 * pic_type: 
 * 1表示用detect且可以测量水位
 * 2表示用detect1，不可以测量水位
 * 3表示用detect1，可以测量水位
 * 4表示用detect2，不可以测量水位
 */
const int pic_type =4;

using namespace std;
using namespace cv;

const int featurenum = 1764;
// PICPATH=1表示处理文件夹a下的图和样本，PICPATH=2表示处理b下的图和样本
const float PI=3.141593;

class Mysvm : public CvSVM
{
public:
	int get_alpha_count()
	{
		return this->sv_total;
	}

	int get_sv_dim()
	{
		return this->var_all;
	}

	int get_sv_count()
	{
		return this->decision_func->sv_count;
	}

	double* get_alpha()
	{
		return this->decision_func->alpha;
	}

	float** get_sv()
	{
		return this->sv;
	}

	float get_rho()
	{
		return this->decision_func->rho;
	}
};

void Train()
{
	char classifierSavePath[256] = ".//hogSVMDetector.txt";


	string positivePath = ".//a//pos//";
	string negativePath = ".//a//neg//";

	int positiveSampleCount = 9;
	int negativeSampleCount = 192;
	int totalSampleCount = positiveSampleCount + negativeSampleCount;

	cout << "//////////////////////////////////////////////////////////////////" << endl;
	cout << "totalSampleCount: " << totalSampleCount << endl;
	cout << "positiveSampleCount: " << positiveSampleCount << endl;
	cout << "negativeSampleCount: " << negativeSampleCount << endl;

	CvMat *sampleFeaturesMat = cvCreateMat(totalSampleCount, featurenum, CV_32FC1);
	//24*24的训练样本，该矩阵将是totalSample*3780,64*64的训练样本，该矩阵将是totalSample*featurenum  
	cvSetZero(sampleFeaturesMat);
	CvMat *sampleLabelMat = cvCreateMat(totalSampleCount, 1, CV_32FC1);//样本标识    
	cvSetZero(sampleLabelMat);

	cout << "************************************************************" << endl;
	cout << "start to training positive samples..." << endl;

	char positiveImgName[256];
	string path;
	for (int i = 0; i<positiveSampleCount; i++)
	{
		memset(positiveImgName, '\0', 256 * sizeof(char));
		sprintf(positiveImgName, "%d.jpg", i + 1);
		int len = strlen(positiveImgName);
		string tempStr = positiveImgName;
		path = positivePath + tempStr;

		cv::Mat img = cv::imread(path);
		if (img.data == NULL)
		{
			//cout << "positive image sample load error: " << i << " " << path << endl;
			//system("pause");
			continue;
		}

		cv::HOGDescriptor hog(cv::Size(32, 32), cv::Size(8, 8), cv::Size(4, 4), cv::Size(4, 4), 9);
		vector<float> featureVec;

		hog.compute(img, featureVec, cv::Size(4, 4));
		int featureVecSize = featureVec.size();

		for (int j = 0; j<featureVecSize; j++)
		{
			CV_MAT_ELEM(*sampleFeaturesMat, float, i, j) = featureVec[j];
		}
		sampleLabelMat->data.fl[i] = 1;
	}
	cout << "end of training for positive samples..." << endl;

	cout << "*********************************************************" << endl;
	cout << "start to train negative samples..." << endl;

	char negativeImgName[256];
	for (int i = 0; i<negativeSampleCount; i++)
	{
		memset(negativeImgName, '\0', 256 * sizeof(char));
		sprintf(negativeImgName, "%d.jpg", i + 1);
		path = negativePath + negativeImgName;
		cv::Mat img = cv::imread(path);
		if (img.data == NULL)
		{
			//cout << "negative image sample load error: " << path << endl;
			continue;
		}

		cv::HOGDescriptor hog(cv::Size(32, 32), cv::Size(8, 8), cv::Size(4, 4), cv::Size(4, 4), 9);
		vector<float> featureVec;

		hog.compute(img, featureVec, cv::Size(4, 4));//计算HOG特征  
		int featureVecSize = featureVec.size();

		for (int j = 0; j<featureVecSize; j++)
		{
			CV_MAT_ELEM(*sampleFeaturesMat, float, i + positiveSampleCount, j) = featureVec[j];
		}

		sampleLabelMat->data.fl[i + positiveSampleCount] = -1;
	}

	cout << "end of training for negative samples..." << endl;
	cout << "********************************************************" << endl;
	cout << "start to train for SVM classifier..." << endl;

	CvSVMParams params;
	params.svm_type = CvSVM::C_SVC;
	params.kernel_type = CvSVM::LINEAR;
	params.term_crit = cvTermCriteria(CV_TERMCRIT_ITER, 1000, FLT_EPSILON);
	params.C = 0.01;

	Mysvm svm;
	svm.train(sampleFeaturesMat, sampleLabelMat, NULL, NULL, params); //用SVM线性分类器训练  
	svm.save(classifierSavePath);

	cvReleaseMat(&sampleFeaturesMat);
	cvReleaseMat(&sampleLabelMat);

	int supportVectorSize = svm.get_support_vector_count();
	cout << "support vector size of SVM：" << supportVectorSize << endl;
	cout << "************************ end of training for SVM ******************" << endl;

	CvMat *sv, *alp, *re;//所有样本特征向量   
	sv = cvCreateMat(supportVectorSize, featurenum, CV_32FC1);
	alp = cvCreateMat(1, supportVectorSize, CV_32FC1);
	re = cvCreateMat(1, featurenum, CV_32FC1);
	CvMat *res = cvCreateMat(1, 1, CV_32FC1);

	cvSetZero(sv);
	cvSetZero(re);

	for (int i = 0; i<supportVectorSize; i++)
	{
		memcpy((float*)(sv->data.fl + i * featurenum), svm.get_support_vector(i), featurenum * sizeof(float));
	}

	double* alphaArr = svm.get_alpha();
	int alphaCount = svm.get_alpha_count();

	for (int i = 0; i<supportVectorSize; i++)
	{
		alp->data.fl[i] = alphaArr[i];
	}
	cvMatMul(alp, sv, re);

	int posCount = 0;
	for (int i = 0; i<featurenum; i++)
	{
		re->data.fl[i] *= -1;
	}

	FILE* fp = fopen(".//hogSVMDetector.txt", "wb");
	if (NULL == fp)
	{
		cout << "can't open the file" << endl;
		exit(-1);
	}
	for (int i = 0; i<featurenum; i++)
	{
		fprintf(fp, "%f \n", re->data.fl[i]);
	}
	float rho = svm.get_rho();
	fprintf(fp, "%f", rho);
	cout << "hogSVMDetector.txt 保存完毕" << endl;//保存HOG能识别的分类器  
	fclose(fp);
}

void Detect(Mat &img, vector<Rect> &rects)
{
	rects.clear();
	vector<float> x;
	ifstream fileIn(".//hogSVMDetector.txt", ios::in);
	float val = 0.0f;
	while (!fileIn.eof())
	{
		fileIn >> val;
		x.push_back(val);
	}
	fileIn.close();
	cv::HOGDescriptor hog(cv::Size(32, 32), cv::Size(8, 8), cv::Size(4, 4), cv::Size(4, 4), 9);
	hog.setSVMDetector(x);
	hog.detectMultiScale(img, rects, 0, cv::Size(4, 4), cv::Size(8, 8), 1.07, 2);
}

void Detect1(Mat &img, vector<Rect> &rects)
{
	rects.clear();
	vector<float> x;
	ifstream fileIn(".//hogSVMDetector1.txt", ios::in);
	float val = 0.0f;
	while (!fileIn.eof())
	{
		fileIn >> val;
		x.push_back(val);
	}
	fileIn.close();
	//cv::HOGDescriptor hog(cv::Size(32, 32), cv::Size(8, 8), cv::Size(4, 4), cv::Size(4, 4), 9);
	cv::HOGDescriptor hog(cv::Size(16, 16), cv::Size(4, 4), cv::Size(4, 4), cv::Size(2, 2), 9);
	hog.setSVMDetector(x);
	//hog.detectMultiScale(img, rects, 0, cv::Size(4, 4), cv::Size(8, 8), 1.07, 2);
	hog.detectMultiScale(img, rects, 0, cv::Size(2, 2), cv::Size(8, 8), 1.07, 2);

}

void Detect2(Mat &img, vector<Rect> &rects)
{
	rects.clear();
	vector<float> x;
	ifstream fileIn(".//hogSVMDetector2.txt", ios::in);
	float val = 0.0f;
	while (!fileIn.eof())
	{
		fileIn >> val;
		x.push_back(val);
	}
	fileIn.close();
	cv::HOGDescriptor hog(cv::Size(32, 32), cv::Size(8, 8), cv::Size(4, 4), cv::Size(4, 4), 9);
	hog.setSVMDetector(x);
	hog.detectMultiScale(img, rects, 0, cv::Size(4, 4), cv::Size(8, 8), 1.02, 2);

}

//在图上画出拟合直线
void drawLine(cv::Mat &image, Vec4f &line, cv::Scalar color)
{
	//phi rho
	double cos_theta = line[0];
	double sin_theta = line[1];
	double x0 = line[2], y0 = line[3];

	double theta = atan2(sin_theta, cos_theta) + PI / 2.0;
	double rho = y0 * cos_theta - x0 * sin_theta;

	//std::cout << "phi = " << theta / PI * 180 << std::endl;
	//std::cout << "rho = " << rho << std::endl;
	if (theta < PI / 4. || theta > 3.*PI / 4.)// ~vertical line
	{
		cv::Point pt1(rho / cos(theta), 0);
		cv::Point pt2((rho - image.rows * sin(theta)) / cos(theta), image.rows);
		cv::line(image, pt1, pt2, cv::Scalar(255), 1);
	}
	else
	{
		cv::Point pt1(0, rho / sin(theta));
		cv::Point pt2(image.cols, (rho - image.cols * cos(theta)) / sin(theta));
		cv::line(image, pt1, pt2, color, 1);
	}
}

//得到水岸线
void grab(Mat &img, Rect rec, int factor, Vec4f &line)
{
	int x_length = img.cols / factor;
	int y_length = img.rows / factor;
	Mat im;
	resize(img, im, Size(x_length, y_length));

	//Rect rect(x_length / 4, y_length / 8, x_length / 2, y_length * 6 / 8);
	//Mat tmp = im(rect);
	//cout << tmp.cols << " " << tmp.rows << endl;
	Rect rect = Rect(rec.x / factor, rec.y / factor, rec.width / factor, rec.height / factor);
	Mat res;
	Mat tmp1, tmp2;
	double tt = getTickCount();
	//rectangle(im, cvPoint(rect.x, rect.y), cvPoint(rect.x + rect.width, rect.y + rect.height), CV_RGB(255, 0, 0), 2);
	
	grabCut(im, res, rect, tmp1, tmp2, 2, GC_INIT_WITH_RECT);
	//imwrite("grab.jpg", im);
	tt = getTickCount() - tt;
	cout << "time:" << 1000 * tt / cv::getTickFrequency() << endl;
	compare(res, GC_PR_FGD, res, CMP_EQ);
	Mat foreground(im.size(), CV_8UC3, Scalar(255, 255, 255));
	im.copyTo(foreground, res);
	cv::imwrite("foreground.jpg", foreground);
	Mat gray_foreground;
	cvtColor(foreground, gray_foreground, CV_RGB2GRAY);

	vector<vector<int> >array2;
	uchar *pImg = gray_foreground.data;

	for (int i = 0; i < gray_foreground.rows; ++i){  //生成图片的像素矩阵
		vector<int> v;
		for (int j = 0; j < gray_foreground.cols; ++j){
			v.push_back(pImg[j]);
		}
		array2.push_back(v);
		v.clear();
		pImg += gray_foreground.step;
	}

	vector<Point>points2;
	int max_y=0;
	for (int i = rect.x; i < rect.x + rect.width; ++i){
		int y = 0;
		for (int k = rect.y+rect.height-1; k >= rect.y; --k){
			if (array2[k][i] != 255){
				y = k;
				Point p(i, y);
				circle(im, p, 1, CV_RGB(0, 255, 255), 2, 8, 0);
				points2.push_back(p);
				break;
			}
		}
		if (y > max_y)
			max_y = y;
	}
	cout << "max_y: " << max_y << endl;
	//imwrite("grab.jpg", im);
	vector<Point>points;
	for (int i = 0; i < points2.size();++i){
		if (max_y - points2[i].y <= 10)
			points.push_back(points2[i]);
	}

	//cout << points.size() << endl;
	for (int i = 0; i < points.size(); ++i){
		points[i].x *= factor;
		points[i].y *= factor;
		//circle(im0, points[i], 1, CV_RGB(0, 255, 255), 2, 8, 0);
		//cout << points[i].x << " " << points[i].y << endl;
	}
	//Vec4f line;
	fitLine(points, line, CV_DIST_HUBER, 0, 0.01, 0.01);

	drawLine(img, line, cv::Scalar(0));
}

int main()
{
	Mat im0 = imread("./final_images/26.jpg");
	Mat img2;
	im0.copyTo(img2);
	int factor = 4;

	Vec4f line;
	size_t rectx = 0, recty = 0, rectw = im0.cols, recth = im0.rows;
	int resize_factor = 1;
	string cfg = "./final_images/26.cfg";
	ifstream cfgFile(cfg);
	if (!cfgFile) {
		cout << cfg << " not found" << endl;
		//exit(-1);
	}
	else{
		cfgFile >> rectx >> recty >> rectw >> recth;
		cfgFile >> resize_factor;
	}
	//cout << "resize_factor:" << resize_factor << endl;
	//cout << "Rect: [" << rectx << ", " << recty << ", " << rectw << ", " << recth << "]" << endl;
	Rect r = Rect(rectx, recty, rectw, recth);
	if(pic_type==1 || pic_type==3)
		grab(im0, r, factor, line);

	cout << "end of detection of water level line!" << endl;

	//检测'E'
	//int resize_factor = 1;
	vector<Rect>rectangels;
	Mat gray;
	cvtColor(img2, gray, CV_RGB2GRAY);
	//imshow("gray", gray);
	resize(gray, gray, Size(gray.cols / resize_factor, gray.rows / resize_factor));

	if (pic_type == 1)	
		Detect(gray, rectangels);
	else if (pic_type == 2 || pic_type == 3)
		Detect1(gray, rectangels);
	else if (pic_type == 4)
		Detect2(gray, rectangels);
	cout << "end of detection of letter E!" << endl;
	
	if (rectangels.size()){
		//按比例恢复
		for (auto& r : rectangels)
		{
			Rect tempRect = Rect(r.x*resize_factor,
				r.y * resize_factor,
				r.width * resize_factor,
				r.height * resize_factor);
			r = tempRect;
		}
		vector<Rect> removedRects;
		filterPoint(rectangels, removedRects, r);
		cout << "the size of E: " << rectangels.size() << endl;
		//rectangle(im0, cvPoint(r.x, r.y), cvPoint(r.x + r.width, r.y + r.height), CV_RGB(255, 0, 0), 2);

		//红色表示删选剩下的，绿色表示被剔除的
		for (int i = 0; i<rectangels.size(); i++)
		{
			Rect tempRect = rectangels[i];
			rectangle(im0, cvPoint(tempRect.x, tempRect.y),cvPoint(tempRect.x + tempRect.width, tempRect.y + tempRect.height), CV_RGB(255, 0, 0), 2);
			
		}
		for (int i = 0; i<removedRects.size(); i++)
		{
			Rect tempRect = removedRects[i];
			rectangle(im0, cvPoint(tempRect.x, tempRect.y), cvPoint(tempRect.x + tempRect.width, tempRect.y + tempRect.height), CV_RGB(0, 255, 0), 2);

		}
		if (pic_type == 1 || pic_type==3){
			Vec4f recLine;
			float level_height = calcWaterHeight(rectangels, line, r, recLine);   //检测焦点
			if (level_height >= 0)
				cout << "the height of water level line: " << level_height << endl;
			drawLine(im0, recLine, cv::Scalar(0));
		}
	}
	
	cv::imwrite("result.jpg", im0);
	cv::waitKey();
	std::system("pause");
	return 0;

}