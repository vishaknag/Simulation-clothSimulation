#include "render.h"
#include "keyboard.h"
#include "camera.h"
#include "cloth.h"
#include "physics.h"

/* Function: keyInit
 * Description: Initialize keyboard input from user.
 * Input: None
 * Output: None
 */
void keyInit()
{
	pause = 1;
//	saveScreenToFile = 0;
} //end keyInit()

/* Function: keyboardKeys
 * Description: Reads keyboard input from user.
 * Input: unsigned char key - ASCII value of the key pressed.
 *		  int x - X-position
 *		  int y - Y-position
 * Output: None
 */
void keyboardKeys(unsigned char key, int x, int y) 
{
	switch(tolower(key))
	{
		case 27:
			exit(0);
			break;
		case 'z':
			zoom(-1, 0);
			break;
		case 'x':
			zoom(1, 0);
			break;
		case ' ':
//			saveScreenToFile = 1 - saveScreenToFile;
		case 'w':
				cameraFreeMove(0);
			break;
		case 's':
				cameraFreeMove(1);
			break;
		case 'q':
				cameraFreeMove(2);
			break;
		case 'e':
				cameraFreeMove(3);
			break;
		case 'd':
				cameraFreeMove(4);
			break;
		case 'a':
				cameraFreeMove(5);
			break;
		case 'f':
//			camFol = 1 - camFol;
			break;
		case 'c':
			DeleteClothes();
			break;
		case 'n':
			AddCloth();
			break;
	} //end switch
} //end keyboardKeys

/* Function: specialKeys
 * Description: Reads special GLUT keyboard input from user.
 * Input: key - ASCII value of the key pressed.
 *		  int x - X-position
 *		  int y - Y-position
 * Output: None
 */
void specialKeys(int key, int x, int y)
{
	printf("key == %d\n", key);
	switch(key)
	{
		case GLUT_KEY_RIGHT:
			MoveClothXZ(clothes->napkin, RIGHT);
			break;
		case GLUT_KEY_LEFT:
			MoveClothXZ(clothes->napkin, LEFT);
			break;
		case GLUT_KEY_UP:
			MoveClothXZ(clothes->napkin, FORWARD);
			break;
		case GLUT_KEY_DOWN:
			MoveClothXZ(clothes->napkin, BACKWARD);
			break;
	} //end switch
} //end specialKeys