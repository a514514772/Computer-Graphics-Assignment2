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
GLint currentModel = 0;


char filename[] = "ColorModels/bunny5KC.obj";
GLMmodel* OBJ;
GLfloat* vertices;
GLfloat* colors;
// geometrical translation
GLfloat dx[] = {0, -3, 3, -3, 3};
GLfloat dy[] = {0, 3, 3, -3, -3};
GLfloat geo_tx[5]={}, geo_ty[5]={}, geo_tz[5]={};
GLfloat geo_sx[5]={1, 1, 1, 1, 1}, geo_sy[5]={1, 1, 1, 1, 1}, geo_sz[5]={1, 1, 1, 1, 1};
GLfloat geo_rx[5]={}, geo_ry[5]={}, geo_rz[5]={};
// viewing matrix
GLfloat eye_position[] = {0, 0, 10};
GLfloat eye_displace[] = {0, 0, 0};
GLfloat center_displace[] = {0, 0, 0};
GLfloat center_position[] = {0, 0, 0};
GLfloat upper_vec[] = {0, 1, 0};

bool projection_toggle=0;

struct Model{
	std::vector<GLfloat> v;
	std::vector<GLfloat> c;
	GLfloat maxVal[3];
	GLfloat minVal[3];
	GLint tNum, vNum;
	Matrix4 M, V, P;
	Matrix4 external_matrix;
};
Model model_info[5];

enum Mode{INIT, TRANSLATION, SCALING, ROTATION, EYE};
Mode mode_state = INIT;

struct MouseEvent{
	GLfloat init_x, init_y, init_z;
	int state;
};
MouseEvent mouseEvent;

void glmNormalize(GLfloat* v)
{
    GLfloat l;
    
    l = (GLfloat)sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
    v[0] /= l;
    v[1] /= l;
    v[2] /= l;
}
void glmCross(GLfloat* u, GLfloat* v, GLfloat* n)
{
    
    n[0] = u[1]*v[2] - u[2]*v[1];
    n[1] = u[2]*v[0] - u[0]*v[2];
    n[2] = u[0]*v[1] - u[1]*v[0];
}
void reset(){
	Matrix4 m = Matrix4(
						1, 0, 0, 0,
						0, 1, 0, 0,
						0, 0, 1, 0,
						0, 0, 0, 1
						);
	for(int i=0;i<5;i++){
		geo_tx[i] = geo_ty[i] = geo_tz[i] = 0;
		geo_sx[i] = geo_sy[i] = geo_sz[i] = 1;
		geo_rx[i] = geo_ry[i] = geo_rz[i] = 0;
		model_info[i].external_matrix = m;
	}
	eye_displace[0] = eye_displace[1] = eye_displace[2] = 0;
	center_displace[0] = center_displace[1] = center_displace[2] = 0;
	upper_vec[0] = 0; upper_vec[1] = 1; upper_vec[2] = 0;

	projection_toggle=0;
}
Matrix4 geoInit(int index, GLfloat dx, GLfloat dy){
		// the current center of the model we load in
		GLfloat model_centerx = (model_info[index].maxVal[0]+model_info[index].minVal[0])/2;
		GLfloat model_centery = (model_info[index].maxVal[1]+model_info[index].minVal[1])/2;
		GLfloat model_centerz = (model_info[index].maxVal[2]+model_info[index].minVal[2])/2;
		// the size of each axis
		GLfloat sx = fabs(model_info[index].maxVal[0]-model_info[index].minVal[0]);
		GLfloat sy = fabs(model_info[index].maxVal[1]-model_info[index].minVal[1]);
		GLfloat sz = fabs(model_info[index].maxVal[2]-model_info[index].minVal[2]);

		GLfloat scale_factor = 2/max(sx, max(sy,sz));

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
		
		return scale_matrix*M;
}
Matrix4 geoTranslation(int index){
	Matrix4 matrix = Matrix4(
							1, 0, 0, geo_tx[index],
							0, 1, 0, geo_ty[index],
							0, 0, 1, geo_tz[index],
							0, 0, 0, 1
							);
	return matrix;
}
Matrix4 geoScaling(int index){
	Matrix4 matrix = Matrix4(
							geo_sx[index], 0, 0, 0,
							0, geo_sy[index], 0, 0,
							0, 0, geo_sz[index], 0,
							0, 0, 0, 1
							);
	return matrix;
}
Matrix4 geoRotation(int index){
	Matrix4 R = Matrix4(
							1, 0, 0, 0,
							0, 1, 0, 0,
							0, 0, 1, 0, 
							0, 0, 0, 1
							);
	if(geo_rx[index] != 0){
		Matrix4 matrix = Matrix4(
								1, 0, 0, 0,
								0, cos(geo_rx[index]), -sin(geo_rx[index]), 0,
								0, sin(geo_rx[index]), cos(geo_rx[index]), 0,
								0, 0, 0, 1
								);
		R = matrix*R;
	}
	if(geo_ry[index] != 0){
		Matrix4 matrix = Matrix4(
								cos(geo_ry[index]), 0, sin(geo_ry[index]), 0,
								0, 1, 0, 0,
								-sin(geo_ry[index]), 0, cos(geo_ry[index]), 0,
								0, 0, 0, 1
								);
		R = matrix*R;
	}
	if(geo_rz[index] != 0){
		Matrix4 matrix = Matrix4(
								cos(geo_rz[index]), -sin(geo_rz[index]), 0, 0,
								sin(geo_rz[index]), cos(geo_rz[index]), 0, 0,
								0, 0, 1, 0,
								0, 0, 0, 1
								);
		R = matrix*R;
	}
	return R;
}
Matrix4 viewInit(){
	GLfloat F[3];
	for(int i=0;i<3;i++){
		F[i] = (center_position[i] + center_displace[i]) - (eye_position[i] + eye_displace[i]);
	}
	glmNormalize(F);
	GLfloat S[3];
	glmCross(F, upper_vec, S);
	glmNormalize(S);
	GLfloat _U[3];
	glmCross(S, F, _U);
	glmNormalize(_U);
	Matrix4 factor = Matrix4(
							S[0], S[1], S[2], 0,
							_U[0], _U[1], _U[2], 0,
							-F[0], -F[1], -F[2], 0,
							0, 0, 0, 1
							);
	Matrix4 posM = Matrix4(
							1, 0, 0, -eye_position[0]-eye_displace[0],
							0, 1, 0, -eye_position[1]-eye_displace[1],
							0, 0, 1, -eye_position[2]-eye_displace[2],
							0, 0, 0, 1
						);
	return factor * posM;
}
GLfloat getDistance(GLfloat *v, GLfloat *u){
	return (GLfloat)sqrt((v[0]-u[0])*(v[0]-u[0]) + (v[1]-u[1])*(v[1]-u[1]) + (v[2]-u[2])*(v[2]-u[2]) );
}
Matrix4 projectionInit(){
	GLfloat top = 1, left = -1, bot = -1, right = 1;
	GLfloat nearVal = 2;
	GLfloat farVal = 100;
	if(projection_toggle == 0){	// perspective
		return Matrix4(
						(2*nearVal)/(right-left), 0, (right+left)/(right-left), 0,
						0, (2*nearVal)/(top-bot), (top+bot)/(top-bot), 0,
						0, 0, -(farVal+nearVal)/(farVal-nearVal), -(2*farVal*nearVal)/(farVal-nearVal),
						0, 0, -1, 0
						);
	}
	else{	// orthographic
		top = right = 5;
		left = bot = -5;
		nearVal = 0.5;
		farVal = 100;
		return Matrix4(
						2/(right-left), 0, 0, -(right+left)/(right-left),
						0, 2/(top-bot), 0, -(top+bot)/(top-bot),
						0, 0, -2/(farVal-nearVal), -(farVal+nearVal)/(farVal-nearVal),
						0, 0, 0, 1
						);
	}
}
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
	model_info[index].external_matrix = Matrix4(
							1, 0, 0, 0, 
							0, 1, 0, 0,
							0, 0, 1, 0,
							0, 0, 0, 1);
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

	// TODO:
	//// Please define the model transformation matrix, viewing transformation matrix, 
	//// projection transformation matrix

	//MVP
	for(int index = 0;index < 5;index++){

		
		Matrix4 M = geoInit(index, dx[(index-currentModel+5)%5], dy[(index-currentModel+5)%5]);
		if(index == currentModel){
			Matrix4 S = geoScaling(index);
			Matrix4 R = geoRotation(index);
			Matrix4 T = geoTranslation(index);
			model_info[index].external_matrix = T*S*R;
		}
		Matrix4 D = Matrix4(
			1, 0, 0, dx[(index-currentModel+5)%5], 
			0, 1, 0, dy[(index-currentModel+5)%5],
							0, 0, 1, 0,
							0, 0, 0, 1);
		M = D*model_info[index].external_matrix * M;

		/*Matrix4 V = Matrix4(
							1, 0, 0, 0, 
							0, 1, 0, 0,
							0, 0, 1, 0,
							0, 0, 0, 1);*/
		Matrix4 V = viewInit();
		Matrix4 P = projectionInit();

		Matrix4 T, S, R;
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

	GLfloat dx, dy, dz;
	switch(who)
	{
		case GLUT_LEFT_BUTTON:   
			printf("left button   "); 
			if(mode_state != EYE) mouseEvent.state = 1;
			else mouseEvent.state = 3;
			break;
		case GLUT_MIDDLE_BUTTON: 
			reset();
			printf("middle button "); 
			break;
		case GLUT_RIGHT_BUTTON:  
			printf("right button  "); 
			if(mode_state != EYE) mouseEvent.state = 2;
			else mouseEvent.state = 4;
			break; 
		case GLUT_WHEEL_UP:     
			if(mode_state != EYE){
				geo_sx[currentModel] = geo_sy[currentModel] = geo_sz[currentModel] += 0.1;
			}
			else{
				eye_displace[2] -= 0.1;
			}
			printf("wheel up      "); 
			break;
		case GLUT_WHEEL_DOWN:   
			if(mode_state != EYE){
				geo_sx[currentModel] = geo_sy[currentModel] = geo_sz[currentModel] -= 0.1;
			}
			else{
				eye_displace[2] += 0.1;
			}
			printf("wheel down    "); 
			break;
		default:                 printf("0x%02X          ", who); break;
	}

	switch(state)
	{
		case GLUT_DOWN: 	
			printf("start "); 
			mouseEvent.init_x = x;
			mouseEvent.init_y = y;
			break;
		case GLUT_UP:   
			printf("end   "); 
			mouseEvent.state = 0;
			break;
	}

	printf("\n");
}

void onMouseMotion(int x, int y)
{
	printf("%18s(): (%d, %d) mouse move\n", __FUNCTION__, x, y);
	if(mouseEvent.state == 1){
		geo_tx[currentModel] += (x - mouseEvent.init_x)/100;
		geo_ty[currentModel] -= (y - mouseEvent.init_y)/100;
		mouseEvent.init_x = x;
		mouseEvent.init_y = y;
	}
	else if(mouseEvent.state == 2){
		geo_ry[currentModel] += (x - mouseEvent.init_x)/200;
		geo_rx[currentModel] += (y - mouseEvent.init_y)/200;
		mouseEvent.init_x = x;
		mouseEvent.init_y = y;
	}
	else if(mouseEvent.state == 3){
		center_displace[0] -= (x - mouseEvent.init_x)/100;
		eye_displace[0] -= (x - mouseEvent.init_x)/100;
		center_displace[1] += (y - mouseEvent.init_y)/100;
		eye_displace[1] += (y - mouseEvent.init_y)/100; 

		mouseEvent.init_x = x;
		mouseEvent.init_y = y;
	}
	else if(mouseEvent.state == 4){
		eye_displace[0] -= (x - mouseEvent.init_x)/50;
		eye_displace[1] += (y - mouseEvent.init_y)/50;
		mouseEvent.init_x = x;
		mouseEvent.init_y = y;
	}
}

void onKeyboard(unsigned char key, int x, int y) 
{
	printf("%18s(): (%d, %d) key: %c(0x%02X) ", __FUNCTION__, x, y, key, key);
	switch(key) 
	{
		case GLUT_KEY_ESC: /* the Esc key */ 
			exit(0); 
			break;
		case 'T':	// translation mode
			mode_state = TRANSLATION;
			printf("current mode : TRANSLATION\n");
			break;
		case 'S':	// scaling mode
			mode_state = SCALING;
			printf("current mode : SCALING\n");
			break;
		case 'R':	// rotation mode
			mode_state = ROTATION;
			printf("current mode : ROTATION\n");
			break;
		case 'E':	// eye mode
			mode_state = EYE;
			printf("current mode : EYE\n");
			break;
		case 'I':	// increase y
			if(mode_state == ROTATION){
				geo_ry[currentModel]+=0.1;
			}
			else if(mode_state == TRANSLATION){
				geo_ty[currentModel]+=0.1;
			}
			else if(mode_state == SCALING){
				geo_sy[currentModel]+=0.1;
			}
			else if(mode_state == EYE){
				eye_displace[1] += 0.5;
				//center_displace[1] +=0.5;
			}
			break;
		case 'K':	// decrease y
			if(mode_state == ROTATION){
				geo_ry[currentModel]-=0.1;
			}
			else if(mode_state == TRANSLATION){
				geo_ty[currentModel]-=0.1;
			}
			else if(mode_state == SCALING){
				geo_sy[currentModel]-=0.1;
			}
			else if(mode_state == EYE){
				eye_displace[1] -= 0.5;
				//center_displace[1] -=0.5;
			}
			break;
		case 'J':	// decrease x
			if(mode_state == ROTATION){
				geo_rx[currentModel]-=0.1;
			}
			else if(mode_state == TRANSLATION){
				geo_tx[currentModel]-=0.1;
			}
			else if(mode_state == SCALING){
				geo_sx[currentModel]-=0.1;
			}
			else if(mode_state == EYE){
				eye_displace[0] -= 0.5;
				//center_displace[0] -=0.5;
			}
			break;
		case 'L':	// increase x
			if(mode_state == ROTATION){
				geo_rx[currentModel]+=0.1;
			}
			else if(mode_state == TRANSLATION){
				geo_tx[currentModel]+=0.1;
			}
			else if(mode_state == SCALING){
				geo_sx[currentModel]+=0.1;
			}
			else if(mode_state == EYE){
				eye_displace[0] += 0.5;
				//center_displace[0] +=0.5;
			}
			break;
		case 'M':	// increase z
			if(mode_state == ROTATION){
				geo_rz[currentModel]+=0.1;
			}
			else if(mode_state == TRANSLATION){
				geo_tz[currentModel]+=0.1;
			}
			else if(mode_state == SCALING){
				geo_sz[currentModel]+=0.1;
			}
			else if(mode_state == EYE){
				eye_displace[2] += 0.5;
				//center_displace[2] +=0.5;
			}
			break;
		case 'O':	// decrease z
			if(mode_state == ROTATION){
				geo_rz[currentModel]-=0.1;
			}
			else if(mode_state == TRANSLATION){
				geo_tz[currentModel]-=0.1;
			}
			else if(mode_state == SCALING){
				geo_sz[currentModel]-=0.1;
			}
			else if(mode_state == EYE){
				eye_displace[2] -= 0.5;
				//center_displace[2] -=0.5;
			}
			break;
		case 'P':
			projection_toggle = !projection_toggle;
			if(projection_toggle == 0){
				printf("projection mode : perspective\n");
			}
			else{
				printf("projection mode : parallel\n");
			}
			break;
		case 'Q':
			reset();
			break;
		case 'H':
			puts("Keyboard :");
			printf("Press T to enter geometrical Translation mode.\n");
			printf("Press R to enter geometrical Rotation mode\n");
			puts("Press S to enter geometrical Scaling mode");
			puts("Press E to enter eyes' position mode");
			puts("Press L to increase x coord.");
			puts("Press J to decrease x coord.");
			puts("Press I to increase y coord.");
			puts("Press K to decrease y coord.");
			puts("Press M to increase z coord.");
			puts("Press O to decrease z coord.");
			puts("Press P to toggle projection mode");
			puts("Press Q to reset ");
			puts("Press left arrow or right arrow to change current model pointer");
			puts("Mouse :");
			puts("Press E/other keys(T, S, R) to switch between geometry mode and eye position mode");
			puts("If in geometry mode,");
			puts("Press left buuton and drag to translate current model");
			puts("Press right button and drag to rotate current model");
			puts("Wheel up and down to scale current model");
			puts("If in eye position mode,");
			puts("Press left buuton and drag to move both eye position and center position");
			puts("Press right button and drag to move only eye position in the x and y axes");
			puts("Wheel up and down to move eye position in the z axis");
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
			currentModel -= 1;
			if(currentModel < 0) currentModel = 4;
			printf(" current mode : %d\n", currentModel);
			break;
			
		case GLUT_KEY_RIGHT:
			printf("key: RIGHT ARROW");
			currentModel = (currentModel+1)%5;
			printf(" current mode : %d\n", currentModel);
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
	loadOBJModel("../ColorModels/dragon10KC.obj",1);
	loadOBJModel("../ColorModels/duck4KC.obj",2);
	loadOBJModel("../ColorModels/blitzcrank_incognito.obj",3);
	loadOBJModel("../ColorModels/frog2KC.obj",4);

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

