#include "cloth.h"
#include "render.h"
#include "mouse.h"
#include "physics.h"
#include "vector.h"
#include "texture.h"
#include "camera.h"
#include "keyboard.h"
#include "ui.h"
#include <time.h>

// Window settings
float ratio;

// Camera controls
point cameraPos, lineOfSight, viewVector, cameraAng;
float cameraRot, cameraDist;

GLUI *glui;
int mainWindowId = 0;

// Camera controls
double Theta = 0;//PI / 6;
double Phi = PI + PI / 2;//PI / 6;
double R = 4;

// Light controls
int lighting;
int setShineLevel, setSpecLevel, setEmissLevel;
int light1, light2, light3, light4, light5, light6, light7, light8;
int materialColor;

// Sphere position in the bounding box
double SPHEREx = 0.0;
double SPHEREy = 0.0;
double SPHEREz = 0.0;
double SPHEREr = 0.8;
double restFactor = 1;

// Mouse controls
point mousePos, pMousePos, userForce;
int leftButton, rightButton, middleButton, objectName = -1;

// these variables control what is displayed on screen
int shear = 1, bend = 1, structural = 1, pause = 1, sphereExists = 0;

// Clothes
int ICounter = -1;
struct instance *clothes;

int windowWidth, windowHeight;

bool shineLevel = false;
bool specLevel = false;
bool emissLevel = false;

int render = 0, light = 0;

// skybox limit
int SKYBOXXLIM = 2;
int SKYBOXYLIM = 2;
int SKYBOXZLIM = 2;

void RenderWorld()
{
//	keyInit();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, ratio, 0.01, 1000.0);

//	glClearColor(0.6, 0.8, 0.196078, 1.0);	// green

	glClearColor (0.4, 0.4, 0.4, 0.0);	// grey

	glEnable(GL_DEPTH_TEST);

	camInit();

	srand(time(NULL));

	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	glShadeModel(GL_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);
	glEnable(GL_LINE_SMOOTH);

//	Snap* snapBK = storeBitmap("wall.bmp");
	Snap* snapBK = storeBitmap("back_face.bmp");
	back_face_id = ImageToGLTexture(snapBK);
	delete snapBK;

//	Snap* snapF = storeBitmap("wall.bmp");
	Snap* snapF = storeBitmap("front_face.bmp");
	front_face_id = ImageToGLTexture(snapF);
	delete snapF;

//	Snap* snapT = storeBitmap("topSky.bmp");
	Snap* snapT = storeBitmap("top_face.bmp");
	top_face_id = ImageToGLTexture(snapT);
	delete snapT;

//	Snap* snapB = storeBitmap("snowFlake.bmp");
	Snap* snapB = storeBitmap("bottom_face.bmp");
	bottom_face_id = ImageToGLTexture(snapB);
	delete snapB;

//	Snap* snapL = storeBitmap("wall.bmp");
	Snap* snapL = storeBitmap("left_face.bmp");
	left_face_id = ImageToGLTexture(snapL);
	delete snapL;

//	Snap* snapR = storeBitmap("wall.bmp");
	Snap* snapR = storeBitmap("right_face.bmp");
	right_face_id = ImageToGLTexture(snapR);
	delete snapR;

	Snap* snapC = storeBitmap("uscWhite.bmp");
	uscWhiteId = ImageToGLTexture(snapC);
	delete snapC;

	snapC = storeBitmap("whiteShirt.bmp");
	whiteShirtId = ImageToGLTexture(snapC);
	delete snapC;

	snapC = storeBitmap("whiteShorts.bmp");
	whiteShortsId = ImageToGLTexture(snapC);
	delete snapC;

	snapC = storeBitmap("whitePant.bmp");
	whitePantId = ImageToGLTexture(snapC);
	delete snapC;

	snapC = storeBitmap("whiteSock.bmp");
	whiteSockId = ImageToGLTexture(snapC);
	delete snapC;

	snapC = storeBitmap("mickey.bmp");
	mickeyId = ImageToGLTexture(snapC);
	delete snapC;

	snapC = storeBitmap("armyCloth.bmp");
	armyClothId = ImageToGLTexture(snapC);
	delete snapC;

	snapC = storeBitmap("clothBag.bmp");
	clothBagId = ImageToGLTexture(snapC);
	delete snapC;

	snapC = storeBitmap("rope.bmp");
	ropeId = ImageToGLTexture(snapC);
	delete snapC;

	glDisable(GL_COLOR_MATERIAL);

	return; 
}

/* Function: renderAxis
 * Description: Renders the xyz Axis in world space.
 * Input: None
 * Output: None
 */
void renderAxis()
{
	glLineWidth(2);
	glDisable(GL_LIGHTING);
	glPushMatrix();
		glBegin(GL_LINES);
			// Render X Axis
			glColor4f(1.0, 0.0, 0.0, 0.0);
			glVertex3f(0.0, 0.0, 0.0);
			glVertex3f(1.0, 0.0, 0.0);

			// Render Y Axis
			glColor4f(0.0, 1.0, 0.0, 0.0);
			glVertex3f(0.0, 0.0, 0.0);
			glVertex3f(0.0, 1.0, 0.0);

			// Render Z Axis
			glColor4f(0.0, 0.0, 1.0, 0.0);
			glVertex3f(0.0, 0.0, 0.0);
			glVertex3f(0.0, 0.0, 1.0);
		glEnd();
	glPopMatrix();
	glEnable(GL_LIGHTING);
} //end renderAxis


/* Function: reshape
 * Description: Redraws the window when the user changes the window size.
 * Input: w - width of the window
 *        h - height of the window
 * Output: None
 */
void reshape (int w, int h)
{
	int winRes = h;
	ratio = 1.0f * w / h;
	glViewport (0, 0, w, h);
	
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(45, ratio, 0.01, 1000);
	glMatrixMode(GL_MODELVIEW);
	setCamera();
} //end reshape


void display()
{
	instance *temp;

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHT2);
	glEnable(GL_LIGHT3);
	glEnable(GL_LIGHT4);
	glEnable(GL_LIGHT5);
	glEnable(GL_LIGHT6);
	glEnable(GL_LIGHT7);
	glEnable(GL_LIGHT0);

	glEnable(GL_NORMALIZE);
	glShadeModel(GL_SMOOTH);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW); 
	glLoadIdentity();

	setCamera();

	GLfloat ambientLight[] = {0.2f, 0.2f, 0.2f, 1.0f};
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight);

	float specLevel = 0.3;
	if (specLevel)
		specLevel = 0.3f;
	else
		specLevel = 1;

	float emissLevel = 0.02;
	if (emissLevel)
		emissLevel = 0.05f;
	else
		emissLevel = 0;

	float shineLevel = 10;
	if (shineLevel)
		shineLevel = 25;
	else
		shineLevel = 12;

//------------------------------
//		SCENE LIGHTING
//------------------------------
	// ---------------------
	//	SPHERE LIGHT 1
	// ---------------------
	GLfloat lightColor1[] = {0.6f, 0.6f, 0.6f, 1.0f};
	GLfloat lightPos1[] = {1.5f * 0.9, 2 * 0.9, 1.5 * 0.9, 1.0f};

	glLightfv(GL_LIGHT1, GL_DIFFUSE, lightColor1);
	glLightfv(GL_LIGHT1, GL_SPECULAR, lightColor1);
	glLightfv(GL_LIGHT1, GL_POSITION, lightPos1);

	// ---------------------
	//	CUBE LIGHT 2
	// ---------------------
	GLfloat lightColor2[] = {0.6f, 0.6f, 0.6f, 1.0f};
	GLfloat lightPos2[] = { -1.9, -1.9, -1.9, 1.0};

	glLightfv(GL_LIGHT2, GL_DIFFUSE, lightColor2);
	glLightfv(GL_LIGHT2, GL_SPECULAR, lightColor2);
	glLightfv(GL_LIGHT2, GL_POSITION, lightPos2);
    
	// ---------------------
	//	CUBE LIGHT 3
	// ---------------------
	GLfloat lightColor3[] = {0.6f, 0.6f, 0.6f, 1.0f};
	GLfloat lightPos3[] = { 1.9, -1.9, -1.9, 1.0};

	glLightfv(GL_LIGHT3, GL_DIFFUSE, lightColor3);
	glLightfv(GL_LIGHT3, GL_SPECULAR, lightColor3);
	glLightfv(GL_LIGHT3, GL_POSITION, lightPos3);

	// ---------------------
	//	CUBE LIGHT 4
	// ---------------------
	GLfloat lightColor4[] = {0.6f, 0.6f, 0.6f, 1.0f};
	GLfloat lightPos4[] = { 1.9, 1.9, -1.9, 1.0 };

	glLightfv(GL_LIGHT4, GL_DIFFUSE, lightColor3);
	glLightfv(GL_LIGHT4, GL_SPECULAR, lightColor3);
	glLightfv(GL_LIGHT4, GL_POSITION, lightPos3);

	// ---------------------
	//	CUBE LIGHT 5
	// ---------------------
	GLfloat lightColor5[] = {0.6f, 0.6f, 0.6f, 1.0f};
	GLfloat lightPos5[] = {-1.9, 1.9, -1.9, 1.0 };

	glLightfv(GL_LIGHT3, GL_DIFFUSE, lightColor3);
	glLightfv(GL_LIGHT3, GL_SPECULAR, lightColor3);
	glLightfv(GL_LIGHT3, GL_POSITION, lightPos3);

	// ---------------------
	//	SPHERE LIGHT 6
	// ---------------------
	GLfloat lightColor6[] = {0.6f, 0.6f, 0.6f, 1.0f};
	GLfloat lightPos6[] = {-1.9, -1.9, 1.9, 1.0};

	glLightfv(GL_LIGHT6, GL_DIFFUSE, lightColor6);
	glLightfv(GL_LIGHT6, GL_SPECULAR, lightColor6);
	glLightfv(GL_LIGHT6, GL_POSITION, lightPos6);

	// ---------------------
	//	SPHERE LIGHT 7
	// ---------------------
	GLfloat lightColor7[] = {0.6f, 0.6f, 0.6f, 1.0f};
	GLfloat lightPos7[] = {1.9, 1.9, 1.9, 1.0 };

	glLightfv(GL_LIGHT7, GL_DIFFUSE, lightColor7);
	glLightfv(GL_LIGHT7, GL_SPECULAR, lightColor7);
	glLightfv(GL_LIGHT7, GL_POSITION, lightPos7);

	// ---------------------
	//	SPHERE LIGHT 8
	// ---------------------
	GLfloat lightColor8[] = {0.6f, 0.6f, 0.6f, 1.0f};
	GLfloat lightPos8[] = {-1.9, 1.9, 1.9, 1.0};

	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor8);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightColor8);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos8);

//------------------------------
//	SPHERE RENDERING
//------------------------------
	
	//The color of the SPHERE Object in the bounding box
	GLfloat materialColor1[] = {1, 1, 0, 1.0f};
	GLfloat materialSpecular1[] = {specLevel, specLevel, specLevel, 1.0f};
	GLfloat materialEmissLevel1[] = {emissLevel, emissLevel, emissLevel, 1.0f};

/*	if(sphereExists)
	{
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, materialColor1);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, materialSpecular1);
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, materialEmissLevel1);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shineLevel); //The shininess parameter

		glDisable(GL_CULL_FACE);
		glPushMatrix();
		glTranslatef(SPHEREx, SPHEREy, SPHEREz);
		glutSolidSphere(SPHEREr, 150, 100);
		glPopMatrix();
	}
*/

// ----------------------------
//	CUBE RENDERING
// ---------------------------
	//The color of the SPHERE Object in the bounding box
	GLfloat materialColor2[] = {1, 0, 0, 1};

	//The specular (shiny) component of the materialf
	GLfloat materialSpecular2[] = {specLevel, specLevel, specLevel, 1.0f};

	//The color emitted by the material
	GLfloat materialEmissLevel2[] = {emissLevel, emissLevel, emissLevel, 1.0f};

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, materialColor2);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, materialSpecular2);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, materialEmissLevel2);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shineLevel);

	glDisable(GL_LIGHTING);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glEnable(GL_LIGHTING);

	
	glBlendFunc(GL_SRC_ALPHA,GL_ONE);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0);

	// Render the box
	RenderBox(front_face_id, back_face_id, right_face_id, left_face_id, top_face_id, bottom_face_id, ropeId);

	if (pause == 0)
	{
		temp = clothes;
		while(temp->next != NULL)
		{
			if(temp->clothType != USC && temp->clothType != MICKEYTOWEL && temp->clothType != ARMY)
			{
				glEnable( GL_BLEND );
				glDisable(GL_DEPTH_TEST);
				glDisable(GL_LIGHTING);
			}

			// Render cloth after the lighting
			RenderClothSystem(temp->napkin, temp->texId);
			
			if(temp->clothType != USC && temp->clothType != MICKEYTOWEL && temp->clothType != ARMY)
			{
				glDisable( GL_BLEND );
				glEnable(GL_DEPTH_TEST);
				glEnable(GL_LIGHTING);
			}
			
			temp = temp->next;
		}
	}

//	renderAxis();

	glutSwapBuffers();
}


void doIdle()
{
	instance *temp;

	temp = clothes;

	if (pause == 0)
	{
		while(temp->next != NULL)
		{
			RK4(temp->napkin);
			temp = temp->next;
		}
	}

	glutPostRedisplay();
}


void AddCloth()
{
	double random = 0;
	int randWidth = 0, randHeight = 0;
	double randSprLen = 0.0, value = 0.0;

	instance *node;
	node = (instance*)malloc(sizeof(instance));
	node->next = clothes;
	clothes = node;

	clothes->index = ++ICounter;

	clothes->napkin = (cloth*)malloc(1 * sizeof(cloth));

	if(gWhichCloth == USC)
	{
		clothes->texId = uscWhiteId;
		clothes->clothType = USC;
	}
	else if(gWhichCloth == MICKEYTOWEL)
	{
		clothes->texId = mickeyId;
		clothes->clothType = MICKEYTOWEL;
	}
	else if(gWhichCloth == ARMY)
	{
		clothes->texId = armyClothId;
		clothes->clothType = ARMY;
	}
	else if(gWhichCloth == CLOTHBAG)
	{
		clothes->texId = clothBagId;
		clothes->clothType = CLOTHBAG;
	}
	else if(gWhichCloth == TSHIRT)
	{
		clothes->texId = whiteShirtId;
		clothes->clothType = TSHIRT;
	}
	else if(gWhichCloth == SHORTS)
	{
		clothes->texId = whiteShortsId;
		clothes->clothType = SHORTS;
	}
	else if(gWhichCloth == PANT)
	{
		clothes->texId = whitePantId;
		clothes->clothType = PANT;
	}
	else if(gWhichCloth == SOCK)
	{
		clothes->texId = whiteSockId;
		clothes->clothType = SOCK;
	}

	randWidth = (int)(rand() % (7 + 1)) + 3;
	randHeight = (int)(rand() % (7 + 1)) + 3;

	randSprLen = (double)(rand() % (4 + 1));
	randSprLen /= 10;
	if(randSprLen == 0.0)
		randSprLen = 0.1;

	random = (double)(rand() % (RANDXLIMIT + 1));
	random -= RANDXLIMIT / 2;
	random /= 100;
	clothes->position.x = random;
/*
	do
	{
		if(value <= -2.0)
		{
			clothes->position.y += 0.1;
		}
		else
		{
			printf("value = %lf\n", value);
			random = (double)(rand() % (RANDYLIMIT + 1));
			random -= RANDYLIMIT / 2;
			random /= 100;
			clothes->position.y = random; 
		}
	}while((value = (clothes->position.y - randSprLen * randHeight)) <= -2.0);
*/

	clothes->position.y = 0.0; 

/*
	random = (double)(rand() % (RANDZLIMIT + 1));
	random -= RANDZLIMIT / 2;
	random /= 100;
	clothes->position.z = random;
*/
	clothes->position.z = 0.0;
	 
//	ClothInit(clothes->napkin, randSprLen, clothes->position, randWidth, randHeight);

	if(gWhichCloth == USC)
	{
		clothes->position = vMake(-0.5, 0.0, 0.0);
		ClothInit(clothes->napkin, 0.1, clothes->position, 10, 10);		
	}
	else if(gWhichCloth == MICKEYTOWEL)
	{
		clothes->position = vMake(-1.8, 0.0, 0.4);
		ClothInit(clothes->napkin, 0.1, clothes->position, 10, 14);
	}
	else if(gWhichCloth == ARMY)
	{
		clothes->position = vMake(0.8, 0.0, 0.4);
		ClothInit(clothes->napkin, 0.1, clothes->position, 10, 14);
	}
	else if(gWhichCloth == TSHIRT)
	{
		clothes->position = vMake(-1.9, 1.5, 0.6);
		ClothInit(clothes->napkin, 0.1, clothes->position, 10, 10);
	}
	else if(gWhichCloth == PANT)
	{
		clothes->position = vMake(-0.8, 1.9, 0.0);
		ClothInit(clothes->napkin, 0.1, clothes->position, 10, 13);
	}
	else if(gWhichCloth == CLOTHBAG)
	{
		clothes->position = vMake(0.3, 1.5, 0.6);
		ClothInit(clothes->napkin, 0.1, clothes->position, 7, 7);
	}
	else if(gWhichCloth == SHORTS)
	{
		clothes->position = vMake(1.1, 1.5, 0.0);
		ClothInit(clothes->napkin, 0.1, clothes->position, 9, 10);
	}
	
	else if(gWhichCloth == SOCK)
	{
//		clothes->position = vMake(1.1, 1.5, 0.0);
		ClothInit(clothes->napkin, 0.1, clothes->position, 10, 15);
	}
	
}


void DeleteClothes()
{
	instance *next, *cur;
	
	next = clothes;
	while(next->next != NULL)
	{
		cur = next;
		next = next->next;	
		free(cur);
	}
	/* To have one left
	clothes = next;
	clothes->index = 0;

	ICounter = 1;
	*/

	clothes = next;
	clothes->index = -1;

	ICounter = -1;
}


void AttachCloth()
{
	instance *node, *last;
	double sprLen = 0.0;
	int indexLeftBottom = 0, indexRightBottom = 0;

	// Recent cloth
	last = clothes;

	node = (instance*)malloc(sizeof(instance));
	node->next = clothes;
	clothes = node;

	clothes->index = ++ICounter;

	clothes->napkin = (cloth*)malloc(1 * sizeof(cloth));

	clothes->texId = uscWhiteId;

	sprLen = last->napkin->strSprLen;

	clothes->napkin->width = last->napkin->width;
	clothes->napkin->height = last->napkin->height;

	indexLeftBottom = last->napkin->width * (last->napkin->height - 1);
	pCPY(last->napkin->positions[indexLeftBottom], clothes->position);

	 
	ClothInit(clothes->napkin, sprLen, clothes->position, clothes->napkin->width, clothes->napkin->height);
}


void reset()
{
	DeleteClothes();
	InitGlobal();
}


int main (int argc, char ** argv)
{ 
	glutInit(&argc,argv);

	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	windowWidth = 800;
	windowHeight = 600;
	glutInitWindowSize (windowWidth, windowHeight);
	glutInitWindowPosition (0,0);
	mainWindowId = glutCreateWindow ("Cloth Simulation");

	// Initialize the global parameters
	InitGlobal();
	InitGlui();

	// Registering all the interactive callback functions
	glutDisplayFunc(display);
	GLUI_Master.set_glutIdleFunc(doIdle);

//	glutIdleFunc(Idle);

	// Read User Mouse Input
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutPassiveMotionFunc(passiveMotion);

	// Read User Keyboard Input
	glutKeyboardFunc(keyboardKeys);	
	glutSpecialFunc(specialKeys);
	
	// Reshape function
	glutReshapeFunc(reshape);

	RenderWorld();

	clothes = (instance*)malloc(sizeof(instance));
	
	clothes->next = NULL;

	AddCloth();

	glutMainLoop();

	return(0);
}

