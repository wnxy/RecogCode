#pragma once
#ifndef _JUDGECODE_H_
#define _JUDGECODE_H_

typedef unsigned char uint8_t;

/**
 * @brief 图片颜色
*/
enum
{
	ERROR = 0,
	RED,
	GREEN,
	YELLOW,
};

/**
 * @brief 图像数据的传输格式
*/
enum
{
	NONE = 0,

	// YUV 4：2：0
	U_YUV_NV12,

};

/**
 * @brief 点定义
*/
typedef struct tagPoint
{
	int x;
	int y;
}uPoint;

/**
 * @brief ROI区域坐标
*/
typedef struct tagROI
{
	uPoint region[4];
}ROI;

/**
 * @brief 图像的分辨率
*/
typedef struct tagImgResolution
{
	long width;
	long height;
}ImgResolution;

/**
 * @brief 定义一个三维标量
*/
typedef struct tagScalar
{
	double v1;
	double v2;
	double v3;
}uScalar;

/**
 * @brief opencv中HCV颜色空间的取值范围为：
 * H [0, 179]  色调
 * S [0, 255]  饱和度
 * V [0, 255]  明度
 * 利用HSV颜色空间进行颜色识别时常见颜色的取值范围：
 *			Red			Yellow			Green		Orange
 * Hmin		0 | 156		26				35			11
 * Hmax		10| 180		34				77			25
 * Smin		43			43				43			43
 * Smax		255			255				255			255
 * Vmin		46			46				46			46
 * Vmax		255			255				255			255
*/
const uScalar yLower = { 0, 128, 128 };       // RGB颜色空间黄色的低阈值
const uScalar yUpper = { 127, 255, 255 };     // RGB颜色空间黄色的高阈值
const uScalar gLower = { 0, 128, 0 };         // RGB颜色空间绿色的低阈值
const uScalar gUpper = { 127, 255, 127 };     // RGB颜色空间绿色的高阈值
const uScalar rLower = { 0, 0, 128 };         // RGB颜色空间红色的低阈值
const uScalar rUpper = { 127, 127, 255 };     // RGB颜色空间红色的高阈值

const uScalar oHSVLower = { 11, 43, 46 };     // HSV颜色空间橙色的低阈值
const uScalar oHSVUpper = { 25, 255, 255 };   // HSV颜色空间橙色的高阈值
const uScalar yHSVLower = { 26, 43, 46 };     // HSV颜色空间黄色的低阈值
const uScalar yHSVUpper = { 34, 255, 255 };   // HSV颜色空间黄色的高阈值
const uScalar gHSVLower = { 35, 43, 46 };     // HSV颜色空间绿色的低阈值
const uScalar gHSVUpper = { 77, 255, 255 };   // HSV颜色空间绿色的高阈值
const uScalar rHSVLower = { 0, 43, 46 };      // HSV颜色空间红色的低阈值
const uScalar rHSVUpper = { 10, 255, 255 };   // HSV颜色空间红色的高阈值

/**
 * @brief 健康码颜色判断API
 * @param srcframe YUV420SP格式的图片数据（NV12）
 * @param resol 图像分辨率
 * @param rect 需要识别颜色的区域
 * @param code 图像数据的传输格式, 默认为YUV420SP NV12
 * @return 1: Red Code, 2: Green Code, 3: Yellow Code, 0: Error
*/
int judgeCode(uint8_t* srcframe, ImgResolution& resol, ROI& rect, int code = U_YUV_NV12);

#endif // !_JUDGECODE_H_