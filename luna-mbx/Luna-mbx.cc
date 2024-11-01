#include "Luna-mbx.h"

#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <cstring>
#if (defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__))
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#else
    #include <unistd.h>
    #include <sys/types.h>
    #include <pwd.h>

    #if defined(__GLIBC__) && __GLIBC_PREREQ(2,16)
        #include <sys/auxv.h>
    #endif
#endif
#include <SDL.h>


static BOOL bActiveWindow = TRUE;
static BOOL bWindow = TRUE;
static BOOL bShowMouse = TRUE;
static long Width = 640;
static long Height = 480;
static char Title[128] = "Luna";


static unsigned char S1 = 0;
static Uint32 BeforeSecond;
static char LogFile[256];
static BOOL bShowFPS;
static Uint32 OldTime;
static float FrameTime;
static long FrameRate;
static int FPS;
static int _FPS;
static int PPS;
static int _PPS;
static char Directory[256];
static BOOL bResize;
static long EnableAPI;
static unsigned long AppCounter;


static SDL_Window *Window;

static SDL_Cursor *DefaultCursor;
static SDL_Cursor *NoCursor;
static Uint8 NoCursorData;


int sdl_versionnum = 0;


void LogOut(const char *format, ...)
{
#if !defined(PANDORA) && !defined(PYRA)
    FILE *f;
    va_list va;

    if ( LogFile[0] != 0 )
    {
        va_start(va, format);
        f = fopen(LogFile, "at");
        if ( f != NULL )
        {
            vfprintf(f, format, va);
            fclose(f);
        }
        va_end(va);
    }
#endif
}


BOOL Luna::InitAPI( void )
{
    EnableAPI |= USE_DIRECT3D;

    puts(" -> Get SystemInformation...");
    GetSysInfo();
    //SDL_Delay(100);

    if ( EnableAPI & (USE_DIRECT3D /*| USE_MEDIASTREAM*/) )
    {
        puts(" -> Initialize DirectX Graphics...");
        if ( !Luna3D::Init() ) return FALSE;
        //LunaFont::Init();
        LunaEffect::Init();
        LunaSurface::Init();
        LunaTexture::Init();
        LunaSprite::Init();
        //LunaModel::Init();
        //SDL_Delay(100);
    }

    if ( EnableAPI & USE_DIRECTSOUND )
    {
        puts(" -> Initialize DirectSound...");
        LunaSound::Init();
        //SDL_Delay(100);
    }

    /*if ( EnableAPI & USE_DIRECTMUSIC )
    {
        puts(" -> Initialize DirectMusic...");
        LunaMusic::Init();
        //SDL_Delay(100);
    }*/

    if ( EnableAPI & USE_DIRECTINPUT )
    {
        puts(" -> Initialize DirectInput...");
        LunaInput::Init();
        //SDL_Delay(100);
    }

    /*if ( EnableAPI & USE_DIRECTPLAY )
    {
        puts(" -> Initialize DirectPlay...");
        LunaNetwork::Init();
        //SDL_Delay(100);
    }*/

    /*if ( EnableAPI & USE_MEDIASTREAM )
    {
        puts(" -> Initialize DirectShow for Video...");
        LunaMedia::Init();
        //SDL_Delay(100);
    }*/

    /*if ( EnableAPI & USE_AUDIOSTREAM )
    {
        puts(" -> Initialize DirectShow for Audio...");
        LunaAudio::Init();
        //SDL_Delay(100);
    }*/

    /*if ( EnableAPI & USE_FEP )
    {
        puts(" -> Initialize FEP System...");
        LunaFEP::Init();
        //SDL_Delay(100);
    }*/

    /*if ( EnableAPI & USE_CDDA )
    {
        puts(" -> Initialize MCI for CDDA...");
        LunaCD::DeviceOpen();
        //SDL_Delay(100);
    }*/

    /*if ( EnableAPI & USE_MIXER )
    {
        puts(" -> Initialize MixerDevice...");
        LunaMixer::DeviceOpen();
        //SDL_Delay(100);
    }*/

    /*puts(" -> Initialize Debug System...");
    LunaDebug::Init();
    //SDL_Delay(100);*/

    puts(" -> Initialize Etc Data...");
    LunaMath::Init();
    LunaGraphicLoader::Init();
    //SDL_Delay(100);

    puts("Initialize Complete!");
    //SDL_Delay(1000);

    return TRUE;
}

void Luna::UnInitAPI( void )
{
    //LunaDebug::UnInit();
    LunaMath::UnInit();
    LunaGraphicLoader::UnInit();
    //if ( EnableAPI & USE_MIXER ) LunaMixer::DeviceClose();
    //if ( EnableAPI & USE_CDDA ) LunaCD::DeviceClose();
    //if ( EnableAPI & USE_FEP ) LunaFEP::UnInit();
    //if ( EnableAPI & USE_AUDIOSTREAM ) LunaAudio::UnInit();
    //if ( EnableAPI & USE_MEDIASTREAM ) LunaMedia::UnInit();
    //if ( EnableAPI & USE_DIRECTPLAY ) LunaNetwork::UnInit();
    if ( EnableAPI & USE_DIRECTINPUT ) LunaInput::UnInit();
    //if ( EnableAPI & USE_DIRECTMUSIC ) LunaMusic::UnInit();
    if ( EnableAPI & USE_DIRECTSOUND ) LunaSound::UnInit();
    if ( EnableAPI & (USE_DIRECT3D /*| USE_MEDIASTREAM*/) )
    {
        //LunaModel::UnInit();
        LunaSprite::UnInit();
        LunaTexture::UnInit();
        LunaSurface::UnInit();
        LunaEffect::UnInit();
        //LunaFont::UnInit();
        Luna3D::UnInit();
    }
}

static void destroy_window(void)
{
    SDL_DestroyWindow(Window);
}

BOOL Luna::InitWindow( void )
{
    if ( SDL_Init (SDL_INIT_VIDEO | SDL_INIT_TIMER ) )
    {
        LogOut("  ERROR Failed SDL_Init: %s\n", SDL_GetError ());
        return FALSE;
    }

    atexit(SDL_Quit);

    SDL_version linked;
    SDL_GetVersion(&linked);
    sdl_versionnum = SDL_VERSIONNUM(linked.major, linked.minor, linked.patch);

    DefaultCursor = SDL_GetCursor();
    NoCursorData = 0;
    NoCursor = SDL_CreateCursor(&NoCursorData, &NoCursorData, 8, 1, 0, 0);

#ifdef USE_GLES
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    // workaround:
    // NVIDIA 64-bit linux driver version 525.147.05
    // ERROR Failed SDL_CreateWindow: unable to create an EGL window surface (call to eglCreateWindowSurface failed, reporting an error of EGL_BAD_ATTRIBUTE)
    SDL_setenv("EGL_EXT_present_opaque", "1", 1); // disable this extension
#endif

    if (bWindow)
    {
        Window = SDL_CreateWindow (Title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, Width, Height, SDL_WINDOW_OPENGL);
    }
    else
    {
        Window = SDL_CreateWindow (Title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 0, 0, SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN_DESKTOP);
    }
    if (Window == NULL)
    {
        LogOut("  ERROR Failed SDL_CreateWindow: %s\n", SDL_GetError ());
        return FALSE;
    }

    atexit(destroy_window);

    puts("■ Luna Ver. 2.55... 2002.07.12");
    puts(" -> Create Window...");

    return TRUE;
}

void Luna::WaitFrame( void )
{
    Uint32 NewTime;
    int TimeDiff;
    int WaitTime;
    char Caption[256];

    NewTime = SDL_GetTicks();
    TimeDiff = NewTime - OldTime;
    if ( FrameRate > 0 )
    {
        WaitTime = (1000 - TimeDiff * FrameRate) / FrameRate;
        if ( WaitTime > 0 ) SDL_Delay(WaitTime);
        NewTime = SDL_GetTicks();
        TimeDiff = NewTime - OldTime;
    }
    OldTime = NewTime;
    FrameTime = (double)(unsigned int)TimeDiff * 0.001;

    if ( !(S1 & 1) )
    {
        S1 |= 1;
        BeforeSecond = SDL_GetTicks();
    }

    if ( NewTime - BeforeSecond >= 1000 )
    {
        BeforeSecond = NewTime;
        FPS = _FPS;
        PPS = _PPS;
        _FPS = 0;
        _PPS = 0;

        if ( bShowFPS )
        {
            sprintf(Caption, "%s  FPS[%d] PPS[%d]", Title, FPS, PPS);
            SDL_SetWindowTitle(Window, Caption);
        }
    }
}

void Luna::GetSysInfo( void )
{
    LogOut("\n============================================== System Information\n");
    LogOut("  ■ Directory  : %s\n", Directory);
}

void Luna::AddFPS( long no )
{
    _FPS += no;
}

void Luna::AddPolygon( long num )
{
    _PPS += num;
}

int Luna::Drive2( int argc, char *argv[] )
{
    SDL_Event event;

#if (defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__))
    GetModuleFileNameA(NULL, Directory, 256);
#else
    const char *pathname;
#if defined(__GLIBC__) && __GLIBC_PREREQ(2,16)
    pathname = (const char *)getauxval(AT_EXECFN);
#else
    pathname = NULL;
#endif
    if (pathname != NULL)
    {
        strncpy(Directory, pathname, 255);
    }
    else
    {
        Directory[0] = '.';
        Directory[1] = 0;
    }
#endif

    int dirlen;
    dirlen = strlen(Directory);
    while ((Directory[dirlen] != '\\') && (Directory[dirlen] != '/'))
    {
        dirlen--;
        if (dirlen < 0) break;
    }
    if (dirlen >= 0)
    {
        Directory[dirlen] = 0;
    }

#if (defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__))
    SetCurrentDirectoryA(Directory);
#else
    chdir(Directory);
#endif

    SetInfo();


    //LunaSSTP::InitWinSock();
    if ( !InitWindow() ) Exit();

    OldTime = SDL_GetTicks();

    if ( InitAPI() )
    {
        LogOut("\n-------------------------->> Start Application <<--------------------------\n\n");

        Init();

        if ( !bShowMouse )
        {
            SDL_SetCursor(NoCursor);
        }

        OldTime = SDL_GetTicks();
        WaitFrame();

        if ( FrameRate >= 0 )
        {
            while ( 1 )
            {
                while ( !SDL_PollEvent(&event) )
                {
                    WaitFrame();

                    if ( EnableAPI & USE_DIRECTINPUT )
                    {
                        //LunaInput::RefreshMouse();
                        LunaInput::RefreshJoystick();
                        LunaInput::RefreshKeyboard();
                        //LunaInput::RefreshKeyboardBuffer();
                    }

                    if ( bActiveWindow )
                    {
                        Luna::MainLoop();
                        ++AppCounter;
                    }
                }

                if (event.type == SDL_QUIT) break;
                if ((event.type == SDL_KEYDOWN) && (event.key.keysym.sym == SDLK_ESCAPE)) break;
            }
        }
        else
        {
            while ( 1 )
            {
                if (SDL_WaitEvent(&event)) continue;
                if (event.type == SDL_QUIT) break;
                if ((event.type == SDL_KEYDOWN) && (event.key.keysym.sym == SDLK_ESCAPE)) break;

                if ( EnableAPI & USE_DIRECTINPUT )
                {
                    //LunaInput::RefreshMouse();
                    LunaInput::RefreshJoystick();
                    LunaInput::RefreshKeyboard();
                    //LunaInput::RefreshKeyboardBuffer();
                }
            }
        }

        LogOut("\n-------------------------->> Exit Application <<--------------------------\n\n");
        UnInit();
    }

    UnInitAPI();

    //LunaSSTP::UninitWinSock();

    if (NoCursor != NULL)
    {
        SDL_FreeCursor(NoCursor);
        NoCursor = NULL;
    }

    return 0;
}

void Luna::SetLogFile ( const char *file )
{
#if (defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__))
    if ( file[1] == ':' )
    {
        if (strlen(file) >= sizeof(LogFile))
        {
            LogFile[0] = 0;
            return;
        }
        strcpy(LogFile, file);
    }
    else if ( (file[0] == '\\') || ( file[0] == '/' ) )
    {
        if (snprintf(LogFile, sizeof(LogFile), "%c:%s", Directory[0], file) >= sizeof(LogFile))
        {
            LogFile[0] = 0;
            return;
        }
    }
    else
    {
        if (snprintf(LogFile, sizeof(LogFile), "%s\\%s", Directory, file) >= sizeof(LogFile))
        {
            LogFile[0] = 0;
            return;
        }
    }
#else
    if ( file[0] == '/' )
    {
        if (strlen(file) >= sizeof(LogFile))
        {
            LogFile[0] = 0;
            return;
        }
        strcpy(LogFile, file);
    }
    else if ( ( file[0] == '~' ) && ( file[1] == '/' ) )
    {
        const char *homedir;

        if ((homedir = getenv("HOME")) == NULL)
        {
            homedir = getpwuid(getuid())->pw_dir;
        }

        if (snprintf(LogFile, sizeof(LogFile), "%s/%s", homedir, &(file[2])) >= (int)sizeof(LogFile))
        {
            LogFile[0] = 0;
            return;
        }
    }
    else
    {
        if (snprintf(LogFile, sizeof(LogFile), "%s/%s", Directory, file) >= (int)sizeof(LogFile))
        {
            LogFile[0] = 0;
            return;
        }
    }
#endif

    remove(LogFile);
}

void Luna::SetUseOption( long flag )
{
    EnableAPI = flag;
}

void Luna::SetWindowTitleUtf8( const char *title )
{
    int len = strlen(title);
    if (len >= (int)sizeof(Title)) len = sizeof(Title) - 1;
    memcpy(Title, title, len);
    Title[len] = 0;
}

void Luna::SetScreenMode( long w, long h, BOOL window, BOOL resize )
{
    Width = w;
    Height = h;
    bResize = (resize != 0);
    if ( resize )
    {
        bWindow = 1;
    }
    else
    {
        bWindow = window;
    }
}

void Luna::SetShowMouse( BOOL flag )
{
    bShowMouse = flag;
}

void Luna::SetFPS( long fps )
{
    FrameRate = fps;
}

void Luna::SetWindowMode( BOOL flag )
{
    bWindow = flag;
}

void Luna::SetShowFPS( BOOL flag )
{
    bShowFPS = flag;
}

BOOL Luna::GetWindowMode( void )
{
    return bWindow;
}

void Luna::GetScreenSize( long *w, long *h )
{
    if (w != NULL) *w = Width;
    if (h != NULL) *h = Height;
}

void* Luna::GetWindowHandle( void )
{
    return Window;
}

unsigned long Luna::GetCounter( void )
{
    return AppCounter;
}

float Luna::GetFrameTime()
{
    return FrameTime;
}

unsigned long Luna::GetFPS( void )
{
    return FPS;
}

void Luna::MsgBox( const char *title, const char *string,... )
{
    char text[256];
    va_list va;

    va_start(va, string);
    vsprintf(text, string, va);
    va_end(va);

    //MessageBoxA(NULL, text, title, MB_ICONEXCLAMATION);
    fputs(title, stderr);
    fputs(text, stderr);
}

void Luna::Exit( void )
{
    UnInit();
    UnInitAPI();

    //LunaSSTP::UninitWinSock();

    if (NoCursor != NULL)
    {
        SDL_FreeCursor(NoCursor);
        NoCursor = NULL;
    }

    exit(0);
}

const char *Luna::GetDir( void )
{
    return &(Directory[0]);
}


int main(int argc, char *argv[])
{
    return Luna::Drive2(argc, argv);
}

