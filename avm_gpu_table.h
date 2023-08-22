

#ifndef _AVM_GPU_TABLE_H_
#define _AVM_GPU_TABLE_H_

#ifdef __cplusplus
extern "C" {
#endif


typedef struct Point3f		//球体模型顶点
{	
	double x;			//顶点的3D空间坐标
	double y;
	double z;

	double alpha;		//模型顶点的alpha值(用于融合)

	double Tex_x;		//贴图坐标
	double Tex_y;
}point;


/***********************************************************************

	绘图方格包含两个方格，主要是需要做贴图融合

		1. 在非融合区域：两个方格的所有信息是一样的，同时在渲染中，也只用了一个方格的信息。
						所有顶点的alpha值都是1

		2. 在融合区域：两个方格的只有位置信息一致，贴图坐标、对应的贴图文件、alpha值都不
						一样，需要在同一个方格位置进行两次贴图。具体说明见：(第一个答案有详细说明)
						http://gamedev.stackexchange.com/questions/71127/opengl-how-to-draw-a-transition-of-two-textures-on-one-quad
***********************************************************************/
typedef struct Quad4p		//绘图方格
{
	int PosID;			//方格所处位置ID

	point LeftDown;		//方格的四个顶点
	point RightDown;
	point LeftUp;
	point RightUp;

	point _LeftDown;		//方格的四个顶点，用于贴图融合(空间坐标一致，但对应的贴图和贴图坐标不一致)
	point _RightDown;
	point _LeftUp;
	point _RightUp;
}quad;

#ifdef __cplusplus
}
#endif


#endif

