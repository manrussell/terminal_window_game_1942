// -------------------------------------------------------------
//|                                                             |
//|                Windows(tm) Console MegaShell V2             |
//|                                                             |
//| (c) Etanza Systems, 1998/2001/2004/2006/2007                |
//|  - Coded by Neil Crossley                                   |
//|                                                             |
//| At last, an oldskool style environment to run your code in  |
//| (just like the old DOS days).                               |
//|                                                             |
// -------------------------------------------------------------

// Set tabs to 2 to read neatly :)


// ----------
//| Includes |
// ----------

#define  WINVER 0x0500
#include <windows.h>
#include <stdio.h>
#include "tmdshell.h"


// ---------
//| Defines |
// ---------

#define WINAPP    1         // 0 = Console app, 1 = Windows app
#define CHUNKSIZE (8*MEGA)  // Chunk size for file read/write (fixes network issues)


// --------
//| Protos |
// --------

static void           init_console(int32 xs, int32 ys);
static void           keyread_thread(uint32 param);
static void           vblank_thread(uint32 threadparam);
static void CALLBACK  systemtimer(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);
static void           split_commandline(void);
static void           error(char* estring);
static uint32         cpu_id(void);
extern int32          appmain(int32 argc, char** argv);
static void           translate_dirent(WIN32_FIND_DATA* wfd, DIR_ENTRY* de);


// --------
//| Tables |
// --------

// Special Numpad key tables

static uint8 numpad_kc[16][2] = 
{
  {KEY_N0,KEY_INSERT},
  {KEY_N1,KEY_END},
  {KEY_N2,KEY_CRSRDN},
  {KEY_N3,KEY_PGDN},
  {KEY_N4,KEY_CRSRLF},
  {KEY_N5,KEY_CLEAR},
  {KEY_N6,KEY_CRSRRT},
  {KEY_N7,KEY_HOME},
  {KEY_N8,KEY_CRSRUP},
  {KEY_N9,KEY_PGUP},
  {KEY_NMULTIPLY,KEY_NMULTIPLY},
  {KEY_NPLUS,KEY_NPLUS},
  {KEY_NENTER,KEY_NENTER},
  {KEY_NMINUS,KEY_NMINUS},
  {KEY_NDPOINT,KEY_DELETE},
  {KEY_NDIVIDE,KEY_NDIVIDE},  
};

static uint8 numpad_ac[16][2] = 
{
  {'0',0},
  {'1',0},
  {'2',0},
  {'3',0},
  {'4',0},
  {'5',0},
  {'6',0},
  {'7',0},
  {'8',0},
  {'9',0},
  {'*','*'},
  {'+','+'},
  {0X0d,0X0d},
  {'-','-'},
  {'.',0},
  {'/','/'},
};

// Appstartup info

extern APPSTARTUPINFO appstartupinfo;


// ---------
//| Globals |
// ---------

static char                 appname[1024];        // Application name       
static char                 apppath[1024];        // Application directory
static HANDLE               kthandle;             // Keyboard reader Thread handle
static uint32               ktid;                 // Keyboard reader Thread ID
static HANDLE               h_stdout;             // Handle to STDOUT
static HANDLE               h_stdin;              // Handle to STDIN
static int32                rcxsize;              // Real X size of our console window 
static int32                rcysize;              // Real Y size of our console window
static uint32               rndseed = 0x1255f00d; // Random number generator seed
static int32                keybuffer_rd;         // read_key() reads from here
static int32                keybuffer_wr;         // thread writes here
static bool                 kb_ignore_numlock;    // TRUE to ignore numlock
static bool                 kb_msdos_enter;       // TRUE for the ASCII code of enter = 0x0d
static bool                 kb_no_repeat;         // TRUE to kill key repeat
static uint8                keytable[256];        // Keyboard state table, 0 = release, 1 = pressed
static uint32               keybuffer[256];       // Ringbuffer for read_key()
static bool                 mouse_enable;         // TRUE if mouse enabled
static MOUSEINFO            mouseinfo;            // Current mouse X/Y/Z and buttons
static APPSTARTUPINFO*      asi;                  // Appstartupinfo pointer
static uint8*               vramstore;            // Character memory (64K padding + 128K max charmap + 64K padding)
static uint8*               vram;                 // Pointer to Vram
static bool                 charmap_locked;       // TRUE if charmap locked (disables autocopy)
static uint8*               syscharmap;           // System character map (for writeconsolescreenbuffer)
static COORD                vbuffpos;             // XY position of blit source for writeconsoleoutput()
static COORD                vbuffsize;            // XY size of blit source      "    "  "      "     "
static SMALL_RECT           conrect;              // XYWH of blit destination    "    "  "      "     " 
static CONSOLE_CURSOR_INFO  ccistart;             // Console cursor info for restore at app exit
static VMODE_INFO           vm;                   // VMODE INFO
static MACHINE_INFO         mi;                   // MACHINE INFO
static int64                cpuspeed;             // In Hz
static int64                frametime;            // In Hz
static HANDLE               vblank_thread_handle; // Vblank drawing thread handle
static DWORD                vblank_thread_id;     // Vblank drawing thread ID
static void*                vblank_callback;      // Vblank function callback address
static uint32               vblank_timer_add;     // Vblank timer adder (12:20 fixed point)
static uint32               vblank_timer_acc;     // Vblank timer accumulator (12:20 fixed point)
static int32                vblank_timer_ticks;   // Vblank timer # of ticks
static volatile bool        vblank_in_thread;     // TRUE if vblank thread running
static int64                vblf;                 // TSC at last vsync
static int32                vblfc;                // Framecount at last vsync
static UINT                 timer_id;             // Timer ID
static volatile bool        timer_intimer;        // TRUE if inside timer handler
static uint32               timer_lasttick;       // Last timer tick
static int32                framecount;           // Current frame count
static void*                audio_callback;       // Pointer to audio callback routine
static uint8                attribute = 0x0f;     // Current textmode attribute
static int32                cursorx = 0;          // Current cursor X pos
static int32                cursory = 0;          // Current cursor Y pos
static uint32               conmode = 0;          // Console output mode flags
static int32                tabsize = 8;          // Tab size
static int32                cwidth;               // Console width
static int32                cwidthm1;             // Console width - 1
static int32                cheight;              // Console height
static int32                cheightm1;            // Console height - 1
static bool                 csrflash = false;     // Cursor flash flag
static int32                csrflashcount = 0;    // Cursor flash count


// ------------------------------------------------------------------------------------------------------
//|                                              MAIN                                                    |
// ------------------------------------------------------------------------------------------------------

// ---------
//| Winmain |
// ---------

// Just split the command line into elements and call main :)

#if WINAPP

static char   cmdline[32768];         // Buffer for commandline
static int32  argc;                   // Number of args
static char*  argv[256];              // Pointers to args
int32  main(int32 argc, char** argv); // Prototype for 'C' main

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  split_commandline();
  return(main(argc, &argv[0]));
}

#endif


// ------
//| Main |
// ------

int32 main(int32 argc, char** argv)
{
  // Locals

  char          tstr[1024];
  int32         sl,ts,te;
  int32         cx,cy,sr;
  uint32        nw;
  uint32        cpuflags;
  uint32        version,v;
  LARGE_INTEGER li;
  int32         err;
  int32         res;
  int32         i,j;

  // Get the path to the executable and the app name

  memfill(tstr, 0, sizeof(tstr));
  memfill(appname, 0, sizeof(appname));
  memfill(apppath, 0, sizeof(apppath));
  sl = GetModuleFileName(NULL, tstr, 1023);
  ts = 0;
  te = -1;
  for (i = sl; i; i--)
  {
    if (tstr[i] == '.') if (te == -1) te = i;
    if (tstr[i] == '\\') 
    {
      ts = i + 1;
      break;
    }
  }
  j=0;
  for (i = 0; i < ts; i++)  apppath[j++] = tstr[i];
  j=0;
  for (i = ts; i < te; i++) appname[j++] = tstr[i];

  // Quickly validate the startup parameters
  // - We'll use messagebox here as we dont
  //   have a console output handle yet!
  
  asi = &appstartupinfo;
  cx = asi->cxsize;
  cy = asi->cysize;
  sr = asi->appsyncrate;
  err = 0;

  if ((cx < 80) || (cx > 256)) err |= 1;
  if ((cy < 25) || (cy > 256)) err |= 2;
  if ((sr < 1) || (sr > 200)) err |= 4;
  if (err)
  {
    wsprintf(tstr,"Error in startup parameter(s) ...    \n\n");
    if (err & 1)  wsprintf(tstr, "%sClient X Size    =  %d  (80..256) \n", tstr, cx);
    if (err & 2)  wsprintf(tstr, "%sClient Y Size    =  %d  (25..256) \n", tstr, cy);
    if (err & 4)  wsprintf(tstr, "%sRefresh Rate  =  %d  (1..200) \n", tstr, sr);
    MessageBox(NULL, tstr, appname, MB_ICONSTOP);
    return(1);
  }

  // Initialise the console I/O

  init_console(cx, cy);

  // Bail if the console size is smaller than requested

  if (asi->flags & ASI_MUSTMATCHMINSIZE)
  {
    if ((rcxsize < cx) || (rcysize < cy))
    {
      wsprintf(tstr," This application requires a console\n"
                    " window which is at least %3d by %3d\n"
                    " characters.  Please adjust the size\n"
                    " of your console window and run this\n"
                    " application again.\n", cx, cy);
      error(tstr);
      return(-1);
    }
  }

  // If the console is larger than we asked for do we use the extra?

  if (asi->flags & ASI_USEREALSIZE)
  {
    cx = rcxsize;
    cy = rcysize;
  }

  // Allocate VRAM and Build the VMODE info structure

  memfill(&vm, 0, sizeof(VMODE_INFO));
  vramstore       = (uint8*)allocmem(256 * KILO);
  vram            = vramstore + (64 * KILO);
  syscharmap      = (uint8*)allocmem(260 * KILO);
  charmap_locked  = false;
  vm.width        = cx;
  vm.height       = cy;
  vm.bytespp      = 2;
  vm.vram_base    = vram;
  vm.vram_size    = cx * cy * 2;
  vm.refresh_rate = sr;
  vm.framecount   = 0;

  // Set the global width/height vars

  cwidth          = cx;
  cwidthm1        = cx - 1;
  cheight         = cy;
  cheightm1       = cy - 1;

  // Build the COORDS and RECT for the blitdown

  vbuffpos.X      = 0;
  vbuffpos.Y      = 0;
  vbuffsize.X     = (int16) cx;
  vbuffsize.Y     = (int16) cy;
  conrect.Left    = (int16) (rcxsize - cx) / 2;
  conrect.Top     = (int16) (rcysize - cy) / 2;
  conrect.Right   = (int16) (conrect.Left + cx) - 1;
  conrect.Bottom  = (int16) (conrect.Top + cy) - 1;

  // Get info about the CPU, check for minimum feature support

  err = 0;
  cpuflags = cpu_id();
  if (cpuflags == CPU_NOTPENTIUM) 
  {
    err = 1;
  }
  else
  {
    if (!(cpuflags & CPU_RDTSC)) err |= 2;
    if (!(cpuflags & CPU_CMOV))  err |= 4;
  }
    
  // Report any CPU errors found

  if (err != 0)
  {
    if (err == 1) wsprintf(tstr," This application requires at least a Pentium class CPU.\n");
    if (err == 2) wsprintf(tstr," This application requires a CPU with the RDTSC instruction.\n");
    if (err == 4) wsprintf(tstr," This application requires a CPU with the CMOV instruction.\n");
    if (err == 6) wsprintf(tstr," This application requires a CPU with the RDTSC and CMOV instructions.\n");
    error(tstr);
    return(-2);
  }

  // Now check we have high resolution timer support

  res = QueryPerformanceFrequency(&li);
  if (res == 0)
  {
    wsprintf(tstr," High resolution performance counter not available!\n");
    error(tstr);
    return(-3);
  }
  cpuspeed = (int64) li.QuadPart;

  // Now build the machine info structure

  version = GetVersion();
  if (version & 0x80000000) cpuflags |= CPU_WINDOWS9X;
  v = version & 0xffff;
  version           = ((v & 0xff) << 8) | (v >> 8);
  frametime         = cpuspeed / sr;
  mi.cpuflags       = (uint16) cpuflags;
  mi.windowsversion = (uint16) version;
  mi.cyclesperframe = (uint32) frametime;
  mi.app_percent    = 0;
  mi.blit_percent   = 0;
  
  // Create the 1000Hz timer and the vsync drawing thread

  vblank_thread_handle = CreateThread(NULL,512 * KILO, (LPTHREAD_START_ROUTINE)&vblank_thread, 0, CREATE_SUSPENDED, &vblank_thread_id);
  SetThreadPriority(vblank_thread_handle, THREAD_PRIORITY_HIGHEST);

  vblank_timer_add   = ((uint32)(sr)  << 20) / 1000;
  vblank_timer_acc   = 0;
  vblank_timer_ticks = 0;
  vblank_in_thread   = false;
  vblank_callback    = NULL;
  framecount         = 0;
  vblfc              = 0;
  vblf               = read_tsc64();

  timeBeginPeriod(1);
  timer_intimer  = false;
  timer_lasttick = timeGetTime();
  audio_callback = NULL;

  timer_id = timeSetEvent(1, 0, &systemtimer, 0, TIME_PERIODIC | TIME_CALLBACK_FUNCTION);

  // Clear the screen

  FillConsoleOutputAttribute(h_stdout, 0, rcxsize * rcysize, vbuffpos, &nw);

  // Jump to the users code

  res = appmain(argc, argv);

  // Minimal cleanup

  timeEndPeriod(1);
  timeKillEvent(timer_id);
  Sleep(2);
  sl = wsprintf(tstr,"\n\nHave a nice day :)\n");
  WriteConsole(h_stdout, tstr, sl, &nw, 0);
  SetConsoleCursorInfo(h_stdout, &ccistart);

  // Return result from application
  
  return(res);
}


// ------------------------------------------------------------------------------------------------------
//|                                            Console Functions                                         |
// ------------------------------------------------------------------------------------------------------

// --------------------
//| Initialise Console |
// --------------------

static void init_console(int32 xs, int32 ys)
{
  // Locals

  COORD                       size;
  COORD                       pos;
  HWND                        cHwnd;
  CONSOLE_SCREEN_BUFFER_INFO  csbi;
  CONSOLE_CURSOR_INFO         cci;
  RECT                        wa;
  WINDOWINFO                  wi;
  char                        randtitle[32];
  bool                        res;
  int32                       dx,dy,dw,dh;
  int32                       wx,wy,ww,wh;
  int32                       i;
  
  // Get size of desktop

  res = SystemParametersInfo(SPI_GETWORKAREA, 0, &wa, 0);
  if (res)
  {
    dw = wa.right - wa.left;
    dh = wa.bottom - wa.top;
    dx = wa.left;
    dy = wa.top;
  }
  else
  {
    dw = GetSystemMetrics(SM_CXSCREEN); 
    dh = GetSystemMetrics(SM_CYSCREEN); 
    dx = 0;
    dy = 0;
  }

  // If we are building a windows app then allocate a console

  #if WINAPP
  AllocConsole();
  #endif

  // Open a new handle to the the console output 
  // - Prevents redirection problems :)

  h_stdout = CreateFile("CONOUT$", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

  // Set the X and Y size of the console
  // - Thanks to Pelle for the tip :)

  if (xs < 80) xs = 80;
  if (ys < 25) ys = 25;
  size.X = (int16) xs; 
  size.Y = (int16) ys; 
  SetConsoleScreenBufferSize(h_stdout, size);
  

  // Find the window handle

  for (i = 0; i < 31; i++) randtitle[i] = (random() & 31) + 65;
  randtitle[31] = 0;
  SetConsoleTitle(randtitle);
  cHwnd = NULL;
  while(cHwnd == NULL)
  {
    Sleep(20);
    cHwnd = FindWindow(NULL, randtitle);
  }

  // Maximize the console window and center on screen
  //
  // - How do we find out if we are running from a windowed command
  //   prompt (coz if so we DONT want to do this!!

  SetWindowPos(cHwnd, 0, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_HIDEWINDOW);
  Sleep(50);
  SetWindowPos(cHwnd, 0, 0, 0, 9999, 9999, SWP_NOZORDER | SWP_NOMOVE | SWP_HIDEWINDOW);
  res = GetWindowInfo(cHwnd, &wi);
  if (res)
  {
    ww = wi.rcWindow.right  - wi.rcWindow.left;
    wh = wi.rcWindow.bottom - wi.rcWindow.top;
    wx = dx + ((dw - ww) / 2);
    wy = dy + ((dh - wh) / 2);
    SetWindowPos(cHwnd, HWND_TOP, wx, wy, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
  }

  // Auto Fullscreen
  
  if (asi->flags & ASI_STARTFULLSCREEN)
  {
    SendMessage(cHwnd, WM_SYSKEYDOWN, KEY_RETURN, 0x60000000);
    SendMessage(cHwnd, WM_SYSKEYUP, KEY_RETURN, 0xc0000000);
  }

  // Find out the width and height we were actually given!
  //
  // - Remember you can't set a smaller width and height than
  //   the window SO if you run from a 100*44 DOS shell and ask
  //   for 80*25 you'll still get 100*44

  GetConsoleScreenBufferInfo(h_stdout, &csbi);
  rcxsize = (csbi.srWindow.Right - csbi.srWindow.Left) + 1;
  rcysize = (csbi.srWindow.Bottom - csbi.srWindow.Top) + 1;

  // Set the window title to something sensible :)

  SetConsoleTitle(appname);

  // Get the current cursor settings

  GetConsoleCursorInfo(h_stdout, &ccistart);

  // Disable the cursor
  // - Why set the size to 100?
  //   coz if u dont the cursor is still visible
  //   in fulscreen mode!!

  cci.bVisible = false;
  cci.dwSize = 100;
  SetConsoleCursorInfo(h_stdout, &cci);

  // Put the cursor at the bottom of the screen

  pos.X = 0;
  pos.Y = rcysize - 1;
  SetConsoleCursorPosition(h_stdout, pos);

  // Now initialise the input

  h_stdin = CreateFile("CONIN$", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  SetConsoleMode(h_stdin, 0);  
  memfill(&keybuffer, 0, sizeof(keybuffer));
  memfill(&keytable, 0, sizeof(keytable));
  memfill(&mouseinfo, 0, sizeof(MOUSEINFO));
  mouse_enable = false;
  keybuffer_rd = 0;
  keybuffer_wr = 0; 
  kb_ignore_numlock = false;
  kb_msdos_enter = false;
  kb_no_repeat = false;
  kthandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&keyread_thread, 0, 0, &ktid);
  SetThreadPriority(kthandle, THREAD_PRIORITY_TIME_CRITICAL);
}


// ------------------------------------------------------------------------------------------------------
//|                                          Threads and Timer functions                                 |
// ------------------------------------------------------------------------------------------------------

// --------------
//| System timer |
// --------------

static void CALLBACK systemtimer(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
  // Locals

  int32   t,ticks;

  // Quit if this isn't our callback :)

  if (uID != timer_id) return;

  // Quit if we're already running
  // - I know we wont be, but its better to be safe than sorry!!

  if (InterlockedExchange((int32*)&timer_intimer,true) == true) return;

  // How many ticks since last call ??

  t = timeGetTime();
  ticks = t - timer_lasttick;
  if (ticks > 1000) ticks = 1000;
  timer_lasttick = t;

  // Cursor flash code

  csrflashcount += ticks;
  csrflash = csrflashcount & 0x100; 

  // Do we wake up the vblank thread?

  vblank_timer_acc += (vblank_timer_add * ticks);
  if (vblank_timer_acc > 0xfffff)
  {
    vblank_timer_acc &= 0xfffff;
    vblank_timer_ticks ++;
    if (!vblank_in_thread) ResumeThread(vblank_thread_handle);
  }
  
  // Service the audio!

  if (audio_callback != NULL) call_routine(audio_callback);

  // Clear the timer running flag

  InterlockedExchange((int32*) &timer_intimer, false);
}


// -----------------------
//| Vblank Drawing Thread |
// -----------------------

static void vblank_thread(uint32 threadparam)
{
  // Locals

  volatile  int32 sz;
  int32     cp;
  uint8     ca,na;
  int64     t;
  
  // Infinite loop ...

  while(1)
  {
    // Flag active

    InterlockedExchange((int32*)&vblank_in_thread,true);

    // Copy the user charmap to the screen

    if (!charmap_locked)
    {
      // Read TSC for profiling

      t = read_tsc64();

      // Translate user charmap to windows format

      sz = (cwidth * cheight) / 8;

      __asm
      {
            push  eax
            push  ecx
            push  esi
            push  edi
            
            mov   esi,vram
            mov   edi,syscharmap
            mov   ecx,sz
  
      lp:   mov   al,[esi +  0] 
            mov   ah,[esi +  1]
            mov   [edi +  0],al
            mov   [edi +  2],ah
            mov   al,[esi +  2]
            mov   ah,[esi +  3]
            mov   [edi +  4],al
            mov   [edi +  6],ah
            mov   al,[esi +  4]
            mov   ah,[esi +  5]
            mov   [edi +  8],al
            mov   [edi + 10],ah
            mov   al,[esi +  6]
            mov   ah,[esi +  7]
            mov   [edi + 12],al
            mov   [edi + 14],ah
            mov   al,[esi +  8]
            mov   ah,[esi +  9]
            mov   [edi + 16],al
            mov   [edi + 18],ah
            mov   al,[esi + 10]
            mov   ah,[esi + 11]
            mov   [edi + 20],al
            mov   [edi + 22],ah
            mov   al,[esi + 12]
            mov   ah,[esi + 13]
            mov   [edi + 24],al
            mov   [edi + 26],ah
            mov   al,[esi + 14]
            mov   ah,[esi + 15]
            mov   [edi + 28],al
            mov   [edi + 30],ah
            add   esi,16
            add   edi,32
            dec   ecx
            jnz   lp

            pop   edi
            pop   esi
            pop   ecx
            pop   eax
      }

      // Add the cursor

      if ((!(conmode & CMFLAG_NOCURSOR)) && csrflash)
      {
        cp = ((cursory * cwidth) + cursorx) * 4;
        ca = syscharmap[cp + 2];
        na = ((ca & 15) << 4) | (ca >> 4);
        syscharmap[cp + 2] = na;
      }

      // Write charmap to console window

      WriteConsoleOutput(h_stdout,(CHAR_INFO*)syscharmap,vbuffsize,vbuffpos,&conrect);

      // Calculate % cpu time used for copy

      t = read_tsc64() - t;
      mi.blit_percent = (int32)((t * 100) / frametime);
    }

    // Call vblank callback?

    if (vblank_callback != NULL) call_routine(vblank_callback);

    // Update frame counters

    framecount = vblank_timer_ticks;
    vm.framecount = framecount;

    // Flag finished and put ourselves to sleep

    InterlockedExchange((int32*)&vblank_in_thread,false);
    SuspendThread(vblank_thread_handle);
  }
}


// ----------------------
//| Keyboard read thread |
// ----------------------

// Dont even ask why this is how it is!

static void keyread_thread(uint32 param)
{
  // Locals
  
  INPUT_RECORD  inrec;
  uint32        rfcount;
  uint32        ctstate;
  bool          enhkey;
  bool          keydown;
  bool          qualkey;
  uint32        ac,kc,sc;   
  uint32        kcextra;
  uint32        kte;
  int32         wr;
  int32         s;
  
  // Code
  
  while(1)
  {
    if (ReadConsoleInput(h_stdin,&inrec,1,&rfcount));
    {
      if (rfcount)
      {
        switch(inrec.EventType)
        {
          case KEY_EVENT:
          {
            // Get The Key information
            
            ctstate = inrec.Event.KeyEvent.dwControlKeyState;
            enhkey  = (ctstate & 0x100) >> 8;
            keydown = inrec.Event.KeyEvent.bKeyDown;
            ac      = inrec.Event.KeyEvent.uChar.AsciiChar;
            kc      = inrec.Event.KeyEvent.wVirtualKeyCode; 
            sc      = inrec.Event.KeyEvent.wVirtualScanCode;
            
            // Bail if illegal keycode

            if (kc < 8)           break;
            if (kc == KEY_NENTER) break;

            // Fixup the left/right hand side keycodes and set the qualkey flag

            qualkey = false;
            kcextra = 0;
  
            switch(kc)
            {
              case KEY_SHIFT:
              {
                qualkey = true; 
                if (sc == 0x36) kcextra = KEY_RSHIFT; else kcextra = KEY_LSHIFT;
              }
              break;

              case KEY_CTRL:
              {
                qualkey = true;
                if (enhkey) kcextra = KEY_RCTRL; else kcextra = KEY_LCTRL;
              }
              break;

              case KEY_ALT:
              {
                qualkey = true;
                if (enhkey) kcextra = KEY_RALT; else kcextra = KEY_LALT;
              }
              break;
          
              case KEY_CAPSLOCK:
              {
                qualkey = true;               
              }
              break;

              case KEY_NUMLOCK:
              {
                qualkey = true;               
              }
              break;

              case KEY_SCRLLOCK:
              {
                qualkey = true;               
              }
              break;

              case KEY_RETURN:
              {
                if (enhkey) kc = KEY_NENTER;
              }
              break;
            }

            // Remap numlock altered keys back to sanity!

            if (!enhkey)
            {
              switch(kc)
              {
                case KEY_INSERT:
                  kc = KEY_N0;
                break;

                case KEY_DELETE:
                  kc = KEY_NDPOINT;
                break;

                case KEY_END:
                  kc = KEY_N1;
                break;

                case KEY_CRSRDN:
                  kc = KEY_N2;
                break;

                case KEY_PGDN:
                  kc = KEY_N3;
                break;

                case KEY_CRSRLF:
                  kc = KEY_N4;
                break;

                case KEY_CLEAR:
                  kc = KEY_N5;
                break;

                case KEY_CRSRRT:
                  kc = KEY_N6;
                break;

                case KEY_HOME:
                  kc = KEY_N7;
                break;

                case KEY_CRSRUP:
                  kc = KEY_N8;
                break;

                case KEY_PGUP:
                  kc = KEY_N9;
                break;
              }
            }

            // Update keytable

            if (keydown)
            {
              keytable[kc] = 1;
              if (kcextra) keytable[kcextra] = 1;
            }
            else
            {
              keytable[kc] = 0;
              if (kcextra) keytable[kcextra] = 0;
            }

            // Now build the shiftstate info
            // - and store to the first 2 bytes of the keytable

            kte = 0;

            if (keytable[KEY_SHIFT])  kte |= SSTATE_ANYSHIFT;
            if (keytable[KEY_CTRL])   kte |= SSTATE_ANYCTRL;
            if (keytable[KEY_ALT])    kte |= SSTATE_ANYALT;
            if (ctstate & 0x80)       kte |= SSTATE_CAPSLOCK;   
            if (ctstate & 0x20)       kte |= SSTATE_NUMLOCK;  
            if (ctstate & 0x40)       kte |= SSTATE_SCRLLOCK;   
            if (keytable[KEY_LSHIFT]) kte |= SSTATE_LSHIFT;
            if (keytable[KEY_RSHIFT]) kte |= SSTATE_RSHIFT;
            if (keytable[KEY_LCTRL])  kte |= SSTATE_LCTRL;
            if (keytable[KEY_RCTRL])  kte |= SSTATE_RCTRL;
            if (keytable[KEY_LALT])   kte |= SSTATE_LALT;
            if (keytable[KEY_RALT])   kte |= SSTATE_RALT;

            keytable[0] = (uint8)((kte >> 16) & 255);
            keytable[1] = (uint8)((kte >> 24) & 255);

            // If the key was released OR was a qualifier key then bail
    
            if ((qualkey) || (!keydown)) break;

            // If keyboard repeat is off and duplicate key then bail

            if (kb_no_repeat) if (keytable[kc] != 0) break;
          
            // Fixup the numpad keys (again)

            if ((kc >= KEY_N0) && (kc <= KEY_NDIVIDE))
            {
              // Flag key from numpad

              kte |= SSTATE_NUMPAD;

              // Fixup keycode and ascii depending on numlock state

              if (kb_ignore_numlock)
              {
                s = 0;
              }
              else
              {
                if (kte & SSTATE_NUMLOCK) s = 0; else s = 1;
              }

              kc = numpad_kc[kc - KEY_N0][s];
              ac = numpad_ac[kc - KEY_N0][s];
            }
  
            // Translate enter key depending on mode

            if ((ac == 0x0d) && (!kb_msdos_enter)) ac = 0x0a;

            // Build ringbuffer longword

            kte |= ((kc << 8) | ac);

            // Store key to ringbuffer
 
            wr = keybuffer_wr;
            keybuffer[wr] = (uint32)kte;
            wr ++;
            wr &= 255;
            keybuffer_wr = wr;
          }
          break;
                          
          case MOUSE_EVENT:
          {
            mouseinfo.buttons = inrec.Event.MouseEvent.dwButtonState & 0xff;
            mouseinfo.x = inrec.Event.MouseEvent.dwMousePosition.X;
            mouseinfo.y = inrec.Event.MouseEvent.dwMousePosition.Y;
            if (inrec.Event.MouseEvent.dwEventFlags & MOUSE_WHEELED)
            {
              if (inrec.Event.MouseEvent.dwButtonState & 0x80000000)
              {
                mouseinfo.z ++;
              }
              else
              {
                mouseinfo.z --;
              }
            }
          }
          break;
        }
      } 
    }
  }
}


// ------------------------------------------------------------------------------------------------------
//|                                            Subroutines                                               |
// ------------------------------------------------------------------------------------------------------

// -------------------
//| Split commandline |
// -------------------

#if WINAPP
static void split_commandline(void)
{
  // Locals

  char*       lpcmdline;
  int32       cmdlen;
  bool        doit;
  bool        findquote;
  bool        findstart;
  int32       sp;

  // Copy command line to local storage and get the length

  memfill(&cmdline,0,sizeof(cmdline));
  lpcmdline = GetCommandLine();
  cmdlen = stringlen(lpcmdline);
  if (cmdlen > (sizeof(cmdline)-1)) cmdlen = sizeof(cmdline)-1;
  memcopy(lpcmdline,&cmdline,cmdlen);
  
  // Clear ARGC and ARGV[]

  memfill(&argv,0,sizeof(argv));
  argc = 0;

  // Slice into bits ...

  doit = true;
  findstart = true;
  findquote = false;
  sp = 0;
  while(doit)
  {
    if (findstart)
    {
      if (cmdline[sp] == 34)
      {
        findquote = true;
        sp++;
      }
      else
      {
        findquote = false;
      }
      argv[argc] = &cmdline[sp];
      argc++;
      findstart = false;
    }
    else
    {
      if (findquote)
      {
        if (cmdline[sp] == 34)
        {
          cmdline[sp] = 0;
          sp++;
          findstart = true;
        }
      }
      else
      {
        if (cmdline[sp] == 32)
        {
          cmdline[sp] = 0;
          findstart = true;
        }
      }
      sp++;
    }
    if (sp >= cmdlen) doit = false; 
  }

}
#endif


// -------
//| Error |
// -------

// Issue an error message to the console window

static void error(char* estring)
{
  int32   sl;
  uint32  nw;
  char    err[256];
  SetConsoleCursorInfo(h_stdout,&ccistart);
  sl = wsprintf(err,"\n ** ERROR **\n\n");
  WriteConsole(h_stdout,err,sl,&nw,0);
  sl = stringlen(estring);
  WriteConsole(h_stdout,estring,sl,&nw,0);
  sl = wsprintf(err,"\n PRESS ANY KEY TO QUIT ... ");
  WriteConsole(h_stdout,err,sl,&nw,0);
  while(!keypressed()) sleep(1);
  sl = wsprintf(err,"\n");
  WriteConsole(h_stdout,err,sl,&nw,0);
}


// --------------
//| Identify CPU |
// --------------

static uint32 cpu_id(void)
{
  // Locals

  SYSTEM_INFO si;
  bool        intel     = false;
  bool        amd       = false;
  uint32      flags     = 0;
  uint32      flags2    = 0;
  uint32      cpuflags  = 0;

  // Can we use CPUID (Processor must be at least a Pentium 1)

  memfill(&si,0,sizeof(si));
  GetSystemInfo(&si);
  if (si.dwProcessorType != PROCESSOR_INTEL_PENTIUM) return(CPU_NOTPENTIUM);  

  // Now some Nasty ASM to get the flags ...

  __asm
  {
              pushad
              mov     eax,0             // Get Brand string
              cpuid
              cmp     ebx,0x756e6547    // check for Intel ("Genu")
              jne     nointel
              mov     intel,1
              jmp     getflags
    nointel:  cmp     ebx,0x68747541    // check for AMD ("Auth")
              jne     unknown
              mov     amd,1
              mov     eax,0x80000000    // check AMD extended feature flags
              cpuid
              cmp     eax,0x80000000
              jbe     getflags
              mov     eax,0x80000001
              cpuid
              mov     flags2,edx
    getflags: mov     eax,1             // get feature flags
              cpuid
              mov     flags,edx
    unknown:  popad

  }

  // Work out what we've got ...

  cpuflags = 0;

  if (!intel && !amd)        cpuflags |= CPU_UNKNOWN;
  if (intel)                 cpuflags |= CPU_INTEL;
  if (amd)                   cpuflags |= CPU_AMD;

  if (flags & 0x00800000)    cpuflags |= CPU_MMX;
  if (flags & 0x02000000)    cpuflags |= CPU_SSE;
  if (flags & 0x04000000)    cpuflags |= CPU_SSE2;
  if (flags & 0x00000010)    cpuflags |= CPU_RDTSC;
  if (flags & 0x00008000)    cpuflags |= CPU_CMOV;

  if (amd)
  {
    if (flags2 & 0x00400000) cpuflags |= CPU_MMX_EXTENDED;
    if (flags2 & 0x80000000) cpuflags |= CPU_3DNOW;
    if (flags2 & 0x40000000) cpuflags |= CPU_3DNOW_EXTENDED;
  }

  return(cpuflags);
}


// ------------------------------------------------------------------------------------------------------
//|                                            Text output functions                                     |
// ------------------------------------------------------------------------------------------------------

// ------------------
//| Set Console Mode |
// ------------------

void set_console_mode(uint32 mode)
{
  conmode = mode;
}


// ------------------
//| Get Console Mode |
// ------------------

uint32 get_console_mode(void)
{
  return(conmode);
}
  

// ------------------
//| Clear the screen |
// ------------------

void clrscr(void)
{
  uint8*  s = vm.vram_base;
  int32   i = cwidth * cheight;

  // Clear the screen

  while (i --) 
  {
    *s ++ = 32;
    *s ++ = attribute;
  }

  // Home the cursor

  cursorx = 0;
  cursory = 0;
}


// --------
//| GotoXY |
// --------

// Pass a negative value for either x or y to leave
// the position unchanged

void gotoxy(int32 x, int32 y)
{
  if (x >= 0)
  {
    if (x > cwidthm1) x = cwidthm1;
    cursorx = x;
  }

  if (y >= 0)
  {
    if (y > cheightm1) y = cheightm1;
    cursory = y;
  }
}


// -----------------
//| Return cursor X |
// -----------------

int32 wherex(void)
{
  return(cursorx);
}


// -----------------
//| Return cursor Y |
// -----------------

int32 wherey(void)
{
  return(cursory);
}


// ----------------------
//| Clear to end of line |
// ----------------------

void clreol(void)
{
  uint8* s = vram + ((cursory * cwidth) + cursorx) * 2;
  int32  i = cwidth - cursorx;
  if (i < 1) return;
  while (i --)
  {
    *s ++ = 32;
    *s ++ = attribute;
  }
}


// -----------------------
//| Set foreground colour |
// -----------------------

void setfgcolour(uint8 fg)
{
  attribute = (attribute & 0xf0) | (fg & 0x0f);
}


// -----------------------
//| Set background colour |
// -----------------------

void setbgcolour(uint8 bg)
{
  attribute = (attribute & 0x0f) | (bg << 4);
}


// ----------------------
//| Set attribute colour |
// ----------------------

void setattribute(uint8 attr)
{
  attribute = attr;
}


// ---------------
//| Get attribute |
// ---------------

uint8 getattribute(int32 which)
{
  switch(which)
  {
    case ATTR_FGCOL:      return(attribute & 0x0f);
    case ATTR_BGCOL:      return(attribute >> 4);
    case ATTR_ATTRIBUTE:  return(attribute);
    default:              return(0);
  }
}


// ------------------
//| Set the TAB size |
// ------------------

void settabsize(int32 size)
{
  tabsize = size;
}


// --------------------
//| Output a character |
// --------------------

// ALL print functions print by using this function.

void outchar(uint8 chr, uint8 attr, uint32 flags)
{
  // Locals

  uint8* s = vram + ((cursory * cwidth) + cursorx) * 2;

  // If the X or Y position is offscreen then bail
  // - Used by the nowrap options!

  if (cursorx > cwidthm1) return;
  if (cursory > cheightm1) return;

  // Use the supplied attribute?
  
  if (!(flags & OCFLAG_USEATTRIBUTE)) attr = attribute;

  // If translation is off then skip the next bit

  if (flags & OCFLAG_NOTRANSLATE) goto skipctrl;
  
  switch(chr)
  {
    // Bell

    case 7:
    {
      return;
    }
    break;

    // Backspace

    case 8:
    {
      cursorx --;
      if (cursorx < 0) cursorx = 0;
      #ifdef DESTRUCTIVE_BACKSPACE
      s[0] = 32;
      s[1] = attr;
      #endif
      return;
    } 
    break;

    // Tab

    case 9:
    {
      cursorx = tabsize * ((cursorx / tabsize) + 1);
      goto fixcursorx;
    }
    break;

    // Linefeed

    case 10:
    {
      if (!(conmode & CMFLAG_MSDOSLF)) cursorx = 0;
      cursory ++;
      goto fixcursory;
    }
    break;

    // Carriage Return

    case 13:
    {
      cursorx = 0;
      return;
    }
  }

  // Print the character

  skipctrl:

  s[0] = chr;
  s[1] = attr;
  cursorx ++;

  // Fix cursor X position

  fixcursorx:

  if (cursorx > cwidthm1)
  {
    // Quit if NOXWRAP

    if (conmode & CMFLAG_NOXWRAP) return;

    // Put cursor to start of line, increment ypos

    cursorx = 0;
    cursory++;
  }

  // Fix cursor Y position

  fixcursory:

  if (cursory > cheightm1)
  {
    // Check for no scroll

    if (conmode & CMFLAG_NOSCROLL)
    {
      // Quit if NOYWRAP

      if (conmode & CMFLAG_NOYWRAP) return;

      // Put cursor to top of screen and quit

      cursory = 0;
      return;
    }

    // Scroll the screen up and put cursor on bottom line

    scrollscreen(SCROLL_UP);
    cursory = cheightm1;
  }
}


// -----------------
//| Output a string |
// -----------------

void outstring(char* str, int32 len, uint32 flags)
{
  uint32  f = flags & 1;
  uint8   a,c;

  if (flags & OSFLAG_USELENGTH)
  {
    if (flags & OSFLAG_HASATTRIBUTES)
    {
      while(len --) 
      {
        c = *str++;
        a = *str++;
        outchar(c,a,f);
      }
    }
    else
    {
      while(len --) outchar(*str++,attribute,f);
    }
  }
  else
  {
    if (flags & OSFLAG_HASATTRIBUTES)
    {
      while(*str != 0) 
      {
        c = *str++;
        a = *str++;
        outchar(c,a,f);
      }
    }
    else
    {
      while(*str != 0) outchar(*str++,attribute,f);
    }
  }
}


// -------------------
//| Scroll the screen |
// -------------------

void scrollscreen(int32 how)
{
  // Locals

  int32   x,y,l,a;
  uint8*  s;
  uint8*  d;

  switch(how)
  {
    case SCROLL_UP:
    {
      d = vram;
      s = d + (cwidth * 2);
      y = cheightm1;
      while (y --)
      {
        x = cwidth * 2;
        while (x --) *d++ = *s++;
      }
      d = vram + (cheightm1 * cwidth * 2);
      x = cwidth * 2;
      while (x --) 
      {
        *d++ = 32;
        *d++ = attribute;
      }
    }
    break;
    
    case SCROLL_DOWN:
    {
      y = cheight;
      l = y;
      while (y--)
      {
        d = vram + (l -- * cwidth * 2);
        s = d - (cwidth * 2);
        x = cwidth * 2;
        while (x --) *d++ = *s++;
      }
      d = vram;
      x = cwidth;
      while (x --) 
      {
        *d++ = 32;
        *d++ = attribute;
      }
    }
    break;

    case SCROLL_LEFT:
    {
      y = cheight;
      l = 0;
      while (y --)
      {
        d = vram + (l * cwidth * 2);
        l ++;
        s = d + 2;
        x = cwidthm1 * 2;
        while (x --) *d++ = *s++;
      }
      a = cwidthm1 * 2;
      d = vram + a;
      y = cheight;
      while (y --)
      {
        *d++ = 32;
        *d++ = attribute;
        d += a;
      }
    }
    break;

    case SCROLL_RIGHT:
    {
      y = cheight;
      l = 0;
      while (y --)
      {
        d = vram + (l * cwidth * 2) + (cwidthm1 * 2);
        l ++;
        s = d - 2;
        x = cwidthm1 * 2;
        while (x --) *d-- = *s--;
      }
      a = cwidthm1 * 2;
      d = vram;
      y = cheight;
      while (y --)
      {
        *d++ = 0x20;
        *d++ = attribute;
        d += a;
      }
    }
    break;
  }
}


// -------
//| PRINT |
// -------

// No need for the va_start nonsense :)

int32 _print(char* fmt,...)
{
  char    buff[4096];
  int32   r;
  r = _vsnprintf(buff,4096,fmt,(char*)&fmt + 4);
  if (r > 4096) return(-1);
  outstring(buff,0,0);
  return(r);
}


// ------------------------------------------------------------------------------------------------------
//|                                            Library Functions                                         |
// ------------------------------------------------------------------------------------------------------

// ------------------
//| Get Machine Info |
// ------------------ 

MACHINE_INFO* get_machine_info(void)
{
  return(&mi);
}


// -----------------
//| Get System Time |
// ----------------- 

uint32 get_time(void)
{
  SYSTEMTIME  st;
  uint32      hour,min,sec,hund;
  GetLocalTime(&st);
  hour      = st.wHour;
  min       = st.wMinute;
  sec       = st.wSecond;
  hund      = st.wMilliseconds / 10;
  return((hour << 24) | (min << 16) | (sec << 8 ) | hund);
}


// -----------------
//| Get System Date |
// ----------------- 

uint32 get_date(void)
{
  SYSTEMTIME  st;
  uint32      year,month,date,day;
  GetLocalTime(&st);
  year      = st.wYear;
  month     = st.wMonth;
  date      = st.wDay;
  day       = st.wDayOfWeek;
  return((year << 16) | (month << 8) | (date << 3) | day);
}


// -------
//| Sleep |
// -------

void sleep(uint32 sleeptime)
{
  Sleep(sleeptime);
}


// ---------------------------
//| Get Application Directory |
// ---------------------------

char* get_app_directory(void)
{
  return(&apppath[0]);
}


// --------------------
//| Get videomode info |
// --------------------
                        
VMODE_INFO* get_videomode_info(void)
{
  return(&vm);
}


// -----------------------
//| Lock / Unlock charmap |
// -----------------------

void lock_charmap(bool lockstate)
{
  charmap_locked = lockstate;
}


// ---------------------
//| Set Vblank Callback |
// ---------------------

void set_vblank_callback(void* vbc)
{
  vblank_callback = vbc;
}


// -------
//| VSYNC |
// -------

// Returns # of frames elapsed since last call :)

int32 vsync(void)
{
  // Locals

  volatile int32  f;  
  uint32          r;
  int64           t;

  // Initialise profiling for app_percent

  t = (read_tsc64() - vblf);

  // Wait for the frame counter to increment

  f = framecount;
  while (framecount == f) Sleep(1);

  // Calc how many frames since last vsync

  f = framecount;
  r = f - vblfc;
  vblfc = f;

  // Calc %cpu time used

  mi.app_percent = (int32)((t * 100) / frametime);
  mi.vs_frames = r;
  vblf = read_tsc64();
  
  // Return framecount and leave ...

  return (r);
}    


// ----------------
//| Get Framecount |
// ----------------

int32 get_framecount(void)
{
  return(framecount);
}


// --------------------
//| Was a key pressed? |
// --------------------

bool keypressed(void)
{
  bool r;
  r = (keybuffer_rd != keybuffer_wr);
  return(r);
}                    


// ----------
//| Read Key |
// ----------

uint32 read_key(void)
{
  uint32 k;
  while(!keypressed()) Sleep(1);;
  k = keybuffer[keybuffer_rd];
  keybuffer_rd ++;
  keybuffer_rd &= 255;
  return(k);  
}


// ----------------------------
//| Return pointer to keytable |
// ----------------------------

uint8* get_keytable(void)
{
  return(&keytable[0]);
}


// --------------------
//| Return Shift State |
// --------------------

uint8 get_shiftstate(void)
{
  return(keytable[0]);
}


// -----------------------
//| Flush Keyboard Buffer |
// -----------------------

void flush_keybuffer(void)
{
  keybuffer_rd = 0;
  keybuffer_wr = 0;
  memfill(&keybuffer,0,sizeof(keybuffer));
}


// -------------------
//| Set Keyboard Mode |
// -------------------

void set_keyboard_mode(uint32 flags)
{
  if (flags & KBMODE_NO_REPEAT) kb_no_repeat = true; else kb_no_repeat = false;
  if (flags & KBMODE_FORCE_NUMLOCK) kb_ignore_numlock = true; else kb_ignore_numlock = false;
  if (flags & KBMODE_MSDOS_ENTER) kb_msdos_enter = true; else kb_msdos_enter = false;
}
 

// --------------
//| Enable Mouse |
// --------------

void enable_mouse(bool enable)
{
  if (mouse_enable == enable) return;
  memfill(&mouseinfo,0,sizeof(MOUSEINFO));
  if (enable)
  {
    SetConsoleMode(h_stdin,ENABLE_MOUSE_INPUT);
  }
  else
  {
    SetConsoleMode(h_stdin,0);
  }
}


// --------------------------
//| Return mouse information |
// --------------------------

void read_mouse(MOUSEINFO* mi)
{
  mi->buttons   = mouseinfo.buttons;
  mi->x         = mouseinfo.x;
  mi->y         = mouseinfo.y;
  mi->z         = mouseinfo.z;
  mouseinfo.z   = 0;
}


// -------------
//| Open a file |
// -------------

// Returns file hanlde (0 if error)
//
// - Fixed to prevent searches in all pathed directories :)

uint32 file_open(char* fname, uint32 flags)
{
  char  name[4096];
  bool  haspath = false;

  // Does the filename refer to an absolute path???
  
  if (fname[0] == '\\') haspath = true;   //  "\dir\something", "\\spoot\blah" etc
  if (fname[2] == '\\') haspath = true;   //  "c:\file.txt" 

  // If there is a path then just open the file ...

  if (haspath) return ((uint32)(mmioOpen(fname, NULL, flags & 3)));

  // If no path prepend '.\' in front of the name

  wsprintf(name, ".\\", fname);
  return ((uint32)(mmioOpen(fname, NULL, flags & 3)));
}


// ---------------
//| Create a file |
// ---------------

// Returns file handle (0 if error)

uint32 file_create(char* fname)
{
  return ((uint32)(mmioOpen(fname,NULL,MMIO_READWRITE | MMIO_CREATE)));  
}


// --------------
//| Close a file |
// --------------

// No return code

void file_close(uint32 handle)
{
  mmioClose((HMMIO)handle,0);
}


// ------
//| Seek |
// ------

// Returns new file position

uint32 file_seek(uint32 handle,int32 pos,uint32 mode)
{
  return ((uint32)(mmioSeek((HMMIO)handle,pos,mode)));
}


// ---------------------------
//| Get file pointer position |
// ---------------------------

// Returns file position

uint32 file_getpos(uint32 handle)
{
  return ((uint32)(mmioSeek((HMMIO)handle,0,SEEK_CURRENT)));
}


// -----------
//| Read data |
// -----------

// Returns # of bytes loaded (0 if error)

uint32 file_blockread(uint32 handle,void* address,uint32 loadsize)
{
  // Locals

  char*   addr;
  int32   loaded;
  int32   toload;
  int32   ls,r;
  bool    loadloop;

  // Initialise load loop

  addr     = address;
  toload   = loadsize;
  loaded   = 0;
  loadloop = true;

  // Load chunks until all done

  do
  {
    if (toload > CHUNKSIZE) 
    {
      ls = CHUNKSIZE; 
      toload -= CHUNKSIZE;
    }
    else
    {
      ls = toload;
      loadloop = false;
    }
    r = mmioRead((HMMIO)handle,addr,ls);
    if (r == -1) return(0);
    loaded += r;
    addr   += r;
  }
  while(loadloop);

  // Return # of bytes loaded

  return((uint32)loaded);
}


// ------------
//| Write data |
// ------------

// Returns # of bytes loaded (0 if error)

uint32 file_blockwrite(uint32 handle,void* address, uint32 savesize)
{
  // Locals

  char*   addr;
  int32   saved;
  int32   tosave;
  int32   ss,r;
  bool    saveloop;
  
  // Initialise save loop

  addr     = address;
  tosave   = savesize;
  saved    = 0;
  saveloop = true;

  // Save chunks until all done

  do
  {
    if (tosave > CHUNKSIZE) 
    {
      ss = CHUNKSIZE; 
      tosave -= CHUNKSIZE;
    }
    else
    {
      ss = tosave;
      saveloop = false;
    }
    r = mmioWrite((HMMIO)handle,addr,ss);
    if (r == -1) return(0);
    saved += r;
    addr  += r;
  }
  while(saveloop);

  // Return # of bytes saved

  return((uint32)saved);
}

// -------------------
//| Check file exists |
// -------------------

// Returns file hanlde (0 if error)

bool file_exists(char* fname)
{
  uint32 fh = (uint32)mmioOpen(fname,NULL,0);
  if (fh == 0) 
  {
    return(false);
  }
  else
  {
    mmioClose((HMMIO)fh,0);
    return(true);
  } 
}


// ---------------
//| Get file size |
// ---------------

// Returns 0 if any error

uint32 file_size(char* fname)
{
  uint32 tmp,len;
  if ((tmp=file_open(fname,FILE_READONLY)) == 0 ) return(0);
  len=file_seek(tmp,0,SEEK_END);
  file_close(tmp);
  return(len);
}


// -------------------
//| Load a whole file |
// -------------------

// Returns # of bytes loaded (0 if error)

uint32 load_data(char* fname, void* address, uint32 loadsize)
{
  uint32 tmp,lsize;
  lsize = loadsize;
  if (loadsize == -1) lsize = file_size(fname);
  if ((tmp = file_open(fname,FILE_READONLY)) == 0) return(0);
  lsize = file_blockread(tmp,address,lsize);
  file_close(tmp);
  return(lsize);
}


// -------------
//| Save a file |
// -------------

// Returns # of bytes written (0 if error)

uint32 save_data(char* fname,void* address, uint32 savesize)
{
  uint32 tmp,ssize;
  if ((tmp = file_create(fname)) == 0) return(0);
  ssize = file_blockwrite(tmp,address,savesize);
  file_close(tmp);
  return (ssize);
}

// -----------------------
//| Get current directory |
// -----------------------

char* get_directory(void)
{
  static char curr_path[4096];
  memfill(&curr_path,0,sizeof(curr_path));
  GetCurrentDirectory(sizeof(curr_path)-2,(char*)&curr_path);
  curr_path[stringlen(curr_path)]=0x5c;
  return ((char*)&curr_path);
}


// -----------------------
//| Set Current Directory |
// -----------------------

bool set_directory(char* dirname)
{
  return ((bool)(SetCurrentDirectory(dirname)));
}
                                  

// ------------------
//| Create Directory |
// ------------------

bool create_directory(char* dirname)
{
  return ((bool)(CreateDirectory(dirname,NULL)));
}


// ------------------
//| Remove Directory |
// ------------------

bool remove_directory(char* dirname)
{
  return ((bool)(RemoveDirectory(dirname)));
}


// -----------------
//| Find First File |
// -----------------

// Returns handle for findnext or 0 if any error (eg directory not found)

uint32 file_findfirst(char* findstr, DIR_ENTRY* de)
{
  WIN32_FIND_DATA wfd;
  HANDLE          wh;
  memfill(de, 0, sizeof(DIR_ENTRY));
  wh = FindFirstFile((char*)findstr, &wfd);
  if (wh == INVALID_HANDLE_VALUE) return(0);
  translate_dirent(&wfd, de);
  return ((uint32)wh);
}
 

// ----------------
//| Find Next File |
// ----------------

// Returns false if there are no more files

bool file_findnext(uint32 handle, DIR_ENTRY* de)
{
  WIN32_FIND_DATA wfd;
  bool            res;
  memfill(de, 0, sizeof(DIR_ENTRY));
  res = FindNextFile((HANDLE)handle, &wfd);
  if (res == false) return(false);
  translate_dirent(&wfd, de);
  return(true);
}


// -----------------
//| Close file find |
// -----------------

void file_findclose(uint32 handle)
{
  FindClose((HANDLE)handle);
}


// -----------------
//| Allocate Memory |
// -----------------

void* allocmem(uint32 size)
{
  void*   addr;
  bool    clear = (size >> 31) ^ 1;
  uint32  sz = size & 0x7fffffff;
  addr = VirtualAlloc(NULL, sz, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
  if (addr != NULL) 
  {
    if (clear) memfill(addr, 0, size);
  }
  return(addr);
}


// -------------
//| Free Memory |
// -------------

void freemem(void* addr)
{
  VirtualFree(addr, 0, MEM_RELEASE);
}


// --------------
//| Fast MEMCOPY |
// --------------

void memcopy(void* src, void* dst, uint32 count)
{
  __asm 
  {
          pushfd
          push    ebx
          push    esi
          push    edi
          cld
          mov     esi,src
          mov     edi,dst
          mov     ecx,count
          mov     ebx,ecx
          and     ebx,3
          shr     ecx,2
          rep     movsd
          mov     ecx,ebx
          rep     movsb
          pop     edi
          pop     esi
          pop     ebx
          popfd
  }
}


// --------------------
//| Fast MEM byte fill |
// --------------------

void memfill(void* dst, uint8 fill, uint32 count)
{
  __asm 
  {
          pushfd
          push    ebx
          push    esi
          push    edi
          cld
          mov     edi,dst
          mov     ecx,count
          mov     ebx,ecx
          and     ebx,3
          shr     ecx,2
          mov     dl,fill
          mov     al,dl
          mov     ah,dl
          shl     eax,16
          mov     al,dl
          mov     ah,dl
          rep     stosd
          mov     ecx,ebx
          rep     stosb
          pop     edi
          pop     esi
          pop     ebx
          popfd
  }
}


// ------------------
//| Fast MEM compare |
// ------------------

bool memcompare(void* src, void* dst, uint32 count)
{
  bool  result;
  __asm
  {
          pushfd
          push    esi
          push    edi
          cld
          mov     result,true
          mov     esi,src
          mov     edi,dst
          mov     ecx,count
          inc     ecx
          repe    cmpsb
          jecxz   found
          mov     result,false
  found:  pop     edi
          pop     esi
          popfd
  }
  return(result);
}


// ----------------------------
//| Set random generator seeed |
// ----------------------------

// Set random seed

void randomize(uint32 rndval)
{
  rndseed = rndval;
}


// ------------------------------------
//| Generate 32bit psuedo random value |
// ------------------------------------

// In Assembler for speed :)

uint32 __declspec(naked) random(void)
{
  __asm
  {
          mov     eax,rndseed
          imul    eax,0x41c64e6d
          add     eax,0x3039
          mov     rndseed,eax
          rol     eax,16
          ret
  }
}


// ----------------
//| Call a routine |
// ----------------

void call_routine(void* routaddr)
{
  __asm 
  {
          pushad
          mov     ebx,routaddr
          call    ebx
          popad
  }
}


// ------------------------
//| Read Timestamp Counter |
// ------------------------

int64 read_tsc64(void)
{
  LARGE_INTEGER li;
  QueryPerformanceCounter(&li);
  return((int64)li.QuadPart);
}


// -------------------------
//| Calculate String Length |
// -------------------------

int32 stringlen(char* str)
{
  int32 l = 0;
  while (*str++) l++;
  return(l);
}


// ----------------
//| Hook audio IRQ |
// ----------------

void set_audio_callback(void* cb_addr)
{
  audio_callback = cb_addr;
}


// ------------------------------------------------------------------------------------------------------
//|                                       Private Library Support Functions                              |
// ------------------------------------------------------------------------------------------------------

// --------------------
//| Translate Win->E3K |
// --------------------

// Translate a windows directory entry into an Etanza E3K entry

static void translate_dirent(WIN32_FIND_DATA* wfd, DIR_ENTRY* de)
{
  // Locals

  uint32      a;
  FILETIME    ft;
  SYSTEMTIME  st;
  bool        r;
  uint32      year,month,date,day;
  uint32      hour,min,sec,hund;
  int32       l;
  uint32      flags;
  bool        execute;
  bool        shellcmd;
  uint32      execheck;

  // First we demangle the file time and date

  de->date = 0x07d0010e; //Saturday January 1st 2000
  de->time = 0x00000000; //Midnight (00:00:00.00)
  r = FileTimeToLocalFileTime(&wfd->ftLastWriteTime, &ft);
  if (r)
  {
    r = FileTimeToSystemTime(&ft,&st);
    if (r)
    {
      year      = st.wYear;
      month     = st.wMonth;
      date      = st.wDay;
      day       = st.wDayOfWeek;
      de->date  = (year << 16) | (month << 8) | (date << 3) | day;
      hour      = st.wHour;
      min       = st.wMinute;
      sec       = st.wSecond;
      hund      = st.wMilliseconds / 10;
      de->time  = (hour << 24) | (min << 16) | (sec << 8 ) | hund;
    }
  }

  // Now we copy in the name

  l = stringlen(wfd->cFileName);
  // Really just a sanity check as windows names cant be this long :)
  if (l > 479) l = 479;               
  memfill(de->name, 0, 480);
  memcopy(wfd->cFileName, de->name, l);

  // Check if the file is executable 
  // - At the moment we check for .com / .exe / .bat

  shellcmd  = false;
  execute   = false;
  execheck  = (de->name[l-3] * 65536) + (de->name[l-2] * 256) + de->name[l-1];
  execheck |= 0x202020;       // make lowercase!
  if (execheck == 0x636f6d)   execute = true;                     // "com"
  if (execheck == 0x657865)   execute = true;                     // "exe"
  if (execheck == 0x626174) { execute = true; shellcmd = true; }  // "bat"

  // Finally we translate the attributes and size ...
  // - We emulate E3K flags as fully as possible ...

  a = wfd->dwFileAttributes;
  flags = DIR_READABLE;
  if (!(a & FILE_ATTRIBUTE_READONLY)) flags |= DIR_WRITABLE | DIR_DELETABLE;
  if (execute)                        flags |= DIR_EXECUTABLE;
  if (a & FILE_ATTRIBUTE_HIDDEN)      flags |= DIR_INVISIBLE;
  if (a & FILE_ATTRIBUTE_SYSTEM)      flags |= DIR_SYSTEM;
  if (a & FILE_ATTRIBUTE_DIRECTORY)   flags |= DIR_DIRECTORY;
  if (a & FILE_ATTRIBUTE_ARCHIVE)     flags |= DIR_ARCHIVE;
  if (shellcmd)                       flags |= DIR_SHELLCMD;
  de->flags = flags; 
  de->size  = wfd->nFileSizeLow;
  de->sizehigh = wfd->nFileSizeHigh;
}







// ------------------------------------------------------------------------------------------------------
//|                                        The End, Thanks for Reading :)                                |
// ------------------------------------------------------------------------------------------------------
=======
// -------------------------------------------------------------
//|                                                             |
//|                Windows(tm) Console MegaShell V2             |
//|                                                             |
//| (c) Etanza Systems, 1998/2001/2004/2006/2007                |
//|  - Coded by Neil Crossley                                   |
//|                                                             |
//| At last, an oldskool style environment to run your code in  |
//| (just like the old DOS days).                               |
//|                                                             |
// -------------------------------------------------------------

// Set tabs to 2 to read neatly :)


// ----------
//| Includes |
// ----------

#define  WINVER 0x0500
#include <windows.h>
#include <stdio.h>
#include "tmdshell.h"


// ---------
//| Defines |
// ---------

#define WINAPP    1         // 0 = Console app, 1 = Windows app
#define CHUNKSIZE (8*MEGA)  // Chunk size for file read/write (fixes network issues)


// --------
//| Protos |
// --------

static void           init_console(int32 xs, int32 ys);
static void           keyread_thread(uint32 param);
static void           vblank_thread(uint32 threadparam);
static void CALLBACK  systemtimer(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);
static void           split_commandline(void);
static void           error(char* estring);
static uint32         cpu_id(void);
extern int32          appmain(int32 argc, char** argv);
static void           translate_dirent(WIN32_FIND_DATA* wfd, DIR_ENTRY* de);


// --------
//| Tables |
// --------

// Special Numpad key tables

static uint8 numpad_kc[16][2] = 
{
  {KEY_N0,KEY_INSERT},
  {KEY_N1,KEY_END},
  {KEY_N2,KEY_CRSRDN},
  {KEY_N3,KEY_PGDN},
  {KEY_N4,KEY_CRSRLF},
  {KEY_N5,KEY_CLEAR},
  {KEY_N6,KEY_CRSRRT},
  {KEY_N7,KEY_HOME},
  {KEY_N8,KEY_CRSRUP},
  {KEY_N9,KEY_PGUP},
  {KEY_NMULTIPLY,KEY_NMULTIPLY},
  {KEY_NPLUS,KEY_NPLUS},
  {KEY_NENTER,KEY_NENTER},
  {KEY_NMINUS,KEY_NMINUS},
  {KEY_NDPOINT,KEY_DELETE},
  {KEY_NDIVIDE,KEY_NDIVIDE},  
};

static uint8 numpad_ac[16][2] = 
{
  {'0',0},
  {'1',0},
  {'2',0},
  {'3',0},
  {'4',0},
  {'5',0},
  {'6',0},
  {'7',0},
  {'8',0},
  {'9',0},
  {'*','*'},
  {'+','+'},
  {0X0d,0X0d},
  {'-','-'},
  {'.',0},
  {'/','/'},
};

// Appstartup info

extern APPSTARTUPINFO appstartupinfo;


// ---------
//| Globals |
// ---------

static char                 appname[1024];        // Application name       
static char                 apppath[1024];        // Application directory
static HANDLE               kthandle;             // Keyboard reader Thread handle
static uint32               ktid;                 // Keyboard reader Thread ID
static HANDLE               h_stdout;             // Handle to STDOUT
static HANDLE               h_stdin;              // Handle to STDIN
static int32                rcxsize;              // Real X size of our console window 
static int32                rcysize;              // Real Y size of our console window
static uint32               rndseed = 0x1255f00d; // Random number generator seed
static int32                keybuffer_rd;         // read_key() reads from here
static int32                keybuffer_wr;         // thread writes here
static bool                 kb_ignore_numlock;    // TRUE to ignore numlock
static bool                 kb_msdos_enter;       // TRUE for the ASCII code of enter = 0x0d
static bool                 kb_no_repeat;         // TRUE to kill key repeat
static uint8                keytable[256];        // Keyboard state table, 0 = release, 1 = pressed
static uint32               keybuffer[256];       // Ringbuffer for read_key()
static bool                 mouse_enable;         // TRUE if mouse enabled
static MOUSEINFO            mouseinfo;            // Current mouse X/Y/Z and buttons
static APPSTARTUPINFO*      asi;                  // Appstartupinfo pointer
static uint8*               vramstore;            // Character memory (64K padding + 128K max charmap + 64K padding)
static uint8*               vram;                 // Pointer to Vram
static bool                 charmap_locked;       // TRUE if charmap locked (disables autocopy)
static uint8*               syscharmap;           // System character map (for writeconsolescreenbuffer)
static COORD                vbuffpos;             // XY position of blit source for writeconsoleoutput()
static COORD                vbuffsize;            // XY size of blit source      "    "  "      "     "
static SMALL_RECT           conrect;              // XYWH of blit destination    "    "  "      "     " 
static CONSOLE_CURSOR_INFO  ccistart;             // Console cursor info for restore at app exit
static VMODE_INFO           vm;                   // VMODE INFO
static MACHINE_INFO         mi;                   // MACHINE INFO
static int64                cpuspeed;             // In Hz
static int64                frametime;            // In Hz
static HANDLE               vblank_thread_handle; // Vblank drawing thread handle
static DWORD                vblank_thread_id;     // Vblank drawing thread ID
static void*                vblank_callback;      // Vblank function callback address
static uint32               vblank_timer_add;     // Vblank timer adder (12:20 fixed point)
static uint32               vblank_timer_acc;     // Vblank timer accumulator (12:20 fixed point)
static int32                vblank_timer_ticks;   // Vblank timer # of ticks
static volatile bool        vblank_in_thread;     // TRUE if vblank thread running
static int64                vblf;                 // TSC at last vsync
static int32                vblfc;                // Framecount at last vsync
static UINT                 timer_id;             // Timer ID
static volatile bool        timer_intimer;        // TRUE if inside timer handler
static uint32               timer_lasttick;       // Last timer tick
static int32                framecount;           // Current frame count
static void*                audio_callback;       // Pointer to audio callback routine
static uint8                attribute = 0x0f;     // Current textmode attribute
static int32                cursorx = 0;          // Current cursor X pos
static int32                cursory = 0;          // Current cursor Y pos
static uint32               conmode = 0;          // Console output mode flags
static int32                tabsize = 8;          // Tab size
static int32                cwidth;               // Console width
static int32                cwidthm1;             // Console width - 1
static int32                cheight;              // Console height
static int32                cheightm1;            // Console height - 1
static bool                 csrflash = false;     // Cursor flash flag
static int32                csrflashcount = 0;    // Cursor flash count


// ------------------------------------------------------------------------------------------------------
//|                                              MAIN                                                    |
// ------------------------------------------------------------------------------------------------------

// ---------
//| Winmain |
// ---------

// Just split the command line into elements and call main :)

#if WINAPP

static char   cmdline[32768];         // Buffer for commandline
static int32  argc;                   // Number of args
static char*  argv[256];              // Pointers to args
int32  main(int32 argc, char** argv); // Prototype for 'C' main

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  split_commandline();
  return(main(argc, &argv[0]));
}

#endif


// ------
//| Main |
// ------

int32 main(int32 argc, char** argv)
{
  // Locals

  char          tstr[1024];
  int32         sl,ts,te;
  int32         cx,cy,sr;
  uint32        nw;
  uint32        cpuflags;
  uint32        version,v;
  LARGE_INTEGER li;
  int32         err;
  int32         res;
  int32         i,j;

  // Get the path to the executable and the app name

  memfill(tstr, 0, sizeof(tstr));
  memfill(appname, 0, sizeof(appname));
  memfill(apppath, 0, sizeof(apppath));
  sl = GetModuleFileName(NULL, tstr, 1023);
  ts = 0;
  te = -1;
  for (i = sl; i; i--)
  {
    if (tstr[i] == '.') if (te == -1) te = i;
    if (tstr[i] == '\\') 
    {
      ts = i + 1;
      break;
    }
  }
  j=0;
  for (i = 0; i < ts; i++)  apppath[j++] = tstr[i];
  j=0;
  for (i = ts; i < te; i++) appname[j++] = tstr[i];

  // Quickly validate the startup parameters
  // - We'll use messagebox here as we dont
  //   have a console output handle yet!
  
  asi = &appstartupinfo;
  cx = asi->cxsize;
  cy = asi->cysize;
  sr = asi->appsyncrate;
  err = 0;

  if ((cx < 80) || (cx > 256)) err |= 1;
  if ((cy < 25) || (cy > 256)) err |= 2;
  if ((sr < 1) || (sr > 200)) err |= 4;
  if (err)
  {
    wsprintf(tstr,"Error in startup parameter(s) ...    \n\n");
    if (err & 1)  wsprintf(tstr, "%sClient X Size    =  %d  (80..256) \n", tstr, cx);
    if (err & 2)  wsprintf(tstr, "%sClient Y Size    =  %d  (25..256) \n", tstr, cy);
    if (err & 4)  wsprintf(tstr, "%sRefresh Rate  =  %d  (1..200) \n", tstr, sr);
    MessageBox(NULL, tstr, appname, MB_ICONSTOP);
    return(1);
  }

  // Initialise the console I/O

  init_console(cx, cy);

  // Bail if the console size is smaller than requested

  if (asi->flags & ASI_MUSTMATCHMINSIZE)
  {
    if ((rcxsize < cx) || (rcysize < cy))
    {
      wsprintf(tstr," This application requires a console\n"
                    " window which is at least %3d by %3d\n"
                    " characters.  Please adjust the size\n"
                    " of your console window and run this\n"
                    " application again.\n", cx, cy);
      error(tstr);
      return(-1);
    }
  }

  // If the console is larger than we asked for do we use the extra?

  if (asi->flags & ASI_USEREALSIZE)
  {
    cx = rcxsize;
    cy = rcysize;
  }

  // Allocate VRAM and Build the VMODE info structure

  memfill(&vm, 0, sizeof(VMODE_INFO));
  vramstore       = (uint8*)allocmem(256 * KILO);
  vram            = vramstore + (64 * KILO);
  syscharmap      = (uint8*)allocmem(260 * KILO);
  charmap_locked  = false;
  vm.width        = cx;
  vm.height       = cy;
  vm.bytespp      = 2;
  vm.vram_base    = vram;
  vm.vram_size    = cx * cy * 2;
  vm.refresh_rate = sr;
  vm.framecount   = 0;

  // Set the global width/height vars

  cwidth          = cx;
  cwidthm1        = cx - 1;
  cheight         = cy;
  cheightm1       = cy - 1;

  // Build the COORDS and RECT for the blitdown

  vbuffpos.X      = 0;
  vbuffpos.Y      = 0;
  vbuffsize.X     = (int16) cx;
  vbuffsize.Y     = (int16) cy;
  conrect.Left    = (int16) (rcxsize - cx) / 2;
  conrect.Top     = (int16) (rcysize - cy) / 2;
  conrect.Right   = (int16) (conrect.Left + cx) - 1;
  conrect.Bottom  = (int16) (conrect.Top + cy) - 1;

  // Get info about the CPU, check for minimum feature support

  err = 0;
  cpuflags = cpu_id();
  if (cpuflags == CPU_NOTPENTIUM) 
  {
    err = 1;
  }
  else
  {
    if (!(cpuflags & CPU_RDTSC)) err |= 2;
    if (!(cpuflags & CPU_CMOV))  err |= 4;
  }
    
  // Report any CPU errors found

  if (err != 0)
  {
    if (err == 1) wsprintf(tstr," This application requires at least a Pentium class CPU.\n");
    if (err == 2) wsprintf(tstr," This application requires a CPU with the RDTSC instruction.\n");
    if (err == 4) wsprintf(tstr," This application requires a CPU with the CMOV instruction.\n");
    if (err == 6) wsprintf(tstr," This application requires a CPU with the RDTSC and CMOV instructions.\n");
    error(tstr);
    return(-2);
  }

  // Now check we have high resolution timer support

  res = QueryPerformanceFrequency(&li);
  if (res == 0)
  {
    wsprintf(tstr," High resolution performance counter not available!\n");
    error(tstr);
    return(-3);
  }
  cpuspeed = (int64) li.QuadPart;

  // Now build the machine info structure

  version = GetVersion();
  if (version & 0x80000000) cpuflags |= CPU_WINDOWS9X;
  v = version & 0xffff;
  version           = ((v & 0xff) << 8) | (v >> 8);
  frametime         = cpuspeed / sr;
  mi.cpuflags       = (uint16) cpuflags;
  mi.windowsversion = (uint16) version;
  mi.cyclesperframe = (uint32) frametime;
  mi.app_percent    = 0;
  mi.blit_percent   = 0;
  
  // Create the 1000Hz timer and the vsync drawing thread

  vblank_thread_handle = CreateThread(NULL,512 * KILO, (LPTHREAD_START_ROUTINE)&vblank_thread, 0, CREATE_SUSPENDED, &vblank_thread_id);
  SetThreadPriority(vblank_thread_handle, THREAD_PRIORITY_HIGHEST);

  vblank_timer_add   = ((uint32)(sr)  << 20) / 1000;
  vblank_timer_acc   = 0;
  vblank_timer_ticks = 0;
  vblank_in_thread   = false;
  vblank_callback    = NULL;
  framecount         = 0;
  vblfc              = 0;
  vblf               = read_tsc64();

  timeBeginPeriod(1);
  timer_intimer  = false;
  timer_lasttick = timeGetTime();
  audio_callback = NULL;

  timer_id = timeSetEvent(1, 0, &systemtimer, 0, TIME_PERIODIC | TIME_CALLBACK_FUNCTION);

  // Clear the screen

  FillConsoleOutputAttribute(h_stdout, 0, rcxsize * rcysize, vbuffpos, &nw);

  // Jump to the users code

  res = appmain(argc, argv);

  // Minimal cleanup

  timeEndPeriod(1);
  timeKillEvent(timer_id);
  Sleep(2);
  sl = wsprintf(tstr,"\n\nHave a nice day :)\n");
  WriteConsole(h_stdout, tstr, sl, &nw, 0);
  SetConsoleCursorInfo(h_stdout, &ccistart);

  // Return result from application
  
  return(res);
}


// ------------------------------------------------------------------------------------------------------
//|                                            Console Functions                                         |
// ------------------------------------------------------------------------------------------------------

// --------------------
//| Initialise Console |
// --------------------

static void init_console(int32 xs, int32 ys)
{
  // Locals

  COORD                       size;
  COORD                       pos;
  HWND                        cHwnd;
  CONSOLE_SCREEN_BUFFER_INFO  csbi;
  CONSOLE_CURSOR_INFO         cci;
  RECT                        wa;
  WINDOWINFO                  wi;
  char                        randtitle[32];
  bool                        res;
  int32                       dx,dy,dw,dh;
  int32                       wx,wy,ww,wh;
  int32                       i;
  
  // Get size of desktop

  res = SystemParametersInfo(SPI_GETWORKAREA, 0, &wa, 0);
  if (res)
  {
    dw = wa.right - wa.left;
    dh = wa.bottom - wa.top;
    dx = wa.left;
    dy = wa.top;
  }
  else
  {
    dw = GetSystemMetrics(SM_CXSCREEN); 
    dh = GetSystemMetrics(SM_CYSCREEN); 
    dx = 0;
    dy = 0;
  }

  // If we are building a windows app then allocate a console

  #if WINAPP
  AllocConsole();
  #endif

  // Open a new handle to the the console output 
  // - Prevents redirection problems :)

  h_stdout = CreateFile("CONOUT$", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

  // Set the X and Y size of the console
  // - Thanks to Pelle for the tip :)

  if (xs < 80) xs = 80;
  if (ys < 25) ys = 25;
  size.X = (int16) xs; 
  size.Y = (int16) ys; 
  SetConsoleScreenBufferSize(h_stdout, size);
  

  // Find the window handle

  for (i = 0; i < 31; i++) randtitle[i] = (random() & 31) + 65;
  randtitle[31] = 0;
  SetConsoleTitle(randtitle);
  cHwnd = NULL;
  while(cHwnd == NULL)
  {
    Sleep(20);
    cHwnd = FindWindow(NULL, randtitle);
  }

  // Maximize the console window and center on screen
  //
  // - How do we find out if we are running from a windowed command
  //   prompt (coz if so we DONT want to do this!!

  SetWindowPos(cHwnd, 0, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_HIDEWINDOW);
  Sleep(50);
  SetWindowPos(cHwnd, 0, 0, 0, 9999, 9999, SWP_NOZORDER | SWP_NOMOVE | SWP_HIDEWINDOW);
  res = GetWindowInfo(cHwnd, &wi);
  if (res)
  {
    ww = wi.rcWindow.right  - wi.rcWindow.left;
    wh = wi.rcWindow.bottom - wi.rcWindow.top;
    wx = dx + ((dw - ww) / 2);
    wy = dy + ((dh - wh) / 2);
    SetWindowPos(cHwnd, HWND_TOP, wx, wy, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
  }

  // Auto Fullscreen
  
  if (asi->flags & ASI_STARTFULLSCREEN)
  {
    SendMessage(cHwnd, WM_SYSKEYDOWN, KEY_RETURN, 0x60000000);
    SendMessage(cHwnd, WM_SYSKEYUP, KEY_RETURN, 0xc0000000);
  }

  // Find out the width and height we were actually given!
  //
  // - Remember you can't set a smaller width and height than
  //   the window SO if you run from a 100*44 DOS shell and ask
  //   for 80*25 you'll still get 100*44

  GetConsoleScreenBufferInfo(h_stdout, &csbi);
  rcxsize = (csbi.srWindow.Right - csbi.srWindow.Left) + 1;
  rcysize = (csbi.srWindow.Bottom - csbi.srWindow.Top) + 1;

  // Set the window title to something sensible :)

  SetConsoleTitle(appname);

  // Get the current cursor settings

  GetConsoleCursorInfo(h_stdout, &ccistart);

  // Disable the cursor
  // - Why set the size to 100?
  //   coz if u dont the cursor is still visible
  //   in fulscreen mode!!

  cci.bVisible = false;
  cci.dwSize = 100;
  SetConsoleCursorInfo(h_stdout, &cci);

  // Put the cursor at the bottom of the screen

  pos.X = 0;
  pos.Y = rcysize - 1;
  SetConsoleCursorPosition(h_stdout, pos);

  // Now initialise the input

  h_stdin = CreateFile("CONIN$", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  SetConsoleMode(h_stdin, 0);  
  memfill(&keybuffer, 0, sizeof(keybuffer));
  memfill(&keytable, 0, sizeof(keytable));
  memfill(&mouseinfo, 0, sizeof(MOUSEINFO));
  mouse_enable = false;
  keybuffer_rd = 0;
  keybuffer_wr = 0; 
  kb_ignore_numlock = false;
  kb_msdos_enter = false;
  kb_no_repeat = false;
  kthandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&keyread_thread, 0, 0, &ktid);
  SetThreadPriority(kthandle, THREAD_PRIORITY_TIME_CRITICAL);
}


// ------------------------------------------------------------------------------------------------------
//|                                          Threads and Timer functions                                 |
// ------------------------------------------------------------------------------------------------------

// --------------
//| System timer |
// --------------

static void CALLBACK systemtimer(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
  // Locals

  int32   t,ticks;

  // Quit if this isn't our callback :)

  if (uID != timer_id) return;

  // Quit if we're already running
  // - I know we wont be, but its better to be safe than sorry!!

  if (InterlockedExchange((int32*)&timer_intimer,true) == true) return;

  // How many ticks since last call ??

  t = timeGetTime();
  ticks = t - timer_lasttick;
  if (ticks > 1000) ticks = 1000;
  timer_lasttick = t;

  // Cursor flash code

  csrflashcount += ticks;
  csrflash = csrflashcount & 0x100; 

  // Do we wake up the vblank thread?

  vblank_timer_acc += (vblank_timer_add * ticks);
  if (vblank_timer_acc > 0xfffff)
  {
    vblank_timer_acc &= 0xfffff;
    vblank_timer_ticks ++;
    if (!vblank_in_thread) ResumeThread(vblank_thread_handle);
  }
  
  // Service the audio!

  if (audio_callback != NULL) call_routine(audio_callback);

  // Clear the timer running flag

  InterlockedExchange((int32*) &timer_intimer, false);
}


// -----------------------
//| Vblank Drawing Thread |
// -----------------------

static void vblank_thread(uint32 threadparam)
{
  // Locals

  volatile  int32 sz;
  int32     cp;
  uint8     ca,na;
  int64     t;
  
  // Infinite loop ...

  while(1)
  {
    // Flag active

    InterlockedExchange((int32*)&vblank_in_thread,true);

    // Copy the user charmap to the screen

    if (!charmap_locked)
    {
      // Read TSC for profiling

      t = read_tsc64();

      // Translate user charmap to windows format

      sz = (cwidth * cheight) / 8;

      __asm
      {
            push  eax
            push  ecx
            push  esi
            push  edi
            
            mov   esi,vram
            mov   edi,syscharmap
            mov   ecx,sz
  
      lp:   mov   al,[esi +  0] 
            mov   ah,[esi +  1]
            mov   [edi +  0],al
            mov   [edi +  2],ah
            mov   al,[esi +  2]
            mov   ah,[esi +  3]
            mov   [edi +  4],al
            mov   [edi +  6],ah
            mov   al,[esi +  4]
            mov   ah,[esi +  5]
            mov   [edi +  8],al
            mov   [edi + 10],ah
            mov   al,[esi +  6]
            mov   ah,[esi +  7]
            mov   [edi + 12],al
            mov   [edi + 14],ah
            mov   al,[esi +  8]
            mov   ah,[esi +  9]
            mov   [edi + 16],al
            mov   [edi + 18],ah
            mov   al,[esi + 10]
            mov   ah,[esi + 11]
            mov   [edi + 20],al
            mov   [edi + 22],ah
            mov   al,[esi + 12]
            mov   ah,[esi + 13]
            mov   [edi + 24],al
            mov   [edi + 26],ah
            mov   al,[esi + 14]
            mov   ah,[esi + 15]
            mov   [edi + 28],al
            mov   [edi + 30],ah
            add   esi,16
            add   edi,32
            dec   ecx
            jnz   lp

            pop   edi
            pop   esi
            pop   ecx
            pop   eax
      }

      // Add the cursor

      if ((!(conmode & CMFLAG_NOCURSOR)) && csrflash)
      {
        cp = ((cursory * cwidth) + cursorx) * 4;
        ca = syscharmap[cp + 2];
        na = ((ca & 15) << 4) | (ca >> 4);
        syscharmap[cp + 2] = na;
      }

      // Write charmap to console window

      WriteConsoleOutput(h_stdout,(CHAR_INFO*)syscharmap,vbuffsize,vbuffpos,&conrect);

      // Calculate % cpu time used for copy

      t = read_tsc64() - t;
      mi.blit_percent = (int32)((t * 100) / frametime);
    }

    // Call vblank callback?

    if (vblank_callback != NULL) call_routine(vblank_callback);

    // Update frame counters

    framecount = vblank_timer_ticks;
    vm.framecount = framecount;

    // Flag finished and put ourselves to sleep

    InterlockedExchange((int32*)&vblank_in_thread,false);
    SuspendThread(vblank_thread_handle);
  }
}


// ----------------------
//| Keyboard read thread |
// ----------------------

// Dont even ask why this is how it is!

static void keyread_thread(uint32 param)
{
  // Locals
  
  INPUT_RECORD  inrec;
  uint32        rfcount;
  uint32        ctstate;
  bool          enhkey;
  bool          keydown;
  bool          qualkey;
  uint32        ac,kc,sc;   
  uint32        kcextra;
  uint32        kte;
  int32         wr;
  int32         s;
  
  // Code
  
  while(1)
  {
    if (ReadConsoleInput(h_stdin,&inrec,1,&rfcount));
    {
      if (rfcount)
      {
        switch(inrec.EventType)
        {
          case KEY_EVENT:
          {
            // Get The Key information
            
            ctstate = inrec.Event.KeyEvent.dwControlKeyState;
            enhkey  = (ctstate & 0x100) >> 8;
            keydown = inrec.Event.KeyEvent.bKeyDown;
            ac      = inrec.Event.KeyEvent.uChar.AsciiChar;
            kc      = inrec.Event.KeyEvent.wVirtualKeyCode; 
            sc      = inrec.Event.KeyEvent.wVirtualScanCode;
            
            // Bail if illegal keycode

            if (kc < 8)           break;
            if (kc == KEY_NENTER) break;

            // Fixup the left/right hand side keycodes and set the qualkey flag

            qualkey = false;
            kcextra = 0;
  
            switch(kc)
            {
              case KEY_SHIFT:
              {
                qualkey = true; 
                if (sc == 0x36) kcextra = KEY_RSHIFT; else kcextra = KEY_LSHIFT;
              }
              break;

              case KEY_CTRL:
              {
                qualkey = true;
                if (enhkey) kcextra = KEY_RCTRL; else kcextra = KEY_LCTRL;
              }
              break;

              case KEY_ALT:
              {
                qualkey = true;
                if (enhkey) kcextra = KEY_RALT; else kcextra = KEY_LALT;
              }
              break;
          
              case KEY_CAPSLOCK:
              {
                qualkey = true;               
              }
              break;

              case KEY_NUMLOCK:
              {
                qualkey = true;               
              }
              break;

              case KEY_SCRLLOCK:
              {
                qualkey = true;               
              }
              break;

              case KEY_RETURN:
              {
                if (enhkey) kc = KEY_NENTER;
              }
              break;
            }

            // Remap numlock altered keys back to sanity!

            if (!enhkey)
            {
              switch(kc)
              {
                case KEY_INSERT:
                  kc = KEY_N0;
                break;

                case KEY_DELETE:
                  kc = KEY_NDPOINT;
                break;

                case KEY_END:
                  kc = KEY_N1;
                break;

                case KEY_CRSRDN:
                  kc = KEY_N2;
                break;

                case KEY_PGDN:
                  kc = KEY_N3;
                break;

                case KEY_CRSRLF:
                  kc = KEY_N4;
                break;

                case KEY_CLEAR:
                  kc = KEY_N5;
                break;

                case KEY_CRSRRT:
                  kc = KEY_N6;
                break;

                case KEY_HOME:
                  kc = KEY_N7;
                break;

                case KEY_CRSRUP:
                  kc = KEY_N8;
                break;

                case KEY_PGUP:
                  kc = KEY_N9;
                break;
              }
            }

            // Update keytable

            if (keydown)
            {
              keytable[kc] = 1;
              if (kcextra) keytable[kcextra] = 1;
            }
            else
            {
              keytable[kc] = 0;
              if (kcextra) keytable[kcextra] = 0;
            }

            // Now build the shiftstate info
            // - and store to the first 2 bytes of the keytable

            kte = 0;

            if (keytable[KEY_SHIFT])  kte |= SSTATE_ANYSHIFT;
            if (keytable[KEY_CTRL])   kte |= SSTATE_ANYCTRL;
            if (keytable[KEY_ALT])    kte |= SSTATE_ANYALT;
            if (ctstate & 0x80)       kte |= SSTATE_CAPSLOCK;   
            if (ctstate & 0x20)       kte |= SSTATE_NUMLOCK;  
            if (ctstate & 0x40)       kte |= SSTATE_SCRLLOCK;   
            if (keytable[KEY_LSHIFT]) kte |= SSTATE_LSHIFT;
            if (keytable[KEY_RSHIFT]) kte |= SSTATE_RSHIFT;
            if (keytable[KEY_LCTRL])  kte |= SSTATE_LCTRL;
            if (keytable[KEY_RCTRL])  kte |= SSTATE_RCTRL;
            if (keytable[KEY_LALT])   kte |= SSTATE_LALT;
            if (keytable[KEY_RALT])   kte |= SSTATE_RALT;

            keytable[0] = (uint8)((kte >> 16) & 255);
            keytable[1] = (uint8)((kte >> 24) & 255);

            // If the key was released OR was a qualifier key then bail
    
            if ((qualkey) || (!keydown)) break;

            // If keyboard repeat is off and duplicate key then bail

            if (kb_no_repeat) if (keytable[kc] != 0) break;
          
            // Fixup the numpad keys (again)

            if ((kc >= KEY_N0) && (kc <= KEY_NDIVIDE))
            {
              // Flag key from numpad

              kte |= SSTATE_NUMPAD;

              // Fixup keycode and ascii depending on numlock state

              if (kb_ignore_numlock)
              {
                s = 0;
              }
              else
              {
                if (kte & SSTATE_NUMLOCK) s = 0; else s = 1;
              }

              kc = numpad_kc[kc - KEY_N0][s];
              ac = numpad_ac[kc - KEY_N0][s];
            }
  
            // Translate enter key depending on mode

            if ((ac == 0x0d) && (!kb_msdos_enter)) ac = 0x0a;

            // Build ringbuffer longword

            kte |= ((kc << 8) | ac);

            // Store key to ringbuffer
 
            wr = keybuffer_wr;
            keybuffer[wr] = (uint32)kte;
            wr ++;
            wr &= 255;
            keybuffer_wr = wr;
          }
          break;
                          
          case MOUSE_EVENT:
          {
            mouseinfo.buttons = inrec.Event.MouseEvent.dwButtonState & 0xff;
            mouseinfo.x = inrec.Event.MouseEvent.dwMousePosition.X;
            mouseinfo.y = inrec.Event.MouseEvent.dwMousePosition.Y;
            if (inrec.Event.MouseEvent.dwEventFlags & MOUSE_WHEELED)
            {
              if (inrec.Event.MouseEvent.dwButtonState & 0x80000000)
              {
                mouseinfo.z ++;
              }
              else
              {
                mouseinfo.z --;
              }
            }
          }
          break;
        }
      } 
    }
  }
}


// ------------------------------------------------------------------------------------------------------
//|                                            Subroutines                                               |
// ------------------------------------------------------------------------------------------------------

// -------------------
//| Split commandline |
// -------------------

#if WINAPP
static void split_commandline(void)
{
  // Locals

  char*       lpcmdline;
  int32       cmdlen;
  bool        doit;
  bool        findquote;
  bool        findstart;
  int32       sp;

  // Copy command line to local storage and get the length

  memfill(&cmdline,0,sizeof(cmdline));
  lpcmdline = GetCommandLine();
  cmdlen = stringlen(lpcmdline);
  if (cmdlen > (sizeof(cmdline)-1)) cmdlen = sizeof(cmdline)-1;
  memcopy(lpcmdline,&cmdline,cmdlen);
  
  // Clear ARGC and ARGV[]

  memfill(&argv,0,sizeof(argv));
  argc = 0;

  // Slice into bits ...

  doit = true;
  findstart = true;
  findquote = false;
  sp = 0;
  while(doit)
  {
    if (findstart)
    {
      if (cmdline[sp] == 34)
      {
        findquote = true;
        sp++;
      }
      else
      {
        findquote = false;
      }
      argv[argc] = &cmdline[sp];
      argc++;
      findstart = false;
    }
    else
    {
      if (findquote)
      {
        if (cmdline[sp] == 34)
        {
          cmdline[sp] = 0;
          sp++;
          findstart = true;
        }
      }
      else
      {
        if (cmdline[sp] == 32)
        {
          cmdline[sp] = 0;
          findstart = true;
        }
      }
      sp++;
    }
    if (sp >= cmdlen) doit = false; 
  }

}
#endif


// -------
//| Error |
// -------

// Issue an error message to the console window

static void error(char* estring)
{
  int32   sl;
  uint32  nw;
  char    err[256];
  SetConsoleCursorInfo(h_stdout,&ccistart);
  sl = wsprintf(err,"\n ** ERROR **\n\n");
  WriteConsole(h_stdout,err,sl,&nw,0);
  sl = stringlen(estring);
  WriteConsole(h_stdout,estring,sl,&nw,0);
  sl = wsprintf(err,"\n PRESS ANY KEY TO QUIT ... ");
  WriteConsole(h_stdout,err,sl,&nw,0);
  while(!keypressed()) sleep(1);
  sl = wsprintf(err,"\n");
  WriteConsole(h_stdout,err,sl,&nw,0);
}


// --------------
//| Identify CPU |
// --------------

static uint32 cpu_id(void)
{
  // Locals

  SYSTEM_INFO si;
  bool        intel     = false;
  bool        amd       = false;
  uint32      flags     = 0;
  uint32      flags2    = 0;
  uint32      cpuflags  = 0;

  // Can we use CPUID (Processor must be at least a Pentium 1)

  memfill(&si,0,sizeof(si));
  GetSystemInfo(&si);
  if (si.dwProcessorType != PROCESSOR_INTEL_PENTIUM) return(CPU_NOTPENTIUM);  

  // Now some Nasty ASM to get the flags ...

  __asm
  {
              pushad
              mov     eax,0             // Get Brand string
              cpuid
              cmp     ebx,0x756e6547    // check for Intel ("Genu")
              jne     nointel
              mov     intel,1
              jmp     getflags
    nointel:  cmp     ebx,0x68747541    // check for AMD ("Auth")
              jne     unknown
              mov     amd,1
              mov     eax,0x80000000    // check AMD extended feature flags
              cpuid
              cmp     eax,0x80000000
              jbe     getflags
              mov     eax,0x80000001
              cpuid
              mov     flags2,edx
    getflags: mov     eax,1             // get feature flags
              cpuid
              mov     flags,edx
    unknown:  popad

  }

  // Work out what we've got ...

  cpuflags = 0;

  if (!intel && !amd)        cpuflags |= CPU_UNKNOWN;
  if (intel)                 cpuflags |= CPU_INTEL;
  if (amd)                   cpuflags |= CPU_AMD;

  if (flags & 0x00800000)    cpuflags |= CPU_MMX;
  if (flags & 0x02000000)    cpuflags |= CPU_SSE;
  if (flags & 0x04000000)    cpuflags |= CPU_SSE2;
  if (flags & 0x00000010)    cpuflags |= CPU_RDTSC;
  if (flags & 0x00008000)    cpuflags |= CPU_CMOV;

  if (amd)
  {
    if (flags2 & 0x00400000) cpuflags |= CPU_MMX_EXTENDED;
    if (flags2 & 0x80000000) cpuflags |= CPU_3DNOW;
    if (flags2 & 0x40000000) cpuflags |= CPU_3DNOW_EXTENDED;
  }

  return(cpuflags);
}


// ------------------------------------------------------------------------------------------------------
//|                                            Text output functions                                     |
// ------------------------------------------------------------------------------------------------------

// ------------------
//| Set Console Mode |
// ------------------

void set_console_mode(uint32 mode)
{
  conmode = mode;
}


// ------------------
//| Get Console Mode |
// ------------------

uint32 get_console_mode(void)
{
  return(conmode);
}
  

// ------------------
//| Clear the screen |
// ------------------

void clrscr(void)
{
  uint8*  s = vm.vram_base;
  int32   i = cwidth * cheight;

  // Clear the screen

  while (i --) 
  {
    *s ++ = 32;
    *s ++ = attribute;
  }

  // Home the cursor

  cursorx = 0;
  cursory = 0;
}


// --------
//| GotoXY |
// --------

// Pass a negative value for either x or y to leave
// the position unchanged

void gotoxy(int32 x, int32 y)
{
  if (x >= 0)
  {
    if (x > cwidthm1) x = cwidthm1;
    cursorx = x;
  }

  if (y >= 0)
  {
    if (y > cheightm1) y = cheightm1;
    cursory = y;
  }
}


// -----------------
//| Return cursor X |
// -----------------

int32 wherex(void)
{
  return(cursorx);
}


// -----------------
//| Return cursor Y |
// -----------------

int32 wherey(void)
{
  return(cursory);
}


// ----------------------
//| Clear to end of line |
// ----------------------

void clreol(void)
{
  uint8* s = vram + ((cursory * cwidth) + cursorx) * 2;
  int32  i = cwidth - cursorx;
  if (i < 1) return;
  while (i --)
  {
    *s ++ = 32;
    *s ++ = attribute;
  }
}


// -----------------------
//| Set foreground colour |
// -----------------------

void setfgcolour(uint8 fg)
{
  attribute = (attribute & 0xf0) | (fg & 0x0f);
}


// -----------------------
//| Set background colour |
// -----------------------

void setbgcolour(uint8 bg)
{
  attribute = (attribute & 0x0f) | (bg << 4);
}


// ----------------------
//| Set attribute colour |
// ----------------------

void setattribute(uint8 attr)
{
  attribute = attr;
}


// ---------------
//| Get attribute |
// ---------------

uint8 getattribute(int32 which)
{
  switch(which)
  {
    case ATTR_FGCOL:      return(attribute & 0x0f);
    case ATTR_BGCOL:      return(attribute >> 4);
    case ATTR_ATTRIBUTE:  return(attribute);
    default:              return(0);
  }
}


// ------------------
//| Set the TAB size |
// ------------------

void settabsize(int32 size)
{
  tabsize = size;
}


// --------------------
//| Output a character |
// --------------------

// ALL print functions print by using this function.

void outchar(uint8 chr, uint8 attr, uint32 flags)
{
  // Locals

  uint8* s = vram + ((cursory * cwidth) + cursorx) * 2;

  // If the X or Y position is offscreen then bail
  // - Used by the nowrap options!

  if (cursorx > cwidthm1) return;
  if (cursory > cheightm1) return;

  // Use the supplied attribute?
  
  if (!(flags & OCFLAG_USEATTRIBUTE)) attr = attribute;

  // If translation is off then skip the next bit

  if (flags & OCFLAG_NOTRANSLATE) goto skipctrl;
  
  switch(chr)
  {
    // Bell

    case 7:
    {
      return;
    }
    break;

    // Backspace

    case 8:
    {
      cursorx --;
      if (cursorx < 0) cursorx = 0;
      #ifdef DESTRUCTIVE_BACKSPACE
      s[0] = 32;
      s[1] = attr;
      #endif
      return;
    } 
    break;

    // Tab

    case 9:
    {
      cursorx = tabsize * ((cursorx / tabsize) + 1);
      goto fixcursorx;
    }
    break;

    // Linefeed

    case 10:
    {
      if (!(conmode & CMFLAG_MSDOSLF)) cursorx = 0;
      cursory ++;
      goto fixcursory;
    }
    break;

    // Carriage Return

    case 13:
    {
      cursorx = 0;
      return;
    }
  }

  // Print the character

  skipctrl:

  s[0] = chr;
  s[1] = attr;
  cursorx ++;

  // Fix cursor X position

  fixcursorx:

  if (cursorx > cwidthm1)
  {
    // Quit if NOXWRAP

    if (conmode & CMFLAG_NOXWRAP) return;

    // Put cursor to start of line, increment ypos

    cursorx = 0;
    cursory++;
  }

  // Fix cursor Y position

  fixcursory:

  if (cursory > cheightm1)
  {
    // Check for no scroll

    if (conmode & CMFLAG_NOSCROLL)
    {
      // Quit if NOYWRAP

      if (conmode & CMFLAG_NOYWRAP) return;

      // Put cursor to top of screen and quit

      cursory = 0;
      return;
    }

    // Scroll the screen up and put cursor on bottom line

    scrollscreen(SCROLL_UP);
    cursory = cheightm1;
  }
}


// -----------------
//| Output a string |
// -----------------

void outstring(char* str, int32 len, uint32 flags)
{
  uint32  f = flags & 1;
  uint8   a,c;

  if (flags & OSFLAG_USELENGTH)
  {
    if (flags & OSFLAG_HASATTRIBUTES)
    {
      while(len --) 
      {
        c = *str++;
        a = *str++;
        outchar(c,a,f);
      }
    }
    else
    {
      while(len --) outchar(*str++,attribute,f);
    }
  }
  else
  {
    if (flags & OSFLAG_HASATTRIBUTES)
    {
      while(*str != 0) 
      {
        c = *str++;
        a = *str++;
        outchar(c,a,f);
      }
    }
    else
    {
      while(*str != 0) outchar(*str++,attribute,f);
    }
  }
}


// -------------------
//| Scroll the screen |
// -------------------

void scrollscreen(int32 how)
{
  // Locals

  int32   x,y,l,a;
  uint8*  s;
  uint8*  d;

  switch(how)
  {
    case SCROLL_UP:
    {
      d = vram;
      s = d + (cwidth * 2);
      y = cheightm1;
      while (y --)
      {
        x = cwidth * 2;
        while (x --) *d++ = *s++;
      }
      d = vram + (cheightm1 * cwidth * 2);
      x = cwidth * 2;
      while (x --) 
      {
        *d++ = 32;
        *d++ = attribute;
      }
    }
    break;
    
    case SCROLL_DOWN:
    {
      y = cheight;
      l = y;
      while (y--)
      {
        d = vram + (l -- * cwidth * 2);
        s = d - (cwidth * 2);
        x = cwidth * 2;
        while (x --) *d++ = *s++;
      }
      d = vram;
      x = cwidth;
      while (x --) 
      {
        *d++ = 32;
        *d++ = attribute;
      }
    }
    break;

    case SCROLL_LEFT:
    {
      y = cheight;
      l = 0;
      while (y --)
      {
        d = vram + (l * cwidth * 2);
        l ++;
        s = d + 2;
        x = cwidthm1 * 2;
        while (x --) *d++ = *s++;
      }
      a = cwidthm1 * 2;
      d = vram + a;
      y = cheight;
      while (y --)
      {
        *d++ = 32;
        *d++ = attribute;
        d += a;
      }
    }
    break;

    case SCROLL_RIGHT:
    {
      y = cheight;
      l = 0;
      while (y --)
      {
        d = vram + (l * cwidth * 2) + (cwidthm1 * 2);
        l ++;
        s = d - 2;
        x = cwidthm1 * 2;
        while (x --) *d-- = *s--;
      }
      a = cwidthm1 * 2;
      d = vram;
      y = cheight;
      while (y --)
      {
        *d++ = 0x20;
        *d++ = attribute;
        d += a;
      }
    }
    break;
  }
}


// -------
//| PRINT |
// -------

// No need for the va_start nonsense :)

int32 _print(char* fmt,...)
{
  char    buff[4096];
  int32   r;
  r = _vsnprintf(buff,4096,fmt,(char*)&fmt + 4);
  if (r > 4096) return(-1);
  outstring(buff,0,0);
  return(r);
}


// ------------------------------------------------------------------------------------------------------
//|                                            Library Functions                                         |
// ------------------------------------------------------------------------------------------------------

// ------------------
//| Get Machine Info |
// ------------------ 

MACHINE_INFO* get_machine_info(void)
{
  return(&mi);
}


// -----------------
//| Get System Time |
// ----------------- 

uint32 get_time(void)
{
  SYSTEMTIME  st;
  uint32      hour,min,sec,hund;
  GetLocalTime(&st);
  hour      = st.wHour;
  min       = st.wMinute;
  sec       = st.wSecond;
  hund      = st.wMilliseconds / 10;
  return((hour << 24) | (min << 16) | (sec << 8 ) | hund);
}


// -----------------
//| Get System Date |
// ----------------- 

uint32 get_date(void)
{
  SYSTEMTIME  st;
  uint32      year,month,date,day;
  GetLocalTime(&st);
  year      = st.wYear;
  month     = st.wMonth;
  date      = st.wDay;
  day       = st.wDayOfWeek;
  return((year << 16) | (month << 8) | (date << 3) | day);
}


// -------
//| Sleep |
// -------

void sleep(uint32 sleeptime)
{
  Sleep(sleeptime);
}


// ---------------------------
//| Get Application Directory |
// ---------------------------

char* get_app_directory(void)
{
  return(&apppath[0]);
}


// --------------------
//| Get videomode info |
// --------------------
                        
VMODE_INFO* get_videomode_info(void)
{
  return(&vm);
}


// -----------------------
//| Lock / Unlock charmap |
// -----------------------

void lock_charmap(bool lockstate)
{
  charmap_locked = lockstate;
}


// ---------------------
//| Set Vblank Callback |
// ---------------------

void set_vblank_callback(void* vbc)
{
  vblank_callback = vbc;
}


// -------
//| VSYNC |
// -------

// Returns # of frames elapsed since last call :)

int32 vsync(void)
{
  // Locals

  volatile int32  f;  
  uint32          r;
  int64           t;

  // Initialise profiling for app_percent

  t = (read_tsc64() - vblf);

  // Wait for the frame counter to increment

  f = framecount;
  while (framecount == f) Sleep(1);

  // Calc how many frames since last vsync

  f = framecount;
  r = f - vblfc;
  vblfc = f;

  // Calc %cpu time used

  mi.app_percent = (int32)((t * 100) / frametime);
  mi.vs_frames = r;
  vblf = read_tsc64();
  
  // Return framecount and leave ...

  return (r);
}    


// ----------------
//| Get Framecount |
// ----------------

int32 get_framecount(void)
{
  return(framecount);
}


// --------------------
//| Was a key pressed? |
// --------------------

bool keypressed(void)
{
  bool r;
  r = (keybuffer_rd != keybuffer_wr);
  return(r);
}                    


// ----------
//| Read Key |
// ----------

uint32 read_key(void)
{
  uint32 k;
  while(!keypressed()) Sleep(1);;
  k = keybuffer[keybuffer_rd];
  keybuffer_rd ++;
  keybuffer_rd &= 255;
  return(k);  
}


// ----------------------------
//| Return pointer to keytable |
// ----------------------------

uint8* get_keytable(void)
{
  return(&keytable[0]);
}


// --------------------
//| Return Shift State |
// --------------------

uint8 get_shiftstate(void)
{
  return(keytable[0]);
}


// -----------------------
//| Flush Keyboard Buffer |
// -----------------------

void flush_keybuffer(void)
{
  keybuffer_rd = 0;
  keybuffer_wr = 0;
  memfill(&keybuffer,0,sizeof(keybuffer));
}


// -------------------
//| Set Keyboard Mode |
// -------------------

void set_keyboard_mode(uint32 flags)
{
  if (flags & KBMODE_NO_REPEAT) kb_no_repeat = true; else kb_no_repeat = false;
  if (flags & KBMODE_FORCE_NUMLOCK) kb_ignore_numlock = true; else kb_ignore_numlock = false;
  if (flags & KBMODE_MSDOS_ENTER) kb_msdos_enter = true; else kb_msdos_enter = false;
}
 

// --------------
//| Enable Mouse |
// --------------

void enable_mouse(bool enable)
{
  if (mouse_enable == enable) return;
  memfill(&mouseinfo,0,sizeof(MOUSEINFO));
  if (enable)
  {
    SetConsoleMode(h_stdin,ENABLE_MOUSE_INPUT);
  }
  else
  {
    SetConsoleMode(h_stdin,0);
  }
}


// --------------------------
//| Return mouse information |
// --------------------------

void read_mouse(MOUSEINFO* mi)
{
  mi->buttons   = mouseinfo.buttons;
  mi->x         = mouseinfo.x;
  mi->y         = mouseinfo.y;
  mi->z         = mouseinfo.z;
  mouseinfo.z   = 0;
}


// -------------
//| Open a file |
// -------------

// Returns file hanlde (0 if error)
//
// - Fixed to prevent searches in all pathed directories :)

uint32 file_open(char* fname, uint32 flags)
{
  char  name[4096];
  bool  haspath = false;

  // Does the filename refer to an absolute path???
  
  if (fname[0] == '\\') haspath = true;   //  "\dir\something", "\\spoot\blah" etc
  if (fname[2] == '\\') haspath = true;   //  "c:\file.txt" 

  // If there is a path then just open the file ...

  if (haspath) return ((uint32)(mmioOpen(fname, NULL, flags & 3)));

  // If no path prepend '.\' in front of the name

  wsprintf(name, ".\\", fname);
  return ((uint32)(mmioOpen(fname, NULL, flags & 3)));
}


// ---------------
//| Create a file |
// ---------------

// Returns file handle (0 if error)

uint32 file_create(char* fname)
{
  return ((uint32)(mmioOpen(fname,NULL,MMIO_READWRITE | MMIO_CREATE)));  
}


// --------------
//| Close a file |
// --------------

// No return code

void file_close(uint32 handle)
{
  mmioClose((HMMIO)handle,0);
}


// ------
//| Seek |
// ------

// Returns new file position

uint32 file_seek(uint32 handle,int32 pos,uint32 mode)
{
  return ((uint32)(mmioSeek((HMMIO)handle,pos,mode)));
}


// ---------------------------
//| Get file pointer position |
// ---------------------------

// Returns file position

uint32 file_getpos(uint32 handle)
{
  return ((uint32)(mmioSeek((HMMIO)handle,0,SEEK_CURRENT)));
}


// -----------
//| Read data |
// -----------

// Returns # of bytes loaded (0 if error)

uint32 file_blockread(uint32 handle,void* address,uint32 loadsize)
{
  // Locals

  char*   addr;
  int32   loaded;
  int32   toload;
  int32   ls,r;
  bool    loadloop;

  // Initialise load loop

  addr     = address;
  toload   = loadsize;
  loaded   = 0;
  loadloop = true;

  // Load chunks until all done

  do
  {
    if (toload > CHUNKSIZE) 
    {
      ls = CHUNKSIZE; 
      toload -= CHUNKSIZE;
    }
    else
    {
      ls = toload;
      loadloop = false;
    }
    r = mmioRead((HMMIO)handle,addr,ls);
    if (r == -1) return(0);
    loaded += r;
    addr   += r;
  }
  while(loadloop);

  // Return # of bytes loaded

  return((uint32)loaded);
}


// ------------
//| Write data |
// ------------

// Returns # of bytes loaded (0 if error)

uint32 file_blockwrite(uint32 handle,void* address, uint32 savesize)
{
  // Locals

  char*   addr;
  int32   saved;
  int32   tosave;
  int32   ss,r;
  bool    saveloop;
  
  // Initialise save loop

  addr     = address;
  tosave   = savesize;
  saved    = 0;
  saveloop = true;

  // Save chunks until all done

  do
  {
    if (tosave > CHUNKSIZE) 
    {
      ss = CHUNKSIZE; 
      tosave -= CHUNKSIZE;
    }
    else
    {
      ss = tosave;
      saveloop = false;
    }
    r = mmioWrite((HMMIO)handle,addr,ss);
    if (r == -1) return(0);
    saved += r;
    addr  += r;
  }
  while(saveloop);

  // Return # of bytes saved

  return((uint32)saved);
}

// -------------------
//| Check file exists |
// -------------------

// Returns file hanlde (0 if error)

bool file_exists(char* fname)
{
  uint32 fh = (uint32)mmioOpen(fname,NULL,0);
  if (fh == 0) 
  {
    return(false);
  }
  else
  {
    mmioClose((HMMIO)fh,0);
    return(true);
  } 
}


// ---------------
//| Get file size |
// ---------------

// Returns 0 if any error

uint32 file_size(char* fname)
{
  uint32 tmp,len;
  if ((tmp=file_open(fname,FILE_READONLY)) == 0 ) return(0);
  len=file_seek(tmp,0,SEEK_END);
  file_close(tmp);
  return(len);
}


// -------------------
//| Load a whole file |
// -------------------

// Returns # of bytes loaded (0 if error)

uint32 load_data(char* fname, void* address, uint32 loadsize)
{
  uint32 tmp,lsize;
  lsize = loadsize;
  if (loadsize == -1) lsize = file_size(fname);
  if ((tmp = file_open(fname,FILE_READONLY)) == 0) return(0);
  lsize = file_blockread(tmp,address,lsize);
  file_close(tmp);
  return(lsize);
}


// -------------
//| Save a file |
// -------------

// Returns # of bytes written (0 if error)

uint32 save_data(char* fname,void* address, uint32 savesize)
{
  uint32 tmp,ssize;
  if ((tmp = file_create(fname)) == 0) return(0);
  ssize = file_blockwrite(tmp,address,savesize);
  file_close(tmp);
  return (ssize);
}

// -----------------------
//| Get current directory |
// -----------------------

char* get_directory(void)
{
  static char curr_path[4096];
  memfill(&curr_path,0,sizeof(curr_path));
  GetCurrentDirectory(sizeof(curr_path)-2,(char*)&curr_path);
  curr_path[stringlen(curr_path)]=0x5c;
  return ((char*)&curr_path);
}


// -----------------------
//| Set Current Directory |
// -----------------------

bool set_directory(char* dirname)
{
  return ((bool)(SetCurrentDirectory(dirname)));
}
                                  

// ------------------
//| Create Directory |
// ------------------

bool create_directory(char* dirname)
{
  return ((bool)(CreateDirectory(dirname,NULL)));
}


// ------------------
//| Remove Directory |
// ------------------

bool remove_directory(char* dirname)
{
  return ((bool)(RemoveDirectory(dirname)));
}


// -----------------
//| Find First File |
// -----------------

// Returns handle for findnext or 0 if any error (eg directory not found)

uint32 file_findfirst(char* findstr, DIR_ENTRY* de)
{
  WIN32_FIND_DATA wfd;
  HANDLE          wh;
  memfill(de, 0, sizeof(DIR_ENTRY));
  wh = FindFirstFile((char*)findstr, &wfd);
  if (wh == INVALID_HANDLE_VALUE) return(0);
  translate_dirent(&wfd, de);
  return ((uint32)wh);
}
 

// ----------------
//| Find Next File |
// ----------------

// Returns false if there are no more files

bool file_findnext(uint32 handle, DIR_ENTRY* de)
{
  WIN32_FIND_DATA wfd;
  bool            res;
  memfill(de, 0, sizeof(DIR_ENTRY));
  res = FindNextFile((HANDLE)handle, &wfd);
  if (res == false) return(false);
  translate_dirent(&wfd, de);
  return(true);
}


// -----------------
//| Close file find |
// -----------------

void file_findclose(uint32 handle)
{
  FindClose((HANDLE)handle);
}


// -----------------
//| Allocate Memory |
// -----------------

void* allocmem(uint32 size)
{
  void*   addr;
  bool    clear = (size >> 31) ^ 1;
  uint32  sz = size & 0x7fffffff;
  addr = VirtualAlloc(NULL, sz, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
  if (addr != NULL) 
  {
    if (clear) memfill(addr, 0, size);
  }
  return(addr);
}


// -------------
//| Free Memory |
// -------------

void freemem(void* addr)
{
  VirtualFree(addr, 0, MEM_RELEASE);
}


// --------------
//| Fast MEMCOPY |
// --------------

void memcopy(void* src, void* dst, uint32 count)
{
  __asm 
  {
          pushfd
          push    ebx
          push    esi
          push    edi
          cld
          mov     esi,src
          mov     edi,dst
          mov     ecx,count
          mov     ebx,ecx
          and     ebx,3
          shr     ecx,2
          rep     movsd
          mov     ecx,ebx
          rep     movsb
          pop     edi
          pop     esi
          pop     ebx
          popfd
  }
}


// --------------------
//| Fast MEM byte fill |
// --------------------

void memfill(void* dst, uint8 fill, uint32 count)
{
  __asm 
  {
          pushfd
          push    ebx
          push    esi
          push    edi
          cld
          mov     edi,dst
          mov     ecx,count
          mov     ebx,ecx
          and     ebx,3
          shr     ecx,2
          mov     dl,fill
          mov     al,dl
          mov     ah,dl
          shl     eax,16
          mov     al,dl
          mov     ah,dl
          rep     stosd
          mov     ecx,ebx
          rep     stosb
          pop     edi
          pop     esi
          pop     ebx
          popfd
  }
}


// ------------------
//| Fast MEM compare |
// ------------------

bool memcompare(void* src, void* dst, uint32 count)
{
  bool  result;
  __asm
  {
          pushfd
          push    esi
          push    edi
          cld
          mov     result,true
          mov     esi,src
          mov     edi,dst
          mov     ecx,count
          inc     ecx
          repe    cmpsb
          jecxz   found
          mov     result,false
  found:  pop     edi
          pop     esi
          popfd
  }
  return(result);
}


// ----------------------------
//| Set random generator seeed |
// ----------------------------

// Set random seed

void randomize(uint32 rndval)
{
  rndseed = rndval;
}


// ------------------------------------
//| Generate 32bit psuedo random value |
// ------------------------------------

// In Assembler for speed :)

uint32 __declspec(naked) random(void)
{
  __asm
  {
          mov     eax,rndseed
          imul    eax,0x41c64e6d
          add     eax,0x3039
          mov     rndseed,eax
          rol     eax,16
          ret
  }
}


// ----------------
//| Call a routine |
// ----------------

void call_routine(void* routaddr)
{
  __asm 
  {
          pushad
          mov     ebx,routaddr
          call    ebx
          popad
  }
}


// ------------------------
//| Read Timestamp Counter |
// ------------------------

int64 read_tsc64(void)
{
  LARGE_INTEGER li;
  QueryPerformanceCounter(&li);
  return((int64)li.QuadPart);
}


// -------------------------
//| Calculate String Length |
// -------------------------

int32 stringlen(char* str)
{
  int32 l = 0;
  while (*str++) l++;
  return(l);
}


// ----------------
//| Hook audio IRQ |
// ----------------

void set_audio_callback(void* cb_addr)
{
  audio_callback = cb_addr;
}


// ------------------------------------------------------------------------------------------------------
//|                                       Private Library Support Functions                              |
// ------------------------------------------------------------------------------------------------------

// --------------------
//| Translate Win->E3K |
// --------------------

// Translate a windows directory entry into an Etanza E3K entry

static void translate_dirent(WIN32_FIND_DATA* wfd, DIR_ENTRY* de)
{
  // Locals

  uint32      a;
  FILETIME    ft;
  SYSTEMTIME  st;
  bool        r;
  uint32      year,month,date,day;
  uint32      hour,min,sec,hund;
  int32       l;
  uint32      flags;
  bool        execute;
  bool        shellcmd;
  uint32      execheck;

  // First we demangle the file time and date

  de->date = 0x07d0010e; //Saturday January 1st 2000
  de->time = 0x00000000; //Midnight (00:00:00.00)
  r = FileTimeToLocalFileTime(&wfd->ftLastWriteTime, &ft);
  if (r)
  {
    r = FileTimeToSystemTime(&ft,&st);
    if (r)
    {
      year      = st.wYear;
      month     = st.wMonth;
      date      = st.wDay;
      day       = st.wDayOfWeek;
      de->date  = (year << 16) | (month << 8) | (date << 3) | day;
      hour      = st.wHour;
      min       = st.wMinute;
      sec       = st.wSecond;
      hund      = st.wMilliseconds / 10;
      de->time  = (hour << 24) | (min << 16) | (sec << 8 ) | hund;
    }
  }

  // Now we copy in the name

  l = stringlen(wfd->cFileName);
  // Really just a sanity check as windows names cant be this long :)
  if (l > 479) l = 479;               
  memfill(de->name, 0, 480);
  memcopy(wfd->cFileName, de->name, l);

  // Check if the file is executable 
  // - At the moment we check for .com / .exe / .bat

  shellcmd  = false;
  execute   = false;
  execheck  = (de->name[l-3] * 65536) + (de->name[l-2] * 256) + de->name[l-1];
  execheck |= 0x202020;       // make lowercase!
  if (execheck == 0x636f6d)   execute = true;                     // "com"
  if (execheck == 0x657865)   execute = true;                     // "exe"
  if (execheck == 0x626174) { execute = true; shellcmd = true; }  // "bat"

  // Finally we translate the attributes and size ...
  // - We emulate E3K flags as fully as possible ...

  a = wfd->dwFileAttributes;
  flags = DIR_READABLE;
  if (!(a & FILE_ATTRIBUTE_READONLY)) flags |= DIR_WRITABLE | DIR_DELETABLE;
  if (execute)                        flags |= DIR_EXECUTABLE;
  if (a & FILE_ATTRIBUTE_HIDDEN)      flags |= DIR_INVISIBLE;
  if (a & FILE_ATTRIBUTE_SYSTEM)      flags |= DIR_SYSTEM;
  if (a & FILE_ATTRIBUTE_DIRECTORY)   flags |= DIR_DIRECTORY;
  if (a & FILE_ATTRIBUTE_ARCHIVE)     flags |= DIR_ARCHIVE;
  if (shellcmd)                       flags |= DIR_SHELLCMD;
  de->flags = flags; 
  de->size  = wfd->nFileSizeLow;
  de->sizehigh = wfd->nFileSizeHigh;
}







// ------------------------------------------------------------------------------------------------------
//|                                        The End, Thanks for Reading :)                                |
// ------------------------------------------------------------------------------------------------------
