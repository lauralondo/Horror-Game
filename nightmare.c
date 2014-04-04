/*
 *	test program to model a character
 */

#include <GL/glut.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define screenWidth 1000
#define screenHeight 700
#define PI 3.14159265		//pi
#define groundSize 20.0 	//size of the ground grid
#define waitTime 16
#define movementSpeed 0.1


float xpos = 0, ypos=0, zpos = 10;				//camera position
float xrot=0, yrot=0;							//camera angle
float xrotChange, yrotChange = 0;				//camera view attributes

int w_state, a_state, s_state,
	d_state, q_state, e_state = 0;					//key presses
int mousePressed, mouseStartX, mouseStartY = 0;		//mouse states

int jumpRising=0;
float jumpSpeed=0;						//jump height increasing

int helpMenu = 1;


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
}


//switches to 2D when true in order to draw on the front of the screen for the menu
void menuMode(int flag) {
	if(flag == 1) {
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();

		gluOrtho2D(0, screenWidth, 0, screenHeight);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		glDisable(GL_DEPTH_TEST);
	}
	else {
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		glEnable(GL_DEPTH_TEST);
	}
}


void menu(void) {
	if(helpMenu) {
		menuMode(1); 						//go into menu mode (switch to 2D)

		float menuPosX = 700, menuPosY=570;	//menu start position
		glColor3f(0.8,0.8,0.8);

		bitmapText(menuPosX,menuPosY, 0, "MENU");

		menuMode(0);						//switch back to 3D mode
	}
}

//Model the ground. consiste of a flat gorund color and a grid
void ground(void) {
	glColor3f(0,0,0);		//grid color
	glLineWidth(1);			//line width

	//draw grid
	for (int i = 0; i < groundSize*2+1; i++) {
		glBegin(GL_LINES);
			glVertex3f(-groundSize + i, 0, groundSize);
			glVertex3f(-groundSize + i, 0, -groundSize);
		glEnd();
	}
	for (int j = 0; j < groundSize*2+1; j++) {
		glBegin(GL_LINES);
			glVertex3f(-groundSize, 0, groundSize - j);
			glVertex3f(groundSize, 0, groundSize - j);
		glEnd();
	}

	//draw ground color under the grid
	glBegin(GL_POLYGON);
	glColor3f(0.1,0.2,0.05);
	glVertex3f(-groundSize, -0.1, -groundSize);
	glVertex3f(-groundSize, -0.1, groundSize);
	glVertex3f(groundSize, -0.1, groundSize);
	glVertex3f(groundSize, -0.1, -groundSize);
	glEnd();
}



void display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 	glLoadIdentity();

 	glRotatef(xrot+xrotChange, 1,0,0);	//viewer x rotation
 	glRotatef(yrot+yrotChange, 0,1,0);	//viewer y rotation
 	glTranslatef(-xpos,-ypos,-zpos);	//viewer position
	gluLookAt(0,3,0,  0,3,5,  0,1,0);	//camera

	ground();							//draw ground



	menu();

	glutSwapBuffers();
}


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
}


//sets the key press states to false when the key is released
void keyboardUp(unsigned char key, int x, int y) {
	if(key == 'a') a_state = 0;		//stop strafe left
	if(key == 'd') d_state = 0;		//stop strafe right
	if(key == 'w') w_state = 0;		//stop move forward
	if(key == 's') s_state = 0;		//stop move backwards
	if(key == 'e') e_state = 0;		//stop rotate right
	if(key == 'q') q_state = 0;		//stop rotate left
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
}

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
}


//applies key press movements and rotation changes and redraws the world at set intervals
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
}


//reshape callback. adjusts both the clipping box and viewport. keeps proportions unchanged
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
}


int main(int argc, char **argv) {
	glutInit(&argc, argv);
 	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
 	glutInitWindowSize(screenWidth, screenHeight);
 	glutCreateWindow("Thing");
 	glEnable(GL_DEPTH_TEST);
 	glClearColor(0,0,0,0);

 	//glutIgnoreKeyRepeat(1);		// disables glut from simulating key press and release
 								// repetitions when holding down a key


 	glutDisplayFunc(display);
 	glutReshapeFunc(reshape);			//reshape window
 	glutKeyboardFunc(keyboard);			//key presses
 	glutKeyboardUpFunc(keyboardUp);		//key release
 	glutTimerFunc(waitTime, timer, 1);	//redraws world at intervals

 	glutMainLoop();
	return 0;
}