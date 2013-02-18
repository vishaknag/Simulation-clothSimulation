#include "cloth.h"
#include "ui.h"
#include "render.h"
#include "camera.h"
#include "physics.h"

int mode = 0;
int pin = 0;

void InitGlobal()
{
	gRenderMode = TRIANGLE;
	gPin = PINNED;
	gDelta = -0.7;//-0.2;
	gGravity = -0.03;//-0.05;
	gNstep = 4;
	gTstep = 0.006;//0.006;
	gKThread = 2.0;//1.0;
	gDThread = 1.0;//0.8;
	gKWall = 5.0;
	gDWall = 2.6;
	gMovePin = MOVELEFTPIN;

	gravity = vMake(0.0, gGravity, 0.0);
}


void control_cb(int control)
{
	instance *temp;

	switch (control)
	{
		case PAUSE:
			pause = 1 - pause;
			break;

		case RESETCAMERA:
			camInit();
			break;

		case RESET:
			reset();
			break;

		case ADDCLOTH:
			AddCloth();
			break;

		case TSTEP:
			temp = clothes;
			while(temp->next != NULL)
			{
				temp->napkin->tStep = gTstep;
				temp = temp->next;
			}
			break;

		case NSTEP:
			temp = clothes;
			while(temp->next != NULL)
			{
				temp->napkin->nStep = gNstep;
				temp = temp->next;
			}
			break;

		case KTHREAD:
			temp = clothes;
			while(temp->next != NULL)
			{
				temp->napkin->kThread = gKThread;
				temp = temp->next;
			}
			break;

		case DTHREAD:
			temp = clothes;
			while(temp->next != NULL)
			{
				temp->napkin->dThread = gDThread;
				temp = temp->next;
			}
			break;

		case KWALL:
			temp = clothes;
			while(temp->next != NULL)
			{
				temp->napkin->kWall = gKWall;
				temp = temp->next;
			}
			break;

		case DWALL:
			temp = clothes;
			while(temp->next != NULL)
			{
				temp->napkin->dWall = gDWall;
				temp = temp->next;
			}
			break;

		case GRAVITY:
			gravity = vMake(0.0, gGravity, 0.0);
			break;

		case MODE:
			if(mode == 0)
				gRenderMode = TRIANGLE;
			if(mode == 1)
				gRenderMode = THREAD;
			break;

		case SCALEUP:
			gScale = INCREASE;
			ScaleCloth(clothes->napkin);
			break;

		case SCALEDOWN:
			gScale = DECREASE;
			ScaleCloth(clothes->napkin);
			break;
	}
}


void InitGlui()
{
	// Create a GLUI window
	glutInitWindowPosition(windowWidth + 30, 0);
	glui = GLUI_Master.create_glui("Cloth Simulation", 0, 600, -1);
	//GLUI *glui = GLUI_Master.create_glui_subwindow( mainWindowId, GLUI_SUBWINDOW_BOTTOM ); 

    glui->set_main_gfx_window( mainWindowId );

	GLUI_Panel *main_panel = glui->add_panel ( "CONTROLS" );

		GLUI_Panel *lights_panel = glui->add_panel_to_panel( main_panel, "LIGHTS", 1 );

			glui->add_statictext_to_panel( lights_panel, "ON / OFF" );
			glui->add_checkbox_to_panel( lights_panel, "Light 1", &light1 );
			glui->add_checkbox_to_panel( lights_panel, "Light 2", &light2 );
			glui->add_checkbox_to_panel( lights_panel, "Light 3", &light3 );
			glui->add_checkbox_to_panel( lights_panel, "Light 4", &light4 );
			glui->add_checkbox_to_panel( lights_panel, "Light 5", &light5 );
			glui->add_checkbox_to_panel( lights_panel, "Light 6", &light6 );
			glui->add_checkbox_to_panel( lights_panel, "Light 7", &light7 );
			glui->add_checkbox_to_panel( lights_panel, "Light 8", &light8 );

			glui->add_column_to_panel(lights_panel, true);

			glui->add_statictext_to_panel( lights_panel, "EFFECTS" );
			glui->add_checkbox_to_panel( lights_panel, "Shininess", &setShineLevel );
			glui->add_checkbox_to_panel( lights_panel, "Specularity", &setSpecLevel );
			glui->add_checkbox_to_panel( lights_panel, "Emissivity", &setEmissLevel );
			glui->add_separator_to_panel(lights_panel);
			
			GLUI_Panel *mode_panel = glui->add_panel_to_panel( main_panel, "MODE", 1 );
			
			GLUI_Panel *render_panel = glui->add_panel_to_panel( mode_panel, "RENDER", 1);
			GLUI_RadioGroup *modeGroup = glui->add_radiogroup_to_panel(render_panel, &mode, MODE, control_cb);
			glui->add_radiobutton_to_group( modeGroup, "Triangles");
			glui->add_radiobutton_to_group( modeGroup, "Threads");

			GLUI_Panel *scale_panel = glui->add_panel_to_panel( mode_panel, "SCALE", 1);
			GLUI_Button *scaleupButton = glui->add_button_to_panel( scale_panel, "SCALE UP", SCALEUP, control_cb);
			GLUI_Button *scaledownButton = glui->add_button_to_panel( scale_panel, "SCALE DOWN", SCALEDOWN, control_cb);

			glui->add_column_to_panel(mode_panel, true);

			GLUI_Panel *pin_panel = glui->add_panel_to_panel( mode_panel, "PIN", 1);
			GLUI_RadioGroup *pinGroup = glui->add_radiogroup_to_panel(pin_panel, &gPin, PIN, control_cb);
			glui->add_radiobutton_to_group( pinGroup, "Both Pinned");
			glui->add_radiobutton_to_group( pinGroup, "UnPin Left");
			glui->add_radiobutton_to_group( pinGroup, "UnPin Right");
			

			glui->bkgd_color[0] = 100;

			GLUI_Panel *movepin_panel = glui->add_panel_to_panel( mode_panel, "MOVE PIN", 1);
			GLUI_RadioGroup *movePinsGroup = glui->add_radiogroup_to_panel(movepin_panel, &gMovePin);
			glui->add_radiobutton_to_group( movePinsGroup, "Left Pin");
			glui->add_radiobutton_to_group( movePinsGroup, "Right Pin");
			glui->add_radiobutton_to_group( movePinsGroup, "Both Pins");
			glui->add_radiobutton_to_group( movePinsGroup, "Bottom Left Pin");
			glui->add_radiobutton_to_group( movePinsGroup, "Bottom Right Pin");


			GLUI_Panel *parameters_panel = glui->add_panel_to_panel( main_panel, "LIVE", 1 );

				GLUI_EditText *tStepParam = glui->add_edittext_to_panel( parameters_panel, "TSTEP", GLUI_EDITTEXT_FLOAT, &gTstep, TSTEP, control_cb);
				GLUI_EditText *nStepParam = glui->add_edittext_to_panel( parameters_panel, "NSTEP", GLUI_EDITTEXT_INT, &gNstep, NSTEP, control_cb);

				GLUI_EditText *kThreadParam = glui->add_edittext_to_panel( parameters_panel, "K THREAD", GLUI_EDITTEXT_FLOAT, &gKThread, KTHREAD, control_cb);
				GLUI_EditText *dThreadParam = glui->add_edittext_to_panel( parameters_panel, "D THREAD", GLUI_EDITTEXT_FLOAT, &gDThread, DTHREAD, control_cb);

				GLUI_EditText *kWallParam = glui->add_edittext_to_panel( parameters_panel, "K WALL", GLUI_EDITTEXT_FLOAT, &gKWall, KWALL, control_cb);
				GLUI_EditText *dWallParam = glui->add_edittext_to_panel( parameters_panel, "D WALL", GLUI_EDITTEXT_FLOAT, &gDWall, DWALL, control_cb);
				
				GLUI_EditText *gravityParam = glui->add_edittext_to_panel( parameters_panel, "GRAVITY", GLUI_EDITTEXT_FLOAT, &gGravity, GRAVITY, control_cb);
				GLUI_EditText *deltaParam = glui->add_edittext_to_panel( parameters_panel, "DELTA", GLUI_EDITTEXT_FLOAT, &gDelta, DELTA, control_cb);

//			glui->add_column_to_panel(parameters_panel, true);

			glui->add_column_to_panel(main_panel, true);

			GLUI_Panel *camera_panel = glui->add_panel_to_panel( main_panel, "", 1 );
			GLUI_Button *cameraButton = glui->add_button_to_panel( camera_panel, "RESET CAMERA", RESETCAMERA, control_cb);

			GLUI_Panel *reset_panel = glui->add_panel_to_panel( main_panel, "", 1 );
			GLUI_Button *resetButton = glui->add_button_to_panel( reset_panel, "RESET", RESET, control_cb);

			GLUI_Panel *pause_panel = glui->add_panel_to_panel( main_panel, "", 1 );
			GLUI_Button *pauseButton = glui->add_button_to_panel( pause_panel, "PAUSE", PAUSE, control_cb);
			
			GLUI_Panel *addcloth_panel = glui->add_panel_to_panel( main_panel, "", 1 );
			GLUI_Button *addClothButton = glui->add_button_to_panel( addcloth_panel, "ADD CLOTH", ADDCLOTH, control_cb);
			GLUI_RadioGroup *selectClothGroup = glui->add_radiogroup_to_panel(addcloth_panel, &gWhichCloth);
			glui->add_radiobutton_to_group( selectClothGroup, "USC Trojan Banner");
			glui->add_radiobutton_to_group( selectClothGroup, "Mickey Towel");
			glui->add_radiobutton_to_group( selectClothGroup, "Army");
			glui->add_radiobutton_to_group( selectClothGroup, "Cloth Bag");
			glui->add_radiobutton_to_group( selectClothGroup, "T-Shirt");
			glui->add_radiobutton_to_group( selectClothGroup, "Shorts");
			glui->add_radiobutton_to_group( selectClothGroup, "Pant");
			glui->add_radiobutton_to_group( selectClothGroup, "Sock");
			
			GLUI_Panel *wind_panel = glui->add_panel_to_panel( main_panel, "WIND", 1 );
			GLUI_RadioGroup *windGroup = glui->add_radiogroup_to_panel(wind_panel, &gWind);
			glui->add_radiobutton_to_group( windGroup, "Wind Off");
			glui->add_radiobutton_to_group( windGroup, "Wind On");
			GLUI_EditText *windXParam = glui->add_edittext_to_panel( wind_panel, "WIND X", GLUI_EDITTEXT_FLOAT, &gWindAmountX, WIND, control_cb);
			GLUI_EditText *windZParam = glui->add_edittext_to_panel( wind_panel, "WIND Z", GLUI_EDITTEXT_FLOAT, &gWindAmountZ, WIND, control_cb);

	glui->set_main_gfx_window( mainWindowId );
}