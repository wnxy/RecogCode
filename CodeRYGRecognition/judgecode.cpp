#include "judgecode.h"
#include <cstring>
#include <algorithm>
#include <cmath>

#include <opencv.hpp>
#include <opencv2/imgproc/imgproc_c.h>

#include <iostream>

static inline int max(long x, long y)
{
	return x > y ? x : y;
}

static inline int maxThree(long a1, long a2, long a3)
{
	return max(a1, max(a2, a3));
}

static inline bool isRange(long low, long up, long t)
{
	if (t >= low && t <= up) return true;
	return false;
}

/**
 * @brief �жϸ����ı����Ƿ���ĳһ����Χ��
 * @param lower
 * @param upper
 * @param b
 * @return
*/
static bool isScalarRange(const uScalar& lower, const uScalar& upper, uScalar& b)
{
	if (isRange(lower.v1, upper.v1, b.v1) &&
		isRange(lower.v2, upper.v2, b.v2) &&
		isRange(lower.v3, upper.v3, b.v3))
	{
		return true;
	}
	return false;
}

/**
 * @brief �ж�p���Ƿ��ڸ�����������
 * ע�⣺�ú���ֻ�ܼ���p���Ƿ���һ��������͹�ı�����
 * @param rect
 * @param p
 * @return
*/
static bool isPointInROI(ROI& rect, uPoint& p)
{
	uPoint A = rect.region[0];
	uPoint B = rect.region[1];
	uPoint C = rect.region[2];
	uPoint D = rect.region[3];
	int val1 = (B.x - A.x) * (p.y - A.y) - (B.y - A.y) * (p.x - A.x);
	int val2 = (C.x - B.x) * (p.y - B.y) - (C.y - B.y) * (p.x - B.x);
	int val3 = (D.x - C.x) * (p.y - C.y) - (D.y - C.y) * (p.x - C.x);
	int val4 = (A.x - D.x) * (p.y - D.y) - (A.y - D.y) * (p.x - D.x);
	if ((val1 > 0 && val2 > 0 && val3 > 0 && val4 > 0) || (val1 < 0 && val2 < 0 && val3 < 0 && val4 < 0))
	{
		return true;
	}
	return false;
}

/**
 * @brief �ж���������������
 * @param val1 
 * @param val2 
 * @return 
*/
static bool isEquals(double val1, double val2)
{
	return fabs(val1 - val2) < 0.001;
}

/**
 * @brief ��BGR��ʽ������ת��ΪHSV��ʽ����
 * ת�������H [0, 360], S [0, 1], V [0, 1]
 * @param bgr 
 * @param hsv 
*/
static void BGR2HSV(BGR& bgr, HSV& hsv)
{
	double b, g, r;
	double h, s, v;
	double min, max;
	double delta;

	b = bgr.b / 255.0;
	g = bgr.g / 255.0;
	r = bgr.r / 255.0;

	if (r > g)
	{
		max = MAX(r, b);
		min = MIN(g, b);
	}
	else
	{
		max = MAX(g, b);
		min = MIN(r, b);
	}

	v = max;
	delta = max - min;

	if (isEquals(max, 0)) s = 0.0;
	else s = delta / max;

	if (max == min) h = 0.0;
	else
	{
		if (isEquals(r, max) && g >= b)
		{
			h = 60 * (g - b) / delta + 0;
		}
		else if (isEquals(r, max) && g < b)
		{
			h = 60 * (g - b) / delta + 360;
		}
		else if (isEquals(g, max))
		{
			h = 60 * (b - r) / delta + 120;
		}
		else if (isEquals(b, max))
		{
			h = 60 * (r - g) / delta + 240;
		}
	}

	hsv.h = (int)(h + 0.5);
	hsv.h = (hsv.h > 359) ? (hsv.h - 360) : hsv.h;
	hsv.h = (hsv.h < 0) ? (hsv.h + 360) : hsv.h;
	hsv.s = s;
	hsv.v = v;
}

/**
 * @brief BGR��ʽͼ������ת��ΪHSVͼ������
 * ת�������H [0, 180], S [0, 255], V [0, 255]
 * @param rgb 
 * @param hsv 
 * @param width 
 * @param height 
*/
void cvtColor_RGB2HSV(uint8_t* rgb, uint8_t* hsv, uint32_t width, uint32_t height)
{
	for (int i = 0; i < height; ++i)
	{
		for (int j = 0; j < width; ++j)
		{
			int index = i * width + j;
			BGR bgrPixel;
			bgrPixel.b = rgb[index * 3 + 0];
			bgrPixel.g = rgb[index * 3 + 1];
			bgrPixel.r = rgb[index * 3 + 2];

			HSV hsvPixel;
			BGR2HSV(bgrPixel, hsvPixel);
			hsv[index * 3 + 0] = (uint8_t)(hsvPixel.h / 2);
			hsv[index * 3 + 1] = (uint8_t)(hsvPixel.s * 255);
			hsv[index * 3 + 2] = (uint8_t)(hsvPixel.v * 255);
		}
	}
}

/**
 * @brief YUV420SP NV12ͼ������ת��ΪBGRͼ������
 * @param yuyv 
 * @param rgb 
 * @param width 
 * @param height 
*/
void cvtColor_YUV2BGR_NV12(uint8_t* yuyv, uint8_t* rgb, uint32_t width, uint32_t height)
{
	const int nv_start = width * height;
	int  index = 0, rgb_index = 0;
	uint8_t y, u, v;
	int r, g, b, nv_index = 0, i, j;

	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			//nv_index = (rgb_index / 2 - width / 2 * ((i + 1) / 2)) * 2;
			nv_index = i / 2 * width + j - j % 2;

			y = yuyv[rgb_index];
			u = yuyv[nv_start + nv_index];
			v = yuyv[nv_start + nv_index + 1];

			r = y + (140 * (v - 128)) / 100;  //r
			g = y - (34 * (u - 128)) / 100 - (71 * (v - 128)) / 100; //g
			b = y + (177 * (u - 128)) / 100; //b

			if (r > 255)   r = 255;
			if (g > 255)   g = 255;
			if (b > 255)   b = 255;
			if (r < 0)     r = 0;
			if (g < 0)     g = 0;
			if (b < 0)     b = 0;

			index = rgb_index % width + (height - i - 1) * width;
			//rgb[index * 3+0] = b;
			//rgb[index * 3+1] = g;
			//rgb[index * 3+2] = r;

			//�ߵ�ͼ��
			//rgb[height * width * 3 - i * width * 3 - 3 * j - 1] = b;
			//rgb[height * width * 3 - i * width * 3 - 3 * j - 2] = g;
			//rgb[height * width * 3 - i * width * 3 - 3 * j - 3] = r;

			//����ͼ��
			rgb[i * width * 3 + 3 * j + 0] = b;
			rgb[i * width * 3 + 3 * j + 1] = g;
			rgb[i * width * 3 + 3 * j + 2] = r;

			rgb_index++;
		}
	}
}

/**
 * @brief ����ƽ����ģֵ
 * @param srcframe Դͼ������BGR��ʽ
 * @param resol ͼ��ֱ���
 * @param maskROI ��ɫ����
 * @return BGR��ʽ��ģ
*/
uScalar calcMask(uint8_t* srcframe, ImgResolution& resol, ROI& maskROI)
{
	//std::cout << "calcMask() " << std::endl;
	if (srcframe == NULL)
	{
		return {0, 0, 0};
	}
	long b = 0, g = 0, r = 0, sum = 0;
	for (int i = 0; i < resol.height; ++i)
	{
		for (int j = 0; j < resol.width; ++j)
		{
			uPoint p = { j, i };
			if (isPointInROI(maskROI, p))
			{
				++sum;
				int index = i * resol.width + j;
				b += srcframe[index * 3 + 0];
				g += srcframe[index * 3 + 1];
				r += srcframe[index * 3 + 2];
			}
		}
	}
	uScalar bgr;
	bgr.v1 = 255 - (b / sum);     // Blue������ƽ��ֵ
	bgr.v2 = 255 - (g / sum);     // Green������ƽ��ֵ
	bgr.v3 = 255 - (r / sum);     // Red������ƽ��ֵ
	return bgr;
}

/**
 * @brief ȥ��BGRͼ���ض������е���ģ
 * @param rgb BGR��ʽͼ������
 * @param width ͼ���
 * @param height ͼ���
 * @param maskROI ROI����
 * @param mask BGR��ʽ��Ĥ
*/
void clearMask(uint8_t* rgb, uint32_t width, uint32_t height, ROI& rect, uScalar& mask)
{
	for (int i = 0; i < height; ++i)
	{
		for (int j = 0; j < width; ++j)
		{
			uPoint p = { j, i };
			if (isPointInROI(rect, p))
			{
				int index = i * width + j;
				rgb[index * 3 + 0] = (rgb[index * 3 + 0] + mask.v1) % 255;
				rgb[index * 3 + 1] = (rgb[index * 3 + 1] + mask.v2) % 255;
				rgb[index * 3 + 2] = (rgb[index * 3 + 2] + mask.v3) % 255;
			}
		}
	}
}

/**
 * @brief ��������ɫ�ж�API��c++ʵ��
 * @param srcframe YUV��ʽ��ͼƬ����
 * @param resol ͼ��ֱ���
 * @param rect ��Ҫʶ����ɫ������
 * @param code ͼ�����ݵĴ����ʽ��Ĭ��ΪYUV420SP NV12
 * @return 1: Red Code, 2: Green Code, 3: Yellow Code, 0: Error
*/
int ujudgeCode(uint8_t* srcframe, ImgResolution& resol, ROI& rect, int code)
{
	if (srcframe == NULL)
	{
		return ERROR;
	}
	const int yuvframesize = resol.width * resol.height * 3 / 2;
	const int rgbframesize = resol.width * resol.height * 3;
	const int hsvframesize = resol.width * resol.height * 3;
	
	uint8_t* rgbImg = new uint8_t[rgbframesize];
	memset(rgbImg, 0, rgbframesize);
	uint8_t* hsvImg = new uint8_t[hsvframesize];
	memset(hsvImg, 0, hsvframesize);
	switch (code)
	{
	case NONE:
	{
		return ERROR;
	}
	case U_YUV_NV12:
	{
		cvtColor_YUV2BGR_NV12(srcframe, rgbImg, resol.width, resol.height);
		//cv::Mat tmpImg;
		//tmpImg.create(resol.height * 3, resol.width, CV_8UC3);
		//memcpy(tmpImg.data, rgbImg, rgbframesize * sizeof(uint8_t));
		//cv::imshow("RGBͼ����ת��", tmpImg);
		//cv::waitKey(0);is
		break;
	}
	default:
		break;
	}

	cvtColor_RGB2HSV(rgbImg, hsvImg, resol.width, resol.height);
	//cv::Mat tmpImg1;
	//tmpImg1.create(resol.height * 3, resol.width, CV_8UC3);
	//memcpy(tmpImg1.data, hsvImg, rgbframesize * sizeof(uint8_t));
	//cv::imshow("HSVͼ����ת��", tmpImg1);
	//cv::waitKey(0);

	// �ֱ�ͳ�ƺ�ɫ���ص㡢��ɫ���ص㡢��ɫ���ص������
	long r_point = 0, g_point = 0, y_point = 0;

	int height = resol.height;
	int width = resol.width;
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			uPoint p = { j, i };
			int index = i * width + j;
			int h = (int)hsvImg[3 * index + 0];
			int s = (int)hsvImg[3 * index + 1];
			int v = (int)hsvImg[3 * index + 2];
			uScalar hsv = { h, s, v };
			if (isPointInROI(rect, p))
			{
				if (isScalarRange(yHSVLower, yHSVUpper, hsv) || isScalarRange(oHSVLower, oHSVUpper, hsv))        // �ж����ص��Ƿ�Ϊ��ɫ���߳�ɫ
				{
					++y_point;
				}
				else if (isScalarRange(gHSVLower, gHSVUpper, hsv))   // �ж����ص��Ƿ�Ϊ��ɫ
				{
					++g_point;
				}
				else if (isScalarRange(rHSVLower1, rHSVUpper1, hsv) || isScalarRange(rHSVLower2, rHSVUpper2, hsv))   // �ж����ص��Ƿ�Ϊ��ɫ
				{
					++r_point;
				}
			}
		}
	}
	//std::cout << "��ɫ���ص㣺" << y_point << std::endl;
	//std::cout << "��ɫ���ص㣺" << g_point << std::endl;
	//std::cout << "��ɫ���ص㣺" << r_point << std::endl;
	if (y_point + g_point + r_point == 0) return ERROR;
	else
	{
		int tmp = maxThree(y_point, g_point, r_point);
		if (y_point == tmp) return YELLOW;
		else if (g_point == tmp) return GREEN;
		else if (r_point == tmp) return RED;
		else return ERROR;
	}
}

/**
 * @brief ��������ɫ�ж�API��c++ʵ�֣���Ҫ�ṩ�Աȵ�ɫ
 * @param srcframe YUV��ʽ��ͼƬ����
 * @param resol ͼ��ֱ���
 * @param rect ��Ҫʶ����ɫ������
 * @param maskROI ��ɫ����
 * @param code ͼ�����ݵĴ����ʽ��Ĭ��ΪYUV420SP NV12
 * @return 1: Red Code, 2: Green Code, 3: Yellow Code, 0: Error
*/
int ujudgeCodeDMask(uint8_t* srcframe, ImgResolution& resol, ROI& rect, ROI& maskROI, int code)
{
	if (srcframe == NULL)
	{
		return ERROR;
	}
	const int yuvframesize = resol.width * resol.height * 3 / 2;
	const int rgbframesize = resol.width * resol.height * 3;
	const int hsvframesize = resol.width * resol.height * 3;

	uint8_t* rgbImg = new uint8_t[rgbframesize];
	memset(rgbImg, 0, rgbframesize);
	uint8_t* hsvImg = new uint8_t[hsvframesize];
	memset(hsvImg, 0, hsvframesize);
	switch (code)
	{
	case NONE:
	{
		return ERROR;
	}
	case U_YUV_NV12:
	{
		cvtColor_YUV2BGR_NV12(srcframe, rgbImg, resol.width, resol.height);
		//cv::Mat tmpImg;
		//tmpImg.create(resol.height * 3, resol.width, CV_8UC3);
		//memcpy(tmpImg.data, rgbImg, rgbframesize * sizeof(uint8_t));
		//cv::imshow("RGBͼ����ת��", tmpImg);
		//cv::waitKey(0);
		break;
	}
	default:
		break;
	}
	uScalar bgrMask = calcMask(rgbImg, resol, maskROI);
	//std::cout << "bgrMask = " << bgrMask.v1 << " " << bgrMask.v2 << " " << bgrMask.v3 << std::endl;
	clearMask(rgbImg, resol.width, resol.height, rect, bgrMask);
	cvtColor_RGB2HSV(rgbImg, hsvImg, resol.width, resol.height);
	//cv::Mat tmpImg1;
	//tmpImg1.create(resol.height * 3, resol.width, CV_8UC3);
	//memcpy(tmpImg1.data, rgbImg, rgbframesize * sizeof(uint8_t));
	//cv::imshow("HSVͼ����ת��", tmpImg1);
	//cv::waitKey(0);

	int height = resol.height;
	int width = resol.width;
	// ͳ�Ƽ�����ģ��ֵ

	// �ֱ�ͳ�ƺ�ɫ���ص㡢��ɫ���ص㡢��ɫ���ص������
	long r_point = 0, g_point = 0, y_point = 0;

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			uPoint p = { j, i };
			int index = i * width + j;
			int h = (int)hsvImg[3 * index + 0];
			int s = (int)hsvImg[3 * index + 1];
			int v = (int)hsvImg[3 * index + 2];
			uScalar hsv = { h, s, v };
			if (isPointInROI(rect, p))
			{
				if (isScalarRange(yHSVLower, yHSVUpper, hsv) || isScalarRange(oHSVLower, oHSVUpper, hsv))        // �ж����ص��Ƿ�Ϊ��ɫ���߳�ɫ
				{
					++y_point;
				}
				else if (isScalarRange(gHSVLower, gHSVUpper, hsv))   // �ж����ص��Ƿ�Ϊ��ɫ
				{
					++g_point;
				}
				else if (isScalarRange(rHSVLower1, rHSVUpper1, hsv) || isScalarRange(rHSVLower2, rHSVUpper2, hsv))   // �ж����ص��Ƿ�Ϊ��ɫ
				{
					++r_point;
				}
			}
		}
	}
	//std::cout << "��ɫ���ص㣺" << y_point << std::endl;
	//std::cout << "��ɫ���ص㣺" << g_point << std::endl;
	//std::cout << "��ɫ���ص㣺" << r_point << std::endl;
	if (y_point + g_point + r_point == 0) return ERROR;
	else
	{
		int tmp = maxThree(y_point, g_point, r_point);
		if (y_point == tmp) return YELLOW;
		else if (g_point == tmp) return GREEN;
		else if (r_point == tmp) return RED;
		else return ERROR;
	}
}