#pragma once
#ifndef _JUDGECODE_H_
#define _JUDGECODE_H_

typedef unsigned char uint8_t;

/**
 * @brief ͼƬ��ɫ
*/
enum
{
	ERROR = 0,
	RED,
	GREEN,
	YELLOW,
};

/**
 * @brief ͼ�����ݵĴ����ʽ
*/
enum
{
	NONE = 0,

	// YUV 4��2��0
	U_YUV_NV12,

};

/**
 * @brief �㶨��
*/
typedef struct tagPoint
{
	int x;
	int y;
}uPoint;

/**
 * @brief ROI��������
*/
typedef struct tagROI
{
	uPoint region[4];
}ROI;

/**
 * @brief ͼ��ķֱ���
*/
typedef struct tagImgResolution
{
	long width;
	long height;
}ImgResolution;

/**
 * @brief ����һ����ά����
*/
typedef struct tagScalar
{
	double v1;
	double v2;
	double v3;
}uScalar;

/**
 * @brief opencv��HCV��ɫ�ռ��ȡֵ��ΧΪ��
 * H [0, 179]  ɫ��
 * S [0, 255]  ���Ͷ�
 * V [0, 255]  ����
 * ����HSV��ɫ�ռ������ɫʶ��ʱ������ɫ��ȡֵ��Χ��
 *			Red			Yellow			Green		Orange
 * Hmin		0 | 156		26				35			11
 * Hmax		10| 180		34				77			25
 * Smin		43			43				43			43
 * Smax		255			255				255			255
 * Vmin		46			46				46			46
 * Vmax		255			255				255			255
*/
const uScalar yLower = { 0, 128, 128 };       // RGB��ɫ�ռ��ɫ�ĵ���ֵ
const uScalar yUpper = { 127, 255, 255 };     // RGB��ɫ�ռ��ɫ�ĸ���ֵ
const uScalar gLower = { 0, 128, 0 };         // RGB��ɫ�ռ���ɫ�ĵ���ֵ
const uScalar gUpper = { 127, 255, 127 };     // RGB��ɫ�ռ���ɫ�ĸ���ֵ
const uScalar rLower = { 0, 0, 128 };         // RGB��ɫ�ռ��ɫ�ĵ���ֵ
const uScalar rUpper = { 127, 127, 255 };     // RGB��ɫ�ռ��ɫ�ĸ���ֵ

const uScalar oHSVLower = { 11, 43, 46 };     // HSV��ɫ�ռ��ɫ�ĵ���ֵ
const uScalar oHSVUpper = { 25, 255, 255 };   // HSV��ɫ�ռ��ɫ�ĸ���ֵ
const uScalar yHSVLower = { 26, 43, 46 };     // HSV��ɫ�ռ��ɫ�ĵ���ֵ
const uScalar yHSVUpper = { 34, 255, 255 };   // HSV��ɫ�ռ��ɫ�ĸ���ֵ
const uScalar gHSVLower = { 35, 43, 46 };     // HSV��ɫ�ռ���ɫ�ĵ���ֵ
const uScalar gHSVUpper = { 77, 255, 255 };   // HSV��ɫ�ռ���ɫ�ĸ���ֵ
const uScalar rHSVLower = { 0, 43, 46 };      // HSV��ɫ�ռ��ɫ�ĵ���ֵ
const uScalar rHSVUpper = { 10, 255, 255 };   // HSV��ɫ�ռ��ɫ�ĸ���ֵ

/**
 * @brief ��������ɫ�ж�API
 * @param srcframe YUV420SP��ʽ��ͼƬ���ݣ�NV12��
 * @param resol ͼ��ֱ���
 * @param rect ��Ҫʶ����ɫ������
 * @param code ͼ�����ݵĴ����ʽ, Ĭ��ΪYUV420SP NV12
 * @return 1: Red Code, 2: Green Code, 3: Yellow Code, 0: Error
*/
int judgeCode(uint8_t* srcframe, ImgResolution& resol, ROI& rect, int code = U_YUV_NV12);

#endif // !_JUDGECODE_H_