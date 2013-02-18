#include "vector.h"
#include "render.h"

// Render Mode
#define TRIANGLE 1
#define THREAD 2

// Pinned
#define PINNED 0
#define UNPINLEFT 1
#define UNPINRIGHT 2

// Move Pins Directions
#define FORWARD 1
#define BACKWARD 2
#define RIGHT 3
#define LEFT 4

// Move Pins
#define MOVELEFTPIN 0
#define MOVERIGHTPIN 1
#define MOVEBOTHPINS 2
#define MOVEBOTTOMLEFTPIN 3
#define MOVEBOTTOMRIGHTPIN 4

// Cloth Scaling
#define INCREASE 0
#define DECREASE 1

extern int gRenderMode;
extern int gPin, gNstep, gMovePin, gScale, gWhichCloth, gWind;
extern float gGravity, gDelta, gTstep;
extern float gKThread, gDThread, gKWall, gDWall, gWindAmountX, gWindAmountZ;

extern point underWaterDamp, gravity;
extern int RANDXLIMIT, RANDYLIMIT, RANDZLIMIT;

typedef struct _cloth
{
  float tStep;						// Simulation timestep
  int nStep;						// Render the cloth every nSteps 
  float kThread;					// Hook's elasticity coefficient for the cloth thread
  float dThread;					// Damping coefficient for the cloth thread
  float kWall;						// Hook's elasticity coefficient for collision springs
  float dWall;						// Damping coefficient collision springs
  double mass;						// mass of each of the cloth control points
  double strSprLen;					// Length of the structural spring in the cloth
  double shrSprLen;					// Length of the shear spring along the side in the cloth
  double bendSprLen;				// Length of the bend spring in the cloth
  int width;						// Width of the cloth
  int height;						// Height of the cloth
  int cArrayLength;					// count of the number of points in the cloth
  point *positions;					// position of all the cloth points
  point *velocities;				// velocities of all the cloth points
  point *acceleration;				// acceleration of all the vertices of the cloth
  point *force;						// Holds the force of all the points of the cloth at every frame
  point *normals;					// Normals for the gourand shading
  int *normalsCount;				// counter for the normals for every point
}cloth;

struct instance
{
	cloth *napkin;
	int index;
	point position;
	GLuint texId;
	int clothType;
	struct instance *next;
};
extern struct instance *clothes;

int FindIndexInArray(int row, int col, int clothWidth);

void CreateCloth(cloth *clothItem, double strSprLen, point birthPosition, int clothWidth, int clothHeight, GLuint texId);

void ClothInit(cloth *clothItem, double strSprLen, point birthPosition, int clothWidth, int clothHeight);

void RenderPoints(cloth *clothItem);

void RenderSpring(cloth *clothItem, node cur, node step);

void RenderClothSystem(cloth *clothItem, GLuint texId);

void CopyToBuffer(cloth *buffer, cloth*clothItem);

void RenderBox(GLuint front_face_id, GLuint back_face_id, GLuint right_face_id, GLuint left_face_id, GLuint top_face_id, GLuint bottom_face_id, GLuint ropeId);

void ScaleCloth(cloth *clothItem);