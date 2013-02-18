#include "cloth.h"
#include "physics.h"
#include "render.h"
#include <cstring>

#define STRUCTURAL 1
#define SHEARSIDE 2
#define BEND 3
#define COLLISION 4
#define KELASTIC 5
#define KCOLLISION 6
#define DELASTIC 7
#define DCOLLISION 8
#define XMIN -2
#define XMAX 2
#define YMIN -2
#define YMAX 2
#define ZMIN -2
#define ZMAX 2
#define WALL 1
#define SPHERE 2

double forceFieldCubeSize = 0;	// length of each face of the cube in the force field
int noOfCubesInRow = 0;			// count of the number of FF cubes along one side of the bounding box

int windCount = 0, flag = 0;

// Hook's Law in 3D
//				F = -k hook ( |L| - R) L / |L|	
// springType	1 = Structural
//				2 = Shear side
//				3 = Shear diagonal
//				4 = Bend
//				5 = Collision
point computeHooksForce(int indexA, int indexB, point collisionP, cloth *clothItem, int springType, int coEffType)
{
	point L, unitV;
	double mag = 0;
	double restLength = 0, length = 0.0;
	point hooksForce;
	
	memset( (void*)&L, 0, sizeof(L));
	memset( (void*)&unitV, 0, sizeof(unitV));
	memset( (void*)&hooksForce, 0, sizeof(hooksForce));

	if(coEffType == KELASTIC)
	{
		L.x = clothItem->positions[indexA].x - clothItem->positions[indexB].x;
		L.y = clothItem->positions[indexA].y - clothItem->positions[indexB].y;
		L.z = clothItem->positions[indexA].z - clothItem->positions[indexB].z;

		mag = sqrt((L.x * L.x) + (L.y * L.y) + (L.z * L.z));

		unitV.x = L.x / mag;
		unitV.y = L.y / mag;
		unitV.z = L.z / mag;
		
		// Find the rest length for the current type of spring
		switch(springType)
		{
			case 1:	//Structural spring
				restLength = clothItem->strSprLen;
				break;
			case 2: //Shear side spring
				restLength = clothItem->shrSprLen;
				break;
			case 3: //Bend spring
				restLength = clothItem->bendSprLen;
				break;
			case 4: //Collision spring
				restLength = 0;
				break;
		}

		hooksForce.x = -(clothItem->kThread) * (mag - restLength) * (unitV.x);
		hooksForce.y = -(clothItem->kThread) * (mag - restLength) * (unitV.y);
		hooksForce.z = -(clothItem->kThread) * (mag - restLength) * (unitV.z);
	}
	else if(coEffType == KCOLLISION)
	{
		L.x = clothItem->positions[indexA].x - collisionP.x;
		L.y = clothItem->positions[indexA].y - collisionP.y;
		L.z = clothItem->positions[indexA].z - collisionP.z;

		mag = sqrt((L.x * L.x) + (L.y * L.y) + (L.z * L.z));

//		unitV.x = clothItem->normals[indexA].x / clothItem->normalsCount[indexA];
//		unitV.y = clothItem->normals[indexA].y / clothItem->normalsCount[indexA];
//		unitV.z = clothItem->normals[indexA].z / clothItem->normalsCount[indexA];

		pNORMALIZE(unitV);
		unitV.x = L.x / mag;
		unitV.y = L.y / mag;
		unitV.z = L.z / mag;

		hooksForce.x = -(clothItem->kWall) * (mag) * (unitV.x);
		hooksForce.y = -(clothItem->kWall) * (mag) * (unitV.y);
		hooksForce.z = -(clothItem->kWall) * (mag) * (unitV.z);
	}
	return hooksForce;
}


// Damping in 3D
//                F = -kd * ( va - vb ) . L / | L | * L / | L |
point computeDampingForce(int indexA, int indexB, point collisionP, cloth *clothItem, int coEffType)
{
	
	point L, unitV, vDiff;
	double mag = 0;
	double dotProd = 0;
	point dampingForce;
	point v1, v2;	// to store the velocities of both the points connected to the spring
	
	memset( (void*)&L, 0, sizeof(L));
	memset( (void*)&unitV, 0, sizeof(unitV));
	memset( (void*)&vDiff, 0, sizeof(vDiff));
	memset( (void*)&dampingForce, 0, sizeof(dampingForce));
	memset( (void*)&v1, 0, sizeof(v1));
	memset( (void*)&v2, 0, sizeof(v2));

	if(coEffType == DELASTIC)
	{
		L.x = clothItem->positions[indexA].x - clothItem->positions[indexB].x;
		L.y = clothItem->positions[indexA].y - clothItem->positions[indexB].y;
		L.z = clothItem->positions[indexA].z - clothItem->positions[indexB].z;
		
		// Magnitude
		mag = sqrt((L.x * L.x) + (L.y * L.y) + (L.z * L.z));
		
		// Normalise the vector
		unitV.x = L.x / mag;
		unitV.y = L.y / mag;
		unitV.z = L.z / mag;

		// Difference in velocities
		v1.x = clothItem->velocities[indexA].x;
		v1.y = clothItem->velocities[indexA].y;
		v1.z = clothItem->velocities[indexA].z;

		v2.x = clothItem->velocities[indexB].x;
		v2.y = clothItem->velocities[indexB].y;
		v2.z = clothItem->velocities[indexB].z;

		vDiff.x = v1.x - v2.x;
		vDiff.y = v1.y - v2.y;
		vDiff.z = v1.z - v2.z;
		
		// vDiff Dot L
		dotProd = (vDiff.x * L.x) + (vDiff.y * L.y) + (vDiff.z * L.z);
		
		dampingForce.x = (-clothItem->dThread) * ( dotProd / mag ) * (unitV.x);
		dampingForce.y = (-clothItem->dThread) * ( dotProd / mag ) * (unitV.y);
		dampingForce.z = (-clothItem->dThread) * ( dotProd / mag ) * (unitV.z);
	}
	else if(coEffType == DCOLLISION)
	{
		L.x = clothItem->positions[indexA].x - collisionP.x;
		L.y = clothItem->positions[indexA].y - collisionP.y;
		L.z = clothItem->positions[indexA].z - collisionP.z;
		
		// Magnitude
		mag = sqrt((L.x * L.x) + (L.y * L.y) + (L.z * L.z));
		
		// Normalise the vector
		unitV.x = L.x / mag;
		unitV.y = L.y / mag;
		unitV.z = L.z / mag;

		// Difference in velocities
		v1.x = clothItem->velocities[indexA].x;
		v1.y = clothItem->velocities[indexA].y;
		v1.z = clothItem->velocities[indexA].z;

		// vDiff Dot L
		dotProd = (v1.x * L.x) + (v1.y * L.y) + (v1.z * L.z);

		dampingForce.x = (-clothItem->dWall) * ( dotProd / mag ) * (unitV.x);
		dampingForce.y = (-clothItem->dWall) * ( dotProd / mag ) * (unitV.y);
		dampingForce.z = (-clothItem->dWall) * ( dotProd / mag ) * (unitV.z);
	}
	
	return dampingForce;
}	


int checkIfInsideCloth(node A, cloth *clothItem)
{
	if(A.x < 0 || (A.x > clothItem->width - 1) || A.y < 0 || (A.y > clothItem->height - 1))
		return 0;
	else
		return 1;
}


int checkIfAlreadyParsed(node typeP, node curP)
{
	if((typeP.x < curP.x) || (typeP.y < curP.y))
		return 0;
	else 
		return 1;
}


void updateForce(node curP, node typeP, cloth *clothItem, int springType)
{
	int inOrOut = 0;			// 1 = inside , 0 = outside
	int parsedOrNot = 0;		// 1 = unparsed, 0 = already parsed
	point hooksF, dampF;		// to store the force computed in the above defined functions
	int indexC, indexT;

	memset( (void*)&hooksF, 0, sizeof(hooksF));
	memset( (void*)&dampF, 0, sizeof(dampF));

	inOrOut = checkIfInsideCloth(typeP, clothItem);

	if(inOrOut)		
	{
		// type# point is inside the jello cube
		parsedOrNot = checkIfAlreadyParsed(typeP, curP);

		if(parsedOrNot)
		{
			indexC = FindIndexInArray(curP.y, curP.x, clothItem->width);
			indexT = FindIndexInArray(typeP.y, typeP.x, clothItem->width);
			// type# point has not been parsed yet

			hooksF = computeHooksForce(indexC, indexT, vMake(0.0), clothItem, springType, KELASTIC);
			dampF = computeDampingForce(indexC, indexT, vMake(0.0), clothItem, DELASTIC);

			// Add the forces to the current point in the loop
			clothItem->force[indexC].x += hooksF.x + dampF.x;
			clothItem->force[indexC].y += hooksF.y + dampF.y;
			clothItem->force[indexC].z += hooksF.z + dampF.z;
			
			// Add the forces to the current type# point
			clothItem->force[indexT].x += -hooksF.x + (-dampF.x);
			clothItem->force[indexT].y += -hooksF.y + (-dampF.y);
			clothItem->force[indexT].z += -hooksF.z + (-dampF.z);

			if(gWind == 1)
			{
				if(windCount < 30 && flag != 1)
				{
					clothItem->force[indexC].x += gWindAmountX;
					clothItem->force[indexC].y += 0.0;
					clothItem->force[indexC].z += gWindAmountZ;
					windCount++;
					if(windCount == 30)
					{
						flag = 1;
					}
				}
				else
				{
					windCount -= 1;
					if(windCount == 0)
					{
						flag = 0;
					}
				}
			}
		}
	}
}


void callWhenCollide(node curP, point collisionP, cloth *clothItem, int springType, int prop)
{
	point hooksF, dampF;		
	int index;
	
	memset( (void*)&hooksF, 0, sizeof(hooksF));
	memset( (void*)&dampF, 0, sizeof(dampF));

	index = FindIndexInArray(curP.y, curP.x, clothItem->width);
	hooksF = computeHooksForce(index, 0,  collisionP, clothItem, springType, KCOLLISION);
	dampF = computeDampingForce(index, 0, collisionP, clothItem, DCOLLISION);
	
	if(prop == WALL)
	{
		// Add the forces to the current point in the loop
		clothItem->force[index].x += (hooksF.x) + dampF.x;
		clothItem->force[index].y += (hooksF.y) + dampF.y;
		clothItem->force[index].z += (hooksF.z) + dampF.z;
	}
	else if(prop == SPHERE)
	{
		// Add the forces to the current point in the loop
		clothItem->force[index].x += (hooksF.x) + dampF.x;
		clothItem->force[index].y += (hooksF.y) + dampF.y;
		clothItem->force[index].z += (hooksF.z) + dampF.z;
	}
}


void checkForObjectCollision(node curP, cloth *clothItem)
{
	double dist = 0, slope = 0, c = 0, ratio = 0;
	point pos;
	point collisionP;
	int index = 0;

	memset( (void*)&pos, 0, sizeof(pos));

	index = FindIndexInArray(curP.y, curP.x, clothItem->width);
	pos.x = clothItem->positions[index].x;
	pos.y = clothItem->positions[index].y;
	pos.z = clothItem->positions[index].z;

	// Compute the distance between the curP and the center of the sphere
	dist = sqrt( ((SPHEREx - pos.x)*(SPHEREx - pos.x)) + ((SPHEREy - pos.y)*(SPHEREy - pos.y)) + ((SPHEREz - pos.z)*(SPHEREz - pos.z)) );

	if(dist <= SPHEREr)
	{
		// cloth colliding with the sphere object present in the bounding box
		
		ratio = SPHEREr / dist;

		collisionP.x = ratio * (SPHEREx - pos.x);
		collisionP.y = ratio * (SPHEREy - pos.y);
		collisionP.z = ratio * (SPHEREz - pos.z); 

		callWhenCollide(curP, collisionP, clothItem, COLLISION, SPHERE);
		memset( (void*)&collisionP, 0, sizeof(collisionP));
	}
}


void checkForCollision(node curP, cloth *clothItem)
{
	point collisionP;
	int index = 0;

	memset( (void*)&collisionP, 0, sizeof(collisionP));
    
	index = FindIndexInArray(curP.y, curP.x, clothItem->width);

	if(sphereExists)
	{
		// Check if the jello collides with the object placed in the scene
		checkForObjectCollision(curP, clothItem);
	}

/*	if(clothItem->positions[index].x > 2)
	{	// collision with right face
		// RIGHT -> (2, y, z) structural spring
		collisionP.x = 2;
		collisionP.y = clothItem->positions[index].y;
		collisionP.z = clothItem->positions[index].z;
		
		callWhenCollide(curP, collisionP, clothItem, COLLISION, WALL);
		memset( (void*)&collisionP, 0, sizeof(collisionP));
	}

	if(clothItem->positions[index].x < -2)
	{	// collision with left face
		// LEFT -> (-2, y, z) structural spring
		collisionP.x = -2;
		collisionP.y = clothItem->positions[index].y;
		collisionP.z = clothItem->positions[index].z;
		
		callWhenCollide(curP, collisionP, clothItem, COLLISION, WALL);
		memset( (void*)&collisionP, 0, sizeof(collisionP));
	}

	if(clothItem->positions[index].y > 2)
	{	// collision with top face
		// TOP -> (x, 2, z) structural spring
		collisionP.x = clothItem->positions[index].x;
		collisionP.y = 2;
		collisionP.z = clothItem->positions[index].z;
		
		callWhenCollide(curP, collisionP, clothItem, COLLISION, WALL);
		memset( (void*)&collisionP, 0, sizeof(collisionP));
	}
*/
	if(clothItem->positions[index].y < -2)
	{	// collision with bottom face
		// BOTTOM -> (x, -2, z) structural spring
		collisionP.x = clothItem->positions[index].x;
		collisionP.y = -2;
		collisionP.z = clothItem->positions[index].z;
		
		callWhenCollide(curP, collisionP, clothItem, COLLISION, WALL);
		memset( (void*)&collisionP, 0, sizeof(collisionP));
	}
/*
	if(clothItem->positions[index].z > 2)
	{	// collision with back face
		// BACK -> (x, y, 2) structural spring
		collisionP.x = clothItem->positions[index].x;
		collisionP.y = clothItem->positions[index].y;
		collisionP.z = 2;
		
		callWhenCollide(curP, collisionP, clothItem, COLLISION, WALL);
		memset( (void*)&collisionP, 0, sizeof(collisionP));
	}

	if(clothItem->positions[index].z < -2)
	{	// collision with front face
		// FRONT -> (x, y, -2) structural spring
		collisionP.x = clothItem->positions[index].x;
		collisionP.y = clothItem->positions[index].y;
		collisionP.z = -2;
		
		callWhenCollide(curP, collisionP, clothItem, COLLISION, WALL);
		memset( (void*)&collisionP, 0, sizeof(collisionP));
	}
*/
}

/*
void Interpolate(point curP, node curN, struct world *jello)
{
	// Interpolate all the 8 FFCube nodes to the position of the curJ node inside the FFCube Node
	point eightForces[8];
	int i = 0;
	double alpha = 0, beta = 0, gama = 0;
	double oneMAlpha = 0, oneMBeta = 0, oneMGama = 0;

	memset( (void*)&eightForces, 0, sizeof(eightForces));
	// can fetch the force from the force field array
	// compute alpha beta gama gDelta and their 1-alpha beta gama gDelta avlues
	// multiply and add them according to formula
	// add the final force to the J point force structure.
	
	for(i = 0; i < 8; i++)
	{
		eightForces[i].x = jello->forceField[FFCube[i].x * jello->resolution * jello->resolution 
			+ FFCube[i].y * jello->resolution + FFCube[i].z].x = 2; 
        eightForces[i].y = jello->forceField[i * jello->resolution * jello->resolution 
			+ FFCube[i].y * jello->resolution + FFCube[i].z].y = 1;
        eightForces[i].z = jello->forceField[i * jello->resolution * jello->resolution 
          + FFCube[i].y * jello->resolution + FFCube[i].z].z = 4;
	}

	// prefetch alpha, beta, gama and their compliments
	alpha = curP.x - FFCubePos[0].x;	//distance of the jello node from FFCube along x axis 
	beta = curP.y - FFCubePos[0].y;		//distance of the jello node from FFCube along y axis
	gama = curP.z - FFCubePos[0].z;		//distance of the jello node from FFCube along z axis

	oneMAlpha = forceFieldCubeSize - alpha;		// compliment of alpha
	oneMBeta = forceFieldCubeSize - beta;		// compliment of beta
	oneMGama = forceFieldCubeSize - gama;		// compliment of gama

	// F1
	eightForces[0].x *= oneMAlpha * oneMBeta * oneMGama;
	eightForces[0].y *= oneMAlpha * oneMBeta * oneMGama;
	eightForces[0].z *= oneMAlpha * oneMBeta * oneMGama;

	// F2
	eightForces[1].x *= alpha * oneMBeta * oneMGama;
	eightForces[1].y *= alpha * oneMBeta * oneMGama;
	eightForces[1].z *= alpha * oneMBeta * oneMGama;

	// F3
	eightForces[2].x *= oneMAlpha * oneMBeta * gama;
	eightForces[2].y *= oneMAlpha * oneMBeta * gama;
	eightForces[2].z *= oneMAlpha * oneMBeta * gama;

	// F4
	eightForces[3].x *= alpha * oneMBeta * gama;
	eightForces[3].y *= alpha * oneMBeta * gama;
	eightForces[3].z *= alpha * oneMBeta * gama;

	// F5
	eightForces[4].x *= oneMAlpha * beta * oneMGama;
	eightForces[4].y *= oneMAlpha * beta * oneMGama;
	eightForces[4].z *= oneMAlpha * beta * oneMGama;

	// F6
	eightForces[5].x *= alpha * beta * oneMGama;
	eightForces[5].y *= alpha * beta * oneMGama;
	eightForces[5].z *= alpha * beta * oneMGama;

	// F7
	eightForces[6].x *= oneMAlpha * beta * gama;
	eightForces[6].y *= oneMAlpha * beta * gama;
	eightForces[6].z *= oneMAlpha * beta * gama;

	// F8
	eightForces[7].x *= alpha * beta * gama;
	eightForces[7].y *= alpha * beta * gama;
	eightForces[7].z *= alpha * beta * gama;
	
	for(i = 0; i < 8; i++)
	{
		force[curN.x][curN.y][curN.z].x += eightForces[i].x;
		force[curN.x][curN.y][curN.z].y += eightForces[i].y;
		force[curN.x][curN.y][curN.z].z += eightForces[i].z;
	}
}
*/


// Validates the Node passed as an argument
int checkNode(node N)
{
	if(N.x > noOfCubesInRow || N.x < 0 || N.y > noOfCubesInRow || N.y < 0)
		return 1;
	else 
		return 0;
}

// Validates the point passed as an argument
int checkPoint(point P)
{

	if(P.x > 2 || P.x < -2 || P.y > 2 || P.y < -2)
		return 1;
	else
		return 0;
}

/*
void computeForceField(struct world *jello)
{

	// loop for all the nodes in the JCube, for every node check for the surrounding FFCube 
	// find the min and max of all the coords , find th jello node offset inside FFCube
	// find the FFCube 8 nodes positions ans node index values, validate them. Invoke interpolation
	// to find the value at the Jello node.
	
	int x = 0, y = 0, z = 0, X = 0, Y = 0, Z = 0;
	int xPos = 0, yPos = 0, zPos = 0, i = 0, flag = 0;
	point curP;
	node curN;

	for(x = 0; x < 8; x++)
		for(y = 0; y < 8; y++)
			for(z = 0; z < 8; z++)
			{
				for(i = 0; i < 8 ; i++)
				{
					memset( (void*)&FFCube[i], 0, sizeof(FFCube[i]));
					memset( (void*)&FFCubePos[i], 0, sizeof(FFCubePos[i]));
				}
				memset( (void*)&curP, 0, sizeof(curP));
				memset( (void*)&curN, 0, sizeof(curN));

				// position of the current jello node in the bounding box
				curP.x = jello->p[x][y][z].x;
				curP.y = jello->p[x][y][z].y;
				curP.z = jello->p[x][y][z].z;
				if(checkPoint(curP))
					continue;

				curN.x = x;
				curN.y = y;
				curN.z = z;
				if(checkNode(curN))
					continue;

				// find the jello node offset inside the surrounding FFCube
				xPos = fmod(jello->p[x][y][z].x, forceFieldCubeSize);
				yPos = fmod(jello->p[x][y][z].y, forceFieldCubeSize);
				zPos = fmod(jello->p[x][y][z].z, forceFieldCubeSize);

				X = jello->p[x][y][z].x - xPos;
				Y = jello->p[x][y][z].y - yPos;
				Z = jello->p[x][y][z].z - zPos;
				
				// Store the positions of all the 8 corners of the FF cube
				// left bottom front
				FFCubePos[0].x = X;
				FFCubePos[0].y = Y;
				FFCubePos[0].z = Z;
				if(checkPoint(FFCubePos[0]))
					continue;

				// right bottom front
				FFCubePos[1].x = X + forceFieldCubeSize;
				FFCubePos[1].y = Y;
				FFCubePos[1].z = Z;
				if(checkPoint(FFCubePos[1]))
					continue;

				// left bottom back
				FFCubePos[2].x = X;
				FFCubePos[2].y = Y;
				FFCubePos[2].z = Z + forceFieldCubeSize;
				if(checkPoint(FFCubePos[2]))
					continue;

				// right bottom back
				FFCubePos[3].x = X + forceFieldCubeSize;
				FFCubePos[3].y = Y;
				FFCubePos[3].z = Z + forceFieldCubeSize;
				if(checkPoint(FFCubePos[3]))
					continue;

				// left top front
				FFCubePos[4].x = X;
				FFCubePos[4].y = Y + forceFieldCubeSize;
				FFCubePos[4].z = Z;
				if(checkPoint(FFCubePos[4]))
					continue;

				// right top front
				FFCubePos[5].x = X + forceFieldCubeSize;
				FFCubePos[5].y = Y + forceFieldCubeSize;
				FFCubePos[5].z = Z;
				if(checkPoint(FFCubePos[5]))
					continue;

				// left top back
				FFCubePos[6].x = X;
				FFCubePos[6].y = Y + forceFieldCubeSize;
				FFCubePos[6].z = Z + forceFieldCubeSize;
				if(checkPoint(FFCubePos[6]))
					continue;

				// right top back
				FFCubePos[7].x = X + forceFieldCubeSize;
				FFCubePos[7].y = Y + forceFieldCubeSize;
				FFCubePos[7].z = Z + forceFieldCubeSize;
				if(checkPoint(FFCubePos[7]))
					continue;
				
				for(i = 0; i < 8; i++)
				{
					FFCube[i].x = FFCubePos[i].x / forceFieldCubeSize;
					FFCube[i].y = FFCubePos[i].y / forceFieldCubeSize;
					FFCube[i].z = FFCubePos[i].z / forceFieldCubeSize;
					if(checkNode(FFCube[i]))
					{
						flag = 1; 
						break;
					}
				}
				if(flag)
					continue;

				Interpolate(curP, curN, jello);
			}
}
*/


// Computes acceleration to every point of the cloth		
void computeAcceleration(cloth *clothItem)
{  
	node typeP, curP;	// typeP = one of the 12 nodes to which the curP point is connected
	int collided = 0, index;

	memset( (void*)&typeP, 0, sizeof(typeP));
	memset( (void*)&curP, 0, sizeof(curP));
	

	/*
	// FORCE FIELD COMPUTATION
	if(ActivateFF)
	{
		// Check for force field
		if(jello->resolution)
		{
			// compute the force field cube size and store it in the global variable
			forceFieldCubeSize = ((double)4)/jello->resolution;
			noOfCubesInRow = (int)(((double)jello->resolution) / forceFieldCubeSize);

			// Compute the effect of force field and update the forces due to it on all the nodes
			computeForceField(jello);
		}
	}
	*/

	// COLLISION DETECTION AND RESPONSE
	for(int row = 0; row < clothItem->height; row++)
	{
		for(int col = 0; col < clothItem->width; col++)
		{
			memset( (void*)&curP, 0, sizeof(curP));

			index = FindIndexInArray(curP.y, curP.x, clothItem->width);
			pCPY(vMake(0.0), clothItem->force[index]);

			curP.x = col;
			curP.y = row;
			
			// Checks whether any of the nodes is colliding with the bounding box and Sphere
			checkForCollision(curP, clothItem);
		}
	}

	for(int row = 0; row < clothItem->height; row++)
	{
		for(int col = 0; col < clothItem->width; col++)
		{
			curP.x = col;
			curP.y = row;

			// Check for all the springs possible = 12
			
			// STRUCTURAL SPRINGS
			//-----------------------------------------------------------------
			// TYPE 1 -> (x+1, y, z) structural spring
			typeP.x = col + 1;
			typeP.y = row;
			
			updateForce(curP, typeP, clothItem, STRUCTURAL);
			memset( (void*)&typeP, 0, sizeof(typeP));
			//-----------------------------------------------------------------
			// TYPE 2 -> (x-1, y, z)
			typeP.x = col - 1;
			typeP.y = row;
			
			updateForce(curP, typeP, clothItem, STRUCTURAL);
			memset( (void*)&typeP, 0, sizeof(typeP));
			//-----------------------------------------------------------------
			// TYPE 3 -> (x, y+1, z)
			typeP.x = col;
			typeP.y = row + 1;
			
			updateForce(curP, typeP, clothItem, STRUCTURAL);
			memset( (void*)&typeP, 0, sizeof(typeP));
			//-----------------------------------------------------------------
			// TYPE 4 -> (x, y-1, z)
			typeP.x = col;
			typeP.y = row - 1;
			
			updateForce(curP, typeP, clothItem, STRUCTURAL);
			memset( (void*)&typeP, 0, sizeof(typeP));
			//-----------------------------------------------------------------

			// SHEAR SPRINGS
			//-----------------------------------------------------------------
			// TYPE 5 -> (x+1, y+1, z)
			typeP.x = col + 1;
			typeP.y = row + 1;
			
			updateForce(curP, typeP, clothItem, SHEARSIDE);
			memset( (void*)&typeP, 0, sizeof(typeP));
			//-----------------------------------------------------------------
			// TYPE 6 -> (x-1, y+1, z)
			typeP.x = col - 1;
			typeP.y = row + 1;
			
			updateForce(curP, typeP, clothItem, SHEARSIDE);
			memset( (void*)&typeP, 0, sizeof(typeP));
			//-----------------------------------------------------------------
			// TYPE 7 -> (x+1, y-1, z)
			typeP.x = col + 1;
			typeP.y = row - 1;
			
			updateForce(curP, typeP, clothItem, SHEARSIDE);
			memset( (void*)&typeP, 0, sizeof(typeP));
			//-----------------------------------------------------------------
			// TYPE 8 -> (x-1, y-1, z)
			typeP.x = col - 1;
			typeP.y = row - 1;
			
			updateForce(curP, typeP, clothItem, SHEARSIDE);
			memset( (void*)&typeP, 0, sizeof(typeP));
			//-----------------------------------------------------------------
			

			// BEND SPRINGS
			//-----------------------------------------------------------------
			// TYPE 9 -> (x+2, y, z)
			typeP.x = col + 2;
			typeP.y = row;
			
			updateForce(curP, typeP, clothItem, BEND);
			memset( (void*)&typeP, 0, sizeof(typeP));
			//-----------------------------------------------------------------
			// TYPE 10 -> (x-2, y, z)
			typeP.x = col - 2;
			typeP.y = row;
			
			updateForce(curP, typeP, clothItem, BEND);
			memset( (void*)&typeP, 0, sizeof(typeP));
			//-----------------------------------------------------------------
			// TYPE 11 -> (x, y+2, z)
			typeP.x = col;
			typeP.y = row + 2;
			
			updateForce(curP, typeP, clothItem, BEND);
			memset( (void*)&typeP, 0, sizeof(typeP));
			//-----------------------------------------------------------------
			// TYPE 12 -> (x, y-2, z)
			typeP.x = col;
			typeP.y = row - 2;
			
			updateForce(curP, typeP, clothItem, BEND);
			memset( (void*)&typeP, 0, sizeof(typeP));
			//-----------------------------------------------------------------
		}// for col
	}// for row
	
	for(int row = 0; row < clothItem->height; row++)
	{
		for(int col = 0; col < clothItem->width; col++)
		{
			index = FindIndexInArray(row, col, clothItem->width);

			clothItem->acceleration[index].x = clothItem->force[index].x / clothItem->mass;
			clothItem->acceleration[index].y = clothItem->force[index].y / clothItem->mass;
			clothItem->acceleration[index].z = clothItem->force[index].z / clothItem->mass;
		}
	}
}



/* Performs one step of Euler Integration 
   as a result, updates the jello structure */
void Euler(cloth *clothItem)
{
  int index;

  computeAcceleration(clothItem);
  
	for(int row = 0; row < clothItem->height; row++)
	{
		for(int col = 0; col < clothItem->width; col++)
		{
			index = FindIndexInArray(row, col, clothItem->width);

			if(index == 0 || index == clothItem->width-1)
				continue;

			clothItem->positions[index].x += clothItem->tStep * clothItem->velocities[index].x;
			clothItem->positions[index].y += clothItem->tStep * clothItem->velocities[index].y;
			clothItem->positions[index].z += clothItem->tStep * clothItem->velocities[index].z;
			
			clothItem->velocities[index].x += clothItem->tStep * clothItem->acceleration[index].x;
			clothItem->velocities[index].y += clothItem->tStep * clothItem->acceleration[index].y;
			clothItem->velocities[index].z += clothItem->tStep * clothItem->acceleration[index].z;
		}
	}
}


void MoveClothXZ(cloth *clothItem, int direction)
{
	for(int  i= 0; i < clothItem->cArrayLength; i++)
	{ 
		if(gMovePin == MOVELEFTPIN)
		{
			if(i == 0)
			{
				if(direction == FORWARD)
				{
					pSUM(clothItem->positions[i], vMake(0.0, 0.0, -0.01), clothItem->positions[i]);
				}
				else if(direction == BACKWARD)
				{
					pSUM(clothItem->positions[i], vMake(0.0, 0.0, 0.01), clothItem->positions[i]);
				}
				else if(direction == RIGHT)
				{
					pSUM(clothItem->positions[i], vMake(0.01, 0.0, 0.0), clothItem->positions[i]);
				}
				else if(direction == LEFT)
				{
					pSUM(clothItem->positions[i], vMake(-0.01, 0.0, 0.0), clothItem->positions[i]);
				}
			}
		}

		if(gMovePin == MOVERIGHTPIN)
		{
			if(i == clothItem->width -1)
			{
				if(direction == FORWARD)
				{
					pSUM(clothItem->positions[i], vMake(0.0, 0.0, -0.01), clothItem->positions[i]);
				}
				else if(direction == BACKWARD)
				{
					pSUM(clothItem->positions[i], vMake(0.0, 0.0, 0.01), clothItem->positions[i]);
				}
				else if(direction == RIGHT)
				{
					pSUM(clothItem->positions[i], vMake(0.01, 0.0, 0.0), clothItem->positions[i]);
				}
				else if(direction == LEFT)
				{
					pSUM(clothItem->positions[i], vMake(-0.01, 0.0, 0.0), clothItem->positions[i]);
				}
			}
		}

		if(gMovePin == MOVEBOTTOMLEFTPIN)
		{
			if(i == (clothItem->width * (clothItem->height-1)))
			{
				if(direction == FORWARD)
				{
					pSUM(clothItem->positions[i], vMake(0.0, 0.0, -0.1), clothItem->positions[i]);
				}
				else if(direction == BACKWARD)
				{
					pSUM(clothItem->positions[i], vMake(0.0, 0.0, 0.1), clothItem->positions[i]);
				}
				else if(direction == RIGHT)
				{
					pSUM(clothItem->positions[i], vMake(0.1, 0.0, 0.0), clothItem->positions[i]);
				}
				else if(direction == LEFT)
				{
					pSUM(clothItem->positions[i], vMake(-0.1, 0.0, 0.0), clothItem->positions[i]);
				}
			}
		}

		if(gMovePin == MOVEBOTTOMRIGHTPIN)
		{
			if(i == clothItem->cArrayLength - 1)
			{
				if(direction == FORWARD)
				{
					pSUM(clothItem->positions[i], vMake(0.0, 0.0, -0.1), clothItem->positions[i]);
				}
				else if(direction == BACKWARD)
				{
					pSUM(clothItem->positions[i], vMake(0.0, 0.0, 0.1), clothItem->positions[i]);
				}
				else if(direction == RIGHT)
				{
					pSUM(clothItem->positions[i], vMake(0.1, 0.0, 0.0), clothItem->positions[i]);
				}
				else if(direction == LEFT)
				{
					pSUM(clothItem->positions[i], vMake(-0.1, 0.0, 0.0), clothItem->positions[i]);
				}
			}
		}

		if(gMovePin == MOVEBOTHPINS)
		{
			if(i == 0 || i == clothItem->width -1)
			{
				if(direction == FORWARD)
				{
					pSUM(clothItem->positions[i], vMake(0.0, 0.0, -0.01), clothItem->positions[i]);
				}
				else if(direction == BACKWARD)
				{
					pSUM(clothItem->positions[i], vMake(0.0, 0.0, 0.01), clothItem->positions[i]);
				}
				else if(direction == RIGHT)
				{
					pSUM(clothItem->positions[i], vMake(0.01, 0.0, 0.0), clothItem->positions[i]);
				}
				else if(direction == LEFT)
				{
					pSUM(clothItem->positions[i], vMake(-0.01, 0.0, 0.0), clothItem->positions[i]);
				}
			}
		}
	}
}


/* performs one step of RK4 Integration */
/* as a result, updates the jello structure */
void RK4(cloth *clothItem)
{
	int index;
	point *F1p, *F1v,
		*F2p, *F2v,
		*F3p, *F3v,
		*F4p, *F4v;

	F1p = (point*)calloc(clothItem->cArrayLength, sizeof(point));
	F1v = (point*)calloc(clothItem->cArrayLength, sizeof(point));
	F2p = (point*)calloc(clothItem->cArrayLength, sizeof(point));
	F2v = (point*)calloc(clothItem->cArrayLength, sizeof(point));
	F3p = (point*)calloc(clothItem->cArrayLength, sizeof(point));
	F3v = (point*)calloc(clothItem->cArrayLength, sizeof(point));
	F4p = (point*)calloc(clothItem->cArrayLength, sizeof(point));
	F4v = (point*)calloc(clothItem->cArrayLength, sizeof(point));

	cloth *buffer = (cloth*)malloc(1 * sizeof(cloth));

	CopyToBuffer(buffer, clothItem);

	computeAcceleration(clothItem);

	for(int row = 0; row < clothItem->height; row++)
	{
		for(int col = 0; col < clothItem->width; col++)
		{
			index = FindIndexInArray(row, col, clothItem->width);
			
			// Pin Check
			if(gPin == PINNED)
			{
				if(index == 0 || index == clothItem->width-1)
				continue;
			}
			else if(gPin == UNPINLEFT)
			{
				if(index == clothItem->width-1)
				continue;
			}
			else if(gPin == UNPINRIGHT)
			{
				if(index == 0)
				continue;
			}

			pMULTIPLY(clothItem->velocities[index], clothItem->tStep, F1p[index]);
			pMULTIPLY(clothItem->acceleration[index], clothItem->tStep, F1v[index]);
			pMULTIPLY(F1p[index], 0.5, buffer->positions[index]);
			pMULTIPLY(F1v[index], 0.5, buffer->velocities[index]);
			pSUM(clothItem->positions[index], buffer->positions[index], buffer->positions[index]);
			pSUM(clothItem->velocities[index], buffer->velocities[index], buffer->velocities[index]);
		}
	}

	for(int  i= 0; i < clothItem->cArrayLength; i++)
	{
		pCPY(clothItem->acceleration[i], buffer->acceleration[i]); 
	}

	computeAcceleration(buffer);

	for(int row = 0; row < clothItem->height; row++)
	{
		for(int col = 0; col < clothItem->width; col++)
		{
			index = FindIndexInArray(row, col, clothItem->width);

			// Pin Check
			if(gPin == PINNED)
			{
				if(index == 0 || index == clothItem->width-1)
				continue;
			}
			else if(gPin == UNPINLEFT)
			{
				if(index == clothItem->width-1)
				continue;
			}
			else if(gPin == UNPINRIGHT)
			{
				if(index == 0)
				continue;
			}

			pMULTIPLY(buffer->velocities[index], clothItem->tStep, F2p[index]);
			pMULTIPLY(buffer->acceleration[index], clothItem->tStep, F2v[index]);
			pMULTIPLY(F2p[index], 0.5, buffer->positions[index]);
			pMULTIPLY(F2v[index], 0.5, buffer->velocities[index]);
			pSUM(clothItem->positions[index], buffer->positions[index], buffer->positions[index]);
			pSUM(clothItem->velocities[index], buffer->velocities[index], buffer->velocities[index]);
		}
	}

	computeAcceleration(buffer);

	for(int row = 0; row < clothItem->height; row++)
	{
		for(int col = 0; col < clothItem->width; col++)
		{
			index = FindIndexInArray(row, col, clothItem->width);

			// Pin Check
			if(gPin == PINNED)
			{
				if(index == 0 || index == clothItem->width-1)
				continue;
			}
			else if(gPin == UNPINLEFT)
			{
				if(index == clothItem->width-1)
				continue;
			}
			else if(gPin == UNPINRIGHT)
			{
				if(index == 0)
				continue;
			}

			pMULTIPLY(buffer->velocities[index], clothItem->tStep, F3p[index]);
			pMULTIPLY(buffer->acceleration[index], clothItem->tStep, F3v[index]);
			pMULTIPLY(F3p[index], 0.5, buffer->positions[index]);
			pMULTIPLY(F3v[index], 0.5, buffer->velocities[index]);
			pSUM(clothItem->positions[index], buffer->positions[index], buffer->positions[index]);
			pSUM(clothItem->velocities[index], buffer->velocities[index], buffer->velocities[index]);
		}
	}
         
	computeAcceleration(buffer);

	for(int row = 0; row < clothItem->height; row++)
	{
		for(int col = 0; col < clothItem->width; col++)
		{
			index = FindIndexInArray(row, col, clothItem->width);

			// Pin Check
			if(gPin == PINNED)
			{
				if(index == 0 || index == clothItem->width-1)
				continue;
			}
			else if(gPin == UNPINLEFT)
			{
				if(index == clothItem->width-1)
				continue;
			}
			else if(gPin == UNPINRIGHT)
			{
				if(index == 0)
				continue;
			}

			pMULTIPLY(buffer->velocities[index], clothItem->tStep, F4p[index]);
			pMULTIPLY(buffer->acceleration[index], clothItem->tStep, F4v[index]);
			pMULTIPLY(F2p[index], 2, buffer->positions[index]);
			pMULTIPLY(F3p[index], 2, buffer->velocities[index]);
			pSUM(buffer->positions[index], buffer->velocities[index], buffer->positions[index]);
			pSUM(buffer->positions[index], F1p[index], buffer->positions[index]);
			pSUM(buffer->positions[index], F4p[index], buffer->positions[index]); 
			pMULTIPLY(buffer->positions[index], 1.0 / 6, buffer->positions[index]);
			pSUM(buffer->positions[index], clothItem->positions[index], clothItem->positions[index]);
			 
			pMULTIPLY(F2v[index], 2, buffer->positions[index]);
			pMULTIPLY(F3v[index], 2, buffer->velocities[index]);
			pSUM(buffer->positions[index], buffer->velocities[index], buffer->positions[index]);
			pSUM(buffer->positions[index], F1v[index], buffer->positions[index]);
			pSUM(buffer->positions[index], F4v[index], buffer->positions[index]);
			pMULTIPLY(buffer->positions[index], 1.0 / 6, buffer->positions[index]);
			pSUM(buffer->positions[index], clothItem->velocities[index], clothItem->velocities[index]);
		}
	}

	for(int  i= 0; i < clothItem->cArrayLength; i++)
	{
		pCPY(buffer->acceleration[i], clothItem->acceleration[i]); 
	}

	for(int  i= 0; i < clothItem->cArrayLength; i++)
	{
		// Pin Check
		if(gPin == PINNED)
		{
			if(index == 0 || index == clothItem->width-1)
			continue;
		}
		else if(gPin == UNPINLEFT)
		{
			if(index == clothItem->width-1)
			continue;
		}
		else if(gPin == UNPINRIGHT)
		{
			if(index == 0)
			continue;
		}

		pMULTIPLY(clothItem->velocities[i], gDelta, underWaterDamp);
		pSUM(clothItem->velocities[i], underWaterDamp, clothItem->velocities[i]);
		pSUM(clothItem->force[i], gravity, clothItem->force[i]);
	}

	free(buffer);
  return;  
}
