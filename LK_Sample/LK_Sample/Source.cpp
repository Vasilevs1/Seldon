#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/video/tracking.hpp"
#include "opencv2\core\utility.hpp"
#include "opencv2/features2d.hpp"
#include "BT_communication.h"
#include <time.h>
#include <thread>

#include <iostream>
#include <ctype.h>
#include <math.h>
#define M_PI 3.1415926535897932384626433832795
using namespace cv;
using namespace std;

Mutex camMutex;
Mat img;

static void help()
{
	// print a welcome message, and the OpenCV version
	cout << "\nThis is a demo of Lukas-Kanade optical flow lkdemo(),\n"
		"Using OpenCV version " << CV_VERSION << endl;
	cout << "\nIt uses camera by default, but you can provide a path to video as an argument.\n";
	cout << "\nHot keys: \n"
		"\tESC - quit the program\n"
		"\tr - auto-initialize tracking\n"
		"\tc - delete all the points\n"
		"\tn - switch the \"night\" mode on/off\n"
		"To add/remove a feature point click it\n" << endl;
}

Point2f point;
bool addRemovePt = false;

static void onMouse(int event, int x, int y, int /*flags*/, void* /*param*/)
{
	if (event == EVENT_LBUTTONDOWN)
	{
		point = Point2f((float)x, (float)y);
		addRemovePt = true;
	}
}

void readCam()
{
	VideoCapture cap(0);
	Mat locimg;
	while (1)
	{
		cap >> locimg;
		camMutex.lock();
		img = locimg;
		camMutex.unlock();
		waitKey(1);
	}
}

int main(int argc, char** argv)
{
	string comport;
	cin >> comport;
	bt_out BT_device(comport);
	thread camRead(readCam);
	camRead.detach();

	TermCriteria termcrit(TermCriteria::COUNT | TermCriteria::EPS, 20, 0.03);
	Size subPixWinSize(10, 10), winSize(31, 31);

	const int MAX_COUNT = 300000;
	bool needToInit = false;
	bool nightMode = false;


	help();
	cv::CommandLineParser parser(argc, argv, "{@input|0|}");
	string input = parser.get<string>("@input");

	namedWindow("LK Demo", 1);
	setMouseCallback("LK Demo", onMouse, 0);

	Mat gray, prevGray, image, frame;
	vector<Point2f> points[2];

	for (;;)
	{
		camMutex.lock();
		frame = img;
		camMutex.unlock();

		if (frame.empty())
			continue;

		frame.copyTo(image);
		cvtColor(image, gray, COLOR_BGR2GRAY);

		if (nightMode)
			image = Scalar::all(0);

		if (needToInit)
		{
			// automatic initialization
			goodFeaturesToTrack(gray, points[1], MAX_COUNT, 0.01, 10, Mat(), 3, 3, 0);
			cornerSubPix(gray, points[1], subPixWinSize, Size(-1, -1), termcrit);
			addRemovePt = false;
		}
		else if (!points[0].empty())
		{
			vector<uchar> status;
			vector<float> err;
			if (prevGray.empty())
				gray.copyTo(prevGray);
			calcOpticalFlowPyrLK(prevGray, gray, points[0], points[1], status, err, winSize,
				3, termcrit, 0, 0.001);
			size_t i, k;
			for (i = k = 0; i < points[1].size(); i++)
			{
				if (addRemovePt)
				{
					if (norm(point - points[1][i]) <= 5)
					{
						addRemovePt = false;
						continue;
					}
				}

				if (!status[i])
					continue;

				points[1][k++] = points[1][i];
				circle(image, points[1][i], 3, Scalar(0, 255, 0), -1, 8);
			}
			points[1].resize(k);
		}

		if (addRemovePt && points[1].size() < (size_t)MAX_COUNT)
		{
			vector<Point2f> tmp;
			tmp.push_back(point);
			cornerSubPix(gray, tmp, winSize, Size(-1, -1), termcrit);
			points[1].push_back(tmp[0]);
			addRemovePt = false;
		}

		needToInit = false;
		imshow("LK Demo", image);

		char c = (char)waitKey(100);
		if (c == 27)
			break;
		switch (c)
		{
		case 'r':
			needToInit = true;
			break;
		case 'c':
			points[0].clear();
			points[1].clear();
			break;
		case 'n':
			nightMode = !nightMode;
			break;
		}

		//cout « points[0]« " " « points[1] « endl;


		/*for (int i = 0; i < points[2].size(); i++)
		{
		cout « points[1][i] « endl;
		}*/
		cout << "-----------------------------------------------------" << endl;

		int x_avrg = 0;
		int y_avrg = 0;
		if (points[1].size() > 20)
		{
			Point2f avgPt = Point(0, 0);
			for (Point2f &pt : points[1])
			{
				// cout « "x: " « pt.x «endl;
				// cout « "y: " « pt.y «endl;
				avgPt += pt;
			}
			avgPt.x /= points[1].size();
			avgPt.y /= points[1].size();

			cout << "x: " << avgPt.x << endl;
			cout << "y: " << avgPt.y << endl;
			int d_x = avgPt.x - 320;
			int d_y = avgPt.y - 240;
			int vect_mod = sqrt(d_x*d_x + d_y*d_y);
			int angle = atan2((float)d_y, (float)d_x)/(180*M_PI);
			unsigned char send_arr[6] = { 0,0,0,0,0,0 };
			if (vect_mod > 255)
			{
				for (int i = 1; i <=4; i++)
				{
					if (255 * i >= vect_mod)
					{
						send_arr[i - 1] = vect_mod - 255 * (i - 1);
						break;
					}
					else
					{
						send_arr[i - 1] = 255;
					}
				}
			}
			else
			{
				send_arr[0] = vect_mod;
			}//запись модуля вектора в массив
			send_arr[4] = abs(angle);//запись модуля значения угла в массив
			if (angle < 0)
				send_arr[5] = 2;//угол отрицательный
			else
				send_arr[5] = 1;//угол пложительный
			BT_device.bt_send(send_arr, 6);
		}
		else
		{
			cout <<"finding new points.." << endl;
			needToInit = true;
		}
		cout <<
			"----------" << endl;
		std::swap(points[1], points[0]);
		cv::swap(prevGray, gray);
	}

	return 0;
}