#include <iostream>
#include <vector>
#include "opencv2/highgui/highgui_c.h"
#include "opencv2/imgproc/imgproc_c.h"
#include <opencv2/opencv.hpp>

#include "judgecode.h"

using namespace cv;
using namespace std;

const int width = 1920;
const int height = 1080;
const int framesize = width * height * 3 / 2;    // 一张YUV图像的大小

FILE* infp;



int loadframe(uint8_t* yuvInput[3], FILE* hInputYUVFile, uint32_t frmIdx, uint32_t width, uint32_t height)
{
	uint64_t fileOffset;
	uint32_t result;
	uint32_t dwInFrameSize = 0;
	int anFrameSize[3] = {};

	dwInFrameSize = width * height * 3 / 2;
	anFrameSize[0] = width * height;
	anFrameSize[1] = anFrameSize[2] = width * height / 4;

	//当前帧在文件中的偏移量：当前index * 每一帧的大小
	fileOffset = (uint64_t)dwInFrameSize * frmIdx;
	//seek到偏移处
	result = _fseeki64(hInputYUVFile, fileOffset, SEEK_SET);
	if (result == -1)
	{
		return -1;
	}
	//把当前帧的Y、U、V数据分别读取到对应的数组中
	fread(yuvInput[0], anFrameSize[0], 1, hInputYUVFile);
	fread(yuvInput[1], anFrameSize[1], 1, hInputYUVFile);
	fread(yuvInput[2], anFrameSize[2], 1, hInputYUVFile);

	return 0;
}

int loadframe(uint8_t* yuvInput, FILE* hInputYUVFile, uint32_t frmIdx, uint32_t width, uint32_t height)
{
	uint64_t fileOffset;
	uint32_t result;
	uint32_t dwInFrameSize = 0;
	//int anFrameSize[3] = {};

	dwInFrameSize = width * height * 3 / 2;
	//anFrameSize[0] = width * height;
	//anFrameSize[1] = anFrameSize[2] = width * height / 4;

	//当前帧在文件中的偏移量：当前index * 每一帧的大小
	fileOffset = (uint64_t)dwInFrameSize * frmIdx;
	//seek到偏移处
	result = _fseeki64(hInputYUVFile, fileOffset, SEEK_SET);
	if (result == -1)
	{
		return -1;
	}
	//把当前帧的Y、U、V数据分别读取到对应的数组中
	fread(yuvInput, dwInFrameSize, 1, hInputYUVFile);

	return 0;
}

int openfile()
{
	errno_t err;
	err = fopen_s(&infp, "20220319121938&1920&1080&.yuv", "rb");
	if (!infp)
	{
		printf("open in file failed\n");
		return -1;
	}
	return 0;
}


int testOpenCV()
{
	openfile();
	//uint8_t* yuv[3];
	uint8_t* yuv = new uint8_t[framesize];
	int lumaPlaneSize, chromaPlaneSize;

	lumaPlaneSize = 1920 * 1080;
	chromaPlaneSize = lumaPlaneSize >> 2;

	//yuv[0] = new uint8_t[lumaPlaneSize];
	//yuv[1] = new uint8_t[chromaPlaneSize];
	//yuv[2] = new uint8_t[chromaPlaneSize];
	//memset(yuv[0], 0, lumaPlaneSize);
	//memset(yuv[1], 0, chromaPlaneSize);
	//memset(yuv[2], 0, chromaPlaneSize);
	memset(yuv, 0, framesize);

	uint64_t file_size = 0;

	_fseeki64(infp, 0, SEEK_END);
	file_size = _ftelli64(infp);
	_fseeki64(infp, 0, SEEK_SET);
	int totalFrames = file_size / (lumaPlaneSize + chromaPlaneSize + chromaPlaneSize);
	printf("YUV图片帧数：%d\n", totalFrames);

	//namedWindow("yuv", 1);
	namedWindow("control", CV_WINDOW_NORMAL);
	int ctrl = 0;
	createTrackbar("ctrl", "control", &ctrl, 7);

	//遍历每一帧YUV数据
	for (int frm = 0; frm < totalFrames; frm++)
	{
		loadframe(yuv, infp, frm, 1920, 1080);
		//处理yuv数据
		printf("成功读取YUV图像\n");

		Mat yuvImg;
		yuvImg.create(height * 3 / 2, width, CV_8UC1);
		memcpy(yuvImg.data, yuv, framesize * sizeof(uint8_t));
		Mat rgbImg;
		cvtColor(yuvImg, rgbImg, CV_YUV2BGR_NV12);

		imshow("yuvImg", yuvImg); //只显示y分量
		imshow("rgbImg", rgbImg);

		Mat imgHSV, imgBGR;
		Mat imgGThresholded, imgYThresholded, imgRThresholded, imgOThresholded;

		vector<Mat> hsvSplit;   //创建向量容器，存放HSV的三通道数据
		cvtColor(rgbImg, imgHSV, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV
		imshow("hsvImg", imgHSV);

		inRange(imgHSV, Scalar(35, 43, 46), Scalar(77, 255, 255), imgGThresholded);   // HSV检查绿色
		inRange(imgHSV, Scalar(0, 43, 46), Scalar(10, 255, 255), imgRThresholded);   // HSV检查红色
		inRange(imgHSV, Scalar(26, 43, 46), Scalar(34, 255, 255), imgYThresholded);   // HSV检查黄色
		inRange(imgHSV, Scalar(11, 43, 46), Scalar(25, 255, 255), imgOThresholded);   // HSV检查橙色

		//split(imgHSV, hsvSplit);			//分类原图像的HSV三通道
		//equalizeHist(hsvSplit[2], hsvSplit[2]);    //对HSV的亮度通道进行直方图均衡
		//merge(hsvSplit, imgHSV);				   //合并三种通道
		//cvtColor(imgHSV, imgBGR, COLOR_HSV2BGR);    //将HSV空间转回至RGB空间，为接下来的颜色识别做准备

		//inRange(imgBGR, Scalar(0, 128, 0), Scalar(127, 255, 127), imgThresholded); //绿色
		//inRange(imgBGR, Scalar(128, 128, 128), Scalar(255, 255, 255), imgThresholded); //白色
		//inRange(imgBGR, Scalar(0, 0, 70), Scalar(127, 127, 255), imgThresholded); //红色
		/*switch (ctrl)
		{
		case 0:
		{
			inRange(imgBGR, Scalar(128, 0, 0), Scalar(255, 127, 127), imgThresholded); //蓝色
			break;
		}
		case 1:
		{
			inRange(imgBGR, Scalar(128, 128, 128), Scalar(255, 255, 255), imgThresholded); //白色
			break;
		}
		case 2:
		{
			inRange(imgBGR, Scalar(128, 128, 0), Scalar(255, 255, 127), imgThresholded); //靛色
			break;
		}
		case 3:
		{
			inRange(imgBGR, Scalar(128, 0, 128), Scalar(255, 127, 255), imgThresholded); //紫色
			break;
		}
		case 4:
		{
			inRange(imgBGR, Scalar(0, 128, 128), Scalar(127, 255, 255), imgThresholded); //黄色
			break;
		}
		case 5:
		{
			inRange(imgBGR, Scalar(0, 128, 0), Scalar(127, 255, 127), imgThresholded); //绿色
			break;
		}
		case 6:
		{
			inRange(imgBGR, Scalar(0, 0, 128), Scalar(127, 127, 255), imgThresholded); //红色
			break;
		}
		case 7:
		{
			inRange(imgBGR, Scalar(0, 0, 0), Scalar(127, 127, 127), imgThresholded); //黑色
			break;
		}
		}*/

		imshow("绿色识别结果", imgGThresholded);
		imshow("红色识别结果", imgRThresholded);
		imshow("黄色识别结果", imgYThresholded);
		imshow("橙色识别结果", imgOThresholded);

		//Mat element = getStructuringElement(MORPH_RECT, Size(5, 5));
		//morphologyEx(imgThresholded, imgThresholded, MORPH_OPEN, element);
		//morphologyEx(imgThresholded, imgThresholded, MORPH_CLOSE, element);

		//imshow("Thresholded Image", imgThresholded); //show the thresholded image
		//imshow("直方图均衡以后", imgBGR);
		//imshow("Original", rgbImg); //show the original image

		waitKey(0);

	}

	fclose(infp);
	//cvDestroyWindow("yuv");

	return 0;
}



int testU()
{
	openfile();

	uint8_t* yuv = new uint8_t[framesize];
	int lumaPlaneSize, chromaPlaneSize;

	lumaPlaneSize = 1920 * 1080;
	chromaPlaneSize = lumaPlaneSize >> 2;

	memset(yuv, 0, framesize);

	uint64_t file_size = 0;

	_fseeki64(infp, 0, SEEK_END);
	file_size = _ftelli64(infp);
	_fseeki64(infp, 0, SEEK_SET);
	int totalFrames = file_size / (lumaPlaneSize + chromaPlaneSize + chromaPlaneSize);
	printf("YUV图片帧数：%d\n", totalFrames);

	//遍历每一帧YUV数据
	for (int frm = 0; frm < totalFrames; frm++)
	{
		loadframe(yuv, infp, frm, 1920, 1080);
		//处理yuv数据
		printf("成功读取YUV图像\n");

		ImgResolution resol = { 1920, 1080 };
		//ROI rect = { 0, 0, 0, 1910, 1000, 1910, 1000, 0 };
		ROI rect1 = { 1506, 196, 1620, 500, 1400, 775, 1130, 460 };   // 938图像对应的ROI区域
		ROI maskROI = { 1258, 186, 1326, 218, 1262, 288, 1122, 240 };   // 938图像对应的maskROI区域
		//cout << "judgeCode()调用结果： " << endl;
		//int res = judgeCode(yuv, resol, rect1);
		//cout << res << endl;
		cout << "ujudgeCode()调用结果： " << endl;
		int res = ujudgeCodeDMask(yuv, resol, rect1, maskROI);
		cout << res << endl;
	}
	return 0;
}

int main()
{
	testOpenCV();
	testU();
	return 0;
}