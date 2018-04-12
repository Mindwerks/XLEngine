//Include File for Weapon Scripts.
//This contains all constants used by the UI scripts.

//Alignment
const int UI_Align_Left   = 0;
const int UI_Align_Right  = 1;
const int UI_Align_Center = 2;

const int UI_Align_Bottom = 0;
const int UI_Align_Top	  = 1;

//Input Messages
const int MSG_KEY_DOWN	= 0;
const int MSG_KEY_UP	= 1;
const int MSG_KEY_HELD	= 2;

//These "KEYS" are virtual.
//This allows scripts to access keys based on an ACTION where appropriate (like KEY_JUMP)
//and to access them in a platform independent manner.
const int KEY_F1  				=  1;
const int KEY_F2  				=  2;
const int KEY_F3  				=  3;
const int KEY_F4  				=  4;
const int KEY_F5  				=  5;
const int KEY_F6  				=  6;
const int KEY_F7  				=  7;
const int KEY_F8  				=  8;
const int KEY_F9  				=  9;
const int KEY_RMOUSE			= 12;
const int KEY_CONSOLE			= 13;
const int KEY_SCREENSHOT		= 14;
const int KEY_PRIMARY_FIRE		= 15;
const int KEY_SECONDARY_FIRE	= 16;
const int KEY_JUMP				= 17;
const int KEY_CROUCH			= 18;
const int KEY_RUN				= 19;
const int KEY_SLOW				= 20;
const int KEY_FORWARD			= 21;
const int KEY_BACKWARD			= 22;
const int KEY_STAFE_LEFT		= 23;
const int KEY_STAFE_RIGHT		= 24;
const int KEY_TURN_LEFT			= 25;
const int KEY_TURN_RIGHT		= 26;
const int KEY_USE				= 27;

const int KEY_ATTACK		    = 0x01;
const int KEY_PGUP				= 0x21;
const int KEY_PGDN				= 0x22;
const int KEY_UP				= 0x26;
const int KEY_RIGHT				= 0x27;
const int KEY_DOWN				= 0x28;
const int KEY_LEFT				= 0x29;
const int KEY_ENTER				= 0x0D;
const int KEY_ESC				= 0x1B;

//Background FX.
const int UI_BCKGRND_FX_NONE = 0;
const int UI_BCKGRND_FX_GRAYSCALE = 1;

//Screen flags
const int UIFLAG_NONE = 0;
const int UIFLAG_OVERLAY = 1;

//Window flags.
const int UIWinFlag_None = 0;
const int UIWinFlag_NonInteractive = 1;	//just for looks.

const int IMAGE_TYPE_ART = 0;
const int IMAGE_TYPE_PCX = 1;
