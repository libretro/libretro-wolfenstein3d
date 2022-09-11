//
//	ID Engine
//	ID_IN.h - Header file for Input Manager
//	v1.0d1
//	By Jason Blochowiak
//

#ifndef	__ID_IN_H_
#define	__ID_IN_H_
#include "wl_def.h"

#ifdef	__DEBUG__
#define	__DEBUG_InputMgr__
#endif

#if defined(SWITCH) || defined (N3DS)
#if SDL_MAJOR_VERSION == 1
#define SDLK_SCROLLLOCK SDLK_SCROLLOCK
#define SDLK_KP_0 SDLK_KP0
#define SDLK_KP_1 SDLK_KP1
#define SDLK_KP_2 SDLK_KP2
#define SDLK_KP_3 SDLK_KP3
#define SDLK_KP_4 SDLK_KP4
#define SDLK_KP_5 SDLK_KP5
#define SDLK_KP_6 SDLK_KP6
#define SDLK_KP_7 SDLK_KP7
#define SDLK_KP_8 SDLK_KP8
#define SDLK_KP_9 SDLK_KP9
#define SDLK_PRINTSCREEN SDLK_PRINT
#define SDLK_NUMLOCKCLEAR SDLK_NUMLOCK
#define SDLK_SCROLLLOCK	SDLK_SCROLLOCK
#define SDL_Keymod SDLMod
#endif
typedef	int		ScanCode;
#define	sc_None			0
#define	sc_Bad			0xff
#define	sc_Return		SDLK_MINUS
#define	sc_Enter		sc_Return // ZR
#define	sc_Escape		SDLK_PLUS //SDLK_j // ZL
#define	sc_Space		SDLK_b
#define	sc_BackSpace	SDLK_BACKSPACE
#define	sc_Tab			SDLK_TAB
#define	sc_Alt			SDLK_x
#define	sc_Control		SDLK_a
#define	sc_CapsLock		SDLK_CAPSLOCK
#define	sc_LShift		SDLK_y
#define	sc_RShift		SDLK_RSHIFT
#define	sc_UpArrow		SDLK_UP
#define	sc_DownArrow	SDLK_DOWN
#define	sc_LeftArrow	SDLK_LEFT
#define	sc_RightArrow	SDLK_RIGHT
#define	sc_Insert		SDLK_INSERT
#define	sc_Delete		SDLK_DELETE
#define	sc_Home			SDLK_HOME
#define	sc_End			SDLK_END
#define	sc_PgUp			SDLK_PAGEUP
#define	sc_PgDn			SDLK_PAGEDOWN
#define	sc_F1			SDLK_F1
#define	sc_F2			SDLK_F2
#define	sc_F3			SDLK_F3
#define	sc_F4			SDLK_F4
#define	sc_F5			SDLK_F5
#define	sc_F6			SDLK_F6
#define	sc_F7			SDLK_F7
#define	sc_F8			SDLK_F8
#define	sc_F9			SDLK_F9
#define	sc_F10			SDLK_F10
#define	sc_F11			SDLK_F11
#define	sc_F12			SDLK_F12

#define sc_ScrollLock		SDLK_SCROLLOCK
#define sc_PrintScreen		SDLK_PRINT

#define	sc_1			SDLK_q
#define	sc_2			SDLK_q
#define	sc_3			SDLK_q
#define	sc_4			SDLK_q
#define	sc_5			SDLK_q
#define	sc_6			SDLK_q
#define	sc_7			SDLK_q
#define	sc_8			SDLK_q
#define	sc_9			SDLK_q
#define	sc_0			SDLK_q

#define	sc_A			SDLK_q
#define	sc_B			SDLK_q
#define	sc_C			SDLK_q
#define	sc_D			SDLK_q
#define	sc_E			SDLK_q
#define	sc_F			SDLK_q
#define	sc_G			SDLK_q
#define	sc_H			SDLK_q
#define	sc_I			SDLK_q
#define	sc_J			SDLK_q
#define	sc_K			SDLK_q
#define	sc_L			SDLK_q
#define	sc_M			SDLK_q
#define	sc_N			SDLK_q
#define	sc_O			SDLK_q
#define	sc_P			SDLK_q
#define	sc_Q			SDLK_q
#define	sc_R			SDLK_q
#define	sc_S			SDLK_q
#define	sc_T			SDLK_q
#define	sc_U			SDLK_q
#define	sc_V			SDLK_q
#define	sc_W			SDLK_q
#define	sc_X			SDLK_q
#define	sc_Y			SDLK_q
#define	sc_Z			SDLK_q

#define	sc_1			SDLK_1
#define	sc_2			SDLK_2
#define	sc_3			SDLK_3
#define	sc_4			SDLK_4
#define	sc_5			SDLK_5
#define	sc_6			SDLK_6
#define	sc_7			SDLK_7
#define	sc_8			SDLK_8
#define	sc_9			SDLK_9
#define	sc_0			SDLK_0

#define	sc_A			SDLK_a
#define	sc_B			SDLK_b
#define	sc_C			SDLK_c
#define	sc_D			SDLK_d
#define	sc_E			SDLK_e
#define	sc_F			SDLK_f
#define	sc_G			SDLK_g
#define	sc_H			SDLK_h
#define	sc_I			SDLK_i
#define	sc_J			SDLK_j
#define	sc_K			SDLK_k
#define	sc_L			SDLK_l
#define	sc_M			SDLK_m
#define	sc_N			SDLK_n
#define	sc_O			SDLK_o
#define	sc_P			SDLK_p
#define	sc_Q			SDLK_q
#define	sc_R			SDLK_r
#define	sc_S			SDLK_s
#define	sc_T			SDLK_t
#define	sc_U			SDLK_u
#define	sc_V			SDLK_v
#define	sc_W			SDLK_w
#define	sc_X			SDLK_x
#define	sc_Y			SDLK_y
#define	sc_Z			SDLK_z
#define	key_None		0

#if SDL_MAJOR_VERSION == 2
#define bt_None SDL_CONTROLLER_BUTTON_INVALID
#define bt_A SDL_CONTROLLER_BUTTON_A
#define bt_B SDL_CONTROLLER_BUTTON_B
#define bt_X SDL_CONTROLLER_BUTTON_X
#define bt_Y SDL_CONTROLLER_BUTTON_Y
#define bt_Back SDL_CONTROLLER_BUTTON_BACK
#define bt_Guide SDL_CONTROLLER_BUTTON_GUIDE
#define bt_Start SDL_CONTROLLER_BUTTON_START
#define bt_LeftStick SDL_CONTROLLER_BUTTON_LEFTSTICK
#define bt_RightStick SDL_CONTROLLER_BUTTON_RIGHTSTICK
#define bt_LeftShoulder SDL_CONTROLLER_BUTTON_LEFTSHOULDER
#define bt_RightShoulder SDL_CONTROLLER_BUTTON_RIGHTSHOULDER
#define bt_DpadUp SDL_CONTROLLER_BUTTON_DPAD_UP
#define bt_DpadDown SDL_CONTROLLER_BUTTON_DPAD_DOWN
#define bt_DpadLeft SDL_CONTROLLER_BUTTON_DPAD_LEFT
#define bt_DpadRight SDL_CONTROLLER_BUTTON_DPAD_RIGHT
#define bt_touchpad SDL_CONTROLLER_BUTTON_TOUCHPAD
#define bt_Max SDL_CONTROLLER_BUTTON_MAX

//Axis stuff
#define gc_axis_invalid SDL_CONTROLLER_AXIS_INVALID
#define gc_axis_leftx SDL_CONTROLLER_AXIS_LEFTX
#define gc_axis_lefty SDL_CONTROLLER_AXIS_LEFTY
#define gc_axis_rightx SDL_CONTROLLER_AXIS_RIGHTX
#define gc_axis_righty SDL_CONTROLLER_AXIS_RIGHTY
#define gc_trigger_left SDL_CONTROLLER_AXIS_TRIGGERLEFT
#define gc_trigger_right SDL_CONTROLLER_AXIS_TRIGGERRIGHT
#define gc_axis_max SDL_CONTROLLER_AXIS_MAX
#endif
#define KEYCOUNT 129
#else
#if SDL_MAJOR_VERSION == 1
#define SDLK_SCROLLLOCK SDLK_SCROLLOCK
#define SDLK_KP_0 SDLK_KP0
#define SDLK_KP_1 SDLK_KP1
#define SDLK_KP_2 SDLK_KP2
#define SDLK_KP_3 SDLK_KP3
#define SDLK_KP_4 SDLK_KP4
#define SDLK_KP_5 SDLK_KP5
#define SDLK_KP_6 SDLK_KP6
#define SDLK_KP_7 SDLK_KP7
#define SDLK_KP_8 SDLK_KP8
#define SDLK_KP_9 SDLK_KP9
#define SDLK_PRINTSCREEN SDLK_PRINT
#define SDLK_NUMLOCKCLEAR SDLK_NUMLOCK
#define SDLK_SCROLLLOCK	SDLK_SCROLLOCK
#define SDL_Keymod SDLMod
#define SDL_Keysym SDL_keysym
#endif
#define KEYCOUNT 129
typedef	int		ScanCode;
#define	sc_None			0
#define	sc_Bad			0xff
#define	sc_Return		SDLK_RETURN
#define	sc_Enter		sc_Return
#define	sc_Escape		SDLK_ESCAPE
#define	sc_Space		SDLK_SPACE
#define	sc_BackSpace	SDLK_BACKSPACE
#define	sc_Tab			SDLK_TAB
#define	sc_Alt			SDLK_LALT
#define	sc_Control		SDLK_LCTRL
#define	sc_CapsLock		SDLK_CAPSLOCK
#define	sc_LShift		SDLK_LSHIFT
#define	sc_RShift		SDLK_RSHIFT
#define	sc_UpArrow		SDLK_UP
#define	sc_DownArrow	SDLK_DOWN
#define	sc_LeftArrow	SDLK_LEFT
#define	sc_RightArrow	SDLK_RIGHT
#define	sc_Insert		SDLK_INSERT
#define	sc_Delete		SDLK_DELETE
#define	sc_Home			SDLK_HOME
#define	sc_End			SDLK_END
#define	sc_PgUp			SDLK_PAGEUP
#define	sc_PgDn			SDLK_PAGEDOWN
#define	sc_F1			SDLK_F1
#define	sc_F2			SDLK_F2
#define	sc_F3			SDLK_F3
#define	sc_F4			SDLK_F4
#define	sc_F5			SDLK_F5
#define	sc_F6			SDLK_F6
#define	sc_F7			SDLK_F7
#define	sc_F8			SDLK_F8
#define	sc_F9			SDLK_F9
#define	sc_F10			SDLK_F10
#define	sc_F11			SDLK_F11
#define	sc_F12			SDLK_F12

#define sc_ScrollLock		SDLK_SCROLLLOCK
#define sc_PrintScreen		SDLK_PRINTSCREEN

#define	sc_1			SDLK_1
#define	sc_2			SDLK_2
#define	sc_3			SDLK_3
#define	sc_4			SDLK_4
#define	sc_5			SDLK_5
#define	sc_6			SDLK_6
#define	sc_7			SDLK_7
#define	sc_8			SDLK_8
#define	sc_9			SDLK_9
#define	sc_0			SDLK_0

#define	sc_A			SDLK_a
#define	sc_B			SDLK_b
#define	sc_C			SDLK_c
#define	sc_D			SDLK_d
#define	sc_E			SDLK_e
#define	sc_F			SDLK_f
#define	sc_G			SDLK_g
#define	sc_H			SDLK_h
#define	sc_I			SDLK_i
#define	sc_J			SDLK_j
#define	sc_K			SDLK_k
#define	sc_L			SDLK_l
#define	sc_M			SDLK_m
#define	sc_N			SDLK_n
#define	sc_O			SDLK_o
#define	sc_P			SDLK_p
#define	sc_Q			SDLK_q
#define	sc_R			SDLK_r
#define	sc_S			SDLK_s
#define	sc_T			SDLK_t
#define	sc_U			SDLK_u
#define	sc_V			SDLK_v
#define	sc_W			SDLK_w
#define	sc_X			SDLK_x
#define	sc_Y			SDLK_y
#define	sc_Z			SDLK_z

#define	key_None		0
#define key_unknown SDLK_UNKNOWN

#define bt_None SDL_CONTROLLER_BUTTON_INVALID
#define bt_A SDL_CONTROLLER_BUTTON_A
#define bt_B SDL_CONTROLLER_BUTTON_B
#define bt_X SDL_CONTROLLER_BUTTON_X
#define bt_Y SDL_CONTROLLER_BUTTON_Y
#define bt_Back SDL_CONTROLLER_BUTTON_BACK
#define bt_Guide SDL_CONTROLLER_BUTTON_GUIDE
#define bt_Start SDL_CONTROLLER_BUTTON_START
#define bt_LeftStick SDL_CONTROLLER_BUTTON_LEFTSTICK
#define bt_RightStick SDL_CONTROLLER_BUTTON_RIGHTSTICK
#define bt_LeftShoulder SDL_CONTROLLER_BUTTON_LEFTSHOULDER
#define bt_RightShoulder SDL_CONTROLLER_BUTTON_RIGHTSHOULDER
#define bt_DpadUp SDL_CONTROLLER_BUTTON_DPAD_UP
#define bt_DpadDown SDL_CONTROLLER_BUTTON_DPAD_DOWN
#define bt_DpadLeft SDL_CONTROLLER_BUTTON_DPAD_LEFT
#define bt_DpadRight SDL_CONTROLLER_BUTTON_DPAD_RIGHT
#define bt_touchpad SDL_CONTROLLER_BUTTON_TOUCHPAD
#define bt_Max SDL_CONTROLLER_BUTTON_MAX

//Axis stuff
#define gc_axis_invalid SDL_CONTROLLER_AXIS_INVALID
#define gc_axis_leftx SDL_CONTROLLER_AXIS_LEFTX
#define gc_axis_lefty SDL_CONTROLLER_AXIS_LEFTY
#define gc_axis_rightx SDL_CONTROLLER_AXIS_RIGHTX
#define gc_axis_righty SDL_CONTROLLER_AXIS_RIGHTY
#define gc_trigger_left SDL_CONTROLLER_AXIS_TRIGGERLEFT
#define gc_trigger_right SDL_CONTROLLER_AXIS_TRIGGERRIGHT
#define gc_axis_max SDL_CONTROLLER_AXIS_MAX
#endif
#if SDL_MAJOR_VERSION == 2
extern boolean GameControllerButtons[bt_Max];
extern int GameControllerLeftStick[2];
extern int GameControllerRightStick[2];
extern SDL_GameController* GameController;
#endif


typedef	enum
{
	demo_Off,
	demo_Record,
	demo_Playback,
	demo_PlayDone
} Demo;

typedef	enum
{
	ctrl_Keyboard,
	ctrl_Keyboard1 = ctrl_Keyboard,
	ctrl_Keyboard2,
	ctrl_Joystick,
	ctrl_Joystick1 = ctrl_Joystick,
	ctrl_Joystick2,
	ctrl_Mouse
} ControlType;

typedef	enum
{
	motion_Left = -1,
	motion_Up = -1,
	motion_None = 0,
	motion_Right = 1,
	motion_Down = 1
} Motion;

typedef	enum
{
	dir_North,
	dir_NorthEast,
	dir_East,
	dir_SouthEast,
	dir_South,
	dir_SouthWest,
	dir_West,
	dir_NorthWest,
	dir_None
} Direction;

typedef	struct
{
	boolean	button0, button1, button2, button3;
	short x, y;
	Motion xaxis, yaxis;
	Direction dir;
} CursorInfo;

typedef	CursorInfo ControlInfo;

typedef	struct
{
	ScanCode button0, button1,
		upleft, up, upright,
		left, right,
		downleft, down, downright;
} KeyboardDef;

typedef	struct
{
	word joyMinX, joyMinY;
	word threshMinX, threshMinY;
	word threshMaxX, threshMaxY;
	word joyMaxX, joyMaxY;
	word joyMultXL, joyMultYL;
	word joyMultXH, joyMultYH;
} JoystickDef;

//Global variables
#if SDL_MAJOR_VERSION == 1
extern volatile boolean KeyboardPress[SDLK_LAST];
#elif SDL_MAJOR_VERSION == 2
extern volatile boolean KeyboardState[129];
#endif
extern boolean MousePresent;
extern volatile boolean Paused;
extern volatile char LastASCII;
extern volatile ScanCode LastScan;
extern int JoyNumButtons;
extern boolean forcegrabmouse;

#define	IN_KeyDown(code)	(Keyboard((code)))
#define	IN_ClearKey(code)	{ KeyboardSet(code, false);\
						      if (code == LastScan) LastScan = sc_None;}


boolean Keyboard(int key);
void KeyboardSet(int key, boolean state);
int KeyboardLookup(int key);


// DEBUG - put names in prototypes
extern	void		IN_Startup(void), IN_Shutdown(void);
extern	void		IN_ClearKeysDown(void);
extern	void		IN_ReadControl(int, ControlInfo*);
extern	void		IN_GetJoyAbs(word joy, word* xp, word* yp);
extern	void		IN_SetupJoy(word joy, word minx, word maxx,
	word miny, word maxy);
extern	void		IN_StopDemo(void), IN_FreeDemoBuffer(void),
IN_Ack(void);
extern	boolean		IN_UserInput(longword delay);
extern	char		IN_WaitForASCII(void);
extern	ScanCode	IN_WaitForKey(void);
extern	word		IN_GetJoyButtonsDB(word joy);
extern	const char* IN_GetScanName(ScanCode);

void    IN_WaitAndProcessEvents();
void    IN_ProcessEvents();

int     IN_MouseButtons(void);

boolean IN_JoyPresent();
void    IN_SetJoyCurrent(int joyIndex);
int     IN_JoyButtons(void);
void    IN_GetJoyDelta(int* dx, int* dy);
void    IN_GetJoyFineDelta(int* dx, int* dy);

void    IN_StartAck(void);
boolean IN_CheckAck(void);
boolean    IN_IsInputGrabbed();
void    IN_CenterMouse();

#endif