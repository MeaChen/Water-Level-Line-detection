#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <vector>
#include <cmath>

using namespace std;
using namespace cv;

inline Point2f lineOrigin(Vec4f l)
{
	return Point2f(l[2], l[3]);
}

inline Point2f lineDir(Vec4f l)
{
	return Point2f(l[0], l[1]);
}

bool intersection(Vec4f l1, Vec4f l2, Point2f &r)
{
	Point2f o1 = lineOrigin(l1);
	Point2f o2 = lineOrigin(l2);
	Point2f d1 = lineDir(l1);
	Point2f d2 = lineDir(l2);
	Point2f x = o2 - o1;

	float cross = d1.x * d2.y - d1.y *d2.x;
	if (fabs(cross) < 1e-8) /* EPS */
		return false;

	double t1 = (x.x * d2.y - x.y * d2.x) / cross;
	r = o1 + d1 * t1;
	return true;
}

Point2f lineAt(Vec4f line, float x)
{
	Point2f o = lineOrigin(line);
	Point2f d = lineDir(line);
	float delta = (x - o.x) / d.x;
	Point2f p = o + delta * d;
	return p;
}

bool hitTest(Rect rect1, Rect rect2)
{
	return (rect1 & rect2).area() > 0;
}

bool isIn(Rect rect1, Rect rect2)
{
	Rect r = rect1 & rect2;
	return r == rect1;
}

void filterPoint(vector<Rect> &matches, vector<Rect> &removedMatches, Rect rect)
{
	// Size is allowed when in [size_avg - diff, size_avg + diff]
	const size_t sizeValidDiff = 20;

	// Max percent of size allowed when two rect are overlapped
	const float overlapMaxPercent = 20.0f;
	sort(matches.begin(), matches.end(), [](Rect a, Rect b) {return a.y < b.y; });
	vector<Rect> filteredMatches;
	map<int, int> sizeCounter;
	int mostFrequentSize = 0;

	// Remove rects out of boundary
	for (auto& r : matches)
	{
		if (!isIn(r, rect)) {
			removedMatches.push_back(r);
			continue;
		}
		filteredMatches.push_back(r);

		int size = r.width;
		if (!sizeCounter.count(size))
			sizeCounter[size] = 0;

		++sizeCounter[size];  // rect is a square
		if (sizeCounter[size] > sizeCounter[mostFrequentSize])
			mostFrequentSize = size;
	}

	matches = filteredMatches;
	filteredMatches.clear();

	// Further filtering
	vector<int>flags;
	for (int i = 0; i < matches.size(); ++i)
	//for (auto& r : matches)
	{
		Rect r = matches[i];
		int size = r.width;

		// Remove too big or too small rects
		if (abs(size - mostFrequentSize) >= sizeValidDiff){
			removedMatches.push_back(r);
			flags.push_back(0);
			continue;
		}
		Rect overlap;
		//Rect overlap = r & rect;
		if (i > 0){
			int k = i - 1;
			while (k >= 0 && flags[k] == 0)
				--k;
			overlap = r&matches[k];
		}
		else
			overlap = r;
		float overlapArea = static_cast<float>(overlap.area());
		float area = static_cast<float>(r.area());

		// If overlapped area >= overlapMaxPercent% * area, then skip it
		if (overlap.area() != 0 && fabs(area - overlapArea) * 100 / area > overlapMaxPercent) {
			removedMatches.push_back(r);
			flags.push_back(0);
			continue;
		}

		filteredMatches.push_back(r);
		flags.push_back(1);
	}

	matches = filteredMatches;
	cout << "size:" << matches.size() << endl;
}

float calcWaterHeight(vector<Rect> &matches, Vec4f waterLevelLine, Rect rect, Vec4f &rectLine)
{
	const float fixedRate = 0.8f;

	//sort(matches.begin(), matches.end(), [](Rect a, Rect b) {return a.y < b.y; });
	// Height line & Rect line
	vector<Point> heightPoints;
	vector<Point> rectPoints;
	Vec4f heightLine;
	//Vec4f rectLine;

	// Add points
	float height = 0.0f;
	float imgY = 0.0f;
	int letterCnt = 0;

	//filterPoint(matches, rect);
	//cout << "size: " << matches.size() << endl;
	if (matches.size() > 0) {
		imgY = static_cast<float>(matches.front().y);
		heightPoints.push_back(Point2f(imgY, 0.0f));
	}

	//cout << "Point: [Y, height]" << endl;
	for (auto itr = matches.begin(); itr != matches.end(); ++itr)
	{
		Rect current = *itr;
		if ((itr + 1) != matches.end())
		{
			Rect nextCurrent = *(itr + 1);
			int size = (nextCurrent.width + current.width) / 2;
			int diff = nextCurrent.y - current.y;

			// If some letters are gone, add estimated letter count
			if (diff > size * 2) {
				letterCnt += round(static_cast<float>(diff - size) / size * fixedRate);
			}

			imgY += diff;
			height = (++letterCnt) * 10.0f;
			heightPoints.push_back(Point2f(imgY, height));
		}

		rectPoints.push_back(
			Point2f(current.x + current.width / 2.0f,
			current.y + current.height / 2.0f));
	}

	/*for (auto& p : heightPoints) {
	cout << "Point: " << p << endl;
	}*/

	if (heightPoints.empty() || rectPoints.empty()) {
		cerr << "No point available" << endl;
		return -1;
	}

	// Linear fitting
	fitLine(heightPoints, heightLine, CV_DIST_HUBER, 0, 0.01, 0.01);
	fitLine(rectPoints, rectLine, CV_DIST_HUBER, 0, 0.01, 0.01);

	//cout << "Water Line Origin: " << lineOrigin(waterLevelLine) << " Dir: " << lineDir(waterLevelLine) << endl;
	//cout << "Height Line Origin: " << lineOrigin(heightLine) << " Dir: " << lineDir(heightLine) << endl;
	//cout << "Rect Line Origin: " << lineOrigin(rectLine) << " Dir: " << lineDir(rectLine) << endl;

	// Choose a proper point on the water level line
	Point2f waterPoint;
	if (!intersection(waterLevelLine, rectLine, waterPoint)) {
		cout << "Line not intersected" << endl;
		return -1;
	}

	// Calculate actual height using imgY of waterPoint;
	Point2f actualHeight = lineAt(heightLine, waterPoint.y);
	//cout << "Actual height: " << setprecision(2) << fixed << actualHeight.y << endl;
	//cout << "Water level point: " << waterPoint << endl;

	return actualHeight.y;
}