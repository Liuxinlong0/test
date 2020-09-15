#include <iostream>
#include<stdlib.h>
#include<stdio.h>
#include<string>
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<librealsense2/rs.hpp>

using namespace std;
using namespace cv;

struct threeFrame
{
	Mat frameNow;//灰度图
	Mat framePre;
	Mat framePrePre;
	Mat Diff1;
	Mat Diff2;
	Mat diffAnd;
	int startDiff = 0;
};

bool threeFrameDiff(Mat frameNow, threeFrame* frame);

bool threeFrameDiff(Mat frameNow,threeFrame* frame)
{
	cvtColor(frameNow, frameNow, CV_BGR2GRAY);
	if (frame->startDiff == 0)
	{
		frame->framePrePre = frameNow;
		frame->startDiff = 1;
	}
	else if (frame->startDiff == 1)
	{
		frame->framePre = frameNow;
		frame->startDiff = 2;
	}
	else if (frame->startDiff == 2)
	{
		frame->frameNow = frameNow;

		//三帧差法
		absdiff(frame->framePrePre, frame->framePre, frame->Diff1);  //帧差1
		absdiff(frame->framePre, frame->frameNow, frame->Diff2);     //帧差2

		threshold(frame->Diff1, frame->Diff1, 15, 255, CV_THRESH_BINARY);  //自适应阈值化
		threshold(frame->Diff2, frame->Diff2, 15, 255, CV_THRESH_BINARY);

		Mat element = getStructuringElement(0, Size(3, 3));  //膨胀核
		dilate(frame->Diff1, frame->Diff1, element);    //膨胀
		dilate(frame->Diff2, frame->Diff2, element);

		bitwise_and(frame->Diff1, frame->Diff2, frame->diffAnd);//与

		imshow("diffAnd", frame->diffAnd);

		frame->framePrePre = frame->framePre;
		frame->framePre = frame->frameNow;
		return true;
	}
	else
	{
		printf("What the fuck flag!");
		return false;
	}
}

int main()
{
	//声明realsense管道，
	rs2::pipeline pipe;
	//数据流配置信息【这步其实很重要】
	rs2::config pipe_config;
	pipe_config.enable_stream(RS2_STREAM_COLOR, 640, 480, RS2_FORMAT_BGR8, 30);
	//开始传送数据流
	rs2::pipeline_profile profile = pipe.start(pipe_config);

	char key = 0;
	threeFrame frame;

	while (key != 27) //Esc
	{
		rs2::frameset data = pipe.wait_for_frames();//等待下一帧
		rs2::frame color = data.get_color_frame();

		//获取宽高
		const int color_w = color.as<rs2::video_frame>().get_width();
		const int color_h = color.as<rs2::video_frame>().get_height();
		Mat color_image(Size(color_w, color_h), CV_8UC3, (void*)color.get_data(), Mat::AUTO_STEP);
		imshow("color_win", color_image);

		threeFrameDiff(color_image, &frame);

		key = cv::waitKey(20);
	}

	//cv destroy  
	cv::destroyAllWindows();

	return EXIT_SUCCESS;
}

