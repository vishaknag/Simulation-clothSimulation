#include "cloth.h"

int gRenderMode;
int gPin, gNstep, gMovePin, gScale = 0, gWhichCloth = 0, gWind = 0;
float gGravity, gDelta, gTstep, gWindAmountX = 0.0, gWindAmountZ = 0.0;
float gKThread, gDThread, gKWall, gDWall;

int RANDXLIMIT = 400, RANDYLIMIT = 100, RANDZLIMIT  = 400;
point underWaterDamp = vMake(0.0), gravity;

void ClothInit(cloth *clothItem, double strSprLen, point birthPosition, int clothWidth, int clothHeight)
{
	point cur;
	memset( (void*)&cur, 0, sizeof(point));
	int index = 0;

	// Time step and nstep for the simulation
	clothItem->nStep = gNstep;
	clothItem->tStep = gTstep;

	// Set the lengths of all the springs in the cloth based of the input strSprLen
	clothItem->strSprLen = strSprLen;
	clothItem->shrSprLen = sqrt(2.0) * strSprLen;
	clothItem->bendSprLen = 2.0 * strSprLen;

	// count of the number of vertives in the cloth
	clothItem->cArrayLength = clothWidth * clothHeight;

	// Hooks coefficients
	clothItem->kThread = gKThread;
	clothItem->kWall = gKWall;
	clothItem->dThread = gDThread;
	clothItem->dWall = gDWall;

	clothItem->width = clothWidth;
	clothItem->height = clothHeight;

	// Same mass for all the vertices of the cloth
	clothItem->mass = 0.02;

	// Set the initial positions based on the structural spring length
	clothItem->positions = (point*)calloc(clothItem->cArrayLength, sizeof(point));
	
	for(int row = 0; row < clothHeight; row++)
	{
		memset( (void*)&cur, 0, sizeof(point));
		
		// new row position
		cur.y = birthPosition.y - (row * strSprLen);

		for(int col = 0; col < clothWidth; col++)
		{
			// new col position
			cur.x = birthPosition.x + (col * strSprLen);

			index = FindIndexInArray(row, col, clothWidth);

			cur.z = birthPosition.z;

			pCPY(cur, clothItem->positions[index]);
		}
	}

	// set the initial velocities for all the vertices of the cloth
	clothItem->velocities = (point*)calloc(clothItem->cArrayLength, sizeof(point));

	// set the initial acceleration for all the vertices of the cloth
	clothItem->acceleration = (point*)calloc(clothItem->cArrayLength, sizeof(point));

	for(int row = 0; row < clothHeight; row++)
	{
		for(int col = 0; col < clothWidth; col++)
		{
			index = FindIndexInArray(row, col, clothWidth);

			// Change these for the initial velocities
			clothItem->acceleration[index].x = 0.0;
			clothItem->acceleration[index].y = 0.0;
			clothItem->acceleration[index].z = 0.0;
		}
	}

	clothItem->force = (point*)calloc(clothItem->cArrayLength, sizeof(point));

	for(int row = 0; row < clothHeight; row++)
	{
		for(int col = 0; col < clothWidth; col++)
		{
			index = FindIndexInArray(row, col, clothWidth);

			// Change these for the initial velocities
			clothItem->force[index].x = 0.0;
			clothItem->force[index].y = 0.0;
			clothItem->force[index].z = 0.0;
		}
	}

	clothItem->normals = (point*)calloc(clothItem->cArrayLength, sizeof(point));

	clothItem->normalsCount = (int*)calloc(clothItem->cArrayLength, sizeof(int));

}// End of cloth initialization


void CopyToBuffer(cloth *buffer, cloth*clothItem)
{
	int index = 0;

	// Time step and nstep for the simulation
	buffer->nStep = clothItem->nStep;
	buffer->tStep = clothItem->tStep;

	// Set the lengths of all the springs in the cloth based of the input strSprLen
	buffer->strSprLen = clothItem->strSprLen;
	buffer->shrSprLen = clothItem->shrSprLen;
	buffer->bendSprLen = clothItem->bendSprLen;

	// count of the number of vertives in the cloth
	buffer->cArrayLength = clothItem->cArrayLength;

	// Hooks coefficients
	buffer->kThread = clothItem->kThread;
	buffer->kWall = clothItem->kWall;
	buffer->dThread = clothItem->dThread;
	buffer->dWall = clothItem->dWall;
	buffer->width = clothItem->width;
	buffer->height = clothItem->height;
	buffer->mass = clothItem->mass;

	buffer->positions = (point*)calloc(buffer->cArrayLength, sizeof(point));
	buffer->velocities = (point*)calloc(buffer->cArrayLength, sizeof(point));
	buffer->acceleration = (point*)calloc(buffer->cArrayLength, sizeof(point));
	buffer->force = (point*)calloc(buffer->cArrayLength, sizeof(point));
	buffer->normals = (point*)calloc(buffer->cArrayLength, sizeof(point));
	buffer->normalsCount = (int*)calloc(buffer->cArrayLength, sizeof(int));
	
	for(int row = 0; row < clothItem->height; row++)
	{
		for(int col = 0; col < clothItem->width; col++)
		{
			index = FindIndexInArray(row, col, clothItem->width);

			pCPY(clothItem->positions[index], buffer->positions[index]);
			pCPY(clothItem->velocities[index], buffer->velocities[index]);
			pCPY(clothItem->velocities[index], buffer->velocities[index]);
			pCPY(clothItem->force[index], buffer->force[index]);
			pCPY(clothItem->normals[index], buffer->normals[index]);
			buffer->normalsCount[index] = clothItem->normalsCount[index];
		}
	}
}


int FindIndexInArray(int row, int col, int clothWidth)
{
	return((row * clothWidth) + col);
}


void RenderPoints(cloth *clothItem)
{
	int index = 0;
	glPointSize(5);
	for(int row = 0; row < clothItem->height; row++)
	{
		for(int col = 0; col < clothItem->width; col++)
		{
			index = FindIndexInArray(row, col, clothItem->width);

			GLfloat materialColor1[] = {1, 0.5, 0, 0.0f};
			GLfloat materialSpecular1[] = {specLevel, specLevel, specLevel, 1.0f};
			GLfloat materialEmissLevel1[] = {emissLevel, emissLevel, emissLevel, 1.0f};

			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, materialColor1);
			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, materialSpecular1);
			glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, materialEmissLevel1);
			glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shineLevel); //The shininess parameter

			glDisable(GL_CULL_FACE);
			glPushMatrix();
			glTranslatef(clothItem->positions[index].x, clothItem->positions[index].y, clothItem->positions[index].z);
			glutSolidSphere(0.004, 30, 30);
			glPopMatrix();
		}
	}
}

void RenderSpring(cloth *clothItem, node cur, node step)
{
	node adj;
	int index = 0;

	adj = nAdd(cur, step);

	if(!( (adj.x>clothItem->width-1) || (adj.x<0) || (adj.y>clothItem->height-1) || (adj.y<0) ))
	{
		index = FindIndexInArray(cur.y, cur.x, clothItem->width);
		glVertex3f(clothItem->positions[index].x, clothItem->positions[index].y, clothItem->positions[index].z);

		index = FindIndexInArray(adj.y, adj.x, clothItem->width);
		glVertex3f(clothItem->positions[index].x, clothItem->positions[index].y, clothItem->positions[index].z);
	}
}

void RenderClothSystem(cloth *clothItem, GLuint texId)
{
	double length = 0.0f;
	
	struct tex
	{
		float u;
		float v;
	};

	struct tex texture, increment;

	if(gRenderMode == THREAD)
	{
		glLineWidth(1);
		glPointSize(5);

		glDisable(GL_LIGHTING);

		// Render the cloth points
		glBegin(GL_POINTS);
			RenderPoints(clothItem);
		glEnd();
	
		// Render the cloth springs
		for(int row = 0; row < clothItem->height; row++)
		{
			for(int col = 0; col < clothItem->width; col++)
			{
				glBegin(GL_LINES);
					// Render the structural springs
					RenderSpring(clothItem, nMake(col, row, 0), nMake(1, 0, 0));
					RenderSpring(clothItem, nMake(col, row, 0), nMake(-1, 0, 0));
					RenderSpring(clothItem, nMake(col, row, 0), nMake(0, 1, 0));
					RenderSpring(clothItem, nMake(col, row, 0), nMake(0, -1, 0));

					// Render shear springs along the side
					RenderSpring(clothItem, nMake(col, row, 0), nMake(1, 1, 0));
					RenderSpring(clothItem, nMake(col, row, 0), nMake(1, -1, 0));
					RenderSpring(clothItem, nMake(col, row, 0), nMake(-1, 1, 0));
					RenderSpring(clothItem, nMake(col, row, 0), nMake(-1, -1, 0));

					// Render bend springs
					RenderSpring(clothItem, nMake(col, row, 0), nMake(2, 0, 0));
					RenderSpring(clothItem, nMake(col, row, 0), nMake(-2, 0, 0));
					RenderSpring(clothItem, nMake(col, row, 0), nMake(0, 2, 0));
					RenderSpring(clothItem, nMake(col, row, 0), nMake(0, -2, 0));
				glEnd();
			}
		}

		glEnable(GL_LIGHTING);
	}

	if(gRenderMode == TRIANGLE)
	{
		glDisable(GL_LIGHTING);
		glDisable(GL_CULL_FACE);

		glPolygonMode(GL_FRONT, GL_FILL);
		
		// Accumulate the normals for all the points in the cloth
		point len1, len2, len3;
		int index, index1, index2, index3;
		for(int row = 0; row < clothItem->height - 1; row++)
		{
			for(int col = 0; col < clothItem->width - 1; col ++)
			{
	/*
			index		index1
				-----------
				|		 /|
				|  T1   / |
				|	   /  |
				|     /   |
				|    /    |
				|   /     |
				|  /      |
				| /  T2   |
				|/        |
				-----------
			index2		index3
	*/

				index = FindIndexInArray(row, col, clothItem->width);
				index1 = FindIndexInArray(row, col + 1, clothItem->width);
				index2 = FindIndexInArray(row + 1, col, clothItem->width);
				index3 = FindIndexInArray(row + 1, col + 1, clothItem->width);

				// First triangle - left top
				pDIFFERENCE(clothItem->positions[index1], clothItem->positions[index], len1);	
				pDIFFERENCE(clothItem->positions[index2], clothItem->positions[index], len2);	
				
				CROSSPRODUCTp(len1, len2, len3);
				pNORMALIZE(len3);

				pSUM(clothItem->normals[index1], len3, clothItem->normals[index1]);
				clothItem->normalsCount[index1]++;

				pSUM(clothItem->normals[index2], len3, clothItem->normals[index2]);
				clothItem->normalsCount[index2]++;

				pSUM(clothItem->normals[index], len3, clothItem->normals[index]);
				clothItem->normalsCount[index]++;

				// Second triangle - right bottom
				pDIFFERENCE(clothItem->positions[index1], clothItem->positions[index3], len1);	
				pDIFFERENCE(clothItem->positions[index2], clothItem->positions[index3], len2);	
				
				CROSSPRODUCTp(len1, len2, len3);
				pNORMALIZE(len3);

				pSUM(clothItem->normals[index1], len3, clothItem->normals[index1]);
				clothItem->normalsCount[index1]++;

				pSUM(clothItem->normals[index2], len3, clothItem->normals[index2]);
				clothItem->normalsCount[index2]++;

				pSUM(clothItem->normals[index3], len3, clothItem->normals[index]);
				clothItem->normalsCount[index3]++;
			}
		}
		
		texture.u = 0.0;
		texture.v = 0.0;

		increment.u = 1.0 / clothItem->width;
		increment.v = 1.0 / clothItem->height;
		// Render the triangles
		for(int row = 0; row < clothItem->height - 1; row++)
		{
			glBindTexture(GL_TEXTURE_2D, texId);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glBegin(GL_TRIANGLE_STRIP);
			
			for(int col = 0; col < clothItem->width - 1; col ++)
			{
				//index index 2 index 1 index 3
				index = FindIndexInArray(row, col, clothItem->width); 
				index1 = FindIndexInArray(row, col + 1, clothItem->width);
				index2 = FindIndexInArray(row + 1, col, clothItem->width);
				index3 = FindIndexInArray(row + 1, col + 1, clothItem->width); 

				// CCW order for the triangle strip
				// left top (0,0)
				glNormal3f(clothItem->normals[index].x / clothItem->normalsCount[index], clothItem->normals[index].y / clothItem->normalsCount[index], clothItem->normals[index].z / clothItem->normalsCount[index]);
				glTexCoord2f(texture.u, texture.v);
				glVertex3f(clothItem->positions[index].x, clothItem->positions[index].y, clothItem->positions[index].z);

				// left bottom (0, 1)
				glNormal3f(clothItem->normals[index2].x / clothItem->normalsCount[index2], clothItem->normals[index2].y / clothItem->normalsCount[index2], clothItem->normals[index2].z / clothItem->normalsCount[index2]);
				glTexCoord2f(texture.u, texture.v + increment.v);
				glVertex3f(clothItem->positions[index2].x, clothItem->positions[index2].y, clothItem->positions[index2].z);

				// right top (1, 0)
				glNormal3f(clothItem->normals[index1].x / clothItem->normalsCount[index1], clothItem->normals[index1].y / clothItem->normalsCount[index1], clothItem->normals[index1].z / clothItem->normalsCount[index1]);
				glTexCoord2f(texture.u + increment.u, texture.v);
				glVertex3f(clothItem->positions[index1].x, clothItem->positions[index1].y, clothItem->positions[index1].z);

				// right bottom (1, 1)
				glNormal3f(clothItem->normals[index3].x / clothItem->normalsCount[index3], clothItem->normals[index3].y / clothItem->normalsCount[index3], clothItem->normals[index3].z / clothItem->normalsCount[index3]);
				glTexCoord2f(texture.u + increment.u, texture.v + increment.v);
				glVertex3f(clothItem->positions[index3].x, clothItem->positions[index3].y, clothItem->positions[index3].z);

				texture.u += increment.u;
			}
			
			texture.u = 0.0;
			texture.v += increment.v;
			glEnd();
		}

		glEnable(GL_LIGHTING);
	}// if gRenderMode
}// end RenderClothSystem


void RenderBox(GLuint front_face_id, GLuint back_face_id, GLuint right_face_id, GLuint left_face_id, GLuint top_face_id, GLuint bottom_face_id, GLuint ropeId)
{
  
	glColor4f(1,1,1,1);

	glEnable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	// LEFT FACE
	
	  glBindTexture(GL_TEXTURE_2D, left_face_id);


	  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	  
	  glBegin(GL_QUADS);
	  
	  glNormal3f(1.0, 0.0f, 0.0f);
	  glTexCoord2f(0.0f, 0.0f);
	  glVertex3f(-SKYBOXXLIM, -SKYBOXYLIM, -SKYBOXZLIM);
	  glTexCoord2f(1.0f, 0.0f);
	  glVertex3f(-SKYBOXXLIM, -SKYBOXYLIM, SKYBOXZLIM);
	  glTexCoord2f(1.0f, 1.0f);
	  glVertex3f(-SKYBOXXLIM, SKYBOXYLIM, SKYBOXZLIM);
	  glTexCoord2f(0.0f, 1.0f);
	  glVertex3f(-SKYBOXXLIM, SKYBOXYLIM, -SKYBOXZLIM);

	  glEnd();

	// RIGHT FACE
	
	  glBindTexture(GL_TEXTURE_2D, right_face_id);


	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBegin(GL_QUADS);

	glNormal3f(-1.0, 0.0f, 0.0f);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(SKYBOXXLIM, -SKYBOXYLIM, SKYBOXZLIM);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(SKYBOXXLIM, -SKYBOXYLIM, -SKYBOXZLIM);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(SKYBOXXLIM, SKYBOXYLIM, -SKYBOXZLIM);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(SKYBOXXLIM, SKYBOXYLIM, SKYBOXZLIM);

	glEnd();

	// BACK FACE
	glBindTexture(GL_TEXTURE_2D, back_face_id);


	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBegin(GL_QUADS);

	glNormal3f(0.0, 0.0f, 1.0f);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(SKYBOXXLIM, -SKYBOXYLIM, -SKYBOXZLIM);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(-SKYBOXXLIM, -SKYBOXYLIM, -SKYBOXZLIM);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(-SKYBOXXLIM, SKYBOXYLIM, -SKYBOXZLIM);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(SKYBOXXLIM, SKYBOXYLIM, -SKYBOXZLIM);

	glEnd();

	// FRONT FACE

	glBindTexture(GL_TEXTURE_2D, front_face_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBegin(GL_QUADS);

	glNormal3f(0.0, 0.0f, -1.0f);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-SKYBOXXLIM, -SKYBOXYLIM, SKYBOXZLIM);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(SKYBOXXLIM, -SKYBOXYLIM, SKYBOXZLIM);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(SKYBOXXLIM, SKYBOXYLIM, SKYBOXZLIM);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-SKYBOXXLIM, SKYBOXYLIM, SKYBOXZLIM);

	glEnd();


	// BOTTOM FACE
	glBindTexture(GL_TEXTURE_2D, bottom_face_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBegin(GL_QUADS);

	glNormal3f(0.0, 1.0f, 0.0f);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(SKYBOXXLIM, -SKYBOXYLIM, -SKYBOXZLIM);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(SKYBOXXLIM, -SKYBOXYLIM, SKYBOXZLIM);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(-SKYBOXXLIM, -SKYBOXYLIM, SKYBOXZLIM);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-SKYBOXXLIM, -SKYBOXYLIM, -SKYBOXZLIM);

	glEnd();


	// TOP FACE
	glBindTexture(GL_TEXTURE_2D, top_face_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBegin(GL_QUADS);

	glNormal3f(0.0, -1.0f, 0.0f);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-SKYBOXXLIM, SKYBOXYLIM, -SKYBOXZLIM);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(-SKYBOXXLIM, SKYBOXYLIM, SKYBOXZLIM);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(SKYBOXXLIM, SKYBOXYLIM, SKYBOXZLIM);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(SKYBOXXLIM, SKYBOXYLIM, -SKYBOXZLIM);

	glEnd();

	glDisable(GL_CULL_FACE);

	// BACK ROPE BELOW
	glBindTexture(GL_TEXTURE_2D, ropeId);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBegin(GL_QUADS);

	glNormal3f(0.0, -1.0f, 0.0f);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-SKYBOXXLIM-1, 0.0, 0.0);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(SKYBOXXLIM+1, 0.0, 0.0);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(SKYBOXXLIM+1, 0.1, 0.0);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(-SKYBOXXLIM-1, 0.1, 0.0);

	glEnd();

	// FRONT ROPE BELOW
	glBindTexture(GL_TEXTURE_2D, ropeId);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBegin(GL_QUADS);

	glNormal3f(0.0, -1.0f, 0.0f);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-SKYBOXXLIM-1, -0.1, 0.4);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(SKYBOXXLIM+1, -0.1, 0.4);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(SKYBOXXLIM+1, 0.0, 0.4);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(-SKYBOXXLIM-1, 0.0, 0.4);

	glEnd();

	// FRONT ROPE ABOVE
	glBindTexture(GL_TEXTURE_2D, ropeId);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBegin(GL_QUADS);

	glNormal3f(0.0, -1.0f, 0.0f);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-SKYBOXXLIM-1, 1.4, 0.6);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(SKYBOXXLIM+1, 1.4, 0.6);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(SKYBOXXLIM+1, 1.5, 0.6);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(-SKYBOXXLIM-1, 1.5, 0.6);

	glEnd();

	// BACK ROPE ABOVE
	glBindTexture(GL_TEXTURE_2D, ropeId);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBegin(GL_QUADS);

	glNormal3f(0.0, -1.0f, 0.0f);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-SKYBOXXLIM-1, 1.4, 0.0);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(SKYBOXXLIM+1, 1.4, 0.0);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(SKYBOXXLIM+1, 1.5, 0.0);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(-SKYBOXXLIM-1, 1.5, 0.0);

	glEnd();



	glEnable(GL_CULL_FACE);

	glEnable(GL_LIGHTING);

	return;
}


void ScaleCloth(cloth *clothItem)
{
	if(gScale == INCREASE)
	{
		clothItem->strSprLen *= 1.1;
		clothItem->shrSprLen = sqrt(2.0) * clothItem->strSprLen;
		clothItem->bendSprLen = 2.0 * clothItem->strSprLen;
	}
	else if(gScale == DECREASE)
	{
		clothItem->strSprLen *= 0.9;
		clothItem->shrSprLen = sqrt(2.0) * clothItem->strSprLen;
		clothItem->bendSprLen = 2.0 * clothItem->strSprLen;
	}
}