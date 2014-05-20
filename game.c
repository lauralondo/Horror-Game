/*
 *	[Horror Game]
 *
 *	[description]
 *
 * 	Directions:
 *		[w] move forward
 *		[s] move backwards
 *		[a] strafe left
 *		[d] strafe right
 *
 *		[mouse click & drag] look around
 *		OR 		[q] look left
 *				[e] look right      (but really, its much nicer with the mouse)
 *
 *		[spacebar] 	jump
 *  	   [h]		show / hide help menu
 *		  [esc]		quit
 */


#include <GL/glut.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#define screenWidth 1000	//initial screem width
#define screenHeight 700	//initial screen height
#define PI 3.14159265		//pi
#define groundSize 20 		//size of the ground grid
#define waitTime 16 		//millisecond wait between redisplays
#define movementSpeed 0.08 	//player movement speed


#define numTextures 9
int textures[numTextures];	//the loaded textures



float lightPos[3][3] = { 	{ -2, 2, -6.0},	    //position for each light
							{-12.5, 1.2, -1.5},
							{ 0.2, 1.0,-5.0} };

float xpos = 0, ypos=0, zpos = 10;				//camera position
float xrot=0, yrot=0;							//camera angle
float xrotChange, yrotChange = 0;				//camera view attributes

int w_state, a_state, s_state,					//key presses
	d_state, q_state, e_state = 0;
int mousePressed, mouseStartX, mouseStartY = 0;	//mouse states

int jumpRising=0;								//if jumping up
float jumpSpeed=0;								//jump height increasing

int helpMenu = 0;								//true if displaying menu

GLUquadricObj *qobj;
float fireKc = 0;
float fireJitter = 0;


//Function to write a string to the screen at a specified location
void bitmapText(float x, float y, float z, char* words) {
	int len = 0, i = 0;
	//Set the location where the string should appear
	glRasterPos3f(x,y,z);
	len = (int) strlen(words);
	//Set the character in the string to helvetica size 18
	for(int i = 0; i < len; i++) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,words[i]);
	}
} //end bitmapText


//switches to 2D when true to draw on the front of the screen for the menu
void menuMode(int flag) {
	if(flag == 1) { //if true, enable 2D mode
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();

		gluOrtho2D(0, screenWidth, 0, screenHeight);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		glDisable(GL_DEPTH_TEST);
	}
	else {	//else, resume 3D mode
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		glEnable(GL_DEPTH_TEST);
	}
} //end menuMode


// draws the menu in menu mode if the help menu is toggled on
void menu(void) {
	if(helpMenu) {
		menuMode(1); 						//go into menu mode (switch to 2D)

		float mPosX = 700, mPosY=570;		//menu start position
		glColor3f(0.8,0.8,0.8);

		bitmapText(mPosX+95,mPosY+100, 0,              "MENU");
											//____________________________
		glBegin(GL_LINES);					    //line underneath "MENU"
		glVertex3f(mPosX, mPosY+90, 0);
		glVertex3f(mPosX+250, mPosY+90, 0);
		glEnd();

		bitmapText(mPosX+10,mPosY+50, 0,     "mouse click & drag to rotate");

		bitmapText(mPosX+90,mPosY, 0,                  "forward");
		bitmapText(mPosX, mPosY-25, 0, "rotate  [q]      [w]      [e]  rotate");

		bitmapText(mPosX+23,mPosY-75,0, "left  [a]       [s]      [d]  right");
		bitmapText(mPosX+105,mPosY-100, 0,              "back");

		bitmapText(mPosX+80,mPosY-150, 0,	         "[spacebar]");
		bitmapText(mPosX+105,mPosY-175, 0,	            "jump");

		bitmapText(mPosX+111,mPosY-225, 0,	            "[h]");
		bitmapText(mPosX+30,mPosY-250, 0,	   "show / hide this menu");

		bitmapText(mPosX+102,mPosY-300, 0,	           "[esc]");
		bitmapText(mPosX+105,mPosY-325, 0,	            "quit");

		menuMode(0);						//switch back to 3D mode
	}
} //end menu


//Loads a texture from an external image file in .bmp format.
void LoadTex(GLuint texture, char *s) {
	//unsigned int Texture;
	FILE* img = NULL;
	img = fopen(s,"rb");
	unsigned long bWidth = 0;
	unsigned long bHeight = 0;
	unsigned long size = 0;

	// Format specific stuff
	fseek(img,18,SEEK_SET);
	fread(&bWidth,4,1,img);
	fread(&bHeight,4,1,img);
	fseek(img,0,SEEK_END);
	size = ftell(img) - 54;

	unsigned char *data = (unsigned char*)malloc(size);

	fseek(img,54,SEEK_SET);	// image data
	fread(data,size,1,img);
	fclose(img);

	//glGenTextures(2, textures);

	glBindTexture(GL_TEXTURE_2D, texture);

	//allows shading to affect textured surfaces
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	// Sets the wrap parameters in both directions
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//using GL_LINEAR_MIPMAP_LINEAR gets rid of texture aliasing!!!
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR_MIPMAP_LINEAR);

	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, bWidth, bHeight,
					  GL_BGR_EXT, GL_UNSIGNED_BYTE, data);


	if (data)		//free allocated space
		free(data);
} //end LoadTex


//texture initializations
void initTex(void) {
	//Specifies the alignment requirement
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	//generate textures
	glGenTextures(numTextures, textures);
	LoadTex(textures[0], "textures/woodplanks1.bmp");
	LoadTex(textures[1], "textures/dirt2.bmp");
	LoadTex(textures[2], "textures/safe.bmp");
	LoadTex(textures[3], "textures/wallpaper2.bmp");
	LoadTex(textures[4], "textures/woodpanel1.bmp");
	LoadTex(textures[5], "textures/woodpanel2.bmp");
	LoadTex(textures[6], "textures/fireplacetop.bmp");
	LoadTex(textures[7], "textures/fireplaceleg.bmp");
	LoadTex(textures[8], "textures/wood2.bmp");
} //end initTex


//define the current drawing material
void material(float red, float green, float blue, float shine) {
	float mat_specular[]={red, green, blue, 1.0};
	float mat_diffuse[] ={red, green, blue, 1.0};
	float mat_ambient[] ={0,0,0,1.0};//{red, green, blue, 1.0};
	float mat_shininess={shine};

	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialf(GL_FRONT, GL_SHININESS, mat_shininess);
}


//define light 0
void light0(void) {
	// GLfloat light_ambient0[]={0.05, 0.05, 0.05, 1.0};
	GLfloat light_ambient0[]={0.5, 0.5, 0.5, 1.0};
	GLfloat light_diffuse0[]={0.5, 0.5, 0.5, 1.0};
	GLfloat light_specular0[]={0.5, 0.5, 0.5, 1.0};
	GLfloat position0[]= {lightPos[0][0], lightPos[0][1], lightPos[0][2], 1.05};
	float kc = 0;//0.3;
	/* Set up ambient, diffuse, and specular components for light 0 */
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient0);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse0);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular0);
	glLightfv (GL_LIGHT0, GL_POSITION, position0);
	glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, kc);
}

//define light 0
void light1(void) {
	// GLfloat light_ambient0[]={0.05, 0.05, 0.05, 1.0};
	GLfloat light_ambient0[]={0.8, 0.7, 0.4, 1.0};
	GLfloat light_diffuse0[]={0.8, 0.7, 0.4, 1.0};
	GLfloat light_specular0[]={0.8, 0.7, 0.4, 1.0};
	GLfloat position0[] = {lightPos[1][0]+fireJitter,
							lightPos[1][1]+fireJitter,
							lightPos[1][2]+fireJitter, 1.0};
	float kc = fireKc;//0.3;
	/* Set up ambient, diffuse, and specular components for light 0 */
	glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient0);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse0);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular0);
	glLightfv (GL_LIGHT1, GL_POSITION, position0);
	glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, kc); //light fades in distance
}

//finds the cross product of two vectors
void crossProduct(float *save,float a[3], float b[3]) {
  save[0] = a[1]*b[2] - b[1]*a[2];
  save[1] = a[2]*b[0] - b[2]*a[0];
  save[2] = a[0]*b[1] - b[0]*a[1];
}


void normalize(float * vect) {	//scales a vector a length of 1
	float length;
	int a;

	length = sqrt(pow(vect[0],2) + pow(vect[1],2) + pow(vect[2],2));

	for (a=0;a<3;++a) {				//divides vector by its length to normalise
		vect[a]/=length;
	}
}


void getFaceNormal(float *norm,float pointa[3],float pointb[3],float pointc[3]){
  	float point[3][3];
  	int p, xyz;
  	//put the tree points into the point array
  	for (p=0; p<3; ++p) {
    	point[0][p] = pointa[p];
    	point[1][p] = pointb[p];
    	point[2][p] = pointc[p];
	}

  	//calculates vectors from point[0] to point[1] and point[0] to point[2]
    float vect[2][3];
  	for (p=0; p<2; ++p) {
  		for (int xyz=0; xyz<3; ++xyz) {
   			vect[p][xyz] = point[2-p][xyz] - point[0][xyz];
    	}
  	}

  	crossProduct(norm,vect[0],vect[1]);           	//calculates vector at 90° to to 2 vectors
  	normalize(norm);                                //makes the vector length 1
}




void initQObj(void) {
	qobj = gluNewQuadric();
  	//gluQuadricNormals(qobj, GLU_SMOOTH);
}
void cleanup(void) {// call once when you exit program
	gluDeleteQuadric(qobj);
	//
}



void enableFog(float r, float g, float b, float density) {
	glEnable(GL_FOG);

	GLfloat fogColor[4] = {r, g, b, 1.0};

	int fogMode = GL_EXP;
	glFogi (GL_FOG_MODE, fogMode);
	glFogfv (GL_FOG_COLOR, fogColor);
	glFogf (GL_FOG_DENSITY, density);
	glHint (GL_FOG_HINT, GL_DONT_CARE);
	glFogf (GL_FOG_START, 1.0);
	glFogf (GL_FOG_END, 5.0);

	glClearColor(r, g, b, 1.0);  /* fog color */
}

//creates a flat, textured rectangle
void texRect(void) {
	float norm[3];
	float point[4][3] = { {-1.0,-1.0, 0.0},
						  {-1.0, 1.0, 0.0},
						  { 1.0, 1.0, 0.0},
						  { 1.0,-1.0, 0.0}
						};

	getFaceNormal(norm,point[2],point[1],point[0]);
	glNormal3fv(norm);

	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0); glVertex3fv(point[0]);//(-1.0, -1.0, 0.0);
		glTexCoord2f(0.0, 1.0); glVertex3fv(point[1]);//(-1.0, 1.0, 0.0);
		glTexCoord2f(1.0, 1.0); glVertex3fv(point[2]);//(1.0, 1.0, 0.0);
		glTexCoord2f(1.0, 0.0); glVertex3fv(point[3]);//(1.0, -1.0, 0.0);
	glEnd();
	glDisable(GL_TEXTURE_2D);
} //end flatTex


//creates a flat, textured rectangle
void texRect2(float width, float height) {
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0); glVertex3f(-width, -height, 0.0);
		glTexCoord2f(0.0, height); glVertex3f(-width, height, 0.0);
		glTexCoord2f(width, height); glVertex3f(width, height, 0.0);
		glTexCoord2f(width, 0.0); glVertex3f(width, -height, 0.0);
	glEnd();
	glDisable(GL_TEXTURE_2D);
} //end flatTex

//creates a flat, textured rectangle
void texRectTiled(float width, float height) {
	float norm[3];
	float point[4][3];/* = { {-width,-height, 0.0},
						  {-width, height, 0.0},
						  { width, height, 0.0},
						  { width,-height, 0.0}
						};*/

	point[0][0]=-width; point[0][1]=-height; point[0][2]=0.0;
	point[1][0]=-width; point[1][1]= height; point[1][2]=0.0;
	point[2][0]= width; point[2][1]= height; point[2][2]=0.0;
	point[3][0]= width; point[3][1]=-height; point[3][2]=0.0;
	getFaceNormal(norm,point[2],point[1],point[0]);
	glNormal3fv(norm);

	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0,   0.0);    glVertex3fv(point[0]);
		glTexCoord2f(0.0,   height); glVertex3fv(point[1]);
		glTexCoord2f(width, height); glVertex3fv(point[2]);
		glTexCoord2f(width, 0.0);    glVertex3fv(point[3]);
	glEnd();
	glDisable(GL_TEXTURE_2D);
} //end flatTex


//creates a flat, textured rectangle
void texTile(float tileHeight, float tileWidth) {
	float norm[3];
	float point[4][3] = { {-1.0,-1.0, 0.0},
						  {-1.0, 1.0, 0.0},
						  { 1.0, 1.0, 0.0},
						  { 1.0,-1.0, 0.0}
						};

	getFaceNormal(norm,point[2],point[1],point[0]);
	glNormal3fv(norm);

	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0); glVertex3fv(point[0]);//(-1.0, -1.0, 0.0);
		glTexCoord2f(0.0, 1.0); glVertex3fv(point[1]);//(-1.0, 1.0, 0.0);
		glTexCoord2f(1.0, 1.0); glVertex3fv(point[2]);//(1.0, 1.0, 0.0);
		glTexCoord2f(1.0, 0.0); glVertex3fv(point[3]);//(1.0, -1.0, 0.0);
	glEnd();
	glDisable(GL_TEXTURE_2D);
} //end flatTex


void tiledTexWall(int width, int height) {
	for(int h=0; h<height; h++){
		for(int w=0; w<width; w++) {
			glPushMatrix();
			glTranslatef(w*2,h*2,0);
			texRect();
			glPopMatrix();
		}
	}
} //end tiledTexWall


//creates a flat, textured circle
void texCircle(int segments) {
	float cx=0, cy=0, cz=0;				//center point
	float radius = 1;					//radius of the circle

	//generate the first point along the radius
	float phi = 0;
	float x1 = radius * cos(phi) + cx;
	float z1 = radius * sin(phi) + cz;
	float first[3] = {x1, 0, z1};			//the first circle vertex
	float tx1 = 0.5 * cos(phi) + 0.5;
	float ty1 = 0.5 * sin(phi) + 0.5;
	float tFirst[2] = {tx1, ty1};			//the first texture vertex

	//loop throuhg all segments of the circle
	for (int i = 0; i<segments; i++) {		//for every segment,
		phi = 2 * PI * (i+1) / segments;
		x1 = radius * cos(phi) + cx;
		z1 = radius * sin(phi) + cz;
		float next[] = {x1, 0, z1};			//get the next circle vertex
		tx1 = 0.5 * cos(phi) + 0.5;
		ty1 = 0.5 * sin(phi) + 0.5;
		float tNext[2] = {tx1, ty1};		//get next texture vertex

		//draw top of the stump
		glBegin(GL_POLYGON);
			glTexCoord2f(0.5,0.5);
			glVertex3f(cx, 0, cz);
			glTexCoord2fv(tFirst);
			glVertex3fv(first);
			glTexCoord2fv(tNext);
			glVertex3fv(next);
		glEnd();

		//next point becomes the first for the next interation
		first[0] = next[0];
		first[1] = next[1];
		first[2] = next[2];
		tFirst[0] = tNext[0];
		tFirst[1] = tNext[1];
	}
} //end texCircle


//Models the ground. consists of a flat gorund color and a grid
void grid(void) {
	glColor3f(1,0,0);		//grid color
	glLineWidth(1);			//line width

	//draw grid
	for (int i = 0; i < groundSize*2+1; i++) { 		//lines along x-axis
		glBegin(GL_LINES);
			glVertex3f(-groundSize + i, 0, groundSize);
			glVertex3f(-groundSize + i, 0, -groundSize);
		glEnd();
	}
	for (int j = 0; j < groundSize*2+1; j++) {		//lines along z-axis
		glBegin(GL_LINES);
			glVertex3f(-groundSize, 0, groundSize - j);
			glVertex3f(groundSize, 0, groundSize - j);
		glEnd();
	}
} //end grid


//key object
void key(void) {
	int stackNslice = 20; //number of stacks and slices for all shapes

	//define key material
	float mat_specular[]={0.9, 0.9, 0.5, 1.0};
	float mat_diffuse[] ={0.7, 0.5, 0.05, 1.0};
	float mat_ambient[] ={0.1, 0.05, 0.01, 1.0};
	float mat_shininess={90};
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialf(GL_FRONT, GL_SHININESS, mat_shininess);

    glEnable(GL_NORMALIZE); //rescales normals
    glPushMatrix();
    glScalef(0.01,0.01,0.01);
		//ringed handle
		glPushMatrix();
			glRotatef(45,0,0,1);

			glutSolidTorus(1,5,stackNslice,stackNslice);

			glPushMatrix();
				glTranslatef(10,0,0);
				glutSolidTorus(1,5,stackNslice,stackNslice);
			glPopMatrix();

			glPushMatrix();
				glTranslatef(10,-10,0);
				glutSolidTorus(1,5,stackNslice,stackNslice);
			glPopMatrix();

			glPushMatrix();
				glTranslatef(0,-10,0);
				glutSolidTorus(1,5,stackNslice,stackNslice);
			glPopMatrix();

			glPushMatrix();
				glTranslatef(5,-5,0);
				glScalef(1.5,1.5,1.5);
				glutSolidTorus(0.5,5,stackNslice,stackNslice);
			glPopMatrix();
		glPopMatrix();

		//stem
		glPushMatrix();
			glScalef(0.85,1.3,0.8);
			glTranslatef(1.3,-4,0);

			//cylinder stem
			glPushMatrix();
			glTranslatef(7,56.5,0);
			glRotatef(90,1,0,0);
			gluCylinder(qobj, 1.5, 1.5, 41, stackNslice, stackNslice);
			glPopMatrix();

			//stem cap
			glPushMatrix();
			glTranslatef(7,56.5,0);
			glRotatef(90,1,0,0);
			gluCylinder(qobj, 0, 1.5, 0, stackNslice, stackNslice);
			glPopMatrix();

			//nub things
			glPushMatrix();
				glTranslatef(7,13,0);
				glScalef(1,0.6,1);
				glutSolidSphere(3.5, stackNslice,stackNslice);
			glPopMatrix();
			glPushMatrix();
				glTranslatef(7,15.5,0);
				glScalef(1,0.3,1);
				glutSolidSphere(2.5, stackNslice,stackNslice);
			glPopMatrix();
			glPushMatrix();
				glTranslatef(7,26,0);
				glScalef(1,0.3,1);
				glutSolidSphere(2.5, stackNslice,stackNslice);
			glPopMatrix();
			glPushMatrix();
				glTranslatef(7,36,0);
				glScalef(1,0.3,1);
				glutSolidSphere(2.5, stackNslice,stackNslice);
			glPopMatrix();
		glPopMatrix();

		//lower key blade
		glPushMatrix();
			glPushMatrix();
				glTranslatef(5,56,0);
				glScalef(1,1.2,0.3);
				glutSolidCube(4);
			glPopMatrix();

			glPushMatrix();
				glTranslatef(2.5,54.6,0);
				glScalef(0.3,0.5,0.3);
				glutSolidCube(4);
			glPopMatrix();

			glPushMatrix();
				glTranslatef(0,56,0);
				glScalef(1,1.2,0.3);
				glutSolidCube(4);
			glPopMatrix();
		glPopMatrix();

		//upper key blade
		glPushMatrix();
			glTranslatef(0,66,0);
			glRotatef(180,1,0,0);

			glPushMatrix();
				glTranslatef(5,2,0);
				glScalef(1,1.2,0.3);
				glutSolidCube(4);
			glPopMatrix();

			glPushMatrix();
				glTranslatef(2.5,0.6,0);
				glScalef(0.3,0.5,0.3);
				glutSolidCube(4);
			glPopMatrix();

			glPushMatrix();
				glTranslatef(0,2,0);
				glScalef(1,1.2,0.3);
				glutSolidCube(4);
			glPopMatrix();
		glPopMatrix();

	glPopMatrix();
	glDisable(GL_NORMALIZE);
} //end key


void safe(void) {

    
	//room 2 safe front

	glPushMatrix();

glTranslatef(20,2,9);

    glScalef(2, 2, 2);

glBindTexture(GL_TEXTURE_2D, textures[2]);

texRect();

glPopMatrix();

    

    //room 2 safe left side

    glPushMatrix();

glTranslatef(22,2,11);

    glRotatef(90, 0, 1, 0);

glBindTexture(GL_TEXTURE_2D, textures[0]);

texRect2(2,2);

glPopMatrix();

    

    //room 2 safe left side

    glPushMatrix();

glTranslatef(18,2,11);

    glRotatef(90, 0, 1, 0);

glBindTexture(GL_TEXTURE_2D, textures[0]);

texRect2(2,2);

glPopMatrix();

    

    //room 2 safe right side

    glPushMatrix();

glTranslatef(20,4,11);

    glRotatef(90, 1, 0, 0);

glBindTexture(GL_TEXTURE_2D, textures[0]);

texRect2(2,2);

glPopMatrix();

}



void fireplace(void) {
	glEnable(GL_TEXTURE_2D);

	//mantle front
	glPushMatrix();
		glTranslatef(0,2.5,0);
		glBindTexture(GL_TEXTURE_2D, textures[6]);
		glBegin(GL_QUADS);
		glTexCoord2f(0.01, 0.01);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(0.98, 0.01);  glVertex3f(5.0,0.0,0.0);
		glTexCoord2f(0.98, 0.18);  glVertex3f(5.0,0.7,0.0);
		glTexCoord2f(0.01, 0.18);   glVertex3f(0.0,0.7,0.0);
		glEnd();
	glPopMatrix();

	//mantle bottom
	glPushMatrix();
		glTranslatef(0,2.5,0.6);
		glRotatef(-90,1,0,0);
		glBindTexture(GL_TEXTURE_2D, textures[8]);
		glBegin(GL_QUADS);
		glTexCoord2f(1.0, 0.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(5.0,0.0,0.0);
		glTexCoord2f(0.8, 1.0);  glVertex3f(5.0,0.6,0.0);
		glTexCoord2f(0.8, 0.0);   glVertex3f(0.0,0.6,0.0);
		glEnd();
	glPopMatrix();

	//mantle top
	glPushMatrix();
		glTranslatef(0,3.2,0.0);
		glRotatef(90,1,0,0);
		glBindTexture(GL_TEXTURE_2D, textures[8]);
		glBegin(GL_QUADS);
		glTexCoord2f(1.0, 0.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(5.0,0.0,0.0);
		glTexCoord2f(0.8, 1.0);  glVertex3f(5.0,0.6,0.0);
		glTexCoord2f(0.8, 0.0);   glVertex3f(0.0,0.6,0.0);
		glEnd();
	glPopMatrix();

	//right leg front
	glPushMatrix();
		glBindTexture(GL_TEXTURE_2D, textures[7]);
		glBegin(GL_QUADS);
		glTexCoord2f(0.01, 0.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(0.21, 0.0);  glVertex3f(0.6,0.0,0.0);
		glTexCoord2f(0.21, 1.0);  glVertex3f(0.6,2.51,0.0);
		glTexCoord2f(0.01, 1.0);   glVertex3f(0.0,2.51,0.0);
		glEnd();
	glPopMatrix();

	//right leg right side
	glPushMatrix();
		glTranslatef(0,0,0.6);
		glRotatef(90,0,1,0);
		glBindTexture(GL_TEXTURE_2D, textures[7]);
		glBegin(GL_QUADS);
		glTexCoord2f(0.01, 0.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(0.21, 0.0);  glVertex3f(0.6,0.0,0.0);
		glTexCoord2f(0.21, 1.0);  glVertex3f(0.6,2.51,0.0);
		glTexCoord2f(0.01, 1.0);   glVertex3f(0.0,2.51,0.0);
		glEnd();
	glPopMatrix();

	//right leg left side
	glPushMatrix();
		glTranslatef(0.6,0,0);
		glRotatef(-90,0,1,0);
		glBindTexture(GL_TEXTURE_2D, textures[7]);
		glBegin(GL_QUADS);
		glTexCoord2f(0.01, 0.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(0.21, 0.0);  glVertex3f(0.6,0.0,0.0);
		glTexCoord2f(0.21, 1.0);  glVertex3f(0.6,2.51,0.0);
		glTexCoord2f(0.01, 1.0);   glVertex3f(0.0,2.51,0.0);
		glEnd();
	glPopMatrix();

	//left leg front
	glPushMatrix();
	glTranslatef(4.4,0,0);
	glPushMatrix();
		glBindTexture(GL_TEXTURE_2D, textures[7]);
		glBegin(GL_QUADS);
		glTexCoord2f(0.01, 0.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(0.21, 0.0);  glVertex3f(0.6,0.0,0.0);
		glTexCoord2f(0.21, 1.0);  glVertex3f(0.6,2.51,0.0);
		glTexCoord2f(0.01, 1.0);   glVertex3f(0.0,2.51,0.0);
		glEnd();
	glPopMatrix();

	//left leg right side
	glPushMatrix();
		glTranslatef(0,0,0.6);
		glRotatef(90,0,1,0);
		glBindTexture(GL_TEXTURE_2D, textures[7]);
		glBegin(GL_QUADS);
		glTexCoord2f(0.01, 0.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(0.21, 0.0);  glVertex3f(0.6,0.0,0.0);
		glTexCoord2f(0.21, 1.0);  glVertex3f(0.6,2.51,0.0);
		glTexCoord2f(0.01, 1.0);   glVertex3f(0.0,2.51,0.0);
		glEnd();
	glPopMatrix();

	//left leg left side
	glPushMatrix();
		glTranslatef(0.6,0,0);
		glRotatef(-90,0,1,0);
		glBindTexture(GL_TEXTURE_2D, textures[7]);
		glBegin(GL_QUADS);
		glTexCoord2f(0.01, 0.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(0.21, 0.0);  glVertex3f(0.6,0.0,0.0);
		glTexCoord2f(0.21, 1.0);  glVertex3f(0.6,2.51,0.0);
		glTexCoord2f(0.01, 1.0);   glVertex3f(0.0,2.51,0.0);
		glEnd();
	glPopMatrix();
	glPopMatrix();

	glDisable(GL_TEXTURE_2D);
}

//makes fire flickering light
void fireLight(void) {

} //end fireLight






//______________________________________________________________________________
//================================= CALLBACKS ==================================
//______________________________________________________________________________

//display callack.
void display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 	glLoadIdentity();
 	glDisable(GL_LIGHTING);
 	//view changes
 	glRotatef(xrot+xrotChange, 1,0,0);	//viewer x rotation
 	glRotatef(yrot+yrotChange, 0,1,0);	//viewer y rotation
 	glTranslatef(-xpos,-ypos,-zpos);	//viewer position
	gluLookAt(0,3,0,  0,3,5,  0,1,0);	//camera

	//grid();							//draw ground

	//draw sphere for light 0
	light0();
	glColor3f(1,1,0.7);
	glPushMatrix();
	glTranslatef(lightPos[0][0], lightPos[0][1], lightPos[0][2]);
	glutSolidSphere(0.05,10,30);
	glPopMatrix();

	light1();
	glColor3f(1,0.7,0.3);
	glPushMatrix();
	glTranslatef(lightPos[1][0]+fireJitter,
			 	 lightPos[1][1]+fireJitter,
				 lightPos[1][2]+fireJitter);
	glutSolidSphere(0.05,10,30);
	glPopMatrix();

	glEnable(GL_LIGHTING);


	// GLfloat dim_light[] = {0.0f, 0.0f, 0.0f, 1.0f};
	// glLightModelfv(GL_LIGHT_MODEL_AMBIENT, dim_light);
	// //repositions specular reflections for view change
	// glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
	// glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);

	//glEnable(GL_POLYGON_SMOOTH);


	//key
	glPushMatrix();
	glTranslatef(0,2.5,-0.5);
	key();
	glPopMatrix();

	//scale
	glPushMatrix();
		glTranslatef(12,0,4);
		glPushMatrix();
			glRotatef(-90,1,0,0);
			gluCylinder(qobj, 0.1,0.3,3,20,20);
		glPopMatrix();

		glPushMatrix();
		glTranslatef(0,3.3,0);
		glutSolidSphere(0.25,20,20);
		glPopMatrix();
	glPopMatrix();



	glPushMatrix();
	glTranslatef(-10,0,0);
	safe();
	glPopMatrix();



	//textured objects
	//allows texture lighting
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glPushMatrix();
	glTranslatef(-12.4,0,-3.5);
	glRotatef(-90,0,1,0);
	fireplace();
	glPopMatrix();





	//room 1
	{
		glBindTexture(GL_TEXTURE_2D, textures[0]); //wood planks
		material(1,0.8,0.5,5);                     //material properties
		//room 1 floor
		glPushMatrix();
			glTranslatef(-12,0,-12);
			glRotatef(90,1,0,0);
			tiledTexWall(13,13);
		glPopMatrix();

		material(1,0.8,0.5,5);

		//room1 wall 1
		glPushMatrix();
			glTranslatef(12,1,-13);
			glRotatef(180,0,1,0);
			glBindTexture(GL_TEXTURE_2D, textures[5]); //wallpaper
			tiledTexWall(13,1);
			glPushMatrix();
				glBindTexture(GL_TEXTURE_2D, textures[3]); //wood panels
				glTranslatef(0,2,0);
				tiledTexWall(13,7);
			glPopMatrix();
		glPopMatrix();

		//room1 wall 2
		glPushMatrix();
			glTranslatef(13,1,12);
			glRotatef(90,0,1,0);
			glBindTexture(GL_TEXTURE_2D, textures[5]); //wood panels
			tiledTexWall(13,1);
			glPushMatrix();
				glBindTexture(GL_TEXTURE_2D, textures[3]); //wallpaper
				glTranslatef(0,2,0);
				tiledTexWall(13,7);
			glPopMatrix();
		glPopMatrix();

		//room1 wall 3
		glPushMatrix();
			glTranslatef(-12,1,13);
			glBindTexture(GL_TEXTURE_2D, textures[5]); //wood panels
			tiledTexWall(13,1);
			glPushMatrix();
				glBindTexture(GL_TEXTURE_2D, textures[3]); //wallpaper
				glTranslatef(0,2,0);
				tiledTexWall(13,7);
			glPopMatrix();
		glPopMatrix();


		//room1 wall 4
		glPushMatrix();
			glTranslatef(-13,1,-12);
			glRotatef(-90,0,1,0);
			glBindTexture(GL_TEXTURE_2D, textures[5]); //wood panels
			tiledTexWall(13,1);
			glPushMatrix();
				glBindTexture(GL_TEXTURE_2D, textures[3]); //wallpaper
				glTranslatef(0,2,0);
				tiledTexWall(13,7);
			glPopMatrix();
		glPopMatrix();

		//room 1 ceiling
		glBindTexture(GL_TEXTURE_2D, textures[4]);
		glPushMatrix();
			glTranslatef(-12,10,12);
			glRotatef(-90,1,0,0);
			tiledTexWall(13,13);
		glPopMatrix();
	}

	//old room 1
	/*
		// //room 1 floor
		// glPushMatrix();
		// glRotatef(90,1,0,0);
		// glBindTexture(GL_TEXTURE_2D, textures[0]);
		// texRectTiled(13,13);
		// glPopMatrix();


		// //room 1 wall
		// glPushMatrix();
		// glTranslatef(0,8,-13);
		// glRotatef(180,0,1,0);
		// glBindTexture(GL_TEXTURE_2D, textures[1]);
		// texRectTiled(13,8);
		// glPopMatrix();

		// //room 1 wall
		// glPushMatrix();
		// glTranslatef(0,8,13);
		// glBindTexture(GL_TEXTURE_2D, textures[1]);
		// texRectTiled(13,8);
		// glPopMatrix();


		// //room 1 wall
		// glPushMatrix();
		// glTranslatef(-13,8,0);
		// glRotatef(-90, 0,1,0);
		// glBindTexture(GL_TEXTURE_2D, textures[1]);
		// texRectTiled(13,8);
		// glPopMatrix();


		// //room 1 ceiling
		// glPushMatrix();
		// glTranslatef(0,16,0);
		// glRotatef(-90, 1,0,0);
		// glBindTexture(GL_TEXTURE_2D, textures[0]);
		// texRectTiled(13,13);
		// glPopMatrix();
	*/





	//room 2
	{
		//room 2 floor
		glPushMatrix();
		glRotatef(90,1,0,0);
		glTranslatef(26,0,0);
		glBindTexture(GL_TEXTURE_2D, textures[1]);
		texRectTiled(13,13);
		glPopMatrix();

		// //room 2 wall
		// glPushMatrix();
		// glTranslatef(26,8,13);
		// glBindTexture(GL_TEXTURE_2D, textures[0]);
		// texRectTiled(13,8);
		// glPopMatrix();

		// //room 2 wall
		// glPushMatrix();
		// glTranslatef(26,8,-13);
		// glBindTexture(GL_TEXTURE_2D, textures[0]);
		// texRectTiled(13,8);
		// glPopMatrix();

		// //room 2 wall
		// glPushMatrix();
		// glRotatef(90, 0,1,0);
		// glTranslatef(0,13,13);
		// glBindTexture(GL_TEXTURE_2D, textures[1]);
		// texRectTiled(13,13);
		// glPopMatrix();
		//
		// 	//room 2 wall
		// glPushMatrix();
		// glRotatef(90, 0,1,0);
		// glTranslatef(0,8,39);
		// glBindTexture(GL_TEXTURE_2D, textures[1]);
		// texRectTiled(13,8);
		// glPopMatrix();

		// //room 1 ceiling
		// glPushMatrix();
		// glTranslatef(26,16,0);
		// glRotatef(90, 1,0,0);
		// glBindTexture(GL_TEXTURE_2D, textures[1]);
		// texRectTiled(13,13);
		// glPopMatrix();
	}




	menu();

	glutSwapBuffers();
} //end display





//sets key press states to true when the key gets pressed
void keyboard(unsigned char key, int x, int y) {
   	if(key == 'a') a_state = 1;		//strafe left
   	if(key == 'd') d_state = 1;		//strafe right
   	if(key == 'w') w_state = 1;		//move forward
   	if(key == 's') s_state = 1;		//move backward
   	if(key == 'e') e_state = 1;		//rotate view right
   	if(key == 'q') q_state = 1;		//rotate view left
   	if(key == 32) {					//spacebar: jump
   		if(ypos == 0.0)	{			//if viewer is on the ground,
   			jumpRising = 1;			//set rising state to true
   			jumpSpeed = 0.3;//1;    //set initial up speed to 1
   		}
   	}
   	if(key == 'h') {				//hide / show the help menu
   		if(helpMenu == 1) helpMenu = 0;
   		else helpMenu = 1;
   	}
   	if((int)key == 27) {
   		cleanup();
   		exit(0);		//exit program
   	}
} //end keyboard


//sets the key press states to false when the key is released
void keyboardUp(unsigned char key, int x, int y) {
	if(key == 'a') a_state = 0;		//stop strafe left
	if(key == 'd') d_state = 0;		//stop strafe right
	if(key == 'w') w_state = 0;		//stop move forward
	if(key == 's') s_state = 0;		//stop move backwards
	if(key == 'e') e_state = 0;		//stop rotate right
	if(key == 'q') q_state = 0;		//stop rotate left
} //end keyboardUp


void specialKey(int key, int x, int y) {
   	if(key == GLUT_KEY_UP) {
   		lightPos[0][0] +=1;
   	}
   	if(key == GLUT_KEY_DOWN) {
   		lightPos[0][0] -=1;
   	}
   	if(key == GLUT_KEY_RIGHT) {
   		lightPos[0][2] +=1;
   	}
   	if(key == GLUT_KEY_LEFT) {
   		lightPos[0][2] -=1;
   	}
}


// Handles the begining and end of a left mouse click for view rotation.
// The temporaty view rotation is applied when mouse click ends
void mouse(int butt, int state, int x,  int y) {
	if (state == GLUT_DOWN  &&  butt == GLUT_LEFT_BUTTON) {	//left click
		if(mousePressed == 0) {		//if this is the innitial click down,
			mouseStartX = x;		//save starting mouse x coordinate
			mouseStartY = y;		//save starting mouse y coordinate
		}
		mousePressed = 1;			//set mouse pressed state to true
	}
	else {							//else the left click is no longer pressed
		mousePressed = 0;			//set pressed state to false
		xrot += xrotChange;			//apply the x rotation change to make it permanent
		yrot += yrotChange;			//apply the y rotation change to make it permanent
		xrotChange = yrotChange = 0;//reset temporary rotation change to 0
	}
} //end mouse


// Changes the temporary view rotation while the left mouse button is pressed.
// The temporary rotation angle is proportional to the distance of the mouse
// pointer from the starting click point.
void motion(int x, int y) {
	if(mousePressed) {								//if the left button is pressed,
		xrotChange = (float)(y - mouseStartY)/3.0;	//set the temp x-axis rot to the mouse y distance

		//limit the x-axis rotation to prevent the camera from being able to flip upside-down
		if(xrot+xrotChange > 90.0) {	//if camera tries to flip over from above
			xrotChange = 90.0 - xrot;
		}
		if(xrot+xrotChange < -90.0) {	//if camera tries to flip over from below
			xrotChange = -90 - xrot;
		}
		yrotChange = (float)(x - mouseStartX)/3.0;	//set the temp y-axis rot to the mouse x distance
	}
} //end motion


//applies movements and rotation changes and redraws the world at set intervals
void timer(int value) {
	//rotation angles = permanent rotation + temporary rotation
	float newxrot = xrot + xrotChange;
	float newyrot = yrot + yrotChange;

	//viewer position change using the w a s d keys. Moves relative to the viewing angle.
	if (a_state) {								//a key is pressed (strafe left)
		float yrotrad;
		yrotrad = (newyrot / 180 * PI);
		xpos -= (float)(cos(yrotrad)) * movementSpeed;
		zpos -= (float)(sin(yrotrad)) * movementSpeed;
	}
	if (d_state) {								//d key is pressed (strafe right)
		float yrotrad;
		yrotrad = (newyrot / 180 * PI);
		xpos += (float)cos(yrotrad) * movementSpeed;
		zpos += (float)sin(yrotrad) * movementSpeed;
	}
	if (w_state) {								//w key is pressed (move forward)
		float xrotrad, yrotrad;
        yrotrad = (newyrot / 180 * PI);
        xrotrad = (newxrot / 180 * PI);
        xpos += (float)(sin(yrotrad)) * movementSpeed;
        zpos -= (float)(cos(yrotrad)) * movementSpeed;
	}
	if (s_state) {								//s key is pressed (move backwards)
		float xrotrad, yrotrad;
        yrotrad = (newyrot / 180 * PI);
        xrotrad = (newxrot / 180 * PI);
        xpos -= (float)(sin(yrotrad)) * movementSpeed;
        zpos += (float)(cos(yrotrad)) * movementSpeed;
	}

	//view rotation using the e and q keys
	if (q_state && !mousePressed) {				//q key is pressed (rotate left)
		yrot -= 1;
        if (yrot < -360)yrot += 360;
	}
	if (e_state && !mousePressed) {				//e key is pressed (rotate right)
		yrot += 1;
        if (yrot >360) yrot -= 360;
	}

	if (jumpRising){				//if jumping up,
		ypos += jumpSpeed;				//move higher
		jumpSpeed *= 0.9;				//decrease jump speed
		if(jumpSpeed < 0.1) {			//when jump speed slows,
			jumpRising = 0;					//no longer rising
			jumpSpeed *= -1;				//reverse speed
		}
	}
	else {							//not jumping up,
		if (ypos > 0.0){				//until we reach the ground,
			ypos += jumpSpeed;				//move lower
			jumpSpeed /= 0.9;				//increase falling speed
			if(ypos < 0.0) {				//if we reach the ground
				ypos = 0;						//land at 0
			}
		}
	}

	glutPostRedisplay();						//redraw scene
	glutTimerFunc(waitTime, timer, 1);			//set next timer
} //end timer


void fireTimer(int value) {
	fireJitter = (rand()%20)/20.0;
	fireKc = (rand()%20)/140.0 + 0.2;
	glutTimerFunc(100, fireTimer, 1);
}


//reshape callback. adjusts the clipping box & viewport. keeps proportions
void reshape(int w, int h) {
	float left=-0.1, right=0.1, bottom=-0.1, top=0.1, znear=0.15, zfar=150;
	float ratio = (float)h / (float)w;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (w <= h)
		glFrustum(left, right, bottom * ratio,
			top * ratio, znear, zfar);
	else
		glFrustum(left / ratio, right / ratio,
			bottom, top, znear, zfar);

	glMatrixMode(GL_MODELVIEW);
} //end reshape






//______________________________________________________________________________
//==================================== MAIN ====================================
//______________________________________________________________________________

//main function
int main(int argc, char **argv) {
	glutInit(&argc, argv);
 	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
 	glutInitWindowSize(screenWidth, screenHeight);
 	glutCreateWindow("Horror Game");
 	glEnable(GL_DEPTH_TEST);
 	glClearColor(0,0,0,0);

 	glShadeModel (GL_SMOOTH);	//smooth shading
 	light0();					//define the three lights
   	//glEnable(GL_LIGHT0);		//enble all three lights
   	glEnable(GL_LIGHT1);
   	glEnable(GL_LIGHTING);		//enable lighting
   	//repositions specular reflections for view change
	glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);


 	glutIgnoreKeyRepeat(1);		// disables glut from simulating key press and
 								// release repetitions when holding down a key

 	initTex();					//create the textures (saved in textures arrray)
 	initQObj();					//create a quadric object for glu shapes
 	srand(time(NULL));
 	//enableFog(0.5,0.5,0.5,0.05);  //fog
 	//enableFog(0.5,0.3,0.1,0.05); //warm



 	//event callbacks
 	glutDisplayFunc(display);			//display
 	glutReshapeFunc(reshape);			//reshape window
 	glutMouseFunc(mouse);				//mouse button clicks
 	glutMotionFunc(motion);				//mouse click movement
 	glutKeyboardFunc(keyboard);			//key presses
 	glutKeyboardUpFunc(keyboardUp);		//key release
 	glutSpecialFunc(specialKey);
 	glutTimerFunc(waitTime, timer, 1);	//redraws world at intervals
 	glutTimerFunc(100, fireTimer, 1);	//fire flicker

 	glutMainLoop();
	return 0;
} //end main
