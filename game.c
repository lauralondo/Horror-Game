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
#include <stdlib.h>

#define screenWidth 1000	//initial screem width
#define screenHeight 700	//initial screen height
#define PI 3.14159265		//pi
#define groundSize 20 		//size of the ground grid
#define waitTime 16 		//millisecond wait between redisplays
#define movementSpeed 0.1 	//player movement speed


int textures[2];								//the loaded textures

float xpos = 0, ypos=0, zpos = 10;				//camera position
float xrot=0, yrot=0;							//camera angle
float xrotChange, yrotChange = 0;				//camera view attributes

int w_state, a_state, s_state,					//key presses
	d_state, q_state, e_state = 0;
int mousePressed, mouseStartX, mouseStartY = 0;	//mouse states

int jumpRising=0;								//if jumping up
float jumpSpeed=0;								//jump height increasing

int helpMenu = 1;								//true if displaying menu


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

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

	// Sets the wrap parameters in both directions
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

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
	glGenTextures(2, textures);
	LoadTex(textures[0], "textures/woodplanks1.bmp");
	LoadTex(textures[1], "textures/dirt2.bmp");
} //end initTex


//creates a flat, textured rectangle
void texRect(void) {
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0); glVertex3f(-1.0, -1.0, 0.0);
		glTexCoord2f(0.0, 1.0); glVertex3f(-1.0, 1.0, 0.0);
		glTexCoord2f(1.0, 1.0); glVertex3f(1.0, 1.0, 0.0);
		glTexCoord2f(1.0, 0.0); glVertex3f(1.0, -1.0, 0.0);
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
void ground(void) {
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

	/*
	//draw flat ground color under the grid
	glBegin(GL_POLYGON);
	glColor3f(0.1,0.2,0.05);
	glVertex3f(-groundSize, -0.1, -groundSize);
	glVertex3f(-groundSize, -0.1, groundSize);
	glVertex3f(groundSize, -0.1, groundSize);
	glVertex3f(groundSize, -0.1, -groundSize);
	glEnd();
	*/
} //end ground






//______________________________________________________________________________
//================================= CALLBACKS ==================================
//______________________________________________________________________________

//display callack.
void display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 	glLoadIdentity();

 	glRotatef(xrot+xrotChange, 1,0,0);	//viewer x rotation
 	glRotatef(yrot+yrotChange, 0,1,0);	//viewer y rotation
 	glTranslatef(-xpos,-ypos,-zpos);	//viewer position
	gluLookAt(0,3,0,  0,3,5,  0,1,0);	//camera

	ground();							//draw ground

	//room 1
	glPushMatrix();
	glRotatef(90,1,0,0);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	texRect2(13,13);
	glPopMatrix();


	//room 1 wall
	glPushMatrix();
	glTranslatef(0,8,-13);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	texRect2(13,8);
	glPopMatrix();

	//room 1 wall
	glPushMatrix();
	glTranslatef(0,8,13);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	texRect2(13,8);
	glPopMatrix();


	//room 1 wall
	glPushMatrix();
	glRotatef(90, 0,1,0);
	glTranslatef(0,8,-13);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	texRect2(13,8);
	glPopMatrix();


	//room 1 ceiling
	glPushMatrix();
	glTranslatef(0,16,0);
	glRotatef(90, 1,0,0);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	texRect2(13,13);
	glPopMatrix();













	//room 2
	glPushMatrix();
	glRotatef(90,1,0,0);
	glTranslatef(26,0,0);
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	texRect2(13,13);
	glPopMatrix();

	//room 2 wall
	glPushMatrix();
	glTranslatef(26,8,13);
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	texRect2(13,8);
	glPopMatrix();

	//room 2 wall
	glPushMatrix();
	glTranslatef(26,8,-13);
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	texRect2(13,8);
	glPopMatrix();

	/*//room 2 wall
	glPushMatrix();
	glRotatef(90, 0,1,0);
	glTranslatef(0,13,13);
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	texRect2(13,13);
	glPopMatrix();
	*/
		//room 2 wall
	glPushMatrix();
	glRotatef(90, 0,1,0);
	glTranslatef(0,8,39);
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	texRect2(13,8);
	glPopMatrix();

	//room 1 ceiling
	glPushMatrix();
	glTranslatef(26,16,0);
	glRotatef(90, 1,0,0);
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	texRect2(13,13);
	glPopMatrix();


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
   	if(key == 32) {					//spacebar. jump
   		if(ypos == 0.0)	{			//if viewer is on the ground,
   			jumpRising = 1;			//set rising state to true
   			jumpSpeed = 1;			//set initial up speed to 1
   		}
   	}
   	if(key == 'h') {				//hide / show the help menu
   		if(helpMenu == 1) helpMenu = 0;
   		else helpMenu = 1;
   	}
   	if((int)key == 27) exit(0);		//exit program
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


//reshape callback. adjusts the clipping box & viewport. keeps proportions
void reshape(int w, int h) {
	float left = -0.1, right = 0.1, bottom = -0.1, top = 0.1, znear = 0.1, zfar = 150;
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
 	glutCreateWindow("Nightmare");
 	glEnable(GL_DEPTH_TEST);
 	glClearColor(0,0,0,0);

 	glutIgnoreKeyRepeat(1);	// disables glut from simulating key press and
 							// release repetitions when holding down a key

 	initTex();				//create the textures (saved in textures arrray)


 	//event callbacks
 	glutDisplayFunc(display);			//display
 	glutReshapeFunc(reshape);			//reshape window
 	glutMouseFunc(mouse);				//mouse button clicks
 	glutMotionFunc(motion);				//mouse click movement
 	glutKeyboardFunc(keyboard);			//key presses
 	glutKeyboardUpFunc(keyboardUp);		//key release
 	glutTimerFunc(waitTime, timer, 1);	//redraws world at intervals

 	glutMainLoop();
	return 0;
} //end main
