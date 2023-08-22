

#ifndef _AVM_GPU_TABLE_H_
#define _AVM_GPU_TABLE_H_

#ifdef __cplusplus
extern "C" {
#endif


typedef struct Point3f		//����ģ�Ͷ���
{	
	double x;			//�����3D�ռ�����
	double y;
	double z;

	double alpha;		//ģ�Ͷ����alphaֵ(�����ں�)

	double Tex_x;		//��ͼ����
	double Tex_y;
}point;


/***********************************************************************

	��ͼ�����������������Ҫ����Ҫ����ͼ�ں�

		1. �ڷ��ں��������������������Ϣ��һ���ģ�ͬʱ����Ⱦ�У�Ҳֻ����һ���������Ϣ��
						���ж����alphaֵ����1

		2. ���ں��������������ֻ��λ����Ϣһ�£���ͼ���ꡢ��Ӧ����ͼ�ļ���alphaֵ����
						һ������Ҫ��ͬһ������λ�ý���������ͼ������˵������(��һ��������ϸ˵��)
						http://gamedev.stackexchange.com/questions/71127/opengl-how-to-draw-a-transition-of-two-textures-on-one-quad
***********************************************************************/
typedef struct Quad4p		//��ͼ����
{
	int PosID;			//��������λ��ID

	point LeftDown;		//������ĸ�����
	point RightDown;
	point LeftUp;
	point RightUp;

	point _LeftDown;		//������ĸ����㣬������ͼ�ں�(�ռ�����һ�£�����Ӧ����ͼ����ͼ���겻һ��)
	point _RightDown;
	point _LeftUp;
	point _RightUp;
}quad;

#ifdef __cplusplus
}
#endif


#endif

