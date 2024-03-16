#include "Luna-mbx.h"

#include <cstdio>
#include <cstdlib>
#include <SDL.h>

#if defined(PANDORA)
#define PANDORA_BUTTON_UP              (SDL_SCANCODE_UP)
#define PANDORA_BUTTON_DOWN            (SDL_SCANCODE_DOWN)
#define PANDORA_BUTTON_LEFT            (SDL_SCANCODE_LEFT)
#define PANDORA_BUTTON_RIGHT           (SDL_SCANCODE_RIGHT)
#define PANDORA_BUTTON_Y               (SDL_SCANCODE_PAGEUP)
#define PANDORA_BUTTON_X               (SDL_SCANCODE_PAGEDOWN)
#define PANDORA_BUTTON_A               (SDL_SCANCODE_HOME)
#define PANDORA_BUTTON_B               (SDL_SCANCODE_END)
#define PANDORA_BUTTON_L               (SDL_SCANCODE_RSHIFT)
#define PANDORA_BUTTON_R               (SDL_SCANCODE_RCTRL)
#define PANDORA_BUTTON_START           (SDL_SCANCODE_LALT)
#define PANDORA_BUTTON_SELECT          (SDL_SCANCODE_LCTRL)
#elif defined(PYRA)
#define PYRA_KEY_UP              (SDL_SCANCODE_UP)
#define PYRA_KEY_DOWN            (SDL_SCANCODE_DOWN)
#define PYRA_KEY_LEFT            (SDL_SCANCODE_LEFT)
#define PYRA_KEY_RIGHT           (SDL_SCANCODE_RIGHT)
#define PYRA_KEY_Y               (SDL_SCANCODE_PAGEUP)
#define PYRA_KEY_X               (SDL_SCANCODE_PAGEDOWN)
#define PYRA_KEY_A               (SDL_SCANCODE_HOME)
#define PYRA_KEY_B               (SDL_SCANCODE_END)
#define PYRA_KEY_INS             (SDL_SCANCODE_INSERT)
#define PYRA_KEY_DEL             (SDL_SCANCODE_DELETE)
#define PYRA_KEY_L1              (SDL_SCANCODE_RSHIFT)
#define PYRA_KEY_R1              (SDL_SCANCODE_RCTRL)
#define PYRA_KEY_R2              (SDL_SCANCODE_RALT)
#define PYRA_KEY_START           (SDL_SCANCODE_LALT)
#define PYRA_KEY_SELECT          (SDL_SCANCODE_LCTRL)
#endif


//==========================================================================
// STRUCT
//==========================================================================

//-----------------------------------------------------
// 軸データ
//-----------------------------------------------------
typedef struct AXISDATA
{
	long Ax, Ay, Az;
}
AXISDATA, *LPAXISDATA;

//-----------------------------------------------------
// ジョイスティック統括
//-----------------------------------------------------
typedef struct JOYSTICKDATA2
{
	SDL_Joystick *lpDevice;
	JOYSTICKDATA Data;
	short bPolling;
	short bForceFeedback;
	AXISDATA Center;
	AXISDATA Max;
}
JOYSTICKDATA2, *LPJOYSTICKDATA2;


static unsigned char KeyboardData[256];
static JOYSTICKDATA2 JoystickData[16];
static int JoystickCount;


BOOL LunaInput::Init( void )
{
    memset(KeyboardData, 0, 256);
    memset(JoystickData, 0, 16 * sizeof(JOYSTICKDATA2));
    JoystickCount = 0;

    if (!SDL_InitSubSystem(SDL_INIT_JOYSTICK))
    {
#if defined(PANDORA)
        int num = SDL_NumJoysticks();
        int stick;
        for (stick = 0; stick < num; stick++)
        {
            if (!strcmp(SDL_JoystickNameForIndex(stick), "nub0"))
            {
                JoystickData[0].lpDevice = SDL_JoystickOpen(stick);
                break;
            }
        }
#else
        if (SDL_NumJoysticks() > 0)
        {
            JoystickData[0].lpDevice = SDL_JoystickOpen(0);
        }
#endif

        if (JoystickData[0].lpDevice != NULL)
        {
            JoystickCount = 1;
        }
    }

    return TRUE;
}

void LunaInput::UnInit( void )
{
    if (JoystickCount)
    {
        SDL_JoystickClose(JoystickData[0].lpDevice);
        JoystickData[0].lpDevice = NULL;
    }
    SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
}

void LunaInput::RefreshKeyboard( void )
{
    int numkeys;
    const Uint8 *keystate = SDL_GetKeyboardState(&numkeys);

#if defined(PANDORA)
    KeyboardData[DIK_UP] = (numkeys >= PANDORA_BUTTON_UP)?(keystate[PANDORA_BUTTON_UP]):0;
    KeyboardData[DIK_DOWN] = (numkeys >= PANDORA_BUTTON_DOWN)?(keystate[PANDORA_BUTTON_DOWN]):0;
    KeyboardData[DIK_LEFT] = (numkeys >= PANDORA_BUTTON_LEFT)?(keystate[PANDORA_BUTTON_LEFT]):0;
    KeyboardData[DIK_RIGHT] = (numkeys >= PANDORA_BUTTON_RIGHT)?(keystate[PANDORA_BUTTON_RIGHT]):0;

    KeyboardData[DIK_NUMPAD2] = 0;
    KeyboardData[DIK_NUMPAD4] = 0;
    KeyboardData[DIK_NUMPAD6] = 0;
    KeyboardData[DIK_NUMPAD8] = 0;

    KeyboardData[DIK_Z] = (numkeys >= PANDORA_BUTTON_B)?(keystate[PANDORA_BUTTON_B]):0;
    KeyboardData[DIK_X] = (numkeys >= PANDORA_BUTTON_Y)?(keystate[PANDORA_BUTTON_Y]):0;
    KeyboardData[DIK_C] = (numkeys >= PANDORA_BUTTON_X)?(keystate[PANDORA_BUTTON_X]):0;
    KeyboardData[DIK_V] = (numkeys >= PANDORA_BUTTON_A)?(keystate[PANDORA_BUTTON_A]):0;
#elif defined(PYRA)
    KeyboardData[DIK_UP] = (numkeys >= PYRA_KEY_UP)?(keystate[PYRA_KEY_UP]):0;
    KeyboardData[DIK_DOWN] = (numkeys >= PYRA_KEY_DOWN)?(keystate[PYRA_KEY_DOWN]):0;
    KeyboardData[DIK_LEFT] = (numkeys >= PYRA_KEY_LEFT)?(keystate[PYRA_KEY_LEFT]):0;
    KeyboardData[DIK_RIGHT] = (numkeys >= PYRA_KEY_RIGHT)?(keystate[PYRA_KEY_RIGHT]):0;

    KeyboardData[DIK_NUMPAD2] = 0;
    KeyboardData[DIK_NUMPAD4] = 0;
    KeyboardData[DIK_NUMPAD6] = 0;
    KeyboardData[DIK_NUMPAD8] = 0;

    KeyboardData[DIK_Z] = (numkeys >= PYRA_KEY_B)?(keystate[PYRA_KEY_B]):0;
    KeyboardData[DIK_X] = (numkeys >= PYRA_KEY_Y)?(keystate[PYRA_KEY_Y]):0;
    KeyboardData[DIK_C] = (numkeys >= PYRA_KEY_X)?(keystate[PYRA_KEY_X]):0;
    KeyboardData[DIK_V] = (numkeys >= PYRA_KEY_A)?(keystate[PYRA_KEY_A]):0;
#else
    KeyboardData[DIK_UP] = (numkeys >= SDL_SCANCODE_UP)?(keystate[SDL_SCANCODE_UP]):0;
    KeyboardData[DIK_DOWN] = (numkeys >= SDL_SCANCODE_DOWN)?(keystate[SDL_SCANCODE_DOWN]):0;
    KeyboardData[DIK_LEFT] = (numkeys >= SDL_SCANCODE_LEFT)?(keystate[SDL_SCANCODE_LEFT]):0;
    KeyboardData[DIK_RIGHT] = (numkeys >= SDL_SCANCODE_RIGHT)?(keystate[SDL_SCANCODE_RIGHT]):0;

    KeyboardData[DIK_NUMPAD2] = (numkeys >= SDL_SCANCODE_KP_2)?(keystate[SDL_SCANCODE_KP_2]):0;
    KeyboardData[DIK_NUMPAD4] = (numkeys >= SDL_SCANCODE_KP_4)?(keystate[SDL_SCANCODE_KP_4]):0;
    KeyboardData[DIK_NUMPAD6] = (numkeys >= SDL_SCANCODE_KP_6)?(keystate[SDL_SCANCODE_KP_6]):0;
    KeyboardData[DIK_NUMPAD8] = (numkeys >= SDL_SCANCODE_KP_8)?(keystate[SDL_SCANCODE_KP_8]):0;

    KeyboardData[DIK_Z] = (numkeys >= SDL_SCANCODE_Z)?(keystate[SDL_SCANCODE_Z]):0;
    KeyboardData[DIK_X] = (numkeys >= SDL_SCANCODE_X)?(keystate[SDL_SCANCODE_X]):0;
    KeyboardData[DIK_C] = (numkeys >= SDL_SCANCODE_C)?(keystate[SDL_SCANCODE_C]):0;
    KeyboardData[DIK_V] = (numkeys >= SDL_SCANCODE_V)?(keystate[SDL_SCANCODE_V]):0;
#endif
}

void LunaInput::RefreshJoystick( void )
{
    if (JoystickCount)
    {
        SDL_JoystickUpdate();

        JoystickData[0].Data.Px = 2 * SDL_JoystickGetAxis(JoystickData[0].lpDevice, 0);
        JoystickData[0].Data.Py = 2 * SDL_JoystickGetAxis(JoystickData[0].lpDevice, 1);

#if !defined(PYRA)
        JoystickData[0].Data.Button[0] = SDL_JoystickGetButton(JoystickData[0].lpDevice, 0);
        JoystickData[0].Data.Button[1] = SDL_JoystickGetButton(JoystickData[0].lpDevice, 1);
        JoystickData[0].Data.Button[2] = SDL_JoystickGetButton(JoystickData[0].lpDevice, 2);
        JoystickData[0].Data.Button[3] = SDL_JoystickGetButton(JoystickData[0].lpDevice, 3);
#endif
    }
}

BOOL LunaInput::GetKeyData( unsigned long key )
{
    return KeyboardData[key] == 1;
}

const LPJOYSTICKDATA LunaInput::GetJoystickData( unsigned long no )
{
    if ( JoystickData[no].lpDevice != NULL )
    {
        return &(JoystickData[no].Data);
    }
    else
    {
        return NULL;
    }
}

