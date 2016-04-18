#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include <GL/glew.h>
#include <freeglut/glut.h>
#include "textfile.h"
#include "glm.h"

#include "Matrices.h"

#pragma comment (lib, "glew32.lib")
#pragma comment (lib, "freeglut.lib")

#ifndef GLUT_WHEEL_UP
# define GLUT_WHEEL_UP   0x0003
# define GLUT_WHEEL_DOWN 0x0004
#endif

#ifndef GLUT_KEY_ESC
# define GLUT_KEY_ESC 0x001B
#endif

#ifndef max
# define max(a,b) (((a)>(b))?(a):(b))
# define min(a,b) (((a)<(b))?(a):(b))
#endif

// Shader attributes
GLint iLocPosition;
GLint iLocColor;
GLint iLocMVP;

char filename[] = "ColorModels/bunny5KC.obj";
GLMmodel* OBJ;
GLfloat* vertices;
GLfloat* colors;
struct Model{
	std::vector<GLfloat> v;
	std::vector<GLfloat> c;
	GLfloat maxVal[3];
	GLfloat minVal[3];
	GLint tNum, vNum;
};
Model model_info[5];
void traverseColorModel(int index)
{
	int i;

	// TODO:
	//// You should traverse the vertices and the colors of each triangle, and 
	//// then normalize the model to unit size by using transformation matrices. 
	//// i.e. Each vertex should be bounded in [-1, 1], which will fit the camera clipping window.


	// number of triangles
	model_info[index].tNum = OBJ->numtriangles;

	// number of vertices
	model_info[index].vNum = OBJ->numvertices;

	// The center position of the model 
	OBJ->position[0] = 0;
	OBJ->position[1] = 0;
	OBJ->position[2] = 0;

	// initialize the min and max value.
	for(int i=0;i<3;i++){
		model_info[index].maxVal[i] = -1e9;
		model_info[index].minVal[i] = 1e9;
	}

	for(i=0; i<(int)OBJ->numtriangles; i++)
	{
		// the index of each vertex
		int indv1 = OBJ->triangles[i].vindices[0];
		int indv2 = OBJ->triangles[i].vindices[1];
		int indv3 = OBJ->triangles[i].vindices[2];

		// the index of each color
		int indc1 = indv1;
		int indc2 = indv2;
		int indc3 = indv3;

		// vertices
		GLfloat vx[3], vy[3], vz[3];
		vx[0] = OBJ->vertices[indv1*3+0];
		vy[0] = OBJ->vertices[indv1*3+1];
		vz[0] = OBJ->vertices[indv1*3+2];

		vx[1] = OBJ->vertices[indv2*3+0];
		vy[1] = OBJ->vertices[indv2*3+1];
		vz[1] = OBJ->vertices[indv2*3+2];

		vx[2] = OBJ->vertices[indv3*3+0];
		vy[2] = OBJ->vertices[indv3*3+1];
		vz[2] = OBJ->vertices[indv3*3+2];
		for (int j=0;j<3;j++){
			if(vx[j] > model_info[index].maxVal[0]) model_info[index].maxVal[0] = vx[j];
			if(vx[j] < model_info[index].minVal[0]) model_info[index].minVal[0] = vx[j];
			if(vy[j] > model_info[index].maxVal[1]) model_info[index].maxVal[1] = vy[j];
			if(vy[j] < model_info[index].minVal[1]) model_info[index].minVal[1] = vy[j];
			if(vz[j] > model_info[index].maxVal[2]) model_info[index].maxVal[2] = vz[j];
			if(vz[j] < model_info[index].minVal[2]) model_info[index].minVal[2] = vz[j];
			model_info[index].v.push_back(vx[j]); model_info[index].v.push_back(vy[j]);
			model_info[index].v.push_back(vz[j]); //model_info[index].v.push_back(1);
		}
		// colors
		GLfloat c1[3], c2[3], c3[3];
		c1[0] = OBJ->colors[indv1*3+0];
		c2[0] = OBJ->colors[indv1*3+1];
		c3[0] = OBJ->colors[indv1*3+2];

		c1[1] = OBJ->colors[indv2*3+0];
		c2[1] = OBJ->colors[indv2*3+1];
		c3[1] = OBJ->colors[indv2*3+2];

		c1[2] = OBJ->colors[indv3*3+0];
		c2[2] = OBJ->colors[indv3*3+1];
		c3[2] = OBJ->colors[indv3*3+2];
		for(int j=0;j<3;j++){
			model_info[index].c.push_back(c1[j]); model_info[index].c.push_back(c2[j]);
			model_info[index].c.push_back(c3[j]);
		}
	}
}

void loadOBJModel(char *filename, int index)
{
	// read an obj model here
	if(OBJ != NULL){
		free(OBJ);
	}
	OBJ = glmReadOBJ(filename);
	printf("%s\n", filename);

	// traverse the color model
	traverseColorModel(index);
}

void onIdle()
{
	glutPostRedisplay();
}

void onDisplay(void)
{
	// clear canvas
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnableVertexAttribArray(iLocPosition);
	glEnableVertexAttribArray(iLocColor);

	// organize the arrays
	static GLfloat triangle_color[] = {
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f
	};

	static GLfloat triangle_vertex[] = {
		 1.0f, -1.0f, 0.0f,
		 0.0f,  1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f
	};

	// TODO:
	//// Please define the model transformation matrix, viewing transformation matrix, 
	//// projection transformation matrix

	//MVP
	for(int index = 0;index < 1;index++){
		// the current center of the model we load in
		GLfloat model_centerx = (model_info[index].maxVal[0]+model_info[index].minVal[0])/2;
		GLfloat model_centery = (model_info[index].maxVal[1]+model_info[index].minVal[1])/2;
		GLfloat model_centerz = (model_info[index].maxVal[2]+model_info[index].minVal[2])/2;
		// the size of each axis
		GLfloat sx = fabs(model_info[index].maxVal[0]-model_info[index].minVal[0]);
		GLfloat sy = fabs(model_info[index].maxVal[1]-model_info[index].minVal[1]);
		GLfloat sz = fabs(model_info[index].maxVal[2]-model_info[index].minVal[2]);

		GLfloat scale_factor = 2/max(sx, max(sy,sz));

		Matrix4 T;
		Matrix4 S;
		Matrix4 R;
		Matrix4 scale_matrix = Matrix4(
										scale_factor, 0, 0, 0,
										0, scale_factor, 0, 0,
										0, 0, scale_factor, 0,
										0, 0, 0, 1
										);
		Matrix4 M = Matrix4(
							1, 0, 0, -model_centerx, 
							0, 1, 0, -model_centery,
							0, 0, 1, -model_centerz,
							0, 0, 0, 1);
		// calculate the final model matrix
		M = scale_matrix*M;

		Matrix4 V = Matrix4(
							1, 0, 0, 0, 
							0, 1, 0, 0,
							0, 0, 1, 0,
							0, 0, 0, 1);
		Matrix4 P = Matrix4(
							1, 0, 0, 0, 
							0, 1, 0, 0,
							0, 0, -1, 0,
							0, 0, 0, 1);

		Matrix4 MVP = P*V*M;

		GLfloat mvp[16];
		// row-major ---> column-major
		mvp[0] = MVP[0];  mvp[4] = MVP[1];   mvp[8]  = MVP[2];    mvp[12] = MVP[3];  
		mvp[1] = MVP[4];  mvp[5] = MVP[5];   mvp[9]  = MVP[6];    mvp[13] = MVP[7];  
		mvp[2] = MVP[8];  mvp[6] = MVP[9];   mvp[10] = MVP[10];   mvp[14] = MVP[11];  
		mvp[3] = MVP[12]; mvp[7] = MVP[13];  mvp[11] = MVP[14];   mvp[15] = MVP[15];

		// bind array pointers to shader
		GLfloat *v = new GLfloat[model_info[index].v.size()];
		for(int i=0;i<(int)model_info[index].v.size();i++){
			v[i] = model_info[index].v[i];
		}
		GLfloat *c = new GLfloat[model_info[index].c.size()];
		for(int i=0;i<(int)model_info[index].c.size();i++){
			c[i] = model_info[index].c[i];
		}
		glVertexAttribPointer(iLocPosition, 3, GL_FLOAT, GL_FALSE, 0, v);
		glVertexAttribPointer(   iLocColor, 3, GL_FLOAT, GL_FALSE, 0, c);
	
	
		// bind uniform matrix to shader
		glUniformMatrix4fv(iLocMVP, 1, GL_FALSE, mvp);

		// draw the array we just bound
		glDrawArrays(GL_TRIANGLES, 0, 3*model_info[index].tNum);

		free(v);
		free(c);
	}
	glutSwapBuffers();
}

void showShaderCompileStatus(GLuint shader, GLint *shaderCompiled)
{
	glGetShaderiv(shader, GL_COMPILE_STATUS, shaderCompiled);
	if(GL_FALSE == (*shaderCompiled))
	{
		GLint maxLength = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character.
		GLchar *errorLog = (GLchar*) malloc(sizeof(GLchar) * maxLength);
		glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);
		fprintf(stderr, "%s", errorLog);

		glDeleteShader(shader);
		free(errorLog);
	}
}

void setShaders()
{
	GLuint v, f, p;
	char *vs = NULL;
	char *fs = NULL;

	v = glCreateShader(GL_VERTEX_SHADER);
	f = glCreateShader(GL_FRAGMENT_SHADER);

	vs = textFileRead("shader.vert");
	fs = textFileRead("shader.frag");

	glShaderSource(v, 1, (const GLchar**)&vs, NULL);
	glShaderSource(f, 1, (const GLchar**)&fs, NULL);

	free(vs);
	free(fs);

	// compile vertex shader
	glCompileShader(v);
	GLint vShaderCompiled;
	showShaderCompileStatus(v, &vShaderCompiled);
	if(!vShaderCompiled) system("pause"), exit(123);

	// compile fragment shader
	glCompileShader(f);
	GLint fShaderCompiled;
	showShaderCompileStatus(f, &fShaderCompiled);
	if(!fShaderCompiled) system("pause"), exit(456);

	p = glCreateProgram();

	// bind shader
	glAttachShader(p, f);
	glAttachShader(p, v);

	// link program
	glLinkProgram(p);

	iLocPosition = glGetAttribLocation (p, "av4position");
	iLocColor    = glGetAttribLocation (p, "av3color");
	iLocMVP		 = glGetUniformLocation(p, "mvp");

	glUseProgram(p);
}


void onMouse(int who, int state, int x, int y)
{
	printf("%18s(): (%d, %d) ", __FUNCTION__, x, y);

	switch(who)
	{
		case GLUT_LEFT_BUTTON:   printf("left button   "); break;
		case GLUT_MIDDLE_BUTTON: printf("middle button "); break;
		case GLUT_RIGHT_BUTTON:  printf("right button  "); break; 
		case GLUT_WHEEL_UP:      printf("wheel up      "); break;
		case GLUT_WHEEL_DOWN:    printf("wheel down    "); break;
		default:                 printf("0x%02X          ", who); break;
	}

	switch(state)
	{
		case GLUT_DOWN: printf("start "); break;
		case GLUT_UP:   printf("end   "); break;
	}

	printf("\n");
}

void onMouseMotion(int x, int y)
{
	printf("%18s(): (%d, %d) mouse move\n", __FUNCTION__, x, y);
}

void onKeyboard(unsigned char key, int x, int y) 
{
	printf("%18s(): (%d, %d) key: %c(0x%02X) ", __FUNCTION__, x, y, key, key);
	switch(key) 
	{
		case GLUT_KEY_ESC: /* the Esc key */ 
			exit(0); 
			break;
	}
	printf("\n");
}

void onKeyboardSpecial(int key, int x, int y){
	printf("%18s(): (%d, %d) ", __FUNCTION__, x, y);
	switch(key)
	{
		case GLUT_KEY_LEFT:
			printf("key: LEFT ARROW");
			break;
			
		case GLUT_KEY_RIGHT:
			printf("key: RIGHT ARROW");
			break;

		default:
			printf("key: 0x%02X      ", key);
			break;
	}
	printf("\n");
}


void onWindowReshape(int width, int height)
{
	printf("%18s(): %dx%d\n", __FUNCTION__, width, height);
}

int main(int argc, char **argv) 
{
	// glut init
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);

	// create window
	glutInitWindowPosition(500, 100);
	glutInitWindowSize(800, 800);
	glutCreateWindow("10420 CS550000 CG HW2 TA");

	glewInit();
	if(glewIsSupported("GL_VERSION_2_0")){
		printf("Ready for OpenGL 2.0\n");
	}else{
		printf("OpenGL 2.0 not supported\n");
		system("pause");
		exit(1);
	}

	// load obj models through glm
	loadOBJModel("../ColorModels/bunny5KC.obj",0);

	// register glut callback functions
	glutDisplayFunc (onDisplay);
	glutIdleFunc    (onIdle);
	glutKeyboardFunc(onKeyboard);
	glutSpecialFunc (onKeyboardSpecial);
	glutMouseFunc   (onMouse);
	glutMotionFunc  (onMouseMotion);
	glutReshapeFunc (onWindowReshape);

	// set up shaders here
	setShaders();
	
	glEnable(GL_DEPTH_TEST);

	// main loop
	glutMainLoop();

	// free
	glmDelete(OBJ);

	return 0;
}

