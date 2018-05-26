#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <iomanip>

using namespace std;
using namespace cv;

void filterPoint(vector<Rect> &matches, vector<Rect> &removedMatches, Rect rect);
float calcWaterHeight(vector<Rect> &matches, Vec4f waterLevelLine, Rect rect, Vec4f &rectLine);