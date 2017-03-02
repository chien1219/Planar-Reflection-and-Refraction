#include "mesh.h"
#include "glew.h"
#include "glut.h"
#include "FreeImage.h"
#include <math.h>
#include <fstream>
#include <iostream>
using namespace std;
int scene_obj_num = 0;
int how_many_light = 0;
int ptobject = 0;
mesh *object[100];
int windowSize[2];
//refraction and reflection
GLfloat transmittance = 0.5;
GLfloat reflectance = 0.5;
//record
int mirror = 0;
int back_bear = 2;
int pass = 0;
////////////////scene declaration///////////////////
GLfloat scale[100][3];
GLfloat rotate_[100][4];   //rotate_[x][0] is Angle
GLfloat translate[100][3];
////////////////view declaration///////////////////
GLfloat eye[3], vat[3], vup[3], viewport[4];
GLfloat fovy, dnear, dfar;
////////////////light declaration///////////////////
GLfloat light_specular[7][4];
GLfloat light_diffuse[7][4] ;
GLfloat light_ambient[7][4] ;
GLfloat light_position[7][4];
GLfloat ambient[3];
////////////////for keyboard use///////////////////
 GLfloat distancex;
 GLfloat distancey;
 GLfloat distancez;
////////////////for mouse use//////////////////////
GLfloat movex;
GLfloat movey;
GLfloat past_x, past_y, rec_x, rec_y;
GLfloat ctr_x[9], ctr_y[9];
////////////////function predeclaration////////////
void light();
void handle_scene();
void handle_view();
void handle_light();
void display();
void keyboard(unsigned char, int, int);
void mouse(int, int ,int , int );
void mousemove(int, int);
void reshape(GLsizei , GLsizei );
void draw_mirror();
void render_obj(bool);
void reflection();
void refraction();
int main(int argc, char** argv)
{
	handle_scene();
	handle_view();
	handle_light();
	glutInit(&argc, argv);
	glutInitWindowSize(viewport[2], viewport[3]);
	glutInitWindowPosition(250,100);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB|GLUT_DEPTH|GL_STENCIL|GLUT_ACCUM);
	glutCreateWindow("0316235_HW2");
	glewInit();
	distancex = vat[0] - eye[0];
	distancey = vat[1] - eye[1];
	distancez = vat[2] - eye[2];
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(mousemove);
	glutReshapeFunc(reshape);
	glutMainLoop();

	return 0;
}

void display()
{
	// clear the buffer
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);      //清除用color
	glClearDepth(1.0f);                        // Depth Buffer (就是z buffer) Setup
	glClearStencil(1);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);//////
	glDepthMask(GL_FALSE);								///////
	glEnable(GL_DEPTH_TEST);                   // Enables Depth Testing
	glDepthFunc(GL_LEQUAL);                    // The Type Of Depth Test To Do
	glEnable(GL_CULL_FACE);
	glEnable(GL_STENCIL_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |GL_STENCIL_BUFFER_BIT| GL_ACCUM_BUFFER_BIT);//這行把畫面清成黑色並且清除z buffer

	glCullFace(GL_BACK);
	// viewport transformation
	//-----------------------------------------------------------------------
	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

	// projection transformation
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovy, viewport[2]/viewport[3], dnear, dfar);
	// viewing and modeling transformation
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	cout << "X: " << -distancex << " y: "<<-distancey<<" Z: "<<-distancez<<endl;
	cout << "reflection: " << reflectance << endl << "transmittance: " << transmittance << endl;
	gluLookAt(  eye[0], eye[1], eye[2],		// eye
				vat[0], vat[1], vat[2],     // center
				vup[0], vup[1], vup[2]);    // up
	//-----------------------------------------------------------------------
	//注意light位置的設定，要在gluLookAt之後
	light();
	//-----------------------------------------------------------------------
	//set stencil buffer
	draw_mirror();

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);
	glClear(GL_DEPTH_BUFFER_BIT);
	pass = 1;

	render_obj(false);
	
	glDepthMask(GL_FALSE);	//////////
	glEnable(GL_BLEND);
	draw_mirror();

	//refraction
	glFrontFace(GL_CCW);
	pass = 0;
	refraction();
	glDisable(GL_BLEND);

	//reflection
	glFrontFace(GL_CW);
	pass = 0;
	reflection();
	// projection transformation
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovy, viewport[2]/viewport[3], dnear, dfar);
	// viewing and modeling transformation
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(  eye[0], eye[1], eye[2],		// eye
				vat[0], vat[1], vat[2],     // center
				vup[0], vup[1], vup[2]);    // up

	//combination
	glAccum(GL_RETURN, 1.0);
	glFrontFace(GL_CCW);
	pass = 1;
	render_obj(false);
	pass = 0;			//avoid front bear being covered by mirror
	render_obj(true);	//在鏡子後的東西不畫

	glutSwapBuffers();
	glFlush();
}
void render_obj(bool bear)
{

	if (pass == 1)glStencilFunc(GL_EQUAL, 1, 1);//是1的位置才畫
	else           glStencilFunc(GL_EQUAL, 0, 1);//是0的位置才畫

	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	for (int gg = 0; gg < scene_obj_num; ++gg){

		if (bear){
			if (gg == mirror || gg == back_bear) continue;
		}
		if (gg == mirror)continue;
		
		int lastMaterial = -1;
		glPushMatrix();
		glTranslatef(translate[gg][0] - ctr_x[gg], translate[gg][1] + ctr_y[gg], translate[gg][2]);
		glRotatef(rotate_[gg][0], rotate_[gg][1], rotate_[gg][2], rotate_[gg][3]);
		glScalef(scale[gg][0], scale[gg][1], scale[gg][2]);
		for (size_t i = 0; i < object[gg]->fTotal; ++i)
		{
			// set material property if this face used different material
			if (lastMaterial != object[gg]->faceList[i].m)
			{
				lastMaterial = (int)object[gg]->faceList[i].m;
				glMaterialfv(GL_FRONT, GL_AMBIENT, object[gg]->mList[lastMaterial].Ka);
				glMaterialfv(GL_FRONT, GL_DIFFUSE, object[gg]->mList[lastMaterial].Kd);
				glMaterialfv(GL_FRONT, GL_SPECULAR, object[gg]->mList[lastMaterial].Ks);
				glMaterialfv(GL_FRONT, GL_SHININESS, &object[gg]->mList[lastMaterial].Ns);

			}
			glBegin(GL_TRIANGLES);

			for (size_t j = 0; j < 3; ++j){
				glNormal3fv(object[gg]->nList[object[gg]->faceList[i][j].n].ptr);
				glVertex3fv(object[gg]->vList[object[gg]->faceList[i][j].v].ptr);
			}
			glEnd();
		}
		glPopMatrix();
	}
}
void draw_mirror(){
	glStencilFunc(GL_ALWAYS, 1, 1);
	glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO);//鏡子改0

	int j = mirror;
	glPushMatrix();
	glTranslatef(translate[j][0] - ctr_x[j], translate[j][1] + ctr_y[j], translate[j][2]);
	glRotatef(rotate_[j][0], rotate_[j][1], rotate_[j][2], rotate_[j][3]);
	glScalef(scale[j][0], scale[j][1], scale[j][2]);

	int lastMaterial = -1;
	for (size_t i = 0; i < object[j]->fTotal; ++i)
	{
		// set material property if this face used different material
		
		if (lastMaterial != object[j]->faceList[i].m)
		{
			lastMaterial = (int)object[j]->faceList[i].m;
			object[j]->mList[lastMaterial].Kd[3] = object[j]->mList[lastMaterial].Tr; //assign alpha
			glMaterialfv(GL_FRONT, GL_AMBIENT, object[j]->mList[lastMaterial].Ka);
			glMaterialfv(GL_FRONT, GL_DIFFUSE, object[j]->mList[lastMaterial].Kd);
			glMaterialfv(GL_FRONT, GL_SPECULAR, object[j]->mList[lastMaterial].Ks);
			glMaterialfv(GL_FRONT, GL_SHININESS, &object[j]->mList[lastMaterial].Ns);

		}
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBegin(GL_TRIANGLES);
		for (size_t k = 0; k < 3; ++k)
		{
			glNormal3fv(object[j]->nList[object[j]->faceList[i][k].n].ptr);
			glVertex3fv(object[j]->vList[object[j]->faceList[i][k].v].ptr);

		}
		glEnd();

	}
	glPopMatrix();

}
void refraction(){
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClearAccum(0.0, 0.0, 0.0, 0.0);

	if (pass == 1) glStencilFunc(GL_EQUAL, 1, 1);//是1的位置才畫
	else           glStencilFunc(GL_EQUAL, 0, 1);//是0的位置才畫
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	for (size_t j = 0; j < scene_obj_num; j++){
		glClear(GL_COLOR_BUFFER_BIT);
		glPushMatrix();
		glTranslatef(translate[j][0] - ctr_x[j], translate[j][1] + ctr_y[j], translate[j][2]);
		glRotatef(rotate_[j][0], rotate_[j][1], rotate_[j][2], rotate_[j][3]);
		glScalef(scale[j][0], scale[j][1], scale[j][2]);

		int lastMaterial = -1;
		for (size_t i = 0; i < object[j]->fTotal; ++i)
		{
			// set material property if this face used different material
			if (lastMaterial != object[j]->faceList[i].m)
			{
				lastMaterial = (int)object[j]->faceList[i].m;
				glMaterialfv(GL_FRONT, GL_AMBIENT, object[j]->mList[lastMaterial].Ka);
				glMaterialfv(GL_FRONT, GL_DIFFUSE, object[j]->mList[lastMaterial].Kd);
				glMaterialfv(GL_FRONT, GL_SPECULAR, object[j]->mList[lastMaterial].Ks);
				glMaterialfv(GL_FRONT, GL_SHININESS, &object[j]->mList[lastMaterial].Ns);

			}
			glBegin(GL_TRIANGLES);
			for (size_t k = 0; k < 3; ++k)
			{
				glNormal3fv(object[j]->nList[object[j]->faceList[i][k].n].ptr);
				glVertex3fv(object[j]->vList[object[j]->faceList[i][k].v].ptr);

			}
			glEnd();
		}
		glPopMatrix();
		glAccum(GL_ACCUM, transmittance);
	}

}
void reflection(){

	glClear(GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(fovy, -viewport[2] / viewport[3], dfar, dnear);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt((-1)*eye[0] - 40, eye[1], eye[2],
			  (-1)*vat[0] - 40, vat[1], vat[2],
			    	vup[0], vup[1], vup[2]);

	if (pass == 1)glStencilFunc(GL_EQUAL, 1, 1);//是1的位置才畫
	else           glStencilFunc(GL_EQUAL, 0, 1);//是0的位置才畫
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	for (size_t j = 0; j < scene_obj_num; j++){
		if (j == back_bear) continue;
		glClear(GL_COLOR_BUFFER_BIT);

		glPushMatrix();
		glTranslatef(translate[j][0] - ctr_x[j], translate[j][1] + ctr_y[j], translate[j][2]);
		glRotatef(rotate_[j][0], rotate_[j][1], rotate_[j][2], rotate_[j][3]);
		glScalef(scale[j][0], scale[j][1], scale[j][2]);

		int lastMaterial = -1;
		for (size_t i = 0; i < object[j]->fTotal; ++i)
		{
			// set material property if this face used different material

			if (lastMaterial != object[j]->faceList[i].m)
			{
				lastMaterial = (int)object[j]->faceList[i].m;
				glMaterialfv(GL_FRONT, GL_AMBIENT, object[j]->mList[lastMaterial].Ka);
				glMaterialfv(GL_FRONT, GL_DIFFUSE, object[j]->mList[lastMaterial].Kd);
				glMaterialfv(GL_FRONT, GL_SPECULAR, object[j]->mList[lastMaterial].Ks);
				glMaterialfv(GL_FRONT, GL_SHININESS, &object[j]->mList[lastMaterial].Ns);
			}
			glBegin(GL_TRIANGLES);
			for (size_t k = 0; k < 3; ++k)
			{
				glNormal3fv(object[j]->nList[object[j]->faceList[i][k].n].ptr);
				glVertex3fv(object[j]->vList[object[j]->faceList[i][k].v].ptr);
			}
			glEnd();
		}
		glPopMatrix();
		glAccum(GL_ACCUM, reflectance);
	}
}

void keyboard(unsigned char key, int x, int y){

	switch (key) {
	case 'w':			///////zoom in
	case 'W':
		eye[0] = -distancex + (distancex / 80)+vat[0];
		eye[1] = -distancey + (distancey / 80)+vat[1];
		eye[2] = -distancez + (distancez / 80)+vat[2];
		distancex = vat[0] - eye[0];
		distancey = vat[1] - eye[1];
		distancez = vat[2] - eye[2];
		glutPostRedisplay();
		break;
	case 'a':
	case 'A':
		eye[0] = -distancex*cos(0.055) -(-distancez)*sin(0.055);
		eye[2] = -distancez*cos(0.055) +(-distancex)*sin(0.055);
		distancex = vat[0] - eye[0];
		distancez = vat[2] - eye[2];
		glutPostRedisplay();
		break;
	case 's':			///////zoom out
	case 'S':
		eye[0] = -distancex - (distancex / 80) + vat[0];
		eye[1] = -distancey - (distancey / 80) + vat[1];
		eye[2] = -distancez - (distancez / 80) + vat[2];
		distancex = vat[0] - eye[0];
		distancey = vat[1] - eye[1];
		distancez = vat[2] - eye[2];
		glutPostRedisplay();
		break;
	case 'd':
	case 'D':
		eye[0] = -distancex*cos(0.055) + (-distancez)*sin(0.055);
		eye[2] = -distancez*cos(0.055) - (-distancex)*sin(0.055);
		distancex = vat[0] - eye[0];
		distancez = vat[2] - eye[2];
		glutPostRedisplay();
		break;
	case 'r':
		if (reflectance < 1) reflectance += 0.1;
		glutPostRedisplay();
		break;
	case 'f':
		if (reflectance - 0.1 > 0) reflectance -= 0.1;
		glutPostRedisplay();
		break;
	case 't':
		if (transmittance < 1) transmittance += 0.1;
		glutPostRedisplay();
		break;
	case 'g':
		if (transmittance - 0.1 > 0) transmittance -= 0.1;
		glutPostRedisplay();
		break;
	case '1':
		ptobject = 0;
		glutPostRedisplay();
		break;
	case '2':
		ptobject = 1;
		glutPostRedisplay();
		break;
	case '3':
		ptobject = 2;
		glutPostRedisplay();
		break;
	case '4':
		ptobject = 3;
		glutPostRedisplay();
		break;
	case '5':
		ptobject = 4;
		glutPostRedisplay();
		break;
	case '6':
		ptobject = 5;
		glutPostRedisplay();
		break;
	case '7':
		ptobject = 6;
		glutPostRedisplay();
		break;
	case '8':
		ptobject = 7;
		glutPostRedisplay();
		break;
	case '9':
		ptobject = 8;
		glutPostRedisplay();
		break;

	}

}
void mouse(int button, int state, int x, int y){
	if (state){
		rec_x += (x - past_x);
		rec_y += (y - past_y);
	}
	else{
		past_x = x;
		past_y = y;
	}
}
void mousemove(int x, int y){
	ctr_x[ptobject] = (GLfloat)(past_x - x)/20;
	ctr_y[ptobject] = (GLfloat)(past_y - y)/20;

	cout << "mouse_x: " << ctr_x[ptobject] << " mouse_y: " << ctr_y[ptobject] << endl;
	glutPostRedisplay();

}
void light(){	
	for (int i = 0; i < how_many_light; i++){
			glShadeModel(GL_SMOOTH);
			// z buffer enable
			glEnable(GL_DEPTH_TEST);
			// enable lighting
			glEnable(GL_LIGHTING);
			// set light property
			glEnable(GL_LIGHT0+(GLfloat)i);
			glLightfv(GL_LIGHT0 + (GLfloat)i, GL_POSITION, light_position[i]);
			glLightfv(GL_LIGHT0 + (GLfloat)i, GL_DIFFUSE, light_diffuse[i]);
			glLightfv(GL_LIGHT0 + (GLfloat)i, GL_SPECULAR, light_specular[i]);
			glLightfv(GL_LIGHT0 + (GLfloat)i, GL_AMBIENT, light_ambient[i]);
			cout << "--------------" << "No. "<< i+1 <<" light is created-----------------" << endl;
		}
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
	how_many_light = 0;
}
void handle_scene(){
	cout << "-----------------scene development-------------------" << endl;

	fstream scene_;
	scene_.open("CornellBox.scene", ios::in);
	char title[30];
	while (scene_ >> title){
		if (!strcmp(title, "model")){
			char name[20];
			scene_ >> name;
			if (!strcmp(name, "Mirror.obj")) mirror = scene_obj_num;
			
			object[scene_obj_num] = new mesh(name);
			for (int i = 0; i < 3; i++){
				scene_ >> (GLfloat)scale[scene_obj_num][i];
				cout << scale[scene_obj_num][i] << " ";
			}
			cout << endl;
			for (int i = 0; i < 4; i++){
				scene_ >> (GLfloat)rotate_[scene_obj_num][i];
				cout << rotate_[scene_obj_num][i] << " ";
			}
			cout << endl;
			for (int i = 0; i < 3; i++){
				scene_ >> (GLfloat)translate[scene_obj_num][i];
				cout << translate[scene_obj_num][i] << " ";
			}
			cout << endl;
			cout << "in scene " << scene_obj_num << " obj: " << name << " created." << endl;
			scene_obj_num++;
		}
	
		else if (!strcmp(title, "no-texture"));
	}
	scene_.close();
}
void handle_view(){

	char cat[10];

	fstream view_;
	view_.open("CornellBox.view", ios::in);
	cout << "-----------------view development------------------" << endl;
	while (view_ >> cat){
		if (!strcmp(cat, "eye")){
			for (int i = 0; i < 3; i++){
				view_ >> (GLfloat)eye[i];
				cout << eye[i] << " ";
			}
			cout << endl;
		}
		else if (!strcmp(cat, "vat")){
			for (int i = 0; i < 3; i++){
				view_ >> (GLfloat)vat[i];
				cout << vat[i] << " ";
			}
			cout << endl;
		}
		else if (!strcmp(cat, "vup")){
			for (int i = 0; i < 3; i++){
				view_ >> (GLfloat)vup[i];
				cout << vup[i] << " ";
			}
			cout << endl;
		}
		else if (!strcmp(cat, "fovy")){
			view_ >> (GLfloat)fovy;
			cout << fovy << endl;
		}
		else if (!strcmp(cat, "dnear")){
			view_ >> (GLfloat)dnear;
			cout << dnear << endl;
		}
		
		else if (!strcmp(cat, "dfar")){
			view_ >> (GLfloat)dfar;
			cout << dfar << endl;
		}
		else if (!strcmp(cat, "viewport")){
			for (int i = 0; i < 4; i++){
				view_ >> (GLfloat)viewport[i];
				cout << viewport[i] << " ";
			}
			cout << endl;
		}
	}
	view_.close();
}
void handle_light(){
	cout << "-----------------light creation-------------------" << endl;
	char buffer[10];
	fstream light_;
	light_.open("CornellBox.light", ios::in);
	while (light_ >> buffer){	
		if (!strcmp(buffer, "light")){
			for (int i = 0; i < 3; i++){
				light_ >> (GLfloat)light_position[how_many_light][i];
				cout << light_position[how_many_light][i] << " ";
			}light_position[how_many_light][3] = 1;
			cout << endl;
			for (int i = 0; i < 3; i++){
				light_ >> (GLfloat)light_ambient[how_many_light][i];
				cout << light_ambient[how_many_light][i] << " ";
			}light_ambient[how_many_light][3] = 1;	cout << endl;
			for (int i = 0; i < 3; i++){
				light_ >> (GLfloat)light_diffuse[how_many_light][i];
				cout << light_diffuse[how_many_light][i] << " ";
			}light_diffuse[how_many_light][3] = 1;	cout << endl;
			for (int i = 0; i < 3; i++){
				light_ >> (GLfloat)light_specular[how_many_light][i];
				cout << light_specular[how_many_light][i] << " ";
			}light_specular[how_many_light][3] = 1;	cout << endl;
			how_many_light++;
		}
		else if (!strcmp(buffer, "ambient")){
			for (int i = 0; i < 3; i++){
				light_ >> (GLfloat)light_ambient[how_many_light][i];
				cout << light_ambient[how_many_light][i] << " ";
			}	cout << endl;
		}
	}
	light_.close();

}
void reshape(GLsizei w, GLsizei h)
{
	windowSize[0] = w;
	windowSize[1] = h;
}