/* Paul Nguyen, Monica Ramakrishnan, Laura Londo
 * Horror Game
 *
 * A haunted mansion in OpenGL.
 * click on the door keypad ot unlock the doors in the mansion.
 * click on a  door to open it.
 * click on the key or mace to pick it up.
 *
 * 	Directions:
 *		[w] move forward
 *		[s] move backwards
 *		[a] strafe left
 *		[d] strafe right
 *
 *		[mouse] look around
 *		[left-click] pickup objects or open doors
 *
 *		[spacebar] 	jump
 *  	   [h]		show / hide menu
 *		  [esc]		quit
 */


#include <GL/glut.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#define PI 3.14159265		//pi
#define groundSize 20 		//size of the ground grid
#define waitTime 16 		//millisecond wait between redisplays
#define movementSpeed 0.25 	//player movement speed
#define numTextures 41

#define numObjs 2 //number of objects that can be picked up
#define KEY 0     //object number for the key
#define MACE 1    //object number for the mace


int textures[numTextures];	//the loaded textures

int fireTexNum = 0;

int screenWidth = 1000;		//initial screem width
int screenHeight = 700;		//initial screen height
int screenCenterX = 500;
int screenCenterY = 350;

//position for each light
float lightPos[4][3] = { 	{-2.0,  3.5, -6.0},
							{-12.5, 1.2, -0.5},
							{ 45,  6.0, 0},
							{ 30,  6.0, 0}   };

float selectPos[3]={0,0,0};  //position of the selection sphere
float selRangeRadius = 3.0;  //distance from camera of the selection sphere
float selSphereRadius = 0.8; //radius of the selection sphere

                               // X   Y   Z     angle  radius
float objPos[numObjs][5] = {	{40.5,2.10,0.0,  90.0,  0.7 },	//KEY
						        {10.0,2.3,10.0,  90.0,  1 }};	//MACE

//position of the selection spheres for the doors
float door1Select[3] = {13,2.1,0};
float door2Select[3] = {32.8,2.1,0};

//radius of the door selection spheres
float doorSelRadius = 1.5;


int selected = -1; //the item held

//if the doors are open
int officeDoorOpen = 0;
int tortureRoomDoorOpen = 0;

//angle the doors are open by
float room1DoorRot = 0;
float room2DoorRot = 0;

//doors are curently closing
int door1Closing = 0;
int door2Closing = 0;

//keypad for unlocking doors
int keypadSelect[3] = {13,3.2,-5};
int keypadSelRadius = 1;
int doorUnlocked = 0;

float xpos = 0, ypos=0, zpos = 0;				//camera position
float xrot=0, yrot=90;							//camera angle
float xrotChange, yrotChange = 0;				//camera view attributes

int w_state, a_state, s_state,					//key presses
	d_state, q_state, e_state = 0;
int mousePressed, mouseStartX, mouseStartY = 0;	//mouse states

int jumpRising=0;								//if jumping up
float jumpSpeed=0;								//jump height increasing

int helpMenu = 0;								//true if displaying menu

GLUquadricObj *qobj;        //used for making glu shapes

float fireKc = 0;			//modifier for fire light
float fireJitter = 0;		//offset position of fire light







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
	LoadTex(textures[9], "textures/books.bmp");
	LoadTex(textures[10], "textures/marbleTile.bmp");
	LoadTex(textures[11], "textures/fire1.bmp");
	LoadTex(textures[12], "textures/fire2.bmp");
	LoadTex(textures[13], "textures/fire3.bmp");
	LoadTex(textures[14], "textures/fire4.bmp");
	LoadTex(textures[15], "textures/fire5.bmp");
	LoadTex(textures[16], "textures/fire6.bmp");
	LoadTex(textures[17], "textures/fire7.bmp");
	LoadTex(textures[18], "textures/fire8.bmp");
	LoadTex(textures[19], "textures/fire9.bmp");
	LoadTex(textures[20], "textures/fire10.bmp");
	LoadTex(textures[21], "textures/door1.bmp");
	LoadTex(textures[22], "textures/castIron.bmp");
	LoadTex(textures[23], "textures/metalGrip.bmp");
	LoadTex(textures[24], "textures/steel.bmp");
	LoadTex(textures[25], "textures/one.bmp");
	LoadTex(textures[26], "textures/two.bmp");
	LoadTex(textures[27], "textures/three.bmp");
	LoadTex(textures[28], "textures/four.bmp");
	LoadTex(textures[29], "textures/five.bmp");
	LoadTex(textures[30], "textures/six.bmp");
	LoadTex(textures[31], "textures/seven.bmp");
	LoadTex(textures[32], "textures/eight.bmp");
	LoadTex(textures[33], "textures/nine.bmp");
	LoadTex(textures[34], "textures/zero.bmp");
	LoadTex(textures[35], "textures/green.bmp");
	LoadTex(textures[36], "textures/red.bmp");
	LoadTex(textures[37], "textures/mirror2.bmp");
	LoadTex(textures[38], "textures/mirror3.bmp");
	LoadTex(textures[39], "textures/mirrorBroken.bmp");
	LoadTex(textures[40], "textures/forest.bmp");
} //end initTex

//define the current drawing material
void material(float red, float green, float blue, float alpha, float shine) {
	float mat_specular[]={red, green, blue, alpha};
	float mat_diffuse[] ={red, green, blue, alpha};
	float mat_ambient[] ={0,0,0,alpha};
	float mat_shininess={shine};

	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialf(GL_FRONT, GL_SHININESS, mat_shininess);
} //end material

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
} //end light0

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
} //end light1

void light2(void) {
	GLfloat light_ambient0[]={0.3, 0.3, 0.3, 1.0};
	GLfloat light_diffuse0[]={0.3, 0.3, 0.3, 1.0};
	GLfloat light_specular0[]={0.3, 0.3, 0.3, 1.0};
	GLfloat position0[]= {lightPos[2][0], lightPos[2][1], lightPos[2][2], 1.05};
	float kc = 0;//0.3;
	/* Set up ambient, diffuse, and specular components for light 0 */
	glLightfv(GL_LIGHT2, GL_AMBIENT, light_ambient0);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, light_diffuse0);
	glLightfv(GL_LIGHT2, GL_SPECULAR, light_specular0);
	glLightfv (GL_LIGHT2, GL_POSITION, position0);
	glLightf(GL_LIGHT2, GL_LINEAR_ATTENUATION, kc);
}

void light3(void) {
	GLfloat light_ambient0[]={0.2, 0.2, 0.2, 1.0};
	GLfloat light_diffuse0[]={0.2, 0.2, 0.2, 1.0};
	GLfloat light_specular0[]={0.2, 0.2, 0.2, 1.0};
	GLfloat position0[]= {lightPos[3][0], lightPos[3][1], lightPos[3][2], 1.05};
	float kc = 0;//0.3;
	/* Set up ambient, diffuse, and specular components for light 0 */
	glLightfv(GL_LIGHT3, GL_AMBIENT, light_ambient0);
	glLightfv(GL_LIGHT3, GL_DIFFUSE, light_diffuse0);
	glLightfv(GL_LIGHT3, GL_SPECULAR, light_specular0);
	glLightfv (GL_LIGHT3, GL_POSITION, position0);
	glLightf(GL_LIGHT3, GL_LINEAR_ATTENUATION, kc);
}

//finds the cross product of two vectors
void crossProduct(float *save,float a[3], float b[3]) {
  save[0] = a[1]*b[2] - b[1]*a[2];
  save[1] = a[2]*b[0] - b[2]*a[0];
  save[2] = a[0]*b[1] - b[0]*a[1];
} //end crosproduct

//normalize a vector
void normalize(float * vect) {	//scales a vector a length of 1
	float length;
	int a;

	length = sqrt(pow(vect[0],2) + pow(vect[1],2) + pow(vect[2],2));

	for (a=0;a<3;++a) {				//divides vector by its length to normalise
		vect[a]/=length;
	}
} //end normalize

//get face normal for the given 3 points
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
} //end getFaceNormal

//initialize quadric object
void initQObj(void) {
	qobj = gluNewQuadric();
  	//gluQuadricNormals(qobj, GLU_SMOOTH);
} //end initQObj

//free allocations
void cleanup(void) {// call once when you exit program
	gluDeleteQuadric(qobj);
	//
} //end cleanup

//enable for effects
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
} //end enableFog






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

	int on = glIsEnabled(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);

	glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0); glVertex3fv(point[0]);//(-1.0, -1.0, 0.0);
		glTexCoord2f(0.0, 1.0); glVertex3fv(point[1]);//(-1.0, 1.0, 0.0);
		glTexCoord2f(1.0, 1.0); glVertex3fv(point[2]);//(1.0, 1.0, 0.0);
		glTexCoord2f(1.0, 0.0); glVertex3fv(point[3]);//(1.0, -1.0, 0.0);
	glEnd();

	if(!on)glDisable(GL_TEXTURE_2D);
} //end flatTex

//creates a flat, textured rectangle
void texRect4(	float t0x, float t0y, float v0x, float v0y,
				float t1x, float t1y, float v1x, float v1y,
				float t2x, float t2y, float v2x, float v2y,
				float t3x, float t3y, float v3x, float v3y)
{

	float norm[3];
	float point[4][3] = { { v0x, v0y, 0},
						  { v1x, v1y, 0},
						  { v2x, v2y, 0},
						  { v3x, v3y, 0}
						};

	getFaceNormal(norm,point[2],point[1],point[0]);
	glNormal3fv(norm);

	int on = glIsEnabled(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);

	glBegin(GL_QUADS);
		glTexCoord2f(t0x, t0y); glVertex3fv(point[0]);//(-1.0, -1.0, 0.0);
		glTexCoord2f(t1x, t1y); glVertex3fv(point[1]);//(-1.0, 1.0, 0.0);
		glTexCoord2f(t2x, t2y); glVertex3fv(point[2]);//(1.0, 1.0, 0.0);
		glTexCoord2f(t3x, t3y); glVertex3fv(point[3]);//(1.0, -1.0, 0.0);
	glEnd();

	if(!on)glDisable(GL_TEXTURE_2D);
} //end flatTex

//creates a flat, textured rectangle
void texRect2(float width, float height) {
	int on = glIsEnabled(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0); glVertex3f(-width, -height, 0.0);
		glTexCoord2f(0.0, height); glVertex3f(-width, height, 0.0);
		glTexCoord2f(width, height); glVertex3f(width, height, 0.0);
		glTexCoord2f(width, 0.0); glVertex3f(width, -height, 0.0);
	glEnd();
	if(!on)glDisable(GL_TEXTURE_2D);
} //end flatTex

//creates a flat, textured rectangle
void sizedTexRect(float width, float height) {
	//float norm[3];
	float point[4][3] = { {-width,-height, 0.0},
						  {-width, height, 0.0},
						  { width, height, 0.0},
						  { width,-height, 0.0}
						};

	//getFaceNormal(norm,point[2],point[1],point[0]);
	//glNormal3fv(norm);

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

void defaultMaterial(void) {
	float mat_specular[]={0.9, 0.9, 0.5, 1.0};
	float mat_diffuse[] ={0.7, 0.5, 0.05, 1.0};
	float mat_ambient[] ={0.1, 0.05, 0.01, 1.0};
	float mat_shininess={90};
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialf(GL_FRONT, GL_SHININESS, mat_shininess);
}



int inOffice(void) {
	if(xpos > -13.2) return 1;
	else return 0;
}

int inHallway(void) {
	if(xpos < -13.2  &&  xpos > -33) return 1;
	else return 0;
}

int inTortureRoom(void) {
	if(xpos < -33) return 1;
	else return 0;
}





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
	int stackNslice = 40; //number of stacks and slices for all shapes

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
    glTranslatef(0,-0.25,0);
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

//safe
void safe(void) {
	// //set material
	float mat_specular[]={0.9, 0.9, 0.9, 1.0};
	float mat_diffuse[] ={0.6, 0.6, 0.6, 1.0};
	float mat_ambient[] ={0.1, 0.05, 0.01, 1.0};
	float mat_shininess={90};
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialf(GL_FRONT, GL_SHININESS, mat_shininess);
	//room 2 safe front
	glEnable(GL_NORMALIZE); //rescales normals
	glPushMatrix();
		glTranslatef(20,2,9);
	    glScalef(2, 2, 2);
		glBindTexture(GL_TEXTURE_2D, textures[2]);
		texRect();
	glPopMatrix();
	glDisable(GL_NORMALIZE);

	material(0.2,0.15,0.1,1.0,5);

	//set material
	float mat_specular2[]={0.9, 0.9, 0.5, 1.0};
	float mat_diffuse2[] ={0.7, 0.5, 0.05, 1.0};
	float mat_ambient2[] ={0.1, 0.05, 0.01, 1.0};
	float mat_shininess2={90};
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular2);
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient2);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse2);
    glMaterialf(GL_FRONT, GL_SHININESS, mat_shininess2);

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
} //end safe

//torch
void torch(void){
	glPushMatrix();
		glTranslatef(12,0,4);
		glPushMatrix();
			glRotatef(-90,1,0,0);
			gluCylinder(qobj, 0.05,0.15,1,10,10);
		glPopMatrix();

		glPushMatrix();
		glTranslatef(0,1,0);
		glutSolidSphere(0.125,10,10);
		glPopMatrix();
	glPopMatrix();
} //end torch

//mace
void mace(void){
	gluQuadricTexture(qobj, GL_TRUE);
	glEnable(GL_TEXTURE_2D);

	float mat_specular2[]={0.9, 0.8, 0.7, 1.0};
	float mat_diffuse2[] ={0.9, 0.8, 0.7, 1.0};
	float mat_ambient2[] ={0.02, 0.01, 0.01, 1.0};
	float mat_shininess2={10};
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular2);
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient2);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse2);
    glMaterialf(GL_FRONT, GL_SHININESS, mat_shininess2);


	glPushMatrix();
    glTranslatef(0.0,-1.2,0.0);
    glRotatef(45,0,1,0);
    glRotatef(-5,0,0,1);
		//handle
	    glPushMatrix();
	 	   glRotatef(-90,1,0,0);

		    //upper handle
			glBindTexture(GL_TEXTURE_2D, textures[22]);
		    glPushMatrix();
			    glTranslatef(0,0,1.2);
			    gluCylinder(qobj, 0.05, 0.05, 0.6, 20, 20);
		    glPopMatrix();
		    glPushMatrix();
			    glTranslatef(0,0,0.6);
			    gluCylinder(qobj, 0.05, 0.05, 0.6, 20, 20);
		    glPopMatrix();
		    //lower handle (grip)
		    glBindTexture(GL_TEXTURE_2D, textures[23]);
		    gluCylinder(qobj, 0.06,  0.06,  0.8, 20, 20);
		    //handle rings
		    glPushMatrix();
		    	glTranslatef(0,0,0.8);
		    	glutSolidTorus(0.025,0.05,10,10);
		    glPopMatrix();
		    glPushMatrix();
		    	glTranslatef(0,0,1.8);
		    	glutSolidTorus(0.025,0.05,20,20);
		    glPopMatrix();
		    glutSolidTorus(0.025,0.05,10,10);
		    glutSolidSphere(0.05, 10,10);
	    glPopMatrix();

	    //head
	    glBindTexture(GL_TEXTURE_2D, textures[22]);
	    glPushMatrix();
	    glTranslatef(0.0,2.0,0.0);
	    gluSphere(qobj,0.205,60,60);
	    glPopMatrix();

	    gluQuadricTexture(qobj, GL_FALSE);
		glDisable(GL_TEXTURE_2D);

		//define material
		float mat_specular[]={0.4, 0.4, 0.2, 1.0};
		float mat_diffuse[] ={0.3, 0.2, 0.1, 1.0};
		float mat_ambient[] ={0.02, 0.0, 0.0, 1.0};
		float mat_shininess={90};
		glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	    glMaterialf(GL_FRONT, GL_SHININESS, mat_shininess);


	    //Spike1
	    glPushMatrix();
	    glTranslatef(0.0,2.0,0.2);
	    //glRotatef();
	    glutSolidCone(0.05,0.1,10,10);
	    glPopMatrix();

	    //Spike2
	    glPushMatrix();
	    glTranslatef(0.0,2.0,-0.2);
	    glRotatef(180,0,1,0);
	    glutSolidCone(0.05,0.1,10,10);
	    glPopMatrix();

	    //Spike3
	    glPushMatrix();
	    glTranslatef(0.2,2.0,0.0);
	    glRotatef(90,0,1,0);
	    glutSolidCone(0.05,0.1,10,10);
	    glPopMatrix();

	    //Spike4
	    glPushMatrix();
	    glTranslatef(-0.2,2.0,0.0);
	    glRotatef(-90,0,1,0);
	    glutSolidCone(0.05,0.1,10,10);
	    glPopMatrix();
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
} //end mace

//vase
void vase(void) {
	double NSEGMENTS = 10;
    glPushMatrix();
    glRotatef(-90,1,0,0);
    gluCylinder(qobj, 0.4, 0.06, 1.0, NSEGMENTS, NSEGMENTS);
    glPopMatrix();

    //Spike4
    glPushMatrix();
    glTranslatef(0,1.2,0.0);
    glRotatef(90,1,0,0);
    gluCylinder(qobj, 0.25, 0.06, 0.5, NSEGMENTS, NSEGMENTS);
    glPopMatrix();
} //end vase

//fireplace
void fireplace(void) {
	glEnable(GL_TEXTURE_2D);
	//marble base =====================================
	glBindTexture(GL_TEXTURE_2D, textures[10]);
	//marble base top
	glPushMatrix();
	glTranslatef(0.5,0.1,-0.4);
	glRotatef(90,1,0,0);
	tiledTexWall(3,1);
	glPopMatrix();
	//marbe base front
	glPushMatrix();
	glTranslatef(0.5,-0.9,-1.4);
	tiledTexWall(3,1);
	glPopMatrix();
	//marbe base right side
	glPushMatrix();
	glTranslatef(-0.5,-0.9,-0.4);
	glRotatef(90,0,1,0);
	tiledTexWall(1,1);
	glPopMatrix();
	//marbe base right side
	glPushMatrix();
	glTranslatef(5.5,-0.9,-0.4);
	glRotatef(-90,0,1,0);
	tiledTexWall(1,1);
	glPopMatrix();



	glEnable(GL_TEXTURE_2D);


	// mantle front & sides ===============================
	glBindTexture(GL_TEXTURE_2D, textures[6]);
	// mantle right side
	glPushMatrix();
	glTranslatef(0,2.5,0.6);
	glRotatef(90,0,1,0);
	glBegin(GL_QUADS);
		glTexCoord2f(0.01, 0.01);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(0.14, 0.01);  glVertex3f(0.6,0.0,0.0);
		glTexCoord2f(0.14, 0.18);  glVertex3f(0.6,0.7,0.0);
		glTexCoord2f(0.01, 0.18);   glVertex3f(0.0,0.7,0.0);
	glEnd();
	glPopMatrix();
	// mantle left side
	glPushMatrix();
	glTranslatef(5,2.5,0);
	glRotatef(-90,0,1,0);
	glBegin(GL_QUADS);
		glTexCoord2f(0.01, 0.01);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(0.14, 0.01);  glVertex3f(0.6,0.0,0.0);
		glTexCoord2f(0.14, 0.18);  glVertex3f(0.6,0.7,0.0);
		glTexCoord2f(0.01, 0.18);   glVertex3f(0.0,0.7,0.0);
	glEnd();
	glPopMatrix();
	//mantle front
	glPushMatrix();
		glTranslatef(0,2.5,0);
		glBegin(GL_QUADS);
		glTexCoord2f(0.01, 0.01);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(0.98, 0.01);  glVertex3f(5.0,0.0,0.0);
		glTexCoord2f(0.98, 0.18);  glVertex3f(5.0,0.7,0.0);
		glTexCoord2f(0.01, 0.18);   glVertex3f(0.0,0.7,0.0);
		glEnd();
	glPopMatrix();




	//mantle top and bottom =============================
	glBindTexture(GL_TEXTURE_2D, textures[8]);
	//mantle bottom
	glPushMatrix();
		glTranslatef(0,2.5,0.6);
		glRotatef(-90,1,0,0);
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
		glBegin(GL_QUADS);
		glTexCoord2f(1.0, 0.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(5.0,0.0,0.0);
		glTexCoord2f(0.8, 1.0);  glVertex3f(5.0,0.6,0.0);
		glTexCoord2f(0.8, 0.0);   glVertex3f(0.0,0.6,0.0);
		glEnd();
	glPopMatrix();



	// legs ============================================
	glBindTexture(GL_TEXTURE_2D, textures[7]);
	//right leg front
	glPushMatrix();
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
		glBegin(GL_QUADS);
		glTexCoord2f(0.01, 0.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(0.21, 0.0);  glVertex3f(0.6,0.0,0.0);
		glTexCoord2f(0.21, 1.0);  glVertex3f(0.6,2.51,0.0);
		glTexCoord2f(0.01, 1.0);   glVertex3f(0.0,2.51,0.0);
		glEnd();
	glPopMatrix();
	glPopMatrix();
} //end fireplace

//makes fire animation
void fire(void) {
	//glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textures[fireTexNum+11]);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.1);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(0.0, 0.68);   glVertex3f(0.0,2.5,0.0);
		glTexCoord2f(1.0, 0.68);   glVertex3f(4.0,2.5,0.0);
		glTexCoord2f(1.0, 0.1);   glVertex3f(4.0,0.0,0.0);
	glEnd();
} //end fireLight

void bookshelf(void) {
    glBindTexture(GL_TEXTURE_2D, textures[9]);

    //books
    glPushMatrix();
    glTranslatef(2.2,2,11);
    sizedTexRect(.5, .5);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(1.2,2,11);
    sizedTexRect(.5, .5);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.2,2,11);
    sizedTexRect(.5, .5);
    glPopMatrix();


    glBindTexture(GL_TEXTURE_2D, textures[0]);

    //back of bookshelf
    glPushMatrix();
    glTranslatef(0,2,12.8);
    glRotatef(90,0,0,1);
    texRect2(3,3);
    glPopMatrix();

    // left side of bookshelf
    glPushMatrix();
    glTranslatef(3,2,12.8);
    glRotatef(90,0,1,0);
    texRect2(1.8,3.1);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(3,2,11);
    glRotatef(90,0,0,1);
    texRect2(3.1, .1);
    glPopMatrix();


    glPushMatrix();
    glTranslatef(3.1,2,12.8);
    glRotatef(90,0,1,0);
    texRect2(1.8,3.1);
    glPopMatrix();

    // right side of bookshelf
    glPushMatrix();
    glTranslatef(-3,2,12.8);
    glRotatef(90,0,1,0);
    texRect2(1.8,3.1);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-3,2,11);
    glRotatef(90,0,0,1);
    texRect2(3.1, .1);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-3.1,2,12.8);
    glRotatef(90,0,1,0);
    texRect2(1.8,3.1);
    glPopMatrix();

    //top of bookshelf
    glPushMatrix();
    glTranslatef(0,5,12.8);
    glRotatef(90,0,0,0);
    texRect2(3,1.8);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0,5.1,11);
    glRotatef(90,0,0,1);
    texRect2(.1,3);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(3,5.1,12.8);
    glRotatef(90,0,1,0);
    texRect2(1.8,.1);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-3,5.1,12.8);
    glRotatef(90,0,1,0);
    texRect2(1.8,.1);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0, 2, 12);
    glRotatef(90, 0, 1, 0);

    //shelves
    glPushMatrix();
    glTranslatef(0,2,0);
    glRotatef(-90,1,0,0);
    texRect2(1,3);
    glPopMatrix();

    //bottom of shelf 1
    glPushMatrix();
    glTranslatef(0,1.6,0);
    glRotatef(-90,1,0,0);
    texRect2(1,3);
    glPopMatrix();

    //side 1
    glPushMatrix();
    glTranslatef(0,1.8,-3);
    glRotatef(90,0,0,1);
    texRect2(.2,1);
    glPopMatrix();

    //side 2
    glPushMatrix();
    glTranslatef(0,1.8,3);
    glRotatef(90,0,0,1);
    texRect2(.2,1);
    glPopMatrix();

    //side 3
    glPushMatrix();
    glTranslatef(1,1.8,0);
    glRotatef(90, 0, 1, 0);
    texRect2(3,0.2);
    glPopMatrix();

    //side 4
    glPushMatrix();
    glTranslatef(-1,1.8,0);
    glRotatef(90, 0, 1, 0);
    texRect2(3,0.2);
    glPopMatrix();

    //second shelf
    //top of bookshelf
    glPushMatrix();
    glTranslatef(0,1,0);
    glRotatef(-90,1,0,0);
    texRect2(1,3);
    glPopMatrix();

    //bottom of table
    glPushMatrix();
    glTranslatef(0,0.6,0);
    glRotatef(-90,1,0,0);
    texRect2(1,3);
    glPopMatrix();

    //side 1
    glPushMatrix();
    glTranslatef(0,0.8,-3);
    glRotatef(90,0,0,1);
    texRect2(.2,1);
    glPopMatrix();

    //side 2
    glPushMatrix();
    glTranslatef(0,0.8,3);
    glRotatef(90,0,0,1);
    texRect2(.2,1);
    glPopMatrix();

    //side 3
    glPushMatrix();
    glTranslatef(1,0.8,0);
    glRotatef(90, 0, 1, 0);
    texRect2(3,0.2);
    glPopMatrix();

    //side 4
    glPushMatrix();
    glTranslatef(-1,0.8,0);
    glRotatef(90, 0, 1, 0);
    texRect2(3,0.2);
    glPopMatrix();

    //third shelf
    //top of bookshelf
	//glPushMatrix();
	//glTranslatef(0,0,0);
	//glRotatef(-90,1,0,0);
	//texRect2(1,3);
	//glPopMatrix();

    //bottom of table
    glPushMatrix();
    glTranslatef(0,-.6,0);
    glRotatef(-90,1,0,0);
    texRect2(1,3);
    glPopMatrix();

    //side 1
    glPushMatrix();
    glTranslatef(0,-.8,-3);
    glRotatef(90,0,0,1);
    texRect2(.2,1);
    glPopMatrix();

    //side 2
    glPushMatrix();
    glTranslatef(0,-.8,3);
    glRotatef(90,0,0,1);
    texRect2(.2,1);
    glPopMatrix();

    //side 3
    glPushMatrix();
    glTranslatef(1,-.8,0);
    glRotatef(90, 0, 1, 0);
    texRect2(3,0.2);
    glPopMatrix();

    //side 4
    glPushMatrix();
    glTranslatef(-1,-.8,0);
    glRotatef(90, 0, 1, 0);
    texRect2(3,0.2);
    glPopMatrix();

    glPopMatrix();
} //end bookshelf

//table
void table(void) {
    int stackNslice = 10;
    //top of table
    glPushMatrix();
    glTranslatef(0,2,0);
    glRotatef(-90,1,0,0);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    texRect2(1,3);
    glPopMatrix();

    //side 1
    glPushMatrix();
    glTranslatef(0,1.8,-3);
    glRotatef(90,0,0,1);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    texRect2(.2,1);
    glPopMatrix();

    //side 2
    glPushMatrix();
    glTranslatef(0,1.8,3);
    glRotatef(90,0,0,1);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    texRect2(.2,1);
    glPopMatrix();

    //side 3
    glPushMatrix();
    glTranslatef(1,1.8,0);
    glRotatef(90, 0, 1, 0);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    texRect2(3,0.2);
    glPopMatrix();

    //side 4
    glPushMatrix();
    glTranslatef(-1,1.8,0);
    glRotatef(90, 0, 1, 0);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    texRect2(3,0.2);
    glPopMatrix();

    // legs
    glPushMatrix();
    glTranslatef(0.8,2,-2.5);
    glRotatef(90,1,0,0);
    gluCylinder(qobj, .1, .1, 2, stackNslice, stackNslice);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-0.8,2,-2.5);
    glRotatef(90,1,0,0);
    gluCylinder(qobj, .1, .1, 2, stackNslice, stackNslice);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.8,2,2.5);
    glRotatef(90,1,0,0);
    gluCylinder(qobj, .1, .1, 2, stackNslice, stackNslice);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-0.8,2,2.5);
    glRotatef(90,1,0,0);
    gluCylinder(qobj, .1, .1, 2, stackNslice, stackNslice);
    glPopMatrix();
} //end table

//chair
void chair(void){
	//glEnable(GL_TEXTURE_2D);

	//LEG#1
	glPushMatrix();
		glTranslatef(0,0,0);
		glRotatef(90,0,0,0);
		glBindTexture(GL_TEXTURE_2D, textures[8]);
		glBegin(GL_QUADS);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(0.1,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(0.1,2.5,0.0);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,2.5,0.0);
		glEnd();
	glPopMatrix();

	glPushMatrix();
		glTranslatef(0,0,0);
		glRotatef(90,0,1,0);
		glBindTexture(GL_TEXTURE_2D, textures[8]);
		glBegin(GL_QUADS);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(0.1,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(0.1,2.5,0.0);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,2.5,0.0);
		glEnd();
	glPopMatrix();

	glPushMatrix();
		glTranslatef(0.1,0,-0.1);
		glRotatef(180,0,1,0);
		glBindTexture(GL_TEXTURE_2D, textures[8]);
		glBegin(GL_QUADS);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(0.1,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(0.1,2.5,0.0);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,2.5,0.0);

		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,2.5,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(0.1,2.5,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(0.1,0.0,0.0);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,0.0,0.0);
		glEnd();
	glPopMatrix();

	glPushMatrix();
		glTranslatef(0.1,0,0.0);
		glRotatef(90,0,1,0);
		glBindTexture(GL_TEXTURE_2D, textures[8]);
		glBegin(GL_QUADS);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(0.1,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(0.1,2.5,0.0);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,2.5,0.0);
		glEnd();
	glPopMatrix();

	//LEG#2
	glPushMatrix();
		glTranslatef(1+0,0,0);
		glRotatef(90,0,0,0);
		glBindTexture(GL_TEXTURE_2D, textures[8]);
		glBegin(GL_QUADS);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(0.1,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(0.1,2.5,0.0);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,2.5,0.0);
		glEnd();
	glPopMatrix();

	glPushMatrix();
		glTranslatef(1+0,0,0);
		glRotatef(90,0,1,0);
		glBindTexture(GL_TEXTURE_2D, textures[8]);
		glBegin(GL_QUADS);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(0.1,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(0.1,2.5,0.0);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,2.5,0.0);
		glEnd();
	glPopMatrix();

	glPushMatrix();
		glTranslatef(1+0.1,0,-0.1);
		glRotatef(180,0,1,0);
		glBindTexture(GL_TEXTURE_2D, textures[8]);
		glBegin(GL_QUADS);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(0.1,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(0.1,2.5,0.0);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,2.5,0.0);
		glEnd();
	glPopMatrix();

	glPushMatrix();
		glTranslatef(1+0.1,0,0.0);
		glRotatef(90,0,1,0);
		glBindTexture(GL_TEXTURE_2D, textures[8]);
		glBegin(GL_QUADS);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(0.1,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(0.1,2.5,0.0);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,2.5,0.0);
		glEnd();
	glPopMatrix();

	//LEG#3 - SHORT
	glPushMatrix();
		glTranslatef(1+0,0,0 + 1);
		glRotatef(90,0,0,0);
		glBindTexture(GL_TEXTURE_2D, textures[8]);
		glBegin(GL_QUADS);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(0.1,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(0.1,1.5,0.0);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,1.5,0.0);
		glEnd();
	glPopMatrix();

	glPushMatrix();
		glTranslatef(1+0,0,0 + 1);
		glRotatef(90,0,1,0);
		glBindTexture(GL_TEXTURE_2D, textures[8]);
		glBegin(GL_QUADS);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(0.1,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(0.1,1.5,0.0);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,1.5,0.0);
		glEnd();
	glPopMatrix();

	glPushMatrix();
		glTranslatef(1+0.1,0,-0.1 + 1);
		glRotatef(180,0,1,0);
		glBindTexture(GL_TEXTURE_2D, textures[8]);
		glBegin(GL_QUADS);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(0.1,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(0.1,1.5,0.0);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,1.5,0.0);
		glEnd();
	glPopMatrix();

	glPushMatrix();
		glTranslatef(1+0.1,0,0.0 + 1);
		glRotatef(90,0,1,0);
		glBindTexture(GL_TEXTURE_2D, textures[8]);
		glBegin(GL_QUADS);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(0.1,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(0.1,1.5,0.0);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,1.5,0.0);
		glEnd();
	glPopMatrix();

	//LEG#4 - SHORT
	glPushMatrix();
		glTranslatef(0,0,0 + 1);
		glRotatef(90,0,0,0);
		glBindTexture(GL_TEXTURE_2D, textures[8]);
		glBegin(GL_QUADS);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(0.1,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(0.1,1.5,0.0);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,1.5,0.0);
		glEnd();
	glPopMatrix();

	glPushMatrix();
		glTranslatef(0,0,0 + 1);
		glRotatef(90,0,1,0);
		glBindTexture(GL_TEXTURE_2D, textures[8]);
		glBegin(GL_QUADS);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(0.1,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(0.1,1.5,0.0);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,1.5,0.0);
		glEnd();
	glPopMatrix();

	glPushMatrix();
		glTranslatef(0.1,0,-0.1 + 1);
		glRotatef(180,0,1,0);
		glBindTexture(GL_TEXTURE_2D, textures[8]);
		glBegin(GL_QUADS);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(0.1,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(0.1,1.5,0.0);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,1.5,0.0);
		glEnd();
	glPopMatrix();

	glPushMatrix();
		glTranslatef(0.1,0,0.0 + 1);
		glRotatef(90,0,1,0);
		glBindTexture(GL_TEXTURE_2D, textures[8]);
		glBegin(GL_QUADS);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(0.1,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(0.1,1.5,0.0);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,1.5,0.0);
		glEnd();
	glPopMatrix();


	//BUTT REST
	glPushMatrix();
		glTranslatef(0.0,1.5,-0.1);
		glRotatef(90,1,0,0);
		glBindTexture(GL_TEXTURE_2D, textures[8]);
		glBegin(GL_QUADS);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(1.1,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(1.1,1.1,0.0);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,1.1,0.0);
		glEnd();
	glPopMatrix();

	glPushMatrix();
		glTranslatef(0.0,1.4,-0.1);
		glRotatef(90,1,0,0);
		glBindTexture(GL_TEXTURE_2D, textures[8]);
		glBegin(GL_QUADS);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(1.1,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(1.1,1.1,0.0);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,1.1,0.0);
		glEnd();
	glPopMatrix();


	glPushMatrix();
		glTranslatef(1.1,1.5,1);
		glRotatef(180,0,0,0);
		glBindTexture(GL_TEXTURE_2D, textures[8]);
		glBegin(GL_QUADS);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(1.1,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(1.1,0.1,0.0);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,0.1,0.0);
		glEnd();
	glPopMatrix();

	glPushMatrix();
		glTranslatef(0.0,0+1.4,0.0+1);
		glRotatef(90,0,1,0);
		glBindTexture(GL_TEXTURE_2D, textures[8]);
		glBegin(GL_QUADS);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(1.1,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(1.1,0.1,0.0);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,0.1,0.0);
		glEnd();
	glPopMatrix();

	glPushMatrix();
		glTranslatef(0.0+1.1,0+1.4,0.0-0.1);
		glRotatef(-90,0,1,0);
		glBindTexture(GL_TEXTURE_2D, textures[8]);
		glBegin(GL_QUADS);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(1.1,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(1.1,0.1,0.0);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,0.1,0.0);
		glEnd();
	glPopMatrix();

	glPushMatrix();
		glTranslatef(0.0+1.1,0+1.4,0.0-0.1);
		glRotatef(-180,0,1,0);
		glBindTexture(GL_TEXTURE_2D, textures[8]);
		glBegin(GL_QUADS);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(1.1,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(1.1,0.1,0.0);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,0.1,0.0);
		glEnd();
	glPopMatrix();


	//BACK REST#1
	glPushMatrix();
		glTranslatef(0.0+1.1,0+1.4+1,0.0-0.1);
		glRotatef(-180,0,1,0);
		glBindTexture(GL_TEXTURE_2D, textures[8]);
		glBegin(GL_QUADS);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(1.1,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(1.1,0.1,0.0);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,0.1,0.0);
		glEnd();
	glPopMatrix();

	glPushMatrix();
		glTranslatef(0.0+1.1,0+1.4+1,0.0);
		glRotatef(-180,0,1,0);
		glBindTexture(GL_TEXTURE_2D, textures[8]);
		glBegin(GL_QUADS);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(1.1,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(1.1,0.1,0.0);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,0.1,0.0);
		glEnd();
	glPopMatrix();

	glPushMatrix();
		glTranslatef(0.0,0+1.4+1,-0.1);
		glRotatef(90,1,0,0);
		glBindTexture(GL_TEXTURE_2D, textures[8]);
		glBegin(GL_QUADS);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(1.1,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(1.1,0.1,0.0);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,0.1,0.0);
		glEnd();
	glPopMatrix();

	//BACK REST#2
	glPushMatrix();
		glTranslatef(0.0+1.1,0+1.2+1,0.0-0.1);
		glRotatef(-180,0,1,0);
		glBindTexture(GL_TEXTURE_2D, textures[8]);
		glBegin(GL_QUADS);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(1.1,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(1.1,0.1,0.0);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,0.1,0.0);
		glEnd();
	glPopMatrix();

	glPushMatrix();
		glTranslatef(0.0+1.1,0+1.2+1,0.0);
		glRotatef(-180,0,1,0);
		glBindTexture(GL_TEXTURE_2D, textures[8]);
		glBegin(GL_QUADS);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(1.1,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(1.1,0.1,0.0);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,0.1,0.0);
		glEnd();
	glPopMatrix();

	glPushMatrix();
		glTranslatef(0.0,2.2,-0.1);
		glRotatef(90,1,0,0);
		glBindTexture(GL_TEXTURE_2D, textures[8]);
		glBegin(GL_QUADS);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(1.1,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(1.1,0.1,0.0);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,0.1,0.0);
		glEnd();
	glPopMatrix();

	//BACK REST#3
	glPushMatrix();
		glTranslatef(0.0+1.1,2.0,-0.1);
		glRotatef(-180,0,1,0);
		glBindTexture(GL_TEXTURE_2D, textures[8]);
		glBegin(GL_QUADS);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(1.1,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(1.1,0.1,0.0);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,0.1,0.0);
		glEnd();
	glPopMatrix();

	glPushMatrix();
		glTranslatef(1.1,2.0,0.0);
		glRotatef(-180,0,1,0);
		glBindTexture(GL_TEXTURE_2D, textures[8]);
		glBegin(GL_QUADS);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(1.1,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(1.1,0.1,0.0);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,0.1,0.0);
		glEnd();
	glPopMatrix();

	glPushMatrix();
		glTranslatef(0.0,2.0,-0.1);
		glRotatef(90,1,0,0);
		glBindTexture(GL_TEXTURE_2D, textures[8]);
		glBegin(GL_QUADS);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(1.1,0.0,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(1.1,0.1,0.0);
		glTexCoord2f(1.0, 1.0);   glVertex3f(0.0,0.1,0.0);
		glEnd();
	glPopMatrix();

	//glDisable(GL_TEXTURE_2D);
} //end chair

//door frame
void doorFrame(void) {
	//left
	glPushMatrix();
	glTranslatef(0,0,-0.28);
		//tall
		glPushMatrix();
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,0.0,0.0);
			glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,6.3,0.0);
			glTexCoord2f(0.1, 1.0);   glVertex3f(0.3,6.3,0.0);
			glTexCoord2f(0.1, 0.0);   glVertex3f(0.3,0.0,0.0);
			glEnd();
		glPopMatrix();
		//side
		glPushMatrix();
		glTranslatef(0,0,0.3);
		glRotatef(90,0,1,0);
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,0.0,0.0);
			glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,6.0,0.0);
			glTexCoord2f(0.1, 1.0);   glVertex3f(0.3,6.3,0.0);
			glTexCoord2f(0.1, 0.0);   glVertex3f(0.3,0.0,0.0);
			glEnd();
		glPopMatrix();
		//side
		glPushMatrix();
		glTranslatef(0.3,0,0);
		glRotatef(-90,0,1,0);
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,0.0,0.0);
			glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,6.3,0.0);
			glTexCoord2f(0.1, 1.0);   glVertex3f(0.3,6.0,0.0);
			glTexCoord2f(0.1, 0.0);   glVertex3f(0.3,0.0,0.0);
			glEnd();
		glPopMatrix();
		//short
		glPushMatrix();
		glTranslatef(0.3,0,0.3);
		glRotatef(180,0,1,0);
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,0.0,0.0);
			glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,6.0,0.0);
			glTexCoord2f(0.1, 1.0);   glVertex3f(0.3,6.0,0.0);
			glTexCoord2f(0.1, 0.0);   glVertex3f(0.3,0.0,0.0);
			glEnd();
		glPopMatrix();
	glPopMatrix();

	//right
	glPushMatrix();
	glTranslatef(0.3,0,6.28);
	glRotatef(180,0,1,0);
		//tall
		glPushMatrix();
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,0.0,0.0);
			glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,6.3,0.0);
			glTexCoord2f(0.1, 1.0);   glVertex3f(0.3,6.3,0.0);
			glTexCoord2f(0.1, 0.0);   glVertex3f(0.3,0.0,0.0);
			glEnd();
		glPopMatrix();
		//side
		glPushMatrix();
		glTranslatef(0,0,0.3);
		glRotatef(90,0,1,0);
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,0.0,0.0);
			glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,6.0,0.0);
			glTexCoord2f(0.1, 1.0);   glVertex3f(0.3,6.3,0.0);
			glTexCoord2f(0.1, 0.0);   glVertex3f(0.3,0.0,0.0);
			glEnd();
		glPopMatrix();
		//side
		glPushMatrix();
		glTranslatef(0.3,0,0);
		glRotatef(-90,0,1,0);
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,0.0,0.0);
			glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,6.3,0.0);
			glTexCoord2f(0.1, 1.0);   glVertex3f(0.3,6.0,0.0);
			glTexCoord2f(0.1, 0.0);   glVertex3f(0.3,0.0,0.0);
			glEnd();
		glPopMatrix();
		//short
		glPushMatrix();
		glTranslatef(0.3,0,0.3);
		glRotatef(180,0,1,0);
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,0.0,0.0);
			glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,6.0,0.0);
			glTexCoord2f(0.1, 1.0);   glVertex3f(0.3,6.0,0.0);
			glTexCoord2f(0.1, 0.0);   glVertex3f(0.3,0.0,0.0);
			glEnd();
		glPopMatrix();
	glPopMatrix();

	//top
	glPushMatrix();
	glTranslatef(0,6.28,0);
	glRotatef(90,1,0,0);
		//tall
		glPushMatrix();
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,-0.3,0.0);
			glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,6.3,0.0);
			glTexCoord2f(0.1, 1.0);   glVertex3f(0.3,6.3,0.0);
			glTexCoord2f(0.1, 0.0);   glVertex3f(0.3,-0.3,0.0);
			glEnd();
		glPopMatrix();
		//side
		glPushMatrix();
		glTranslatef(0,0,0.3);
		glRotatef(90,0,1,0);
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,0.0,0.0);
			glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,6.0,0.0);
			glTexCoord2f(0.1, 1.0);   glVertex3f(0.3,6.3,0.0);
			glTexCoord2f(0.1, 0.0);   glVertex3f(0.3,-0.3,0.0);
			glEnd();
		glPopMatrix();
		//side
		glPushMatrix();
		glTranslatef(0.3,0,0);
		glRotatef(-90,0,1,0);
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,-0.3,0.0);
			glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,6.3,0.0);
			glTexCoord2f(0.1, 1.0);   glVertex3f(0.3,6.0,0.0);
			glTexCoord2f(0.1, 0.0);   glVertex3f(0.3,0.0,0.0);
			glEnd();
		glPopMatrix();
		//short
		glPushMatrix();
		glTranslatef(0.3,0,0.3);
		glRotatef(180,0,1,0);
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,0.0,0.0);
			glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,6.0,0.0);
			glTexCoord2f(0.1, 1.0);   glVertex3f(0.3,6.0,0.0);
			glTexCoord2f(0.1, 0.0);   glVertex3f(0.3,0.0,0.0);
			glEnd();
		glPopMatrix();
	glPopMatrix();
} //end doorFrame

//mirror
void mirror(void) {
	material(0.4,0.4,0.2,1,0);
	glBindTexture(GL_TEXTURE_2D, textures[8]);
	//left edge
	glPushMatrix();
	glTranslatef(0,2,1.72);
		//tall
		glPushMatrix();
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,-0.3,0.0);
			glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,4.3,0.0);
			glTexCoord2f(0.1, 1.0);   glVertex3f(0.3,4.3,0.0);
			glTexCoord2f(0.1, 0.0);   glVertex3f(0.3,-0.3,0.0);
			glEnd();
		glPopMatrix();
		//side
		glPushMatrix();
		glTranslatef(0,0,0.3);
		glRotatef(90,0,1,0);
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,0.0,0.0);
			glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,4.0,0.0);
			glTexCoord2f(0.1, 1.0);   glVertex3f(0.3,4.3,0.0);
			glTexCoord2f(0.1, 0.0);   glVertex3f(0.3,-0.3,0.0);
			glEnd();
		glPopMatrix();
		//side
		glPushMatrix();
		glTranslatef(0.3,0,0);
		glRotatef(-90,0,1,0);
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,-0.3,0.0);
			glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,4.3,0.0);
			glTexCoord2f(0.1, 1.0);   glVertex3f(0.3,4.0,0.0);
			glTexCoord2f(0.1, 0.0);   glVertex3f(0.3,0.0,0.0);
			glEnd();
		glPopMatrix();
		//short
		glPushMatrix();
		glTranslatef(0.3,0,0.3);
		glRotatef(180,0,1,0);
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,0.0,0.0);
			glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,4.0,0.0);
			glTexCoord2f(0.1, 1.0);   glVertex3f(0.3,4.0,0.0);
			glTexCoord2f(0.1, 0.0);   glVertex3f(0.3,0.0,0.0);
			glEnd();
		glPopMatrix();
	glPopMatrix();

	//right edge
	glPushMatrix();
	glTranslatef(0.3,2,6.28);
	glRotatef(180,0,1,0);
		glPushMatrix();
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,-0.3,0.0);
			glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,4.3,0.0);
			glTexCoord2f(0.1, 1.0);   glVertex3f(0.3,4.3,0.0);
			glTexCoord2f(0.1, 0.0);   glVertex3f(0.3,-0.3,0.0);
			glEnd();
		glPopMatrix();
		//side
		glPushMatrix();
		glTranslatef(0,0,0.3);
		glRotatef(90,0,1,0);
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,0.0,0.0);
			glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,4.0,0.0);
			glTexCoord2f(0.1, 1.0);   glVertex3f(0.3,4.3,0.0);
			glTexCoord2f(0.1, 0.0);   glVertex3f(0.3,-0.3,0.0);
			glEnd();
		glPopMatrix();
		//side
		glPushMatrix();
		glTranslatef(0.3,0,0);
		glRotatef(-90,0,1,0);
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,-0.3,0.0);
			glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,4.3,0.0);
			glTexCoord2f(0.1, 1.0);   glVertex3f(0.3,4.0,0.0);
			glTexCoord2f(0.1, 0.0);   glVertex3f(0.3,0.0,0.0);
			glEnd();
		glPopMatrix();
		//short
		glPushMatrix();
		glTranslatef(0.3,0,0.3);
		glRotatef(180,0,1,0);
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,0.0,0.0);
			glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,4.0,0.0);
			glTexCoord2f(0.1, 1.0);   glVertex3f(0.3,4.0,0.0);
			glTexCoord2f(0.1, 0.0);   glVertex3f(0.3,0.0,0.0);
			glEnd();
		glPopMatrix();
	glPopMatrix();

	//top
	glPushMatrix();
	glTranslatef(0,6.28,2);
	glRotatef(90,1,0,0);
		//tall
		glPushMatrix();
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,-0.3,0.0);
			glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,4.3,0.0);
			glTexCoord2f(0.1, 1.0);   glVertex3f(0.3,4.3,0.0);
			glTexCoord2f(0.1, 0.0);   glVertex3f(0.3,-0.3,0.0);
			glEnd();
		glPopMatrix();
		//side
		glPushMatrix();
		glTranslatef(0,0,0.3);
		glRotatef(90,0,1,0);
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,0.0,0.0);
			glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,4.0,0.0);
			glTexCoord2f(0.1, 1.0);   glVertex3f(0.3,4.3,0.0);
			glTexCoord2f(0.1, 0.0);   glVertex3f(0.3,-0.3,0.0);
			glEnd();
		glPopMatrix();
		//side
		glPushMatrix();
		glTranslatef(0.3,0,0);
		glRotatef(-90,0,1,0);
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,-0.3,0.0);
			glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,4.3,0.0);
			glTexCoord2f(0.1, 1.0);   glVertex3f(0.3,4.0,0.0);
			glTexCoord2f(0.1, 0.0);   glVertex3f(0.3,0.0,0.0);
			glEnd();
		glPopMatrix();
		//short
		glPushMatrix();
		glTranslatef(0.3,0,0.3);
		glRotatef(180,0,1,0);
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,0.0,0.0);
			glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,4.0,0.0);
			glTexCoord2f(0.1, 1.0);   glVertex3f(0.3,4.0,0.0);
			glTexCoord2f(0.1, 0.0);   glVertex3f(0.3,0.0,0.0);
			glEnd();
		glPopMatrix();
	glPopMatrix();

	//bottom
	glPushMatrix();
	glTranslatef(0,1.72,6);
	glRotatef(180,1,0,0);
	glRotatef(90,1,0,0);
		//tall
		glPushMatrix();
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,-0.3,0.0);
			glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,4.3,0.0);
			glTexCoord2f(0.1, 1.0);   glVertex3f(0.3,4.3,0.0);
			glTexCoord2f(0.1, 0.0);   glVertex3f(0.3,-0.3,0.0);
			glEnd();
		glPopMatrix();
		//side
		glPushMatrix();
		glTranslatef(0,0,0.3);
		glRotatef(90,0,1,0);
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,0.0,0.0);
			glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,4.0,0.0);
			glTexCoord2f(0.1, 1.0);   glVertex3f(0.3,4.3,0.0);
			glTexCoord2f(0.1, 0.0);   glVertex3f(0.3,-0.3,0.0);
			glEnd();
		glPopMatrix();
		//side
		glPushMatrix();
		glTranslatef(0.3,0,0);
		glRotatef(-90,0,1,0);
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,-0.3,0.0);
			glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,4.3,0.0);
			glTexCoord2f(0.1, 1.0);   glVertex3f(0.3,4.0,0.0);
			glTexCoord2f(0.1, 0.0);   glVertex3f(0.3,0.0,0.0);
			glEnd();
		glPopMatrix();
		//short
		glPushMatrix();
		glTranslatef(0.3,0,0.3);
		glRotatef(180,0,1,0);
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,0.0,0.0);
			glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,4.0,0.0);
			glTexCoord2f(0.1, 1.0);   glVertex3f(0.3,4.0,0.0);
			glTexCoord2f(0.1, 0.0);   glVertex3f(0.3,0.0,0.0);
			glEnd();
		glPopMatrix();
	glPopMatrix();

	//mirror surface
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	material(4,3,1,0.5,30);

	glBindTexture(GL_TEXTURE_2D, textures[37]);
	glPushMatrix();
	glTranslatef(0.15,2,2);
	glRotatef(-90,0,1,0);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,4.0,0.0);
		glTexCoord2f(1.0, 1.0);   glVertex3f(4.0,4.0,0.0);
		glTexCoord2f(1.0, 0.0);   glVertex3f(4.0,0.0,0.0);
	glEnd();
	glPopMatrix();
	glDisable(GL_BLEND);
} //end mirror

//mirror
void window(void) {
	material(0.4,0.4,0.2,1,0);
	glBindTexture(GL_TEXTURE_2D, textures[8]);
	//left edge
	glPushMatrix();
	glTranslatef(0,2,1.72);
		//tall
		glPushMatrix();
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,-0.3,0.0);
			glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,12.0,0.0);
			glTexCoord2f(0.1, 1.0);   glVertex3f(0.3,12.0,0.0);
			glTexCoord2f(0.1, 0.0);   glVertex3f(0.3,-0.3,0.0);
			glEnd();
		glPopMatrix();
		//side
		glPushMatrix();
		glTranslatef(0,0,0.3);
		glRotatef(90,0,1,0);
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,0.0,0.0);
			glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,11.7,0.0);
			glTexCoord2f(0.1, 1.0);   glVertex3f(0.3,12.0,0.0);
			glTexCoord2f(0.1, 0.0);   glVertex3f(0.3,-0.3,0.0);
			glEnd();
		glPopMatrix();
		//side
		glPushMatrix();
		glTranslatef(0.3,0,0);
		glRotatef(-90,0,1,0);
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,-0.3,0.0);
			glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,12.0,0.0);
			glTexCoord2f(0.1, 1.0);   glVertex3f(0.3,11.7,0.0);
			glTexCoord2f(0.1, 0.0);   glVertex3f(0.3,0.0,0.0);
			glEnd();
		glPopMatrix();
		//short
		glPushMatrix();
		glTranslatef(0.3,0,0.3);
		glRotatef(180,0,1,0);
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,0.0,0.0);
			glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,11.7,0.0);
			glTexCoord2f(0.1, 1.0);   glVertex3f(0.3,11.7,0.0);
			glTexCoord2f(0.1, 0.0);   glVertex3f(0.3,0.0,0.0);
			glEnd();
		glPopMatrix();
	glPopMatrix();

	//right edge
	glPushMatrix();
	glTranslatef(0.3,2,18.28);
	glRotatef(180,0,1,0);
		glPushMatrix();
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,-0.3,0.0);
			glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,12.0,0.0);
			glTexCoord2f(0.1, 1.0);   glVertex3f(0.3,12.0,0.0);
			glTexCoord2f(0.1, 0.0);   glVertex3f(0.3,-0.3,0.0);
			glEnd();
		glPopMatrix();
		//side
		glPushMatrix();
		glTranslatef(0,0,0.3);
		glRotatef(90,0,1,0);
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,0.0,0.0);
			glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,11.7,0.0);
			glTexCoord2f(0.1, 1.0);   glVertex3f(0.3,12.0,0.0);
			glTexCoord2f(0.1, 0.0);   glVertex3f(0.3,-0.3,0.0);
			glEnd();
		glPopMatrix();
		//side
		glPushMatrix();
		glTranslatef(0.3,0,0);
		glRotatef(-90,0,1,0);
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,-0.3,0.0);
			glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,12.0,0.0);
			glTexCoord2f(0.1, 1.0);   glVertex3f(0.3,11.7,0.0);
			glTexCoord2f(0.1, 0.0);   glVertex3f(0.3,0.0,0.0);
			glEnd();
		glPopMatrix();
		//short
		glPushMatrix();
		glTranslatef(0.3,0,0.3);
		glRotatef(180,0,1,0);
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,0.0,0.0);
			glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,11.7,0.0);
			glTexCoord2f(0.1, 1.0);   glVertex3f(0.3,11.7,0.0);
			glTexCoord2f(0.1, 0.0);   glVertex3f(0.3,0.0,0.0);
			glEnd();
		glPopMatrix();
	glPopMatrix();

	//top
	glPushMatrix();
	glTranslatef(0,13.98,2);
	glRotatef(90,1,0,0);
		//tall
		glPushMatrix();
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,-0.3,0.0);
			glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,16.3,0.0);
			glTexCoord2f(0.1, 1.0);   glVertex3f(0.3,16.3,0.0);
			glTexCoord2f(0.1, 0.0);   glVertex3f(0.3,-0.3,0.0);
			glEnd();
		glPopMatrix();
		//side
		glPushMatrix();
		glTranslatef(0,0,0.3);
		glRotatef(90,0,1,0);
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,0.0,0.0);
			glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,16.0,0.0);
			glTexCoord2f(0.1, 1.0);   glVertex3f(0.3,16.3,0.0);
			glTexCoord2f(0.1, 0.0);   glVertex3f(0.3,-0.3,0.0);
			glEnd();
		glPopMatrix();
		//side
		glPushMatrix();
		glTranslatef(0.3,0,0);
		glRotatef(-90,0,1,0);
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,-0.3,0.0);
			glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,16.3,0.0);
			glTexCoord2f(0.1, 1.0);   glVertex3f(0.3,16.0,0.0);
			glTexCoord2f(0.1, 0.0);   glVertex3f(0.3,0.0,0.0);
			glEnd();
		glPopMatrix();
		//short
		glPushMatrix();
		glTranslatef(0.3,0,0.3);
		glRotatef(180,0,1,0);
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,0.0,0.0);
			glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,16.0,0.0);
			glTexCoord2f(0.1, 1.0);   glVertex3f(0.3,16.0,0.0);
			glTexCoord2f(0.1, 0.0);   glVertex3f(0.3,0.0,0.0);
			glEnd();
		glPopMatrix();
	glPopMatrix();

	//bottom
	glPushMatrix();
	glTranslatef(0,1.72,18);
	glRotatef(180,1,0,0);
	glRotatef(90,1,0,0);
		//tall
		glPushMatrix();
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,-0.3,0.0);
			glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,16.3,0.0);
			glTexCoord2f(0.1, 1.0);   glVertex3f(0.3,16.3,0.0);
			glTexCoord2f(0.1, 0.0);   glVertex3f(0.3,-0.3,0.0);
			glEnd();
		glPopMatrix();
		//side
		glPushMatrix();
		glTranslatef(0,0,0.3);
		glRotatef(90,0,1,0);
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,0.0,0.0);
			glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,16.0,0.0);
			glTexCoord2f(0.1, 1.0);   glVertex3f(0.3,16.3,0.0);
			glTexCoord2f(0.1, 0.0);   glVertex3f(0.3,-0.3,0.0);
			glEnd();
		glPopMatrix();
		//side
		glPushMatrix();
		glTranslatef(0.3,0,0);
		glRotatef(-90,0,1,0);
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,-0.3,0.0);
			glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,16.3,0.0);
			glTexCoord2f(0.1, 1.0);   glVertex3f(0.3,16.0,0.0);
			glTexCoord2f(0.1, 0.0);   glVertex3f(0.3,0.0,0.0);
			glEnd();
		glPopMatrix();
		//short
		glPushMatrix();
		glTranslatef(0.3,0,0.3);
		glRotatef(180,0,1,0);
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,0.0,0.0);
			glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,16.0,0.0);
			glTexCoord2f(0.1, 1.0);   glVertex3f(0.3,16.0,0.0);
			glTexCoord2f(0.1, 0.0);   glVertex3f(0.3,0.0,0.0);
			glEnd();
		glPopMatrix();
	glPopMatrix();


	//horizontal
	for(int i=0; i<12; i+=3) {
		glPushMatrix();
		glTranslatef(0,4.98+i,2);
		glRotatef(90,1,0,0);
			//tall
			glPushMatrix();
				glBegin(GL_QUADS);
				glTexCoord2f(0.0, 0.0);   glVertex3f(0.0, 0.0,0.0);
				glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,16.0,0.0);
				glTexCoord2f(0.1, 1.0);   glVertex3f(0.3,16.0,0.0);
				glTexCoord2f(0.1, 0.0);   glVertex3f(0.3, 0.0,0.0);
				glEnd();
			glPopMatrix();
			//side
			glPushMatrix();
			glTranslatef(0,0,0.3);
			glRotatef(90,0,1,0);
				glBegin(GL_QUADS);
				glTexCoord2f(0.0, 0.0);   glVertex3f(0.0, 0.0,0.0);
				glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,16.0,0.0);
				glTexCoord2f(0.1, 1.0);   glVertex3f(0.3,16.0,0.0);
				glTexCoord2f(0.1, 0.0);   glVertex3f(0.3, 0.0,0.0);
				glEnd();
			glPopMatrix();
			//side
			glPushMatrix();
			glTranslatef(0.3,0,0);
			glRotatef(-90,0,1,0);
				glBegin(GL_QUADS);
				glTexCoord2f(0.0, 0.0);   glVertex3f(0.0, 0.0,0.0);
				glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,16.0,0.0);
				glTexCoord2f(0.1, 1.0);   glVertex3f(0.3,16.0,0.0);
				glTexCoord2f(0.1, 0.0);   glVertex3f(0.3, 0.0,0.0);
				glEnd();
			glPopMatrix();
			//short
			glPushMatrix();
			glTranslatef(0.3,0,0.3);
			glRotatef(180,0,1,0);
				glBegin(GL_QUADS);
				glTexCoord2f(0.0, 0.0);   glVertex3f(0.0, 0.0,0.0);
				glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,16.0,0.0);
				glTexCoord2f(0.1, 1.0);   glVertex3f(0.3,16.0,0.0);
				glTexCoord2f(0.1, 0.0);   glVertex3f(0.3, 0.0,0.0);
				glEnd();
			glPopMatrix();
		glPopMatrix();
	}

	//vertical
	for (int i=1; i<4; i++) {
		//middle edge
		glPushMatrix();
		glTranslatef(0,2,1.72+(i*4));
			//tall
			glPushMatrix();
				glBegin(GL_QUADS);
				glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,0.0,0.0);
				glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,11.7,0.0);
				glTexCoord2f(0.1, 1.0);   glVertex3f(0.3,11.7,0.0);
				glTexCoord2f(0.1, 0.0);   glVertex3f(0.3,0.0,0.0);
				glEnd();
			glPopMatrix();
			//side
			glPushMatrix();
			glTranslatef(0,0,0.3);
			glRotatef(90,0,1,0);
				glBegin(GL_QUADS);
				glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,0.0,0.0);
				glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,11.7,0.0);
				glTexCoord2f(0.1, 1.0);   glVertex3f(0.3,11.7,0.0);
				glTexCoord2f(0.1, 0.0);   glVertex3f(0.3,0.0,0.0);
				glEnd();
			glPopMatrix();
			//side
			glPushMatrix();
			glTranslatef(0.3,0,0);
			glRotatef(-90,0,1,0);
				glBegin(GL_QUADS);
				glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,0.0,0.0);
				glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,11.7,0.0);
				glTexCoord2f(0.1, 1.0);   glVertex3f(0.3,11.7,0.0);
				glTexCoord2f(0.1, 0.0);   glVertex3f(0.3,0.0,0.0);
				glEnd();
			glPopMatrix();
			//short
			glPushMatrix();
			glTranslatef(0.3,0,0.3);
			glRotatef(180,0,1,0);
				glBegin(GL_QUADS);
				glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,0.0,0.0);
				glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,11.7,0.0);
				glTexCoord2f(0.1, 1.0);   glVertex3f(0.3,11.7,0.0);
				glTexCoord2f(0.1, 0.0);   glVertex3f(0.3,0.0,0.0);
				glEnd();
			glPopMatrix();
		glPopMatrix();
	}


	//mirror surface
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//material(8,6,2,0.25,60);
	//glBindTexture(GL_TEXTURE_2D, textures[38]);

	for(int i=0; i<16; i+=4) {
		for (int j=0; j<12; j+=3) {
			if( i==12 && j==9   ||   i==4 && j==3 ) {
				glBindTexture(GL_TEXTURE_2D, textures[39]);
				material(8,6,2,0.4,60);
			}
			else {
				material(8,6,2,0.25,60);
				glBindTexture(GL_TEXTURE_2D, textures[38]);
			}
			glPushMatrix();
			glTranslatef(0.15,2+j,2+i);
			glRotatef(-90,0,1,0);
			glBegin(GL_QUADS);
				glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,0.0,0.0);
				glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,3.0,0.0);
				glTexCoord2f(1.0, 1.0);   glVertex3f(4.0,3.0,0.0);
				glTexCoord2f(1.0, 0.0);   glVertex3f(4.0,0.0,0.0);
			glEnd();
			glPopMatrix();
		}
	}




	glDisable(GL_BLEND);
} //end mirror

//door
void door(int LorR) {
	glBindTexture(GL_TEXTURE_2D, textures[21]);
	//front
	glPushMatrix();
	glTranslatef(0,0,0);
	glRotatef(90,0,1,0);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,6.0,0.0);
		glTexCoord2f(1.0, 1.0);   glVertex3f(3.0,6.0,0.0);
		glTexCoord2f(1.0, 0.0);   glVertex3f(3.0,0.0,0.0);
		glEnd();
	glPopMatrix();
	//side
	glPushMatrix();
	glTranslatef(0.2,0,0);
	glRotatef(180,0,1,0);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,6.0,0.0);
		glTexCoord2f(0.1, 1.0);   glVertex3f(0.2,6.0,0.0);
		glTexCoord2f(0.1, 0.0);   glVertex3f(0.2,0.0,0.0);
		glEnd();
	glPopMatrix();
	//side
	glPushMatrix();
	glTranslatef(0,0,-3);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,6.0,0.0);
		glTexCoord2f(0.1, 1.0);   glVertex3f(0.2,6.0,0.0);
		glTexCoord2f(0.1, 0.0);   glVertex3f(0.2,0.0,0.0);
		glEnd();
	glPopMatrix();
	//back
	glPushMatrix();
	glTranslatef(0.2,0,-3);
	glRotatef(-90,0,1,0);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,6.0,0.0);
		glTexCoord2f(1.0, 1.0);   glVertex3f(3.0,6.0,0.0);
		glTexCoord2f(1.0, 0.0);   glVertex3f(3.0,0.0,0.0);
		glEnd();
	glPopMatrix();

	glDisable(GL_TEXTURE_2D);

	if(!LorR) {
		glPushMatrix();
		glTranslatef(-0.2,2.2,-0.2);
		glRotatef(90,0,1,0);
		gluCylinder(qobj, 0.05, 0.05, 0.6, 8, 2);
		glPopMatrix();
		glPushMatrix();
		glTranslatef(-0.2,2.2,-0.2);
		glutSolidSphere(0.1,20,20);
		glPopMatrix();
		glPushMatrix();
		glTranslatef(0.4,2.2,-0.2);
		glutSolidSphere(0.1,20,20);
		glPopMatrix();
	}
	else{
		glPushMatrix();
		glTranslatef(-0.2,2.2,-2.8);
		glRotatef(90,0,1,0);
		gluCylinder(qobj, 0.05, 0.05, 0.6, 8, 2);
		glPopMatrix();
		glPushMatrix();
		glTranslatef(-0.2,2.2,-2.8);
		glutSolidSphere(0.1,20,20);
		glPopMatrix();
		glPushMatrix();
		glTranslatef(0.4,2.2,-2.8);
		glutSolidSphere(0.1,20,20);
		glPopMatrix();
	}
	glEnable(GL_TEXTURE_2D);
} //emd door

//human statue thing...
void humanScale(void) {
	//human scale reference
	glPushMatrix();
	glTranslatef(lightPos[0][0], 0, lightPos[0][2]);

		glPushMatrix();
			glRotatef(-90,1,0,0);
			gluCylinder(qobj, 0.1,0.3,3,20,20);
		glPopMatrix();

		glDisable(GL_LIGHTING);
		glPushMatrix();
		glTranslatef(0,3.3,0);
		glutSolidSphere(0.25,20,20);
		glPopMatrix();
		glEnable(GL_LIGHTING);
	glPopMatrix();
} //end human scale


void chandelier(void) {
	//human scale reference
	glPushMatrix();
	glTranslatef(0, 0, 0);

		glPushMatrix();
			glRotatef(-90,1,0,0);
			gluCylinder(qobj, 0.1,0.3,3,20,20);
		glPopMatrix();

		glDisable(GL_LIGHTING);
		glPushMatrix();
		glTranslatef(0,3.3,0);
		glutSolidSphere(0.25,20,20);
		glPopMatrix();
		glEnable(GL_LIGHTING);
	glPopMatrix();
}

//Extra Table
void vartable(void){
	int stackNslice = 10;
    //top of table
    glPushMatrix();
    glTranslatef(0,2,0);
    glRotatef(-90,1,0,0);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    texRect2(1,3);
    glPopMatrix();

    //side 1
    glPushMatrix();
    glTranslatef(0,1.8,-3);
    glRotatef(90,0,0,1);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    texRect2(.2,1);
    glPopMatrix();

    //side 2
    glPushMatrix();
    glTranslatef(0,1.8,3);
    glRotatef(90,0,0,1);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    texRect2(.2,1);
    glPopMatrix();

    //side 3
    glPushMatrix();
    glTranslatef(1,1.8,0);
    glRotatef(90, 0, 1, 0);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    texRect2(3,0.2);
    glPopMatrix();

    //side 4
    glPushMatrix();
    glTranslatef(-1,1.8,0);
    glRotatef(90, 0, 1, 0);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    texRect2(3,0.2);
    glPopMatrix();
}

//ottoman
void ottoman(void){
	int stackNslice = 10;
    //top of table
    glPushMatrix();
    glTranslatef(0,2-1,0);
    glRotatef(-90,1,0,0);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    texRect2(1,1);
    glPopMatrix();

    //top side 1
    glPushMatrix();
    glTranslatef(0,1.8-1,-1);
    glRotatef(90,0,0,1);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    texRect2(.2,1);
    glPopMatrix();

    //top side 2
    glPushMatrix();
    glTranslatef(0,1.8-1,1);
    glRotatef(90,0,0,1);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    texRect2(.2,1);
    glPopMatrix();

    //top side 3
    glPushMatrix();
    glTranslatef(1,1.8-1,0);
    glRotatef(90, 0, 1, 0);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    texRect2(1,0.2);
    glPopMatrix();

    //top side 4
    glPushMatrix();
    glTranslatef(-1,1.8-1,0);
    glRotatef(90, 0, 1, 0);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    texRect2(1,0.2);
    glPopMatrix();

    //bottom side 1
    glPushMatrix();
    glTranslatef(0,0.5,-0.7);
    glRotatef(90,0,0,1);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    texRect2(0.5,0.7);
    glPopMatrix();

    //bottom side 2
    glPushMatrix();
    glTranslatef(0,0.5,0.7);
    glRotatef(90,0,0,1);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    texRect2(0.5,0.7);
    glPopMatrix();

    //bottom side 3
    glPushMatrix();
    glTranslatef(0.7,0.5,0);
    glRotatef(90, 0, 1, 0);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    texRect2(0.7,0.5);
    glPopMatrix();

    //top side 4
    glPushMatrix();
    glTranslatef(-0.7,0.5,0);
    glRotatef(90, 0, 1, 0);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    texRect2(0.7,0.5);
    glPopMatrix();
}

//macecase
void masecase(void){
	int stackNslice = 10;
    //top of table
    glPushMatrix();
    glTranslatef(0,2.2,0);
    glRotatef(-90,1,0,0);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    texRect2(1.5,0.8);
    glPopMatrix();

    //top side 1
    glPushMatrix();
    glTranslatef(0,2,-0.8);
    glRotatef(90,0,0,1);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    texRect2(.2,1.5);
    glPopMatrix();

    //top side 2
    glPushMatrix();
    glTranslatef(0,2,0.8);
    glRotatef(90,0,0,1);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    texRect2(.2,1.5);
    glPopMatrix();

    //top side 3
    glPushMatrix();
    glTranslatef(1.5,2,0);
    glRotatef(90, 0, 1, 0);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    texRect2(0.8,0.2);
    glPopMatrix();

    //top side 4
    glPushMatrix();
    glTranslatef(-1.5,2,0);
    glRotatef(90, 0, 1, 0);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    texRect2(0.8,0.2);
    glPopMatrix();

    //bottom side 1
    glPushMatrix();
    glTranslatef(0,0.5,-0.5);
    glRotatef(90,0,0,1);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    texRect2(1.3,1);
    glPopMatrix();

    //bottom side 2
    glPushMatrix();
    glTranslatef(0,0.5,0.5);
    glRotatef(90,0,0,1);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    texRect2(1.3,1);
    glPopMatrix();

    //bottom side 3
    glPushMatrix();
    glTranslatef(1,0.5,0);
    glRotatef(90, 0, 1, 0);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    texRect2(0.5,1.3);
    glPopMatrix();

    //top side 4
    glPushMatrix();
    glTranslatef(-1,0.5,0);
    glRotatef(90, 0, 1, 0);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    texRect2(0.5,1.3);
    glPopMatrix();
}

//portrait
void portrait(void){
    //top of table
    glPushMatrix();
    glTranslatef(0,4,0);
    glRotatef(-90,0,1,0);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    texRect2(2,3);
    glPopMatrix();

    //top of table
    glPushMatrix();
    glTranslatef(0-0.15,4,0);
    glRotatef(90,0,1,0);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    texRect2(2,3);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-0.08,2-1,0);
    glRotatef(-90,1,0,0);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    texRect2(0.08,2);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-0.08,7,0);
    glRotatef(-90,1,0,0);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    texRect2(0.08,2);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-0.08,4,-2);
    glRotatef(90,0,0,1);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    texRect2(3,0.08);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-0.08,4,2);
    glRotatef(90,0,0,1);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    texRect2(3,0.08);
    glPopMatrix();
}

void hallway(void) {

	//Wall Left
	glPushMatrix();
		glTranslatef(14,1,5);
		glRotatef(0,0,1,0);
		glBindTexture(GL_TEXTURE_2D, textures[5]); //wood panels
		tiledTexWall(10,1);
		glPushMatrix();
			glBindTexture(GL_TEXTURE_2D, textures[3]); //wallpaper
			glTranslatef(0,2,0);
			tiledTexWall(2,2);
		glPopMatrix();
		glPushMatrix();
			glBindTexture(GL_TEXTURE_2D, textures[3]); //wallpaper
			glTranslatef(0,6,0);
			tiledTexWall(10,1);
		glPopMatrix();
	glPopMatrix();

	glPushMatrix();
		glTranslatef(22,1,5);
		glRotatef(0,0,1,0);
		glPushMatrix();
			glBindTexture(GL_TEXTURE_2D, textures[3]); //wallpaper
			glTranslatef(0,2,0);
			tiledTexWall(2,2);
		glPopMatrix();
	glPopMatrix();

	glPushMatrix();
		glTranslatef(30,1,5);
		glRotatef(0,0,1,0);

		glPushMatrix();
			glBindTexture(GL_TEXTURE_2D, textures[3]); //wallpaper
			glTranslatef(0,2,0);
			tiledTexWall(2,2);
		glPopMatrix();
	glPopMatrix();

	//Wall Right
	glPushMatrix();
		glTranslatef(32,1,-5);
		glRotatef(180,0,1,0);
		glBindTexture(GL_TEXTURE_2D, textures[5]); //wood panels
		tiledTexWall(10,1);
		glPushMatrix();
			glBindTexture(GL_TEXTURE_2D, textures[3]); //wallpaper
			glTranslatef(0,2,0);
			tiledTexWall(10,3);
		glPopMatrix();
	glPopMatrix();

	//Floor
	glBindTexture(GL_TEXTURE_2D, textures[0]); //wood planks
	material(1,0.8,0.5,1,5);                     //material properties
	glPushMatrix();
		glTranslatef(14,0,-4);
		glRotatef(90,1,0,0);
		tiledTexWall(10,5);
	glPopMatrix();

	//torches
	for(int i = 15; i >= 0; i-=5){
		glPushMatrix();
		glTranslatef(i+3,3,-9);
		torch();
		glPopMatrix();
	}
	// for(int i = 15; i >= 0; i-=5){
	// 	glPushMatrix();
	// 	glTranslatef(i+3,3, 1);
	// 	torch();
	// 	glPopMatrix();
	// }
	//Ceiling
	glBindTexture(GL_TEXTURE_2D, textures[4]);
	glPushMatrix();
		glTranslatef(14,8,4);
		glRotatef(-90,1,0,0);
		tiledTexWall(10,5);
	glPopMatrix();


	//Connecting Wall
		//room1 wall 2, right side
		glPushMatrix();
			glTranslatef(13.25,1,4);
			glRotatef(-90,0,1,0);
			glBindTexture(GL_TEXTURE_2D, textures[5]); //wood panels
			tiledTexWall(1,1);
			glPushMatrix();
				glBindTexture(GL_TEXTURE_2D, textures[3]); //wallpaper
				glTranslatef(0,2,0);
				tiledTexWall(1,4);
			glPopMatrix();
		glPopMatrix();

		//room1 wall 2, left side
		glPushMatrix();
			glTranslatef(13.25,1,-4);
			glRotatef(-90,0,1,0);
			glBindTexture(GL_TEXTURE_2D, textures[5]); //wood panels
			tiledTexWall(1,1);
			glPushMatrix();
				glBindTexture(GL_TEXTURE_2D, textures[3]); //wallpaper
				glTranslatef(0,2,0);
				tiledTexWall(1,4);
			glPopMatrix();
		glPopMatrix();

		glPushMatrix();
			glBindTexture(GL_TEXTURE_2D, textures[3]); //wallpaper
			glTranslatef(13.25,7,-2);
			glRotatef(-90,0,1,0);
			tiledTexWall(3,2);
		glPopMatrix();

		//room1 wall 2, right side
		glPushMatrix();
			glTranslatef(32.9,1,4);
			glRotatef(90,0,1,0);
			glBindTexture(GL_TEXTURE_2D, textures[5]); //wood panels
			tiledTexWall(1,1);
			glPushMatrix();
				glBindTexture(GL_TEXTURE_2D, textures[3]); //wallpaper
				glTranslatef(0,2,0);
				tiledTexWall(1,4);
			glPopMatrix();
		glPopMatrix();

		//room1 wall 2, left side
		glPushMatrix();
			glTranslatef(32.9,1,-4);
			glRotatef(90,0,1,0);
			glBindTexture(GL_TEXTURE_2D, textures[5]); //wood panels
			tiledTexWall(1,1);
			glPushMatrix();
				glBindTexture(GL_TEXTURE_2D, textures[3]); //wallpaper
				glTranslatef(0,2,0);
				tiledTexWall(1,4);
			glPopMatrix();
		glPopMatrix();

		glPushMatrix();
			glBindTexture(GL_TEXTURE_2D, textures[3]); //wallpaper
			glTranslatef(32.9,7,2);
			glRotatef(90,0,1,0);
			tiledTexWall(3,2);
		glPopMatrix();
}

//keypad
void keypad(void){
	//set material
	float mat_specular[]={0.7, 0.7, 0.7, 1.0};
	float mat_diffuse[] ={0.6, 0.6, 0.6, 1.0};
	float mat_ambient[] ={0.1, 0.05, 0.01, 1.0};
	float mat_shininess={90};
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialf(GL_FRONT, GL_SHININESS, mat_shininess);

	//glEnable(GL_TEXTURE_2D);
    glPushMatrix();
    glTranslatef(0,3,0);
    glRotatef(-90,0,1,0);
    glBindTexture(GL_TEXTURE_2D, textures[24]);
    texRect2(0.4,0.5);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.2,3,0);
    glRotatef(-90,0,1,0);
    glBindTexture(GL_TEXTURE_2D, textures[24]);
    texRect2(0.4,0.5);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.1,3,0.4);
    glRotatef(-90,0,0,0);
    glBindTexture(GL_TEXTURE_2D, textures[24]);
    texRect2(0.1,0.5);
    glPopMatrix();

	glPushMatrix();
    glTranslatef(0.1,3,-0.4);
    glRotatef(-90,0,0,0);
    glBindTexture(GL_TEXTURE_2D, textures[24]);
    texRect2(0.1,0.5);
    glPopMatrix();

	glPushMatrix();
    glTranslatef(0.1,3.5,0);
    glRotatef(-90,1,0,0);
    glBindTexture(GL_TEXTURE_2D, textures[24]);
    texRect2(0.1,0.4);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.1,2.5,0);
    glRotatef(-90,1,0,0);
    glBindTexture(GL_TEXTURE_2D, textures[24]);
    texRect2(0.1,0.4);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.25,3,0.15);
    glRotatef(-90,0,1,0);
    glBindTexture(GL_TEXTURE_2D, textures[25]);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(0.0, 1.0);  glVertex3f(0.0,0.1,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(0.1,0.1,0.0);
		glTexCoord2f(1.0, 0.0);   glVertex3f(0.1,0.0,0.0);
		glEnd();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.25,3,0.0);
    glRotatef(-90,0,1,0);
    glBindTexture(GL_TEXTURE_2D, textures[26]);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(0.0, 1.0);  glVertex3f(0.0,0.1,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(0.1,0.1,0.0);
		glTexCoord2f(1.0, 0.0);   glVertex3f(0.1,0.0,0.0);
		glEnd();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.25,3,-0.15);
    glRotatef(-90,0,1,0);
    glBindTexture(GL_TEXTURE_2D, textures[27]);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(0.0, 1.0);  glVertex3f(0.0,0.1,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(0.1,0.1,0.0);
		glTexCoord2f(1.0, 0.0);   glVertex3f(0.1,0.0,0.0);
		glEnd();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.25,2.85,0.15);
    glRotatef(-90,0,1,0);
    glBindTexture(GL_TEXTURE_2D, textures[28]);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(0.0, 1.0);  glVertex3f(0.0,0.1,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(0.1,0.1,0.0);
		glTexCoord2f(1.0, 0.0);   glVertex3f(0.1,0.0,0.0);
		glEnd();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.25,2.85,0.0);
    glRotatef(-90,0,1,0);
    glBindTexture(GL_TEXTURE_2D, textures[29]);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(0.0, 1.0);  glVertex3f(0.0,0.1,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(0.1,0.1,0.0);
		glTexCoord2f(1.0, 0.0);   glVertex3f(0.1,0.0,0.0);
		glEnd();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.25,2.85,-0.15);
    glRotatef(-90,0,1,0);
    glBindTexture(GL_TEXTURE_2D, textures[30]);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(0.0, 1.0);  glVertex3f(0.0,0.1,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(0.1,0.1,0.0);
		glTexCoord2f(1.0, 0.0);   glVertex3f(0.1,0.0,0.0);
		glEnd();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.25,2.7,0.15);
    glRotatef(-90,0,1,0);
    glBindTexture(GL_TEXTURE_2D, textures[31]);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(0.0, 1.0);  glVertex3f(0.0,0.1,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(0.1,0.1,0.0);
		glTexCoord2f(1.0, 0.0);   glVertex3f(0.1,0.0,0.0);
		glEnd();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.25,2.7,0.0);
    glRotatef(-90,0,1,0);
    glBindTexture(GL_TEXTURE_2D, textures[32]);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(0.0, 1.0);  glVertex3f(0.0,0.1,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(0.1,0.1,0.0);
		glTexCoord2f(1.0, 0.0);   glVertex3f(0.1,0.0,0.0);
		glEnd();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.25,2.7,-0.15);
    glRotatef(-90,0,1,0);
    glBindTexture(GL_TEXTURE_2D, textures[33]);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(0.0, 1.0);  glVertex3f(0.0,0.1,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(0.1,0.1,0.0);
		glTexCoord2f(1.0, 0.0);   glVertex3f(0.1,0.0,0.0);
		glEnd();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.25,2.55,0.0);
    glRotatef(-90,0,1,0);
    glBindTexture(GL_TEXTURE_2D, textures[34]);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(0.0, 1.0);  glVertex3f(0.0,0.1,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(0.1,0.1,0.0);
		glTexCoord2f(1.0, 0.0);   glVertex3f(0.1,0.0,0.0);
		glEnd();
    glPopMatrix();



    material(0.2,0.2,0.2,1,10);
    if(doorUnlocked) {
    	glDisable(GL_LIGHTING);
    }
    glPushMatrix();
    glTranslatef(0.25,3.15,0.15);
    glRotatef(-90,0,1,0);
    glBindTexture(GL_TEXTURE_2D, textures[35]);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(0.0, 1.0);  glVertex3f(0.0,0.1,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(0.1,0.1,0.0);
		glTexCoord2f(1.0, 0.0);   glVertex3f(0.1,0.0,0.0);
		glEnd();
    glPopMatrix();
    glEnable(GL_LIGHTING);

    if(!doorUnlocked) {
    	glDisable(GL_LIGHTING);
    }
    glPushMatrix();
    glTranslatef(0.25,3.15,0.0);
    glRotatef(-90,0,1,0);
    glBindTexture(GL_TEXTURE_2D, textures[36]);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(0.0, 1.0);  glVertex3f(0.0,0.1,0.0);
		glTexCoord2f(1.0, 1.0);  glVertex3f(0.1,0.1,0.0);
		glTexCoord2f(1.0, 0.0);   glVertex3f(0.1,0.0,0.0);
		glEnd();
    glPopMatrix();
    glEnable(GL_LIGHTING);
}

void outWindow(void) {
	glDisable(GL_LIGHTING);
	glBindTexture(GL_TEXTURE_2D, textures[40]);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0);   glVertex3f(0.0,0.0,0.0);
		glTexCoord2f(0.0, 1.0);   glVertex3f(0.0,30.0,0.0);
		glTexCoord2f(1.0, 1.0);   glVertex3f(50.0,30.0,0.0);
		glTexCoord2f(1.0, 0.0);   glVertex3f(50.0,0.0,0.0);
	glEnd();
	material(0,0,0,1,0);
	glBegin(GL_QUADS);
		glVertex3f(-30.0,-50.0,  0.005);
		glVertex3f(-30.0,50.0,   0.005);
		glVertex3f(80.0,50.0, 0.005);
		glVertex3f(80.0,-50.0,0.005);
	glEnd();
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);
	defaultMaterial();

}





//______________________________________________________________________________
//================================= CALLBACKS ==================================
//______________________________________________________________________________

//display callack.
void display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 	glLoadIdentity();

 	glDisable(GL_TEXTURE_2D);
 	glEnable(GL_LIGHTING);
 	defaultMaterial();

 	//office lights
 	if(inOffice()  ||  officeDoorOpen) {
 		glEnable(GL_LIGHT1);
 		glEnable(GL_LIGHT0);
 	}
 	else {
 		glDisable(GL_LIGHT1);
 		glDisable(GL_LIGHT0);
 	}

 	//torture room lights
 	if(inTortureRoom()  ||  tortureRoomDoorOpen) {
 		glEnable(GL_LIGHT2);
 	}
 	else{
 		glDisable(GL_LIGHT2);
 	}

 	//hallway lights
 	if (inHallway() ||  officeDoorOpen  || tortureRoomDoorOpen) {
 		glEnable(GL_LIGHT3);
 	}
 	else {
 		glDisable(GL_LIGHT3);
 	}



 	//the held item
 	if(selected == KEY) {
		glPushMatrix();
		glTranslatef(0.8,-0.2,-1);
		key();
		glPopMatrix();
	}
	else if(selected == MACE) {
		glPushMatrix();
		glTranslatef(1.0,0,-1.5);
		glRotatef(-20,1,0,0);
		mace();
		glPopMatrix();
	}



 	glDisable(GL_LIGHTING);

 	//view changes
 	glRotatef(xrot+xrotChange, 1,0,0);	//viewer x rotation
 	glRotatef(yrot+yrotChange, 0,1,0);	//viewer y rotation
 	glTranslatef(-xpos,-ypos,-zpos);	//viewer position
	gluLookAt(0,3,0,  0,3,5,  0,1,0);	//camera


	light0(); // large light
	light1(); // fireplace
	light2(); //hallway light
	light3(); //torture room chandelier



	glEnable(GL_LIGHTING);
	// objects /////////////////////////////////////////////////////////////////



	glBindTexture(GL_TEXTURE_2D, textures[3]);

	//statue
	humanScale();

	//chandelier
	{
		glPushMatrix();
		glTranslatef(43,10,0);
		glRotatef(180,0,0,1);
		chandelier();
		glPopMatrix();

		glPushMatrix();
		glTranslatef(43.5,11,0);
		glRotatef(180,0,0,1);
		chandelier();
		glPopMatrix();

		glPushMatrix();
		glTranslatef(42.5,11,0);
		glRotatef(180,0,0,1);
		chandelier();
		glPopMatrix();
	}

	//key
	if(selected != KEY) {
		glPushMatrix();
		glTranslatef(objPos[KEY][0], objPos[KEY][1], objPos[KEY][2]);
		glRotatef(objPos[KEY][3], 1,0,0);
		key();
		glPopMatrix();
	}

	//mace
	if(selected != MACE) {
		glPushMatrix();
		glTranslatef(objPos[MACE][0], objPos[MACE][1], objPos[MACE][2]);
		glRotatef(objPos[MACE][3], 1,0,0);
		glRotatef(-85,0,0,1);
		mace();
		glPopMatrix();
	}

	//vase
		glPushMatrix();
		glTranslatef(43,0.8,-8);
		vase();
		glPopMatrix();

	//safe
		glPushMatrix();
		glTranslatef(30,0,0);
		glRotatef(180,0,1,0);
		safe();
		glPopMatrix();

	//keypad
		glPushMatrix();
		glTranslatef(13,0,-5);
		glRotatef(180,0,1,0);
		keypad();
		glPopMatrix();

	defaultMaterial();



	//office room door
		//door 1 right
		glPushMatrix();
		glTranslatef(13,0,3);
		glRotatef(room1DoorRot,0,1,0);         //open door to this rotation
		door(1);
		glPopMatrix();
		//door1 left
		glPushMatrix();
		glTranslatef(13,0,-3);
		glRotatef(-room1DoorRot,0,1,0);        //open door to this rotation
		glTranslatef(0,0,3);
		door(0);
		glPopMatrix();
		//door frame
		glPushMatrix();
		glTranslatef(12.96,0,-3);
		doorFrame();
		glPopMatrix();

	//torture room door
		//door right
		glPushMatrix();
		glTranslatef(32.88,0,3);
		glRotatef(-room2DoorRot,0,1,0);         //open door to this rotation
		door(1);
		glPopMatrix();
		//door left
		glPushMatrix();
		glTranslatef(32.88,0,-3);
		glRotatef(room2DoorRot,0,1,0);        //open door to this rotation
		glTranslatef(0,0,3);
		door(0);
		glPopMatrix();
		//door frame
		glPushMatrix();
		glTranslatef(32.83,0,-3);
		doorFrame();
		glPopMatrix();


	// room 1 Objects
	// render room 1 objects if player is in room 1 or can see it from the hall
	if(inOffice() || officeDoorOpen) {

		//fireplace
		glPushMatrix();
		glTranslatef(-12.4,0,-2.5);
		glRotatef(-90,0,1,0);
		fireplace();
		glPopMatrix();

		//bookshelf
		glPushMatrix();
		glTranslatef(0,1,0);
		bookshelf();
		glPopMatrix();

		//table
		glPushMatrix();
		glTranslatef(-8,0,8);
		glRotatef(45,0,1,0);
		table();
		glPopMatrix();

		//chair
		glPushMatrix();
		glTranslatef(-8,0,6);
		chair();
		glPopMatrix();

		//portrait
		glPushMatrix();
		glTranslatef(-12.7,2.5,0);
		portrait();
		glPopMatrix();

		//masecase
		glPushMatrix();
		glTranslatef(10,0,10);
		glRotatef(30,0,1,0);
		masecase();
		glPopMatrix();
	}

	// room 2 Objects
	// render room 2 objects if player is in room 2 or can see it from the hall
	if(inTortureRoom() || tortureRoomDoorOpen) {

		//chairs at table
			glPushMatrix();
			glTranslatef(44,0,-1.5);
			glRotatef(90,0,0,0);
			chair();
			glPopMatrix();

			glPushMatrix();
			glTranslatef(45,0,1.5);
			glRotatef(180,0,1,0);
			chair();
			glPopMatrix();

			glPushMatrix();
			glTranslatef(44-1.5,0,-1.5);
			glRotatef(90,0,0,0);
			chair();
			glPopMatrix();

			glPushMatrix();
			glTranslatef(45-1.5,0,1.5);
			glRotatef(180,0,1,0);
			chair();
			glPopMatrix();

			glPushMatrix();
			glTranslatef(44-3,0,-1.5);
			glRotatef(90,0,0,0);
			chair();
			glPopMatrix();

			glPushMatrix();
			glTranslatef(45-3,0,1.5);
			glRotatef(180,0,1,0);
			chair();
			glPopMatrix();


		//vartable
		glPushMatrix();
		glTranslatef(43,0,0);
		glRotatef(90,0,1,0);
		vartable();
		glPopMatrix();

		//ottoman
		glPushMatrix();
		glTranslatef(40+3,0,0-8);
		glRotatef(0,0,1,0);
		ottoman();
		glPopMatrix();


		//picture
		glPushMatrix();
		glTranslatef(37,2,-9.999);
		glRotatef(90,0,1,0);
		portrait();
		glPopMatrix();

		glPushMatrix();
		glTranslatef(43,2,-9.999);
		glRotatef(90,0,1,0);
		portrait();
		glPopMatrix();

		glPushMatrix();
		glTranslatef(49,2,-9.999);
		glRotatef(90,0,1,0);
		portrait();
		glPopMatrix();


	}

	// room 1
	// render room 1 if player is in room 1 or can see it from the hall
	if(inOffice() || officeDoorOpen) {
		glBindTexture(GL_TEXTURE_2D, textures[0]); //wood planks
		material(1,0.8,0.5,1,5);                     //material properties
		//room 1 floor
		glPushMatrix();
			glTranslatef(-12,0,-12);
			glRotatef(90,1,0,0);
			tiledTexWall(13,13);
		glPopMatrix();

		material(1,0.8,0.5,1,5);

		//room1 wall 1
		glPushMatrix();
			glTranslatef(12,1,-13);
			glRotatef(180,0,1,0);
			glBindTexture(GL_TEXTURE_2D, textures[5]); //wallpaper
			tiledTexWall(13,1);
			glPushMatrix();
				glBindTexture(GL_TEXTURE_2D, textures[3]); //wood panels
				glTranslatef(0,2,0);
				tiledTexWall(13,4);
			glPopMatrix();
		glPopMatrix();

		//room1 wall 2, right side
		glPushMatrix();
			glTranslatef(13,1,12);
			glRotatef(90,0,1,0);
			glBindTexture(GL_TEXTURE_2D, textures[5]); //wood panels
			tiledTexWall(5,1);
			glPushMatrix();
				glBindTexture(GL_TEXTURE_2D, textures[3]); //wallpaper
				glTranslatef(0,2,0);
				tiledTexWall(5,4);
			glPopMatrix();
		glPopMatrix();

		//room1 wall 2, left side
		glPushMatrix();
			glTranslatef(13,1,-4);
			glRotatef(90,0,1,0);
			glBindTexture(GL_TEXTURE_2D, textures[5]); //wood panels
			tiledTexWall(5,1);
			glPushMatrix();
				glBindTexture(GL_TEXTURE_2D, textures[3]); //wallpaper
				glTranslatef(0,2,0);
				tiledTexWall(5,4);
			glPopMatrix();
		glPopMatrix();

		//room1 wall2, over door
		glPushMatrix();
			glBindTexture(GL_TEXTURE_2D, textures[3]); //wallpaper
			glTranslatef(13,7,2);
			glRotatef(90,0,1,0);
			tiledTexWall(3,2);
		glPopMatrix();

		//room1 wall 3
		glPushMatrix();
			glTranslatef(-12,1,13);
			glBindTexture(GL_TEXTURE_2D, textures[5]); //wood panels
			tiledTexWall(13,1);
			glPushMatrix();
				glBindTexture(GL_TEXTURE_2D, textures[3]); //wallpaper
				glTranslatef(0,2,0);
				tiledTexWall(13,4);
			glPopMatrix();
		glPopMatrix();


		//room1 wall 4
		//glDisable(GL_LIGHT1);
		glPushMatrix();
			glTranslatef(-13,1,-12);
			glRotatef(-90,0,1,0);
			glBindTexture(GL_TEXTURE_2D, textures[5]); //wood panels
			tiledTexWall(13,1);
			glPushMatrix();
				glBindTexture(GL_TEXTURE_2D, textures[3]); //wallpaper
				glTranslatef(0,2,0);
				tiledTexWall(13,4);
			glPopMatrix();
		glPopMatrix();
		//glEnable(GL_LIGHT1);

		//room 1 ceiling
		glBindTexture(GL_TEXTURE_2D, textures[4]);
		glPushMatrix();
			glTranslatef(-12,10,12);
			glRotatef(-90,1,0,0);
			tiledTexWall(13,13);
		glPopMatrix();
	}

	// room 2
	// render room 2 if player is in room 2 or can see it from the hall
	if(inTortureRoom() || tortureRoomDoorOpen) {
		//room 2
		glPushMatrix();
		glTranslatef(45.99,0,0);
			glBindTexture(GL_TEXTURE_2D, textures[0]); //wood planks
			material(1,0.8,0.5,1,5);                     //material properties
			//room 2 floor
			glPushMatrix();
				glTranslatef(-12,0,-9);
				glRotatef(90,1,0,0);
				tiledTexWall(10,10);
			glPopMatrix();

			material(1,0.8,0.5,1,5);

			//room1 wall 1
			glPushMatrix();
				glTranslatef(6,1,-10);
				glRotatef(180,0,1,0);
				glBindTexture(GL_TEXTURE_2D, textures[5]); //wallpaper
				tiledTexWall(10,1);
				glPushMatrix();
					glBindTexture(GL_TEXTURE_2D, textures[3]); //wood panels
					glTranslatef(0,2,0);
					tiledTexWall(10,7);
				glPopMatrix();
			glPopMatrix();

			//Far wall
			glPushMatrix();
				glTranslatef(7,1,9);
				glRotatef(90,0,1,0);
				glBindTexture(GL_TEXTURE_2D, textures[5]); //wood panels
				tiledTexWall(10,1);
				glPushMatrix();
					glBindTexture(GL_TEXTURE_2D, textures[3]); //wallpaper
					glTranslatef(0,2,0);
					tiledTexWall(10,7);
				glPopMatrix();
			glPopMatrix();

			//room1 wall 3
			glPushMatrix();
				glTranslatef(-12,1,10);
				glBindTexture(GL_TEXTURE_2D, textures[5]); //wood panels
				tiledTexWall(10,1);
				glPushMatrix();
					glBindTexture(GL_TEXTURE_2D, textures[3]); //wallpaper
					glTranslatef(0,14,0);
					tiledTexWall(10,1);
				glPopMatrix();
				glPushMatrix();
					glBindTexture(GL_TEXTURE_2D, textures[3]); //wallpaper
					glTranslatef(0,0,0);
					tiledTexWall(1,7);
				glPopMatrix();
				glPushMatrix();
					glBindTexture(GL_TEXTURE_2D, textures[3]); //wallpaper
					glTranslatef(18,0,0);
					tiledTexWall(1,7);
				glPopMatrix();
			glPopMatrix();

			//room 2 ceiling
			glBindTexture(GL_TEXTURE_2D, textures[4]);
			glPushMatrix();
				glTranslatef(-12,16,9);
				glRotatef(-90,1,0,0);
				tiledTexWall(10,10);
			glPopMatrix();

			//Connecting Wall
				//room1 wall 2, right side
				glPushMatrix();
					glTranslatef(-12.93,1,4);
					glRotatef(-90,0,1,0);
					glBindTexture(GL_TEXTURE_2D, textures[5]); //wood panels
					tiledTexWall(4,1);
					glPushMatrix();
						glBindTexture(GL_TEXTURE_2D, textures[3]); //wallpaper
						glTranslatef(0,2,0);
						tiledTexWall(4,7);
					glPopMatrix();
				glPopMatrix();

				//room1 wall 2, left side
				glPushMatrix();
					glTranslatef(-12.93,1,-10);
					glRotatef(-90,0,1,0);
					glBindTexture(GL_TEXTURE_2D, textures[5]); //wood panels
					tiledTexWall(4,1);
					glPushMatrix();
						glBindTexture(GL_TEXTURE_2D, textures[3]); //wallpaper
						glTranslatef(0,2,0);
						tiledTexWall(4,7);
					glPopMatrix();
				glPopMatrix();

				glPushMatrix();
					glBindTexture(GL_TEXTURE_2D, textures[3]); //wallpaper
					glTranslatef(-12.93,7,-2);
					glRotatef(-90,0,1,0);
					tiledTexWall(3,7);
				glPopMatrix();
		glPopMatrix();


		glPushMatrix();
		glTranslatef(25,-5,20);
		outWindow();
		glPopMatrix();

		//window
		glPushMatrix();
		glTranslatef(33,0.29,10.15);
		glRotatef(90,0,1,0);
		window();
		glPopMatrix();
	}







	defaultMaterial();

	//Hallway
	hallway();


	//reverse hallway
	glPushMatrix();
	glScalef(1,1,-1);
	glTranslatef(0,0,-10);
	hallway();
	glPopMatrix();


	//mirror doors
	glPushMatrix();
	glTranslatef(0,0,10);
		//office room door reflection. remove if in the office so the door won't
		//show through the wall.
		if(!inOffice()) {
			//door right
			glPushMatrix();
			glTranslatef(13,0,3);
			door(1);
			glPopMatrix();
			//door left
			glPushMatrix();
			glTranslatef(13,0,0);
			door(0);
			glPopMatrix();
			//door frame
			glPushMatrix();
			glTranslatef(13,0,-3);
			doorFrame();
			glPopMatrix();
		}

		//torture room door reflection. remove if in the room so the door won't
		//show through the wall.
		if (!inTortureRoom()) {
			//door right
			glPushMatrix();
				glTranslatef(32.88,0,3);
				door(1);
				glPopMatrix();
				//door left
				glPushMatrix();
				glTranslatef(32.88,0,0);
				door(0);
				glPopMatrix();
				//door frame
				glPushMatrix();
				glTranslatef(32.83,0,-3);
				doorFrame();
			glPopMatrix();
		}
	glPopMatrix();

	//hallway mirror1
	glPushMatrix();
	glTranslatef(23,0,5.15);
	glRotatef(90,0,1,0);
	mirror();
	glPopMatrix();

	//hallway mirror2
	glPushMatrix();
	glTranslatef(15,0,5.15);
	glRotatef(90,0,1,0);
	mirror();
	glPopMatrix();








	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);

	//fire animation
	if(inOffice()  ||  officeDoorOpen) {
		glEnable(GL_TEXTURE_2D);
		glPushMatrix();
		glTranslatef(-12.85,0,2);
		glRotatef(90,0,1,0);
		fire();
		glPopMatrix();
		glDisable(GL_TEXTURE_2D);
	}

	menu();

	glutSwapBuffers();
} //end display




//opening animation for door 1
void openDoor1(int value) {
	if (room1DoorRot < 120.0 && !door1Closing) { //if door isn't open and is not closing
		room1DoorRot += 1;
		glutTimerFunc(16, openDoor1, 1); //repeat
	}
}

//opening animation for door 1
void openDoor2(int value) {
	if (room2DoorRot < 120.0 && !door2Closing) { //if door isn't open and is not closing
		room2DoorRot += 1;
		glutTimerFunc(16, openDoor2, 1); //repeat
	}
}

//closing animation for door 1
void closeDoor1(int value) {
	if (room1DoorRot > 0) { //if door isn't closed
		room1DoorRot -= 1.5;
		glutTimerFunc(16, closeDoor1, 1); //repeat
	}
	else {
		room1DoorRot = 0;
		officeDoorOpen = 0; //door is closed
		door1Closing = 0; 	//door is no longer closing
	}
}

//closing animation for door 1
void closeDoor2(int value) {
	if (room2DoorRot > 0) { //if door isn't closed
		room2DoorRot -= 1.5;
		glutTimerFunc(16, closeDoor2, 1); //repeat
	}
	else {
		room2DoorRot = 0;
		tortureRoomDoorOpen = 0; //door is closed
		door2Closing = 0;		 //door is no longer closing
	}
}

// interacts with items around the environment
void mouse(int butt, int state, int x,  int y) {

	if (state == GLUT_DOWN  &&  butt == GLUT_LEFT_BUTTON) {	//left click
		float newxrot = xrot + xrotChange;
		float newyrot = yrot + yrotChange;

		float xrotrad, yrotrad;
	    yrotrad = ((newyrot+90.0) / 180 * PI);
	    xrotrad = ((newxrot+90.0) / 180 * PI);

	    float x2,y2,z2,dist;

	    selectPos[0] = (sin(xrotrad)) * (cos(yrotrad)) * selRangeRadius - xpos ;
	    selectPos[1] = (cos(xrotrad)) * selRangeRadius + ypos+3;
	    selectPos[2] = (sin(xrotrad)) * (sin(yrotrad)) * selRangeRadius -zpos ;

	    //if doors are unlocked, check for a door click
	    if(doorUnlocked) {
		    //get distance from player selection sphere to door 1
		    x2 = pow(selectPos[0] - door1Select[0], 2);
			y2 = pow(selectPos[1] - door1Select[1], 2);
			z2 = pow(selectPos[2] - door1Select[2], 2);
			dist = sqrt(x2 + y2 + z2);

			if(dist < selSphereRadius+doorSelRadius) {//if office door is in range
				officeDoorOpen = 1;
				glutTimerFunc(16, openDoor1, 1);
			}

			//get distance from player selection sphere to door 2
			x2 = pow(selectPos[0] - door2Select[0], 2);
			y2 = pow(selectPos[1] - door2Select[1], 2);
			z2 = pow(selectPos[2] - door2Select[2], 2);
			dist = sqrt(x2 + y2 + z2);

			if(dist < selSphereRadius+doorSelRadius) {//if torture room door is in range
				tortureRoomDoorOpen = 1;
				glutTimerFunc(16, openDoor2, 1);
			}
		}
		else { //else ckeck for a keypad click to unlock the doors
			//get distance from player selection sphere to door 2
			x2 = pow(selectPos[0] - keypadSelect[0], 2);
			y2 = pow(selectPos[1] - keypadSelect[1], 2);
			z2 = pow(selectPos[2] - keypadSelect[2], 2);
			dist = sqrt(x2 + y2 + z2);

			if(dist < selSphereRadius+keypadSelRadius) {//if keypad is in range
				doorUnlocked = 1;
			}
		}



	    //if we are holding something, put it down
		if(selected != -1) {
			objPos[selected][3] = 90;
			objPos[selected][0] = selectPos[0];
			objPos[selected][1] = 0;
			objPos[selected][2] = selectPos[2];
			selected = -1;
		}
		//else, not holding an item
		else {
			int closest = -1; //closest item so far
			float closestDist = 999;
			for (int i=0; i<numObjs; i++) { //check collision with all interactable objects
				float x2 = pow(selectPos[0] - objPos[i][0], 2);
				float y2 = pow(selectPos[1] - objPos[i][1], 2);
				float z2 = pow(selectPos[2] - objPos[i][2], 2);
				float dist = sqrt(x2 + y2 + z2);

				if(dist < selSphereRadius+objPos[i][4]) {//if object is in range
					if(dist < closestDist) {
						closest = i;
						closestDist = dist;
					}
				}
			} //end for each object
			if (closest != -1) {
				selected = closest;
			}
		}//end else
	} //end left-click
} //end mouse


// the old mouse function. because monica's computer is silly. XD
void oldMouse(int butt, int state, int x,  int y) {
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

// Changes the temporary view rotation when the mouse moves
// The temporary rotation angle is proportional to the distance of the mouse
// pointer from the starting click point.
void motion(int x, int y) {
	xrotChange = (float)(y - screenCenterY)/3.0;	//set the temp x-axis rot to the mouse y distance
	yrotChange = (float)(x - screenCenterX)/3.0;	//set the temp y-axis rot to the mouse x distance

	//limit the x-axis rotation to prevent the camera from being able to flip upside-down
	if(xrot+xrotChange > 70.0) {	//if camera tries to flip over from above
		xrotChange = 70.0 - xrot;
	}
	if(xrot+xrotChange < -70.0) {	//if camera tries to flip over from below
		xrotChange = -70 - xrot;
	}
} //end motion


//sets the key press states to true when the key gets pressed
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
   		if(helpMenu == 1) {
   			helpMenu = 0;
   			glutMotionFunc(motion);
   			glutPassiveMotionFunc(motion);
   			glutSetCursor(GLUT_CURSOR_NONE);
   		}
   		else {
   			helpMenu = 1;
   			glutMotionFunc(NULL);
   			glutPassiveMotionFunc(NULL);
   			glutSetCursor(GLUT_CURSOR_FULL_CROSSHAIR);
   		}
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


//arrow keys for moving the light source
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


//applies movements and rotation changes and redraws the world at set intervals
void timer(int value) {

	//movement when keys are pressed. freezes if the menu is open
	if (!helpMenu) {
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
			//float xrotrad;
			float yrotrad;
	        yrotrad = (newyrot / 180 * PI);
	        //xrotrad = (newxrot / 180 * PI);
	        xpos += (float)(sin(yrotrad)) * movementSpeed;
	        zpos -= (float)(cos(yrotrad)) * movementSpeed;
		}
		if (s_state) {								//s key is pressed (move backwards)
			//float xrotrad;
			float yrotrad;
	        yrotrad = (newyrot / 180 * PI);
	        //xrotrad = (newxrot / 180 * PI);
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

		if(inHallway()) {
			if(officeDoorOpen && xpos<-23){
				glutTimerFunc(16,closeDoor1,1);
				door1Closing = 1;
			}
			if(tortureRoomDoorOpen && xpos>-24){
				glutTimerFunc(16,closeDoor2,1);
				door2Closing = 1;
			}
		}



		//limit walking through wall to the next room
		if(inOffice() && xpos < -12  &&  !officeDoorOpen) {
			xpos = -12;
		}
		else if (inHallway()) {
			if(xpos > -14 && !officeDoorOpen){
				xpos = -14;
			}
			else if(xpos < -32 && !tortureRoomDoorOpen){
				xpos = -32;
			}
		}
		else if(inTortureRoom()  &&  xpos > -34  &&  !tortureRoomDoorOpen) {
			xpos = -34;
		}



		//MONICA, comment this stuff
		// trap the cursor in the center of the window unless the menu is open
		// this allows the user to move the mouse to change the viewing angle
		// without running off the sides of the window.
		xrot += xrotChange;
		yrot += yrotChange;
		xrotChange = 0;
		yrotChange = 0;
		glutWarpPointer(screenCenterX, screenCenterY);
	} //end if helpmenu



	glutPostRedisplay();						//redraw scene
	glutTimerFunc(waitTime, timer, 1);			//set next timer
} //end timer


//controls the fire flickering effect
void fireJitterTimer(int value) {
	fireJitter = (rand()%20)/20.0;
	fireKc = (rand()%20)/140.0 + 0.2;
	fireTexNum ++;
	if(fireTexNum>9) fireTexNum = 0;
	glutTimerFunc(100, fireJitterTimer, 1);
}

//switches out hte fire animation frames
void fireTimer(int value) {
	// fireJitter = (rand()%20)/20.0;
	// fireKc = (rand()%20)/140.0 + 0.2;
	fireTexNum ++;
	if(fireTexNum>9) fireTexNum = 0;
	glutTimerFunc(60, fireTimer, 1);
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

	screenWidth = w;
	screenHeight = h;
	screenCenterX = w/2;
	screenCenterY = h/2;

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
   	//glEnable(GL_LIGHT1);
   	//glEnable(GL_LIGHT2);
   	glEnable(GL_LIGHTING);		//enable lighting
   	//repositions specular reflections for view change
	glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);


 	glutFullScreen();

   	glutSetCursor(GLUT_CURSOR_NONE);   //MONICA comment this line


   	glutWarpPointer(screenCenterX, screenCenterY);   //MONICA comment this line
   	float xrot=0, yrot=90;
 	glutIgnoreKeyRepeat(1);		// disables glut from simulating key press and
 								// release repetitions when holding down a key

 	initTex();					//create the textures (saved in textures arrray)
 	initQObj();					//create a quadric object for glu shapes
 	srand(time(NULL));
 	//enableFog(0.5,0.5,0.5,0.05); //fog
 	enableFog(0.5,0.3,0.1,0.03); //warm


 	//event callbacks
 	glutDisplayFunc(display);			//display
 	glutReshapeFunc(reshape);			//reshape window

 	glutMouseFunc(mouse);				//mouse button clicks   MONICA comment this
 	//glutMouseFunc(oldMouse);  		//MONICA un-comment this
 	glutMotionFunc(motion);				//mouse click movement
 	glutPassiveMotionFunc(motion);   	//MONICA  comment this line

 	glutKeyboardFunc(keyboard);			//key presses
 	glutKeyboardUpFunc(keyboardUp);		//key release
 	glutSpecialFunc(specialKey);

 	glutTimerFunc(waitTime, timer, 1);	//redraws world at intervals
 	glutTimerFunc(100, fireJitterTimer, 1);	//fire flicker
 	glutTimerFunc(60, fireTimer, 1);	//fire image change

 	glutMainLoop();
	return 0;
} //end main