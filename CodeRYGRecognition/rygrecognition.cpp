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
const int framesize = width * height * 3 / 2;    // һ��YUVͼ��Ĵ�С

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

	//��ǰ֡���ļ��е�ƫ��������ǰindex * ÿһ֡�Ĵ�С
	fileOffset = (uint64_t)dwInFrameSize * frmIdx;
	//seek��ƫ�ƴ�
	result = _fseeki64(hInputYUVFile, fileOffset, SEEK_SET);
	if (result == -1)
	{
		return -1;
	}
	//�ѵ�ǰ֡��Y��U��V���ݷֱ��ȡ����Ӧ��������
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

	//��ǰ֡���ļ��е�ƫ��������ǰindex * ÿһ֡�Ĵ�С
	fileOffset = (uint64_t)dwInFrameSize * frmIdx;
	//seek��ƫ�ƴ�
	result = _fseeki64(hInputYUVFile, fileOffset, SEEK_SET);
	if (result == -1)
	{
		return -1;
	}
	//�ѵ�ǰ֡��Y��U��V���ݷֱ��ȡ����Ӧ��������
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
	printf("YUVͼƬ֡����%d\n", totalFrames);

	//namedWindow("yuv", 1);
	namedWindow("control", CV_WINDOW_NORMAL);
	int ctrl = 0;
	createTrackbar("ctrl", "control", &ctrl, 7);

	//����ÿһ֡YUV����
	for (int frm = 0; frm < totalFrames; frm++)
	{
		loadframe(yuv, infp, frm, 1920, 1080);
		//����yuv����
		printf("�ɹ���ȡYUVͼ��\n");

		Mat yuvImg;
		yuvImg.create(height * 3 / 2, width, CV_8UC1);
		memcpy(yuvImg.data, yuv, framesize * sizeof(uint8_t));
		Mat rgbImg;
		cvtColor(yuvImg, rgbImg, CV_YUV2BGR_NV12);

		imshow("yuvImg", yuvImg); //ֻ��ʾy����
		imshow("rgbImg", rgbImg);

		Mat imgHSV, imgBGR;
		Mat imgGThresholded, imgYThresholded, imgRThresholded, imgOThresholded;

		vector<Mat> hsvSplit;   //�����������������HSV����ͨ������
		cvtColor(rgbImg, imgHSV, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV
		imshow("hsvImg", imgHSV);

		inRange(imgHSV, Scalar(35, 43, 46), Scalar(77, 255, 255), imgGThresholded);   // HSV�����ɫ
		inRange(imgHSV, Scalar(0, 43, 46), Scalar(10, 255, 255), imgRThresholded);   // HSV����ɫ
		inRange(imgHSV, Scalar(26, 43, 46), Scalar(34, 255, 255), imgYThresholded);   // HSV����ɫ
		inRange(imgHSV, Scalar(11, 43, 46), Scalar(25, 255, 255), imgOThresholded);   // HSV����ɫ

		//split(imgHSV, hsvSplit);			//����ԭͼ���HSV��ͨ��
		//equalizeHist(hsvSplit[2], hsvSplit[2]);    //��HSV������ͨ������ֱ��ͼ����
		//merge(hsvSplit, imgHSV);				   //�ϲ�����ͨ��
		//cvtColor(imgHSV, imgBGR, COLOR_HSV2BGR);    //��HSV�ռ�ת����RGB�ռ䣬Ϊ����������ɫʶ����׼��

		//inRange(imgBGR, Scalar(0, 128, 0), Scalar(127, 255, 127), imgThresholded); //��ɫ
		//inRange(imgBGR, Scalar(128, 128, 128), Scalar(255, 255, 255), imgThresholded); //��ɫ
		//inRange(imgBGR, Scalar(0, 0, 70), Scalar(127, 127, 255), imgThresholded); //��ɫ
		/*switch (ctrl)
		{
		case 0:
		{
			inRange(imgBGR, Scalar(128, 0, 0), Scalar(255, 127, 127), imgThresholded); //��ɫ
			break;
		}
		case 1:
		{
			inRange(imgBGR, Scalar(128, 128, 128), Scalar(255, 255, 255), imgThresholded); //��ɫ
			break;
		}
		case 2:
		{
			inRange(imgBGR, Scalar(128, 128, 0), Scalar(255, 255, 127), imgThresholded); //��ɫ
			break;
		}
		case 3:
		{
			inRange(imgBGR, Scalar(128, 0, 128), Scalar(255, 127, 255), imgThresholded); //��ɫ
			break;
		}
		case 4:
		{
			inRange(imgBGR, Scalar(0, 128, 128), Scalar(127, 255, 255), imgThresholded); //��ɫ
			break;
		}
		case 5:
		{
			inRange(imgBGR, Scalar(0, 128, 0), Scalar(127, 255, 127), imgThresholded); //��ɫ
			break;
		}
		case 6:
		{
			inRange(imgBGR, Scalar(0, 0, 128), Scalar(127, 127, 255), imgThresholded); //��ɫ
			break;
		}
		case 7:
		{
			inRange(imgBGR, Scalar(0, 0, 0), Scalar(127, 127, 127), imgThresholded); //��ɫ
			break;
		}
		}*/

		imshow("��ɫʶ����", imgGThresholded);
		imshow("��ɫʶ����", imgRThresholded);
		imshow("��ɫʶ����", imgYThresholded);
		imshow("��ɫʶ����", imgOThresholded);

		//Mat element = getStructuringElement(MORPH_RECT, Size(5, 5));
		//morphologyEx(imgThresholded, imgThresholded, MORPH_OPEN, element);
		//morphologyEx(imgThresholded, imgThresholded, MORPH_CLOSE, element);

		//imshow("Thresholded Image", imgThresholded); //show the thresholded image
		//imshow("ֱ��ͼ�����Ժ�", imgBGR);
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
	printf("YUVͼƬ֡����%d\n", totalFrames);

	//����ÿһ֡YUV����
	for (int frm = 0; frm < totalFrames; frm++)
	{
		loadframe(yuv, infp, frm, 1920, 1080);
		//����yuv����
		printf("�ɹ���ȡYUVͼ��\n");

		ImgResolution resol = { 1920, 1080 };
		//ROI rect = { 0, 0, 0, 1910, 1000, 1910, 1000, 0 };
		ROI rect1 = { 1506, 196, 1620, 500, 1400, 775, 1130, 460 };   // 938ͼ���Ӧ��ROI����
		ROI maskROI = { 1258, 186, 1326, 218, 1262, 288, 1122, 240 };   // 938ͼ���Ӧ��maskROI����
		//cout << "judgeCode()���ý���� " << endl;
		//int res = judgeCode(yuv, resol, rect1);
		//cout << res << endl;
		cout << "ujudgeCode()���ý���� " << endl;
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