#include "judgecode.h"
#include <cstring>
#include <algorithm>
#include <opencv.hpp>
#include <opencv2/imgproc/imgproc_c.h>


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
 * @brief 判断给定的标量是否在某一个范围内
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
 * @brief 判断p点是否在给定的区域内
 * 注意：该函数只能计算p点是否在一个给定的凸四边形内
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
 * @brief 去除BGR图像特定区域中的掩模
 * @param rgb BGR格式图像数据
 * @param width 图像宽
 * @param height 图像高
 * @param maskROI ROI区域
 * @param mask BGR格式掩膜
*/
void clearMask(cv::Mat rgbImg, ROI& rect, uScalar& mask)
{
	int height = rgbImg.rows;
	int width = rgbImg.cols;
	for (int i = 0; i < height; ++i)
	{
		for (int j = 0; j < width; ++j)
		{
			uPoint p = { j, i };
			if (isPointInROI(rect, p))
			{
				int index = i * width + j;
				rgbImg.data[index * 3 + 0] = (rgbImg.data[index * 3 + 0] + mask.v1) % 255;
				rgbImg.data[index * 3 + 1] = (rgbImg.data[index * 3 + 1] + mask.v2) % 255;
				rgbImg.data[index * 3 + 2] = (rgbImg.data[index * 3 + 2] + mask.v3) % 255;
			}
		}
	}
}

/**
 * @brief 健康码颜色判断API，opencv实现，使用时需要安装opencv环境
 * @param srcframe YUV格式的图片数据
 * @param resol 图像分辨率
 * @param rect 需要识别颜色的区域
 * @param code 图像数据的传输格式，默认为YUV420SP NV12
 * @return 1: Red Code, 2: Green Code, 3: Yellow Code, 0: Error
*/
int judgeCode(uint8_t* srcframe, ImgResolution& resol, ROI& rect, int code)
{
	if (srcframe == NULL)
	{
		return ERROR;
	}
	const int framesize = resol.width * resol.height * 3 / 2;
	cv::Mat yuvImg;
	yuvImg.create(resol.height * 3 / 2, resol.width, CV_8UC1);
	memcpy(yuvImg.data, srcframe, framesize * sizeof(uint8_t));
	cv::Mat rgbImg, hsvImg;
	switch (code)
	{
	case NONE:
	{
		return ERROR;
	}
	case U_YUV_NV12:
	{
		cvtColor(yuvImg, rgbImg, CV_YUV2BGR_NV12);
		break;
	}
	default:
		break;
	}

	uScalar bgrMask = { 80, 120, 129 };
	clearMask(rgbImg, rect, bgrMask);
	cv::imshow("RGB图像去除掩膜", rgbImg);
	cv::waitKey(0);

	cvtColor(rgbImg, hsvImg, cv::ColorConversionCodes::COLOR_BGR2HSV);

	// 分别统计红色像素点、绿色像素点、黄色像素点的数量
	long r_point = 0, g_point = 0, y_point = 0;

	int height = rgbImg.rows;
	int width = rgbImg.cols;
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			uPoint p = { j, i };
			int index = i * width + j;
			int h = (int)hsvImg.data[3 * index + 0];
			int s = (int)hsvImg.data[3 * index + 1];
			int v = (int)hsvImg.data[3 * index + 2];
			uScalar hsv = { h, s, v };
			if (isPointInROI(rect, p))
			{
				if (isScalarRange(yHSVLower, yHSVUpper, hsv) || isScalarRange(oHSVLower, oHSVUpper, hsv))        // 判断像素点是否为黄色或者橙色
				{
					++y_point;
				}
				else if (isScalarRange(gHSVLower, gHSVUpper, hsv))   // 判断像素点是否为绿色
				{
					++g_point;
				}
				else if (isScalarRange(rHSVLower1, rHSVUpper1, hsv) || isScalarRange(rHSVLower2, rHSVUpper2, hsv))   // 判断像素点是否为红色
				{
					++r_point;
				}
			}
		}
	}
	std::cout << "黄色像素点：" << y_point << std::endl;
	std::cout << "绿色像素点：" << g_point << std::endl;
	std::cout << "红色像素点：" << r_point << std::endl;
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