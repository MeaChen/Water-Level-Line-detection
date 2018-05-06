/*
 *此程序输入图像，打印出此图像的最佳阈值，并生成对应的二值化图像和灰度直方图
 */

#include <iostream>
#include <highgui.h>
#include <cv.h>
#include <cxcore.h>
using namespace std;
using namespace cv;

const int grey_level = 256;

/*根据得到的灰度值分布数组输出灰度直方图（未去除黑色背景）*/
void drawHistogram(int array[])
{
	int bins = 256;
	double max_val = 0;    //最多的点数
	for (int x = 0; x < grey_level; ++x){
		if (array[x]>max_val)
			max_val = array[x];
	}
	int scale = 2;
	int hist_height = 256;
	Mat hist_img = Mat::zeros(hist_height, bins*scale, CV_8UC3);
	for (int i = 0; i<bins; i++)
	{
		float bin_val = array[i];
		int intensity = cvRound(bin_val*hist_height / max_val);  //要绘制的高度  
		rectangle(hist_img, Point(i*scale, hist_height - 1),
			Point((i + 1)*scale - 1, hist_height - intensity),
			CV_RGB(255, 255, 255));
	}
	imwrite("histogram.png", hist_img);
	waitKey();
}

/* the implementation of ostu's method */
int ostu(Mat &img)
{
	int width = img.cols;
	int height = img.rows;
	//cout << width << "  " << height << endl;

	int grey_total = 0;
	int spots = width*height;
	//int grey_matrix[256][256];
	int histogram[grey_level] = { 0 };     //灰度直方图
	int spot_total[grey_level] = { 0 };   //灰度值小于等于n的总像素点数
	int grey_all[grey_level] = { 0 };    //灰度值小于等于n的总灰度值

	/* 得到图像的灰度分布 */
	uchar *pImg = img.data;
	for (int i = 0; i < height; ++i){
		for (int j = 0; j < width; ++j){
			int intensity = pImg[j];
			grey_total += intensity;
			++histogram[intensity];
		}
		pImg += img.step;
	}

	drawHistogram(histogram);  //输出灰度直方图

	double ave = (double)grey_total / spots;  //平均灰度
	//cout << "the average intensity is " << ave << endl;

	/* 数组赋值 */
	spot_total[0] = histogram[0];
	grey_all[0] = 0;
	for (int i = 1; i < grey_level; ++i){
		spot_total[i] = spot_total[i - 1] + histogram[i];
		grey_all[i] = grey_all[i - 1] + i*histogram[i];
	}

	int threshold = 0;
	double var = 0;
	for (int i = 0; i < grey_level; ++i){
		double rate = (double)spot_total[i] / spots;
		//cout << "比例为：" << rate << endl;
		double fore;
		if (spot_total[i] == 0)
			fore = 0;
		else
			fore = (double)grey_all[i] / spot_total[i];
		//cout << "前景平均灰度值：" << fore << endl;
		double back;
		if (spots == spot_total[i])
			back = 0;
		else
			back = (double)(grey_total - grey_all[i]) / (spots - spot_total[i]);
		//cout << "后景平均灰度值：" << back << endl;
		double value = rate*(fore - ave)*(fore - ave) + double(1 - rate)*(back - ave)*(back - ave);
		if (value > var){
			var = value;
			threshold = i;
		}
	}
	return threshold;
}


int main(int argc, char** argv)
{
	Mat img = imread("cherry.png");
	Mat newImg;
	cvtColor(img, newImg, CV_BGR2GRAY);
	//drawHistogram(newImg);
	int threshold = ostu(newImg);
	//int threshold = ostu(img);

	cout << "the best thresholding value is " << threshold << endl;
	int width = newImg.cols;
	int height = newImg.rows;
	uchar *pImg = newImg.data;
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			pImg[j] = (pImg[j] > threshold) ? 255 : 0;
		}
		pImg += newImg.step;
	}
	imwrite("result.png", newImg);
	system("pause");
	return 0;
}

