#ifdef WIN32
  #include <windows.h>
#endif

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "openGL-headers.h"
#include "vector.h"
#include "float.h"

#define  PI 3.141592653589793238462643383279 
#define BUFSIZE 512

extern int windowWidth, windowHeight;
extern GLUI *glui;
extern int mainWindowId;

// Camera controls
extern point cameraPos, lineOfSight, viewVector, cameraAng;
extern float cameraRot, cameraDist;
extern double Theta, Phi, R;

// Mouse controls
extern point mousePos, pMousePos, userForce;
extern int leftButton, rightButton, middleButton, objectName;

// Light Settings
extern int lighting;
extern int setShineLevel, setSpecLevel, setEmissLevel;
extern int light1, light2, light3, light4, light5, light6, light7, light8; 
extern int materialColor;

// Sphere position in the bounding box
extern double SPHEREx;
extern double SPHEREy;
extern double SPHEREz;
extern double SPHEREr;
extern double restFactor;

extern bool shineLevel;
extern bool specLevel;
extern bool emissLevel;

// these variables control what is displayed on the screen
extern int shear, bend, structural, pause, viewingMode, sphereExists;

extern int SKYBOXXLIM, SKYBOXYLIM, SKYBOXZLIM;

void clothMain();

void renderAxis();

void camInit();

void AddCloth();

void AttachCloth();

void DeleteClothes();

void reset();
