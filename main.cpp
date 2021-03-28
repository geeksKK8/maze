#define _CRT_SECURE_NO_WARNINGS
#include <cstdlib>
#include "gl/glut.h"
#include <cstdio>
#include <cstring>
#include <ctime>
#include <Windows.h>
#include <iostream>
#include <algorithm>
#include <mmsystem.h>
#pragma comment(lib,"winmm.lib")
using namespace std;

#define MAXN 40//地图数组的大小
#define MAX_TEXTURE 10//最多纹理数目
//#define MAP_SIZE (39)//迷宫大小
#define WinHeight 1080 //Display Height
#define WinWidth 1960 //Display Width
int MAP_SIZE = 5;

int ObjPrintLine = 0;
int status = 1;
float doorRotate1 = 0,doorRotate2 = 0;
GLint cubeList = 0;
int Map[MAXN][MAXN];
GLfloat light_color[] = { 0.5, 0.5, 0.5, 1 };
GLfloat light_pos[] = { MAP_SIZE / 2, 30, MAP_SIZE / 2, 1 };
GLfloat finalMaterialColor[] = { 0.5, 0.5, 0.5, 1 };
const float allone[] = { 0.2f, 0.2f, 0.2f, 1.0f };
const float allzero[] = { 0.0f, 0.0f, 0.0f, 0.0f };
#define INF 1e8f

int start[2];
int finish[2];

//Global Variable start and finish position
//Start : 1,0
//Finish : 1,n-1
//Attention : n must be an odd number

int InGame = 0;
int InMenu = 0;
int gameover = 0;
int MenuGameFit;
int fa[MAXN*MAXN];
int getfa(int x)
{
	if (x == fa[x]) return x;
	else return fa[x] = getfa(fa[x]);
}
void MakeMap(int a[][MAXN], int n)
{
	start[0] = 0;
	start[1] = 1;
	finish[0] = n - 1;
	finish[1] = n - 2;
	srand(time(0));
	int id[MAXN][MAXN], idx = 0;
	int edge[MAXN*MAXN][3], cnt = 0;
	if (!(n & 1)) n |= 1;
	for (int i = 0; i < n; ++i)
		for (int j = 0; j < n; ++j)
			a[i][j] = 0;
	memset(id, 0, sizeof id);
	for (int i = 0; i < n; ++i)
		for (int j = 0; j < n; ++j)
			if (!(i & 1) || !(j & 1)) a[i][j] = 1;
			else id[i][j] = ++idx;
	for (int i = 1; i <= idx; ++i) fa[i] = i;
	a[start[0]][start[1]] = 0;
	a[finish[0]][finish[1]] = 0;
	for (int i = 1; i < n - 1; ++i)
		for (int j = 1; j < n - 1; ++j)
			if (a[i][j] == 1 && ((i & 1) || (j & 1))) {
				edge[cnt][0] = i;
				edge[cnt][1] = j;
				edge[cnt][2] = rand();
				cnt++;
			}
	for (int i = 0; i < cnt; ++i)
		for (int j = i + 1; j < cnt; ++j)
			if (edge[j][2] < edge[i][2]) {
				swap(edge[j][2], edge[i][2]);
				swap(edge[j][1], edge[i][1]);
				swap(edge[j][0], edge[i][0]);
			}
	for (int i = 0; i < cnt; ++i) {
		int u, v;
		if (edge[i][0] & 1) {
			u = id[edge[i][0]][edge[i][1] - 1];
			v = id[edge[i][0]][edge[i][1] + 1];
		}
		else {
			u = id[edge[i][0] - 1][edge[i][1]];
			v = id[edge[i][0] + 1][edge[i][1]];
		}
		if (getfa(u) != getfa(v)) {
			fa[getfa(u)] = getfa(v);
			a[edge[i][0]][edge[i][1]] = 0;
		}
	}
}

#define EyeHeight 0.1
#define ViewDistance 40.0f
#define pi (acos(-1.0))
#define TriFloatPair pair<pair<float,float>,float>
#define TriIntPair pair<pair<int,int>,int>
#define QuadIntPair pair<pair<int,int>,pair<int,int>>
#define X first.first
#define Y first.second
#define Z second
#define P second.first
#define Q second.second

float eye[] = { 0.9, 0.1,0.5 };//人眼位置
const float gamestart[] = { 0.9, 0.1, 0.5 };
const float gamestartang = 3 * pi / 2;
float center[] = { MAP_SIZE / 2.0, 0, -MAP_SIZE / 2.0 };
const float viewstartang = 3 * pi / 2;
float viewstart[] = { center[0] - (float)cos(viewstartang) * ViewDistance, 5.0f, center[2] - (float)sin(viewstartang) * ViewDistance };
float fitstep[3];
float fitang;
float faceang = 3 * pi / 2;
float fStep = 0.03f;//移动步长
float angStep = 0.02f;//视角步长
float moveangStep = 0.1f;
float nurbsphase = 0.0f;
float boundry[3] = { -fStep * 3, 0, fStep * 3 };
float fTranslate = 0.2f;	//终点物体平移步长
float fScale = 1.0f;//终点物体缩放步长
float fRotate = 90;//终点物体旋转步长

int mainMenu;
int subMenu1, subMenu2, subMenu3, subMenu4, subMenu5;

float deltaA[] = { 52.0f + 8.0f - 10.5f + MAP_SIZE / 2, 4.0f , -MAP_SIZE / 2 }; int IDA = 2;
float deltaM[] = { 2.8f + 4.0f - 10.5f + MAP_SIZE / 2, 4.0f ,-MAP_SIZE / 2 }; int IDM = 14;
float deltaE[] = { 2.5f + 52.0f - 10.5f + MAP_SIZE / 2,4.0f ,-MAP_SIZE / 2 }; int IDE = 6;
float deltaZ[] = { -38.0f - 10.5f + MAP_SIZE / 2,4.0f ,-MAP_SIZE / 2 }; int IDZ = 27;
float deltaVoid[] = { 0.0f, 0.0f, 0.0f };
unsigned int texture[MAX_TEXTURE];
GLfloat ctlpoints[4][4][3];
int showPoints = 0;
GLUnurbsObj *theNurb;

void getobj(const char * filename, struct ObjFile * obj);
void getline(FILE * fin, char *s);

struct ObjFile {//OBJ文件读入区域
	char name[101][101];
	int st[101], ed[101]; //The Start of the surface ID
	int vc;
	int sc;
	int mc;
	TriFloatPair vertex[200001];
	QuadIntPair surface[200001];
}letter;

//读入24位bmp图，仅限图片宽度为4的倍数时可用
unsigned char *LoadBitmapFile(char *filename, BITMAPINFOHEADER *bitmapInfoHeader)
{
	FILE *filePtr;
	BITMAPFILEHEADER bitmapFileHeader;
	unsigned char	*bitmapImage;
	unsigned int	imageIdx = 0;
	unsigned char	tempRGB;

	filePtr = fopen(filename, "rb");
	if (filePtr == NULL) {
		printf("Error : Cant Open File %s\n", filename);
		return NULL;
	}
	fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);

	if (bitmapFileHeader.bfType != 0x4D42) {
		printf("Error in LoadBitmapFile: the file is not a bitmap file\n");
		return NULL;
	}

	fread(bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);
	fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);
	bitmapImage = new unsigned char[bitmapInfoHeader->biSizeImage];
	if (!bitmapImage) {
		printf("Error in LoadBitmapFile: memory error\n");
		return NULL;
	}

	fread(bitmapImage, 1, bitmapInfoHeader->biSizeImage, filePtr);
	if (bitmapImage == NULL) {
		printf("Error in LoadBitmapFile: memory error\n");
		return NULL;
	}

	for (imageIdx = 0; imageIdx < bitmapInfoHeader->biSizeImage; imageIdx += 3) {
		tempRGB = bitmapImage[imageIdx];
		bitmapImage[imageIdx] = bitmapImage[imageIdx + 2];
		bitmapImage[imageIdx + 2] = tempRGB;
	}
	fclose(filePtr);
	return bitmapImage;
}

//加载纹理
void texload(int i, char *filename)
{

	BITMAPINFOHEADER bitmapInfoHeader;
	unsigned char*   bitmapData;

	bitmapData = LoadBitmapFile(filename, &bitmapInfoHeader);
	glBindTexture(GL_TEXTURE_2D, texture[i]);
	// 指定当前纹理的放大/缩小过滤方式
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D,
		0, 	    //mipmap层次(通常为0，表示最上层)
		GL_RGB,	//我们希望该纹理有红、绿、蓝数据
		bitmapInfoHeader.biWidth, //纹理宽带，必须是n，若有边框+2
		bitmapInfoHeader.biHeight, //纹理高度，必须是n，若有边框+2
		0, //边框(0=无边框, 1=有边框)
		GL_RGB,	//bitmap数据的格式
		GL_UNSIGNED_BYTE, //每个颜色数据的类型
		bitmapData);	//bitmap数据指针
	printf("Load Over %s\n", filename);
}

//输出24位bmp文件，截图时使用
void WriteBitmapFile(char * filename, int width, int height, unsigned char * bitmapData)
{
	BITMAPFILEHEADER bitmapFileHeader;
	memset(&bitmapFileHeader, 0, sizeof(BITMAPFILEHEADER));
	bitmapFileHeader.bfSize = sizeof(BITMAPFILEHEADER);
	bitmapFileHeader.bfType = 0x4d42;	//BM
	bitmapFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	BITMAPINFOHEADER bitmapInfoHeader;
	memset(&bitmapInfoHeader, 0, sizeof(BITMAPINFOHEADER));
	bitmapInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitmapInfoHeader.biWidth = width;
	bitmapInfoHeader.biHeight = height;
	bitmapInfoHeader.biPlanes = 1;
	bitmapInfoHeader.biBitCount = 24;
	bitmapInfoHeader.biCompression = BI_RGB;
	bitmapInfoHeader.biSizeImage = width * abs(height) * 3;

	FILE * filePtr;
	unsigned char tempRGB;
	unsigned int imageIdx;

	for (imageIdx = 0; imageIdx < bitmapInfoHeader.biSizeImage; imageIdx += 3)
	{
		tempRGB = bitmapData[imageIdx];
		bitmapData[imageIdx] = bitmapData[imageIdx + 2];
		bitmapData[imageIdx + 2] = tempRGB;
	}

	filePtr = fopen(filename, "wb");
	if (filePtr == NULL) {
		printf("Cant open file %s\n", filename);
		return;
	}
	fwrite(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);
	fwrite(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);
	fwrite(bitmapData, bitmapInfoHeader.biSizeImage, 1, filePtr);
	fclose(filePtr);
	return;
}

//截图并且保存
void PrtScrn()
{
	static void * screenData;
	int len = WinHeight * WinWidth * 3;
	screenData = malloc(len);
	memset(screenData, 0, len);
	glReadPixels(0, 0, WinWidth, WinHeight, GL_RGB, GL_UNSIGNED_BYTE, screenData);

	time_t tm = 0;
	tm = time(NULL);
	char lpstrFilename[256] = { 0 };
	sprintf_s(lpstrFilename, sizeof(lpstrFilename), "%d.bmp", (int)tm);
	WriteBitmapFile(lpstrFilename, WinWidth, WinHeight, (unsigned char*)screenData);
	free(screenData);

}

//重置地图
void ResetMap()
{
	MakeMap(Map, MAP_SIZE);
}

//菜单初始化，未用到
void Initial_Menu()
{

}

void OutObjFile(char * s, ObjFile * letter)
{
	FILE * fin;
	fin = fopen(s, "w");
	for (int i = 1; i <= letter->vc; ++i) fprintf(fin, "v %.6f %.6f %.6f\n", letter->vertex[i].X, letter->vertex[i].Y, letter->vertex[i].Z);
	for (int i = 1; i <= letter->sc; ++i) fprintf(fin, "f %d %d %d %d\n", letter->surface[i].X, letter->surface[i].Y, letter->surface[i].P, letter->surface[i].Q);
	fclose(fin);
	fin = NULL;
}

//全程序初始化
void Initial()
{
	MakeMap(Map, MAP_SIZE);
	Initial_Menu();
	InGame = 0;
	gameover = 0;
	InMenu = 1;
	getobj("Letter.obj", &letter);
	glGenTextures(MAX_TEXTURE, texture); // 第一参数是需要生成标示符的个数, 第二参数是返回标示符的数组
	texload(0, "Water.bmp");
	texload(1, "Crack.bmp");
	texload(2, "Gold.bmp");
	texload(3, "Floor.bmp");
	texload(4, "Door.bmp");
	OutObjFile("Out.obj", &letter);
}

//画柱子并设置材质
void drawCube()
{
	int i, j;
	const GLfloat x1 = -0.5, x2 = 0.5;
	const GLfloat y1 = -0.5, y2 = 0.5;
	const GLfloat z1 = -0.5, z2 = 0.5;

	//指定六个面的四个顶点，每个顶点用3个坐标值表示  
	GLfloat point[6][4][3] = {
		{ { x1,y1,z1 },{ x2,y1,z1 },{ x2,y2,z1 },{ x1,y2,z1 } },
		{ { x1,y1,z1 },{ x2,y1,z1 },{ x2,y1,z2 },{ x1,y1,z2 } },
		{ { x2,y1,z1 },{ x2,y2,z1 },{ x2,y2,z2 },{ x2,y1,z2 } },
		{ { x1,y1,z1 },{ x1,y2,z1 },{ x1,y2,z2 },{ x1,y1,z2 } },
		{ { x1,y2,z1 },{ x2,y2,z1 },{ x2,y2,z2 },{ x1,y2,z2 } },
		{ { x1,y1,z2 },{ x2,y1,z2 },{ x2,y2,z2 },{ x1,y2,z2 } }
	};
	int dir[4][2] = { {0,0},{1,0},{1,1},{0,1} };
	//设置正方形绘制模式  

	glBegin(GL_QUADS);
	for (i = 0; i < 6; i++) {
		for (j = 0; j < 4; j++) {
			glTexCoord2iv(dir[j]);
			glVertex3fv(point[i][j]);
		}
	}
	glEnd();

}

//绘制门
void drawdoor()
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture[4]);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glPushMatrix();
	glTranslated(0.5, 0, -0.5);
	glRotatef(doorRotate1, 0, 1, 0);
	glTranslatef(0.5, 0, 0);
	glScalef(1, 1, 0.1);
	drawCube();
	glPopMatrix();

	glPushMatrix();
	glTranslated(0.5+MAP_SIZE-3, 0, -0.5-MAP_SIZE+1);
	glRotatef(doorRotate2, 0, 1, 0);
	glTranslatef(0.5, 0, 0);
	glScalef(1, 1, 0.1);
	drawCube();
	glPopMatrix();

	glDisable(GL_TEXTURE);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
}

//创建列表，在显示列表模式下绘制柱子
GLint GenCubeList()
{
	GLint lid = glGenLists(1);
	//创建柱子显示列表
	glNewList(lid, GL_COMPILE);

	glPushMatrix();
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, allone);
	glMaterialfv(GL_FRONT, GL_SPECULAR, allone);
	glMaterialfv(GL_FRONT, GL_SHININESS, allzero);
	drawCube();
	glPopMatrix();

	glEndList();
	return lid;
}

//显示列表模式柱子
void Draw_Cube_List()
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture[status]);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glPushMatrix();
	for (int i = 0; i < MAP_SIZE; i++) {
		for (int j = 0; j < MAP_SIZE; j++)
		{
			if (Map[i][j] == 1)
			{
				glCallList(cubeList);
			}
			glTranslatef(1, 0, 0);
		}
		glTranslatef(-MAP_SIZE, 0, -1);
	}
	glPopMatrix();
	glDisable(GL_TEXTURE);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture[3]);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glPushMatrix();
	//glTranslatef(0,0,MAP_SIZE);
	glTranslatef(0, -0.5, 0);
	glScalef(1, 0.1, 1);
	for (int i = 0; i < MAP_SIZE; i++) {
		for (int j = 0; j < MAP_SIZE; j++)
		{
			if (Map[i][j] == 0)
			{
				glCallList(cubeList);
			}
			glTranslatef(1, 0, 0);
		}
		glTranslatef(-MAP_SIZE, 0, -1);
	}
	glPopMatrix();
	glDisable(GL_TEXTURE);
}

//绘制终点处多面体
void draw_gameover()
{
	//圆柱
	glPushMatrix();
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, finalMaterialColor);
	glMaterialfv(GL_FRONT, GL_SPECULAR, finalMaterialColor);
	GLUquadric *pObj;
	pObj = gluNewQuadric();
	glTranslatef(0, 1, 0);
	glRotatef(90, 1, 0, 0);
	gluCylinder(pObj, 1, 1, 2, 32, 5);
	glPopMatrix();

	//球
	glPushMatrix();
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, finalMaterialColor);
	glMaterialfv(GL_FRONT, GL_SPECULAR, finalMaterialColor);
	glTranslatef(0, 1.7, 0);
	glutSolidSphere(0.6, 30, 30);
	glPopMatrix();

	//圆锥
	glPushMatrix();
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, finalMaterialColor);
	glMaterialfv(GL_FRONT, GL_SPECULAR, finalMaterialColor);
	glTranslatef(0, 2.3, 0);
	GLUquadric *pObj1;
	pObj1 = gluNewQuadric();
	glRotatef(-90, 1, 0, 0);
	gluCylinder(pObj, 0.7, 0, 1.5, 32, 5);
	glPopMatrix();
	
	//三棱柱
	glPushMatrix();
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, finalMaterialColor);
	glMaterialfv(GL_FRONT, GL_SPECULAR, finalMaterialColor);
	GLUquadric *pObj2;
	pObj2 = gluNewQuadric();
	glRotatef(90, 1, 0, 0);
	glRotatef(30, 0, 1, 0);
	glTranslatef(1.1, 0, 0);
	gluCylinder(pObj2, 0.5, 0.5, 1.2, 3, 5);
	glPopMatrix();

	glPushMatrix();
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, finalMaterialColor);
	glMaterialfv(GL_FRONT, GL_SPECULAR, finalMaterialColor);
	GLUquadric *pObj3;
	pObj3 = gluNewQuadric();
	glRotatef(90, 1, 0, 0);
	glRotatef(-30, 0, 1, 0);
	glTranslatef(-1.1, 0, 0);
	gluCylinder(pObj2, 0.5, 0.5, 1.2, 3, 5);
	glPopMatrix();

	//棱台
	glPushMatrix();
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, finalMaterialColor);
	glMaterialfv(GL_FRONT, GL_SPECULAR, finalMaterialColor);
	glTranslatef(0, -2, 0);
	GLUquadric *pObj4;
	pObj4 = gluNewQuadric();
	glRotatef(-90, 1, 0, 0);
	gluCylinder(pObj4, 1.5, 0.4, 1, 4, 5);
	glPopMatrix();
}

//重要
void reshape(int width, int height)
{
	if (height == 0)										// Prevent A Divide By Zero By
	{
		height = 1;										// Making Height Equal One
	}

	glViewport(0, 0, width, height);						// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix

	float whRatio = (GLfloat)width / (GLfloat)height;
	gluPerspective(40, whRatio, 0.1, 1000);
	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
}

//重要
void idle()
{
	glutPostRedisplay();
}

pair <int, int> FindPosition(double a, double b)
{
	pair <int, int> ret;
	ret.first = floor((a + 0.5) / 1.0);
	ret.second = floor((-b + 0.5) / 1.0);
	return ret;
}

//判断该位置是否离墙过近
bool IsWall(pair <int, int> pos)
{
	if (Map[pos.second][pos.first] == 0) return false;
	return true;
}

bool InFinish()
{
	pair <int, int> pos;
	pos = FindPosition(eye[0], eye[2]);
	if (pos.first == MAP_SIZE - 2 && pos.second == MAP_SIZE - 1) return true;
//	printf("now pos = %d %d\n", pos.first, pos.second);
	return false;
}

bool OutRange(double a, double b)
{
	if (a<-0.5 || a>MAP_SIZE - 0.5 || b > 0.61 || b < 0.5 - MAP_SIZE) return true;
	else return false;
}

//判断是否可以向该点移动
bool CanMove(double a, double b)
{
	pair <int, int> pos = FindPosition(a, b);
	for (int i = 0; i < 3; ++i)
		for (int j = 0; j < 3; ++j) {
			if (OutRange(a + boundry[i], b + boundry[j]) || IsWall(FindPosition(a + boundry[i], b + boundry[j]))) {
				return false;
			}
		}
	return true;
}

//键盘读入
void key(unsigned char k, int x, int y)
{
	if (MenuGameFit) return;
	switch (k)
	{
	case 27://esc
		exit(0);
		break;
	case 'q': {
		if (InGame || gameover) {
			MenuGameFit = 60;
			fitstep[0] = (viewstart[0] - eye[0]) / 60;
			fitstep[1] = (viewstart[1] - eye[1]) / 60;
			fitstep[2] = (viewstart[2] - eye[2]) / 60;
			float delta = INF, displace = 0;
			if (fabs(viewstartang - faceang) < delta) delta = fabs(viewstartang - faceang), displace = viewstartang - faceang;
			if (fabs(viewstartang - faceang + 2 * pi) < delta) delta = fabs(viewstartang - faceang), displace = viewstartang - faceang + 2 * pi;
			if (fabs(viewstartang - faceang - 2 * pi) < delta) delta = fabs(viewstartang - faceang), displace = viewstartang - faceang - 2 * pi;
			fitang = displace / 60;
			if (gameover) {
				eye[0] = viewstart[0];
				eye[1] = viewstart[1];
				eye[2] = viewstart[2];
				MenuGameFit = 0;
			}
			InGame = 0;
			gameover = 0;
			InMenu = 1;
		}
		break;
	}

	case 'a':
	{
		if (InGame) {
			if (CanMove(eye[0] + fStep * sin(faceang), eye[2] - fStep * cos(faceang))) {
				eye[0] += fStep * sin(faceang);
				eye[2] -= fStep * cos(faceang);
			}
			if (InFinish()) {
				InGame = 0;
				gameover = 1;
			}
		}
		break;
	}
	case 'd':
	{
		if (InGame) {
			if (CanMove(eye[0] - fStep * sin(faceang), eye[2] + fStep * cos(faceang))) {
				eye[0] -= fStep * sin(faceang);
				eye[2] += fStep * cos(faceang);
			}
			if (InFinish()) {
				InGame = 0;
				gameover = 1;
			}
		}
		break;
	}
	case 'w':
	{
		int mod = glutGetModifiers();
		if (InGame) {
			if (CanMove(eye[0] + fStep * cos(faceang), eye[2] + fStep * sin(faceang))) {
				if (mod == GLUT_ACTIVE_ALT)
				{
					eye[0] += 2 * fStep * cos(faceang);
					eye[2] += 2 * fStep * sin(faceang);
				}
				else
				{
					eye[0] += fStep * cos(faceang);
					eye[2] += fStep * sin(faceang);
				}
			}
			if (InFinish()) {
				InGame = 0;
				gameover = 1;
			}
		}
		break;
	}
	case 's':
	{
		int mod = glutGetModifiers();
		if (InGame) {
			if (CanMove(eye[0] - fStep * cos(faceang), eye[2] - fStep * sin(faceang))) {
				if (mod == GLUT_ACTIVE_ALT)
				{
					eye[0] -= 2 * fStep * cos(faceang);
					eye[2] -= 2 * fStep * sin(faceang);
				}
				else
				{
					eye[0] -= fStep * cos(faceang);
					eye[2] -= fStep * sin(faceang);
				}
			}
			if (InFinish()) {
				InGame = 0;
				gameover = 1;
			}
		}
		break;
	}
	case 'n':
	{
		if (InGame) {
			faceang -= moveangStep;
			while (faceang < 0) faceang += 2 * pi;
		}
		break;
	}
	case 'm':
	{
		if (InGame) {
			faceang += moveangStep;
			while (faceang > 2 * pi) faceang -= 2 * pi;
		}
		break;
	}
	case 'r':
	{
		if (!InGame)
			ResetMap();
		break;
	}
	case '-':
	{
		PrtScrn();
		break;
	}
	case 'i':
	{
		light_pos[1] += 1;
		break;
	}
	case 'k':
	{
		light_pos[1] -= 1;
		break;
	}
	case 'j':
	{
		light_pos[0] -= 1;
		break;
	}
	case 'l':
	{
		light_pos[0] += 1;
		break;
	}
	case 'u':
	{
		light_pos[2] += 1;
		break;
	}
	case 'o':
	{
		light_pos[2] -= 1;
		break;
	}
	case ' ':
	{
		if (InMenu) {
			MenuGameFit = 60;
			fitstep[0] = (gamestart[0] - eye[0]) / 60;
			fitstep[1] = (gamestart[1] - eye[1]) / 60;
			fitstep[2] = (gamestart[2] - eye[2]) / 60;
			float delta = INF, displace = 0;
			if (abs(gamestartang - faceang) < delta) delta = abs(gamestartang - faceang), displace = gamestartang - faceang;
			if (abs(gamestartang - faceang + 2 * pi) < delta) delta = abs(gamestartang - faceang), displace = gamestartang - faceang + 2 * pi;
			if (abs(gamestartang - faceang - 2 * pi) < delta) delta = abs(gamestartang - faceang), displace = gamestartang - faceang - 2 * pi;
			fitang = displace / 60;
			InMenu = 0;
			InGame = 1;
		}
		break;
	}
	case '=':
	{
		ObjPrintLine ^= 1;
	}
	default:
		printf("Get %d\n", k);
		break;
	}
}

//获取FPS值
void getFPS()
{
	static int frame = 0, time, timebase = 0;
	static char buffer[256];

	char mode[64];
	strcpy(mode, "display list");

	frame++;
	time = glutGet(GLUT_ELAPSED_TIME);
	if (time - timebase > 1000) {
		sprintf(buffer, "FPS:%4.2f %s",
			frame*1000.0 / (time - timebase), mode);
		timebase = time;
		frame = 0;
	}

	char *c;
	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);  // 选择投影矩阵
	glPushMatrix();               // 保存原矩阵
	glLoadIdentity();             // 装入单位矩阵
	glOrtho(0, WinWidth, 0, WinHeight, -1, 1);    // 位置正投影
	glMatrixMode(GL_MODELVIEW);   // 选择Modelview矩阵
	glPushMatrix();               // 保存原矩阵
	glLoadIdentity();             // 装入单位矩阵
	glRasterPos2f(10, 10);
	for (c = buffer; *c != '\0'; c++) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
	}
	glMatrixMode(GL_PROJECTION);  // 选择投影矩阵
	glPopMatrix();                // 重置为原保存矩阵
	glMatrixMode(GL_MODELVIEW);   // 选择Modelview矩阵
	glPopMatrix();                // 重置为原保存矩阵
	glEnable(GL_DEPTH_TEST);
}

//输出OBJ文件下的某一个模型
void OutPutObj(struct ObjFile * obj, int id, float * delta)
{
	glColor3f(1, 1, 0);
	for (int i = obj->st[id]; i <= obj->ed[id]; ++i) {
		if (ObjPrintLine) {
			glBegin(GL_LINES);
			glVertex3f(obj->vertex[obj->surface[i].X].X + delta[0], obj->vertex[obj->surface[i].X].Y + delta[1], obj->vertex[obj->surface[i].X].Z + delta[2]);
			glVertex3f(obj->vertex[obj->surface[i].Y].X + delta[0], obj->vertex[obj->surface[i].Y].Y + delta[1], obj->vertex[obj->surface[i].Y].Z + delta[2]);
			glVertex3f(obj->vertex[obj->surface[i].Y].X + delta[0], obj->vertex[obj->surface[i].Y].Y + delta[1], obj->vertex[obj->surface[i].Y].Z + delta[2]);
			glVertex3f(obj->vertex[obj->surface[i].P].X + delta[0], obj->vertex[obj->surface[i].P].Y + delta[1], obj->vertex[obj->surface[i].P].Z + delta[2]);
			glVertex3f(obj->vertex[obj->surface[i].P].X + delta[0], obj->vertex[obj->surface[i].P].Y + delta[1], obj->vertex[obj->surface[i].P].Z + delta[2]);
			glVertex3f(obj->vertex[obj->surface[i].Q].X + delta[0], obj->vertex[obj->surface[i].Q].Y + delta[1], obj->vertex[obj->surface[i].Q].Z + delta[2]);
			glVertex3f(obj->vertex[obj->surface[i].Q].X + delta[0], obj->vertex[obj->surface[i].Q].Y + delta[1], obj->vertex[obj->surface[i].Q].Z + delta[2]);
			glVertex3f(obj->vertex[obj->surface[i].X].X + delta[0], obj->vertex[obj->surface[i].X].Y + delta[1], obj->vertex[obj->surface[i].X].Z + delta[2]);
			glEnd();
		}
		else {
			glBegin(GL_POLYGON);
			glVertex3f(obj->vertex[obj->surface[i].X].X + delta[0], obj->vertex[obj->surface[i].X].Y + delta[1], obj->vertex[obj->surface[i].X].Z + delta[2]);
			glVertex3f(obj->vertex[obj->surface[i].Y].X + delta[0], obj->vertex[obj->surface[i].Y].Y + delta[1], obj->vertex[obj->surface[i].Y].Z + delta[2]);
			glVertex3f(obj->vertex[obj->surface[i].P].X + delta[0], obj->vertex[obj->surface[i].P].Y + delta[1], obj->vertex[obj->surface[i].P].Z + delta[2]);
			glVertex3f(obj->vertex[obj->surface[i].Q].X + delta[0], obj->vertex[obj->surface[i].Q].Y + delta[1], obj->vertex[obj->surface[i].Q].Z + delta[2]);
			glEnd();
		}
	}
}

//读取obj文件
void getobj(const char * filename, struct ObjFile * obj)
{
	FILE * fin;
	fin = fopen(filename, "r");
	if (fin == NULL) {
		printf("Error! Can't find the file\n");
	}

	obj->vc = 0;
	obj->sc = 0;
	obj->mc = 0;

	char buf[1 << 10];
	while (fscanf(fin, "%s", buf) != EOF) {
		switch (buf[0]) {
		case '#':
			getline(fin, buf);
			printf("The Message %s\n", buf);
			break;
		case 'g':
			getline(fin, obj->name[obj->mc + 1]);
			printf("Find a name %s\n", obj->name[obj->mc + 1]);
			obj->ed[obj->mc] = obj->sc;
			obj->st[obj->mc + 1] = obj->sc + 1;
			obj->mc++;
			break;
		case 'v':
			++(obj->vc);
			fscanf(fin, "%f", &((obj->vertex[obj->vc]).X));
			fscanf(fin, "%f", &((obj->vertex[obj->vc]).Y));
			fscanf(fin, "%f", &((obj->vertex[obj->vc]).Z));
			break;
		case 'f':
			++(obj->sc);
			fscanf(fin, "%d", &((obj->surface[obj->sc]).X));
			fscanf(fin, "%d", &((obj->surface[obj->sc]).Y));
			fscanf(fin, "%d", &((obj->surface[obj->sc]).P));
			fscanf(fin, "%d", &((obj->surface[obj->sc]).Q));
			break;
		}
	}
	obj->ed[obj->mc] = obj->sc;
}

//直接读取一行的字符串
void getline(FILE * fin, char *s)
{
	char ch;
	int idx = 0;
	fscanf(fin, "%c", &ch);
	while (ch != '\n' && ch != '\r' && ch != EOF) {
		s[idx++] = ch;
		fscanf(fin, "%c", &ch);
	}
	s[idx++] = '\0';
}

//渲染MAZE四个字母
void render_maze()
{
	glColor3f(0.0f, 0.0f, 1.0f);
	glPushMatrix();
	/*glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture[2]);*/

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	const float diffuse[] = { 0.247250, 0.224500, 0.064500, 1.000000 };
	const float ambient[] = { 0.346150, 0.314300, 0.090300, 1.000000 };
	const float specular[] = { 0.797357, 0.723991, 0.208006, 1.000000 };
	const float shininess[] = { 83.199997 };
	/*
		Meterial : Gold
		0.247250, 0.224500, 0.064500, 1.000000,
		0.346150, 0.314300, 0.090300, 1.000000,
		0.797357, 0.723991, 0.208006, 1.000000,
		83.199997,
	*/

	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
	OutPutObj(&letter, IDA, deltaA);
	OutPutObj(&letter, IDM, deltaM);
	OutPutObj(&letter, IDZ, deltaZ);
	OutPutObj(&letter, IDE, deltaE);

	//glDisable(GL_TEXTURE);
	glPopMatrix();
}

//游戏界面渲染
void render_game()
{
	if (InMenu) render_maze();
	Draw_Cube_List();
	drawdoor();
	if (InGame&&!MenuGameFit) {
		if (doorRotate1 < 90 && eye[2] < 0.45) {
			doorRotate1 += 1;
		}
		if (doorRotate2 < 90 && eye[2] < 0.45 - MAP_SIZE + 2) {
			doorRotate2 += 1;
		}
	}
	//	getFPS();
}

void init_surface()
{
	int u, v;
	for (u = 0; u < 4; u++) {
		for (v = 0; v < 4; v++) {
			ctlpoints[u][v][0] = MAP_SIZE / 2 + 2.0*((GLfloat)u - 1.5);
			ctlpoints[u][v][1] = 8.0f + sin((u + v)*7.0f + nurbsphase);
			ctlpoints[u][v][2] = -MAP_SIZE / 2 + 2.0*((GLfloat)v - 1.5);
		}
	}

	nurbsphase += 0.10f;
}

void render_nurbs()
{
	GLfloat knots[8] = { 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0 };
	init_surface();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture[2]);

	theNurb = gluNewNurbsRenderer();
	gluNurbsProperty(theNurb, GLU_SAMPLING_TOLERANCE, 25.0);
	gluNurbsProperty(theNurb, GLU_DISPLAY_MODE, GLU_FILL);
	gluBeginSurface(theNurb);
	glTexCoord2i(1, 1); glVertex3i(ctlpoints[3][3][0], ctlpoints[3][3][1], ctlpoints[3][3][2]);
	glTexCoord2i(1, 0); glVertex3i(ctlpoints[3][0][0], ctlpoints[3][0][1], ctlpoints[3][0][2]);
	glTexCoord2i(0, 0); glVertex3i(ctlpoints[0][0][0], ctlpoints[0][0][1], ctlpoints[0][0][2]);
	glTexCoord2i(0, 1); glVertex3i(ctlpoints[0][3][0], ctlpoints[0][3][1], ctlpoints[0][3][2]);
	gluNurbsSurface(theNurb,
		8, knots, 8, knots,
		4 * 3, 3, &ctlpoints[0][0][0],
		4, 4, GL_MAP2_VERTEX_3);
	gluEndSurface(theNurb);

	if (showPoints) {
		glPointSize(5.0);
		glDisable(GL_LIGHTING);
		glColor3f(1.0, 1.0, 0.0);
		glBegin(GL_POINTS);
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				glVertex3f(ctlpoints[i][j][0],
					ctlpoints[i][j][1], ctlpoints[i][j][2]);
			}
		}
		glEnd();
		glEnable(GL_LIGHTING);
	}
	glPopMatrix();
	glDisable(GL_TEXTURE);
}

//菜单界面渲染
void render_menu()//环绕迷宫进行观察
{
	faceang += angStep / 5;
	while (faceang > 2 * pi) faceang -= 2 * pi;
	eye[0] = center[0] - cos(faceang) * ViewDistance;
	eye[1] = 5;
	eye[2] = center[2] - sin(faceang) * ViewDistance;
	render_game();
	render_nurbs();
}

void render_gameover() {
	/*faceang += angStep / 5;
	while (faceang > 2 * pi) faceang -= 2 * pi;*/
	eye[0] = center[0] - cos(faceang) * ViewDistance;
	eye[1] = 5;
	eye[2] = center[2] - sin(faceang) * ViewDistance;

	glPushMatrix();
		glTranslatef(-8.0f, 0.0f,-6.0f);		
		glTranslatef(0.0f, fTranslate, 0.0f);			
		draw_gameover();						
	glPopMatrix();

	glPushMatrix();
		glTranslatef(0.0f, 0.0f,-6.0f);		
		glScalef(fScale, fScale, fScale);	
		draw_gameover();				
	glPopMatrix();

	glPushMatrix();
		glTranslatef(8.0f, 0.0f,-6.0f);		
		glRotatef(fRotate, 0, 1.0f, 0);
		draw_gameover();			
	glPopMatrix();

	fRotate	   += 3.0f;
	fTranslate += 0.03f;
	fScale     += 0.005f;

	if(fTranslate > 2.0f) fTranslate = -1.0f;
	if(fScale > 1.5f) fScale = 0.5f;
}
//视角转变
void FitLookAt()//渐变调整视角
{
	eye[0] += fitstep[0];
	eye[1] += fitstep[1];
	eye[2] += fitstep[2];
	faceang += fitang;
	while (faceang > 2 * pi) faceang -= 2 * pi;
	render_game();
}

//主渲染函数
void render()//主渲染入口
{
	//	printf("Now Postion = %f, %f, %f block %d, %d\n", eye[0], eye[1], eye[2],FindPosition(eye[0],eye[2]).first, FindPosition(eye[0],eye[2]).second);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0, 0, 0, 1);
	glLoadIdentity();									// Reset The Current Modelview Matrix
//	printf("Now %f, %f, %f\n", eye[0], eye[1], eye[2]);
	gluLookAt(eye[0], eye[1], eye[2],
		eye[0] + cos(faceang), eye[1], eye[2] + sin(faceang),
		0, 1, 0);				// 场景（0，0，0）的视点中心 (0, 5, 50)，Y轴向上

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);

	//开灯
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_color);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_color);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_color);
	glLightfv(GL_LIGHT0, GL_POSITION, light_pos);

	glEnable(GL_LIGHT0);

	if (MenuGameFit) {//渐变效果
		FitLookAt();
		MenuGameFit--;
	}
	else if (gameover) {//如果游戏结束
		render_gameover();
	}
	else if (InGame) {//如果在游戏当中
		render_game();
	}
	else if (InMenu) {//如果在目录上
		render_menu();
	}
	glutSwapBuffers();
}

//第一个子菜单响应函数
void submenufunc1(int data) {
	switch (data) {
	case 0:
		light_color[0] = 0.5;
		light_color[1] = 0.5;
		light_color[2] = 0.5;
		break;
	case 1:
		light_color[0] = 1;
		light_color[1] = 1;
		light_color[2] = 1;
		break;
	case 2:
		light_color[0] = 1;
		light_color[1] = 0;
		light_color[2] = 0;
		break;
	case 3:
		light_color[0] = 0;
		light_color[1] = 1;
		light_color[2] = 0;
		break;
	case 4:
		light_color[0] = 0;
		light_color[1] = 0;
		light_color[2] = 1;
		break;
	}
}

//第二个子菜单响应函数
void submenufunc2(int data) {
	//getcurrentmenu();
	switch (data) {
	case 1:
		glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1.0);
		break;
	case 2:
		glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0.5);
		break;
	case 3:
		glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0.33);
		break;
	case 4:
		glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0.25);
		break;
	case 5:
		glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0.1);
		break;
	}
}

//第三个子菜单响应函数
void submenufunc3(int data) {
	switch (data) {
	case 1:
		status = 0;
		break;
	case 2:
		status = 1;
		break;
	}
}

//第四个子菜单响应函数
void submenufunc4(int data) {
	switch (data) {
	case 1:
		if (!InGame) {
			MAP_SIZE = 5;
			deltaA[0] = 52.0f + 8.0f - 10.5f + MAP_SIZE / 2;
			deltaA[2] = -MAP_SIZE / 2;
			deltaM[0] = 2.8f + 4.0f - 10.5f + MAP_SIZE / 2;
			deltaM[2] = -MAP_SIZE / 2;
			deltaE[0] = 2.5f + 52.0f - 10.5f + MAP_SIZE / 2;
			deltaE[2] = -MAP_SIZE / 2;
			deltaZ[0] = -38.0f - 10.5f + MAP_SIZE / 2;
			deltaZ[2] = -MAP_SIZE / 2;
			center[0] = MAP_SIZE / 2;
			center[2] = -MAP_SIZE / 2;
			viewstart[0] = center[0] - (float)cos(viewstartang) * ViewDistance;
			viewstart[2] = center[2] - (float)sin(viewstartang) * ViewDistance;
			ResetMap();
		}
		break;
	case 2:
		if (!InGame) {
			MAP_SIZE = 13;
			deltaA[0] = 52.0f + 8.0f - 10.5f + MAP_SIZE / 2;
			deltaA[2] = -MAP_SIZE / 2;
			deltaM[0] = 2.8f + 4.0f - 10.5f + MAP_SIZE / 2;
			deltaM[2] = -MAP_SIZE / 2;
			deltaE[0] = 2.5f + 52.0f - 10.5f + MAP_SIZE / 2;
			deltaE[2] = -MAP_SIZE / 2;
			deltaZ[0] = -38.0f - 10.5f + MAP_SIZE / 2;
			deltaZ[2] = -MAP_SIZE / 2;
			center[0] = MAP_SIZE / 2;
			center[2] = -MAP_SIZE / 2;
			viewstart[0] = center[0] - (float)cos(viewstartang) * ViewDistance;
			viewstart[2] = center[2] - (float)sin(viewstartang) * ViewDistance;
			ResetMap();
		}
		break;
	case 3:
		if (!InGame) {
			MAP_SIZE = 21;
			deltaA[0] = 52.0f + 8.0f - 10.5f + MAP_SIZE / 2;
			deltaA[2] = -MAP_SIZE / 2;
			deltaM[0] = 2.8f + 4.0f - 10.5f + MAP_SIZE / 2;
			deltaM[2] = -MAP_SIZE / 2;
			deltaE[0] = 2.5f + 52.0f - 10.5f + MAP_SIZE / 2;
			deltaE[2] = -MAP_SIZE / 2;
			deltaZ[0] = -38.0f - 10.5f + MAP_SIZE / 2;
			deltaZ[2] = -MAP_SIZE / 2;
			center[0] = MAP_SIZE / 2;
			center[2] = -MAP_SIZE / 2;
			viewstart[0] = center[0] - (float)cos(viewstartang) * ViewDistance;
			viewstart[2] = center[2] - (float)sin(viewstartang) * ViewDistance;
			ResetMap();
		}
		break;
	case 4:
		if (!InGame) {
			MAP_SIZE = 29;
			deltaA[0] = 52.0f + 8.0f - 10.5f + MAP_SIZE / 2;
			deltaA[2] = -MAP_SIZE / 2;
			deltaM[0] = 2.8f + 4.0f - 10.5f + MAP_SIZE / 2;
			deltaM[2] = -MAP_SIZE / 2;
			deltaE[0] = 2.5f + 52.0f - 10.5f + MAP_SIZE / 2;
			deltaE[2] = -MAP_SIZE / 2;
			deltaZ[0] = -38.0f - 10.5f + MAP_SIZE / 2;
			deltaZ[2] = -MAP_SIZE / 2;
			center[0] = MAP_SIZE / 2;
			center[2] = -MAP_SIZE / 2;
			viewstart[0] = center[0] - (float)cos(viewstartang) * ViewDistance;
			viewstart[2] = center[2] - (float)sin(viewstartang) * ViewDistance;
			ResetMap();
		}
		break;
	case 5:
		if (!InGame) {
			MAP_SIZE = 37;
			deltaA[0] = 52.0f + 8.0f - 10.5f + MAP_SIZE / 2;
			deltaA[2] = -MAP_SIZE / 2;
			deltaM[0] = 2.8f + 4.0f - 10.5f + MAP_SIZE / 2;
			deltaM[2] = -MAP_SIZE / 2;
			deltaE[0] = 2.5f + 52.0f - 10.5f + MAP_SIZE / 2;
			deltaE[2] = -MAP_SIZE / 2;
			deltaZ[0] = -38.0f - 10.5f + MAP_SIZE / 2;
			deltaZ[2] = -MAP_SIZE / 2;
			center[0] = MAP_SIZE / 2;
			center[2] = -MAP_SIZE / 2;
			viewstart[0] = center[0] - (float)cos(viewstartang) * ViewDistance;
			viewstart[2] = center[2] - (float)sin(viewstartang) * ViewDistance;
			ResetMap();
		}
		break;
	}
}

//第五个子菜单响应函数
void submenufunc5(int data) {
	switch (data) {
	case 0:
		finalMaterialColor[0] = 0.5;
		finalMaterialColor[1] = 0.5;
		finalMaterialColor[2] = 0.5;
		break;
	case 1:
		finalMaterialColor[0] = 1;
		finalMaterialColor[1] = 1;
		finalMaterialColor[2] = 1;
		break;
	case 2:
		finalMaterialColor[0] = 1;
		finalMaterialColor[1] = 0;
		finalMaterialColor[2] = 0;
		break;
	case 3:
		finalMaterialColor[0] = 0;
		finalMaterialColor[1] = 1;
		finalMaterialColor[2] = 0;
		break;
	case 4:
		finalMaterialColor[0] = 0;
		finalMaterialColor[1] = 0;
		finalMaterialColor[2] = 1;
		break;
	}
}

//主菜单响应函数——目前暂无
void menufunc(int data) {

}

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(WinWidth, WinHeight);
	int windowHandle = glutCreateWindow("Maze");

	Initial();
	//构建子菜单1的内容
	subMenu1 = glutCreateMenu(submenufunc1);
	glutAddMenuEntry("Gray", 0);
	glutAddMenuEntry("White", 1);
	glutAddMenuEntry("Red", 2);
	glutAddMenuEntry("Green", 3);
	glutAddMenuEntry("Blue", 4);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	//构建子菜单2的内容
	subMenu2 = glutCreateMenu(submenufunc2);
	glutAddMenuEntry("弱", 1);
	glutAddMenuEntry("较弱", 2);
	glutAddMenuEntry("一般", 3);
	glutAddMenuEntry("较强", 4);
	glutAddMenuEntry("强", 5);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	//构建子菜单3的内容
	subMenu3 = glutCreateMenu(submenufunc3);
	glutAddMenuEntry("水波", 1);
	glutAddMenuEntry("裂缝", 2);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	//构建子菜单4的内容
	subMenu4 = glutCreateMenu(submenufunc4);
	glutAddMenuEntry("新手模式", 1);
	glutAddMenuEntry("简单模式", 2);
	glutAddMenuEntry("一般模式", 3);
	glutAddMenuEntry("困难模式", 4);
	glutAddMenuEntry("噩梦模式", 5);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	//构建子菜单5的内容
	subMenu5 = glutCreateMenu(submenufunc5);
	glutAddMenuEntry("Gray", 0);
	glutAddMenuEntry("White", 1);
	glutAddMenuEntry("Red", 2);
	glutAddMenuEntry("Green", 3);
	glutAddMenuEntry("Blue", 4);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	//构建主菜单的内容
	mainMenu = glutCreateMenu(menufunc);

	//将两个菜单变为主菜单的子菜单
	glutAddSubMenu("光源颜色", subMenu1);
	glutAddSubMenu("光源强度", subMenu2);
	glutAddSubMenu("墙体纹理", subMenu3);
	glutAddSubMenu("游戏难度", subMenu4);
	glutAddSubMenu("终点物体材质", subMenu5);
	//点击鼠标右键时显示菜单
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	//播放背景音乐
	PlaySound(TEXT(".\\rise.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
	glutDisplayFunc(render);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(key);
	glutIdleFunc(idle);
	cubeList = GenCubeList();
	glutMainLoop();
	return 0;
}