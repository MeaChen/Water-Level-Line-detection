/*
 *学号：5140379003
 *姓名：陈慕凡
 *此程序测试SIFT算法
 */
#include "opencv2/core/core.hpp"  
#include "highgui.h"  
#include "opencv2/imgproc/imgproc.hpp"  
#include "opencv2/features2d/features2d.hpp"  
#include "opencv2/nonfree/nonfree.hpp" 
#include "opencv2/legacy/legacy.hpp"
#include <iostream>
#include <highgui.h>
#include <cv.h>
#include <cxcore.h>
#include <vector>
using namespace std;
using namespace cv;

int main(int argc, char** argv)
{
	//待匹配的两幅图像，从img1中识别出img2  
	Mat img1 = imread("1.jpg");
	Mat img2 = imread("3.jpg");

	SIFT sift1, sift2;

	vector<KeyPoint> key_points1, key_points2;

	Mat descriptors1, descriptors2, mascara;

	sift1(img1, mascara, key_points1, descriptors1);
	sift2(img2, mascara, key_points2, descriptors2);

	BruteForceMatcher<L2<float>> matcher;
	vector<DMatch>matches;
	matcher.match(descriptors1, descriptors2, matches);  //描述符之间的匹配

	std::nth_element(matches.begin(),  matches.begin() + 29, matches.end());       
	matches.erase(matches.begin() + 30, matches.end());

	namedWindow("SIFT_matches");
	Mat img_matches;
	//在输出图像中绘制匹配结果  
	drawMatches(img1, key_points1,         //第一幅图像和它的特征点  
		img2, key_points2,      //第二幅图像和它的特征点  
		matches,       //匹配器算子  
		img_matches,      //匹配输出图像  
		Scalar(255, 255, 255));     //用白色直线连接两幅图像中的特征点  
	imshow("SIFT_matches", img_matches);
	waitKey(0);
	return 0;
}

