// DrawSomething.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "stdlib.h"
#include <pthread.h>
#include <gl/freeglut.h>

#include <fstream>
#include <stdio.h>
#include <vector>
#include <map>

#include <string.h>
#define _USE_MATH_DEFINES
#include <math.h>

#include "ObjImporter.h"

using namespace std;

int win_width = 800;
int win_height = 600;

int windowId;
int  vboVertexHandle;
bool poseMode;
GLfloat camX, camY, camV, camZoom;

const int threadNum = 3;
pthread_mutex_t mutex;


vector<float>	sceneVertices;
vector<float>	sceneNormals;
vector<unsigned int>	sceneTriIndices;
vector<unsigned int>	sceneQuadIndices;

struct ObjImporter::SceneObject object;
float light0Pos[] = {0.0f, 0.0f, 0.0f, 0.0f};
float light1Pos[] = {0.0f, 0.0f, 0.0f, 0.0f};
float light2Pos[] = {0.0f, 0.0f, 0.0f, 0.0f};
	
float light0Col[] = {1.0f, 0.0f, 0.0f, 1.0f};
float light1Col[] = {0.0f, 1.0f, 0.0f, 1.0f};
float light2Col[] = {0.0f, 0.0f, 1.0f, 1.0f};


void init( void )
{
	printf( "OpenGL version: %s\n", (char*)glGetString(GL_VERSION));
	printf( "OpenGL renderer: %s\n", (char*)glGetString(GL_RENDERER));

	float ambient_color[]={0.5f, 0.58f, 0.5f, 1.0f};

	poseMode = false;
	camX = 40.0f;
	camY = 0.0f;
	camV = 0.0f;
	camZoom = 40.0f;

	glLightfv(GL_LIGHT0, GL_POSITION, light0Pos);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light0Col);
	glLightfv(GL_LIGHT1, GL_POSITION, light1Pos);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light1Col);
	glLightfv(GL_LIGHT2, GL_POSITION, light2Pos);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, light2Col);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient_color);

	glShadeModel(GL_SMOOTH);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHT2);
	glEnable(GL_DEPTH_TEST);
}

void initThreads()
{
    ////initialize mutex
	pthread_mutex_init(&mutex,0);

}

void timer(int value)
{
	//update();
	glutPostRedisplay();
	glutTimerFunc(1000/60, timer, windowId);
}

void display( void )
{
	glClearColor(0,0,0,1);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();
	gluLookAt(camX, camV, camY, 0, 0, 0, 0, 1.0f, 0);

	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);

	//glNormalPointer(GL_FLOAT, 0, &object.normals[0]);// &sceneNormals[0]);
	//glVertexPointer(3, GL_FLOAT, 0, &object.vertices[0]);// &sceneVertices[0]);

	//if( object.triIndices->size() /*sceneTriIndices.size()*/ > 0)
	//	glDrawElements(GL_TRIANGLES, object.triIndices->size() /* sceneTriIndices.size()*/, GL_UNSIGNED_INT, &object.triIndices[0]); //&sceneTriIndices[0]);

	//if( object.quadIndices->size() /*sceneQuadIndices.size()*/ > 0)
	//	glDrawElements(GL_QUADS, object.quadIndices->size() /*sceneQuadIndices.size()*/, GL_UNSIGNED_INT, &object.quadIndices[0]); //&sceneQuadIndices[0]);
		
	glNormalPointer(GL_FLOAT, 0, &sceneNormals[0]);
	glVertexPointer(3, GL_FLOAT, 0, &sceneVertices[0]);

	if( sceneTriIndices.size() > 0)
		glDrawElements(GL_TRIANGLES, sceneTriIndices.size(), GL_UNSIGNED_INT, &sceneTriIndices[0]);

	if( sceneQuadIndices.size() > 0)
		glDrawElements(GL_QUADS, sceneQuadIndices.size(), GL_UNSIGNED_INT, &sceneQuadIndices[0]);


	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

	
	//glutSolidSphere(10.0f,16,16);
	glutSolidCube(10.0f);
	glutSwapBuffers();
}

void reshape( int w, int h ){
	win_width = w;
	win_height = h;
	glViewport( 0, 0, w, h );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	gluPerspective(45.0,(GLdouble)win_width/(GLdouble)win_height,0.1,1000.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glutPostRedisplay();
}

void RotateCam(int x, int y){
	float spinH = (2 * M_PI * x) / win_width;
	float spinV = (M_PI * y) / win_height;
	camX = camZoom * cos(spinH);
	camY = camZoom * sin(spinH);
	camV = camZoom * cos(spinV);
}

void UpdateOnMouseClick(int button, int state, int x, int y){
	if(poseMode)
		RotateCam(x, y);
}

void UpdateOnMouseMove(int x, int y){
	if(!poseMode)
		RotateCam(x, y);
}

void keyboard( unsigned char key, int x, int y ) {
	switch(key) {
		case 'q':
			poseMode = true;
			break;
		case 'e':
			poseMode = false;
			break;
		case 'w':
			camZoom -= 5.0f;
			break;
		case 's':
			camZoom += 5.0f;
			break;
		case 27: // Escape key
			exit(0);
			break;
	}
}


int main( int argc, char** argv ){

	glutInit( &argc, argv );
	glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH|GLUT_ALPHA);
	glutInitWindowSize (win_width, win_height);
	glutInitWindowPosition (100, 100);
	windowId = glutCreateWindow( "Draw Something" );
	glutSetWindow(windowId);

	glutTimerFunc(1000/60,timer,windowId);
	glutDisplayFunc( display );
	glutReshapeFunc( reshape );
	glutKeyboardFunc( keyboard );
	glutPassiveMotionFunc( UpdateOnMouseMove );
	glutMouseFunc( UpdateOnMouseClick );

	init();
	initThreads();
	ObjImporter importer;

	char* fileName;
	if(argc ==2){
		fileName = argv[1];
	}else{
		fileName = "./bulldog.obj"; //"F:\\Stanford\\Senior\\CS 248\\HW1\\DrawSomething\\DrawSomething\\bulldog.obj";
	}
	bool imported = importer.ImportObj(fileName, object);
	if(!imported)
		return -1;

	// Temporary measures to fix memory access violation in display ://
	for(int i = 0; i < object.vertices->size(); i++)
		sceneVertices.push_back((*object.vertices)[i]);
	for(int i = 0; i < object.normals->size(); i++)
		sceneNormals.push_back((*object.normals)[i]);
	for(int i = 0; i < object.triIndices->size(); i++)
		sceneTriIndices.push_back((*object.triIndices)[i]);
	for(int i = 0; i < object.quadIndices->size(); i++)
		sceneQuadIndices.push_back((*object.quadIndices)[i]);


	glutMainLoop();

	return 0;
}