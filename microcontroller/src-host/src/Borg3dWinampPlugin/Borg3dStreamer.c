// Winamp test visualization library v1.0
// Copyright (C) 1997-1998, Justin Frankel/Nullsoft
// Feel free to base any plugins on this "framework"...

#include <windows.h>
#include "winamp\wa_ipc.h"

#include "vis.h"
#include "uart.h"
#include "pixel3d.h"
#include "resource.h"

char szAppName[] = "Borg 3D Visualisation"; // Our window class, etc
char comPort[5] = "COM1";

// configuration declarations
int config_x=50, config_y=50;	// screen X position and Y position, repsectively
void config_read(struct winampVisModule *this_mod);		// reads the configuration
void config_write(struct winampVisModule *this_mod);	// writes the configuration
void config_getinifn(struct winampVisModule *this_mod, char *ini_file); // makes the .ini file filename

// returns a winampVisModule when requested. Used in hdr, below
winampVisModule *getModule(int which);

// "member" functions
void config(struct winampVisModule *this_mod); // configuration dialog
int init(struct winampVisModule *this_mod);	   // initialization for module
//int render1(struct winampVisModule *this_mod);  // rendering for module 1
//int render2(struct winampVisModule *this_mod);  // rendering for module 2
int render3(struct winampVisModule *this_mod);  // rendering for module 3
void quit(struct winampVisModule *this_mod);   // deinitialization for module
int random_presets_flag = 0;

int bSimulator = 1;
int bStreaming = 0;


// our window procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
HWND hMainWnd; // main window handle

// Double buffering data
HDC memDC;		// memory device context
HBITMAP	memBM,  // memory bitmap (for memDC)
		oldBM;  // old bitmap (from memDC)


// Module header, includes version, description, and address of the module retriever function
winampVisHeader hdr = { VIS_HDRVER, "Borg-3d Streamer", getModule };

/*
// first module (oscilliscope)
winampVisModule mod1 =
{
	"Oscilloscope",
	NULL,	// hwndParent
	NULL,	// hDllInstance
	0,		// sRate
	0,		// nCh
	25,		// latencyMS
	25,		// delayMS
	0,		// spectrumNch
	2,		// waveformNch
	{ 0, },	// spectrumData
	{ 0, },	// waveformData
	config,
	init,
	render1, 
	quit
};

// second module (spectrum analyser)
winampVisModule mod2 =
{
	"Spectrum Analyser",
	NULL,	// hwndParent
	NULL,	// hDllInstance
	0,		// sRate
	0,		// nCh
	25,		// latencyMS
	25,		// delayMS
	2,		// spectrumNch
	0,		// waveformNch
	{ 0, },	// spectrumData
	{ 0, },	// waveformData
	config,
	init,
	render2, 
	quit
};
*/

// third module (VU meter)
winampVisModule mod3 =
{
	"Borg Bars",
	NULL,	// hwndParent
	NULL,	// hDllInstance
	0,		// sRate
	0,		// nCh
	25,		// latencyMS
	25,		// delayMS
	0,		// spectrumNch
	2,		// waveformNch
	{ 0, },	// spectrumData
	{ 0, },	// waveformData
	config,
	init,
	render3, 
	quit
};


// this is the only exported symbol. returns our main header.
// if you are compiling C++, the extern "C" { is necessary, so we just #ifdef it
#ifdef __cplusplus
extern "C" {
#endif
__declspec( dllexport ) winampVisHeader *winampVisGetHeader()
{
	return &hdr;
}
#ifdef __cplusplus
}
#endif

// getmodule routine from the main header. Returns NULL if an invalid module was requested,
// otherwise returns either mod1, mod2 or mod3 depending on 'which'.
winampVisModule *getModule(int which)
{
	switch (which)
	{
		//case 0: return &mod1;
		//case 1: return &mod2;
		case 0: return &mod3;
		default:return NULL;
	}
}


LRESULT CALLBACK ConfigProc( HWND hDlg, UINT uMsg,
                               WPARAM wParam, LPARAM lParam );
// configuration. Passed this_mod, as a "this" parameter. Allows you to make one configuration
// function that shares code for all your modules (you don't HAVE to use it though, you can make
// config1(), config2(), etc...)
void config(struct winampVisModule *this_mod)
{
     DialogBox(this_mod->hDllInstance, MAKEINTRESOURCE(IDD_CONFIGDIALOG), this_mod->hwndParent, (DLGPROC)ConfigProc);     
}

//Nachrichtenverarbeitung f�r die "Listbox
LRESULT CALLBACK ConfigProc( HWND hDlg, UINT uMsg,
                               WPARAM wParam, LPARAM lParam )
{                               
	switch( uMsg ){
		case WM_INITDIALOG:
			SetDlgItemText(hDlg, IDC_COMPORT, comPort);
            CheckDlgButton(hDlg, IDC_SIMULATOR, bSimulator);
            CheckDlgButton(hDlg, IDC_SERIALSTREAMING, bStreaming);
			return TRUE;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK) {
                GetDlgItemText(hDlg, IDC_COMPORT, comPort, 5);
                bSimulator = IsDlgButtonChecked(hDlg, IDC_SIMULATOR);
                bStreaming = IsDlgButtonChecked(hDlg, IDC_SERIALSTREAMING);
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			} else if (LOWORD(wParam) == IDCANCEL) {
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}						
			break;
	}
    return FALSE;
}




embedWindowState myWindowState;

winampVisModule *g_mod = NULL;

int width;
int height;


// initialization. Registers our window class, creates our window, etc. Again, this one works for
// both modules, but you could make init1() and init2()...
// returns 0 on success, 1 on failure.
int init(struct winampVisModule *this_mod)
{	
  int styles;
  HWND parent = NULL;
  HWND (*e)(embedWindowState *v);

	width = (this_mod == &mod3)?256:288; // width and height are the same for mod1 and mod2,
	height = (this_mod == &mod3)?32:256; // but mod3 is shaped differently

  g_mod = this_mod;

	config_read(this_mod);

    //hConfig = CreateDialog(this_mod->hDllInstance, MAKEINTRESOURCE(IDD_CONFIGDIALOG), NULL, (DLGPROC)ConfigProc);
    //if (hConfig == 0)
   //  /  MessageBox(parent, "Init error",0,0);
	


  // uncomment this line if your plugin draws to the screen using directx OVERLAY mode
  // myWindowState.flags |= EMBED_FLAGS_NOTRANSPARENCY; 
  
  myWindowState.r.left = config_x;
  myWindowState.r.top = config_y;
  myWindowState.r.right = config_x + width;
  myWindowState.r.bottom = config_y + height;
     
  *(void**)&e = (void *)SendMessage(this_mod->hwndParent,WM_WA_IPC,(LPARAM)0,IPC_GET_EMBEDIF);

  if (!e)
  {
		MessageBox(this_mod->hwndParent,"This plugin requires Winamp 5.0+","blah",MB_OK);
		return 1;
  }

  parent = e(&myWindowState);

  SetWindowText(myWindowState.me, this_mod->description); // set our window title
	
	{	// Register our window class
		WNDCLASS wc;
		memset(&wc,0,sizeof(wc));
		wc.lpfnWndProc = WndProc;				// our window procedure
		wc.hInstance = this_mod->hDllInstance;	// hInstance of DLL
		wc.lpszClassName = szAppName;			// our window class name
	
		if (!RegisterClass(&wc)) 
		{
			MessageBox(this_mod->hwndParent,"Error registering window class","blah",MB_OK);
			return 1;
		}
	}

  styles = WS_VISIBLE|WS_CHILDWINDOW|WS_OVERLAPPED|WS_CLIPCHILDREN|WS_CLIPSIBLINGS;


	hMainWnd = CreateWindowEx(
		0,	// these exstyles put a nice small frame, 
											// but also a button in the taskbar
		szAppName,							// our window class name
		NULL,				// no title, we're a child
		styles,				                   // do not make the window visible 
		config_x,config_y,					// screen position (read from config)
		width,height,						// width & height of window (need to adjust client area later)
		parent,				// parent window (winamp main window)
		NULL,								// no menu
		this_mod->hDllInstance,				// hInstance of DLL
		0); // no window creation data

	if (!hMainWnd) 
	{
		MessageBox(this_mod->hwndParent,"Error creating window","blah",MB_OK);
		return 1;
	}


	SetWindowLong(hMainWnd,GWL_USERDATA,(LONG)this_mod); // set our user data to a "this" pointer

/*	{	// adjust size of window to make the client area exactly width x height
		RECT r;
		GetClientRect(hMainWnd,&r);
		SetWindowPos(hMainWnd,0,0,0,width*2-r.right,height*2-r.bottom,SWP_NOMOVE|SWP_NOZORDER);
	}*/

  SendMessage(this_mod->hwndParent, WM_WA_IPC, (int)hMainWnd, IPC_SETVISWND);

	// create our doublebuffer
	memDC = CreateCompatibleDC(NULL);
	memBM = CreateCompatibleBitmap(memDC,width,height);
	oldBM = SelectObject(memDC,memBM);

  {
    RECT r={0,0,width,height};
    FillRect(memDC, &r, GetStockObject(WHITE_BRUSH));
  }

	// show the window
	//ShowWindow(parent,SW_SHOWNORMAL);
	uart_init(comPort);
	return 0;
}
/*
// render function for oscilliscope. Returns 0 if successful, 1 if visualization should end.
int render1(struct winampVisModule *this_mod)
{
	int x, y;
	// clear background
	Rectangle(memDC,0,0,288,256);
	// draw oscilliscope
	for (y = 0; y < this_mod->nCh; y ++)
	{
		MoveToEx(memDC,0,(y*256)>>(this_mod->nCh-1),NULL);
		for (x = 0; x < 288; x ++)
		{
			LineTo(memDC,x,(y*256 + this_mod->waveformData[y][x]^128)>>(this_mod->nCh-1));
		}
	}
	{ // copy doublebuffer to window
		HDC hdc = GetDC(hMainWnd);
		BitBlt(hdc,0,0,288,256,memDC,0,0,SRCCOPY);
		ReleaseDC(hMainWnd,hdc);
	}
	return 0;
}

// render function for analyser. Returns 0 if successful, 1 if visualization should end.
int render2(struct winampVisModule *this_mod)
{
	int x, y;
	// clear background
	Rectangle(memDC,0,0,288,256);
	// draw analyser
	for (y = 0; y < this_mod->nCh; y ++)
	{
		for (x = 0; x < 288; x ++)
		{
			MoveToEx(memDC,x,(y*256+256)>>(this_mod->nCh-1),NULL);
			LineTo(memDC,x,(y*256 + 256 - this_mod->spectrumData[y][x])>>(this_mod->nCh-1));
		}
	}
	{ // copy doublebuffer to window
		HDC hdc = GetDC(hMainWnd);
		BitBlt(hdc,0,0,288,256,memDC,0,0,SRCCOPY);
		ReleaseDC(hMainWnd,hdc);
	}
	return 0;
}
*/

int count = 0;
// render function for VU meter. Returns 0 if successful, 1 if visualization should end.
int render3(struct winampVisModule *this_mod)
{
	int x, y;
	// clear background
	clear_screen(0);
	// draw VU meter
	for (y = 0; y < 2; y ++)
	{
		int last=this_mod->waveformData[y][0];
		int total=0, total2;
		for (x = 1; x < 576; x ++)
		{
			total += abs(last - this_mod->waveformData[y][x]);
			total2 += this_mod->waveformData[y][x];
			last = this_mod->waveformData[y][x];
		}
		total /= 288;
		if (total > 127) total = 127;
		total /= 16;
		
        if (count++ > 10000) {
        	if (y) {
               drawBar(0, total % 8);
               drawBar(1, (total2/288) % 8);
            } else {
                   drawBar(3, total % 8);
                   drawBar(2, (total2/288) % 8);
            }
        } else {
 	           if (y) growingCubeFilled(total % 8); 
        }
	}
	if (count == 20000) {
       uart_sendStopp();
    } 
	sendPixmap();
	return 0;
}

// cleanup (opposite of init()). Destroys the window, unregisters the window class
void quit(struct winampVisModule *this_mod)
{
  SendMessage(this_mod->hwndParent, WM_WA_IPC, 0, IPC_SETVISWND);

	config_write(this_mod);		// write configuration
	SelectObject(memDC,oldBM);	// delete our doublebuffer
	DeleteObject(memDC);
	DeleteObject(memBM);
    uart_delete();	
  // delete our window
  if (myWindowState.me) 
  {
    SetForegroundWindow(g_mod->hwndParent);
    DestroyWindow(myWindowState.me);
  }
	UnregisterClass(szAppName,this_mod->hDllInstance); // unregister window class
}

void next_preset()
{
}

void previous_preset()
{
}

void load_random_preset()
{
}

void set_random(int r)
{
  random_presets_flag = r;
}

void go_fullscreen()
{
  if (SendMessage(g_mod->hwndParent,WM_WA_IPC,0,IPC_IS_PLAYING_VIDEO)>1) 
  {
    MessageBox(g_mod->hwndParent, "Can't go fullscreen while video is playing", "SVis", 0);
  }
  else
  {
    SendMessage(g_mod->hwndParent,WM_WA_IPC,1,IPC_SET_VIS_FS_FLAG);

    // ... now do the work of actually going fullscreen ...

  }
}

void go_windowed()
{
  SendMessage(g_mod->hwndParent,WM_WA_IPC,0,IPC_SET_VIS_FS_FLAG);
}

void open_configuration()
{
}

void open_popup_menu()
{
}

// window procedure for our window
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CREATE:		return 0;
		case WM_ERASEBKGND: return 0;
		case WM_PAINT:
			{ // update from doublebuffer
				PAINTSTRUCT ps;
				RECT r;
				HDC hdc = BeginPaint(hwnd,&ps);
				GetClientRect(hwnd,&r);
				BitBlt(hdc,0,0,r.right,r.bottom,memDC,0,0,SRCCOPY);
        {
          RECT x={r.left+width, r.top, r.right, r.bottom};
          RECT y={r.left, r.top+height, r.right, r.bottom};
          FillRect(hdc, &x, GetStockObject(WHITE_BRUSH));
          FillRect(hdc, &y, GetStockObject(WHITE_BRUSH));
        }
				EndPaint(hwnd,&ps);
			}
		return 0;
		case WM_DESTROY: PostQuitMessage(0); return 0;
		case WM_KEYDOWN: // pass keyboard messages to main winamp window (for processing)
		case WM_KEYUP:
			{	// get this_mod from our window's user data
				winampVisModule *this_mod = (winampVisModule *) GetWindowLong(hwnd,GWL_USERDATA);
				PostMessage(this_mod->hwndParent,message,wParam,lParam);
			}
		return 0;
		case WM_WINDOWPOSCHANGING:
			{	// get config_x and config_y for configuration
				RECT r;
				GetWindowRect(myWindowState.me,&r);
				config_x = r.left;
				config_y = r.top;
			}
		return 0;
    case WM_COMMAND: {
      int id = LOWORD(wParam);
      switch (id) {

        // user clicked on 'next' preset button
        case ID_VIS_NEXT: next_preset(); break;

        // user clicked on 'previous' preset button
        case ID_VIS_PREV: previous_preset(); break;

        // user clicked on 'random' togglebutton
        case ID_VIS_RANDOM: {
          // determine if we're switching random on or off or if Winamp is asking us about the state of our random flag
          int v = HIWORD(wParam) ? 1 : 0; 

          // are we being asked about the state of our random flag ?
          if (wParam >> 16 == 0xFFFF) {
            // tell winamp about our state
            SendMessage(g_mod->hwndParent,WM_WA_IPC,random_presets_flag,IPC_CB_VISRANDOM);
            break;
          }
      
          // changes random_preset_flag 
          set_random(v); 

          // if we are turning random on, we should switch to a new random preset right away
          if (v) load_random_preset();

          break;
        }
        case ID_VIS_FS: go_fullscreen(); break;
        case ID_VIS_CFG: open_configuration(); break;
        case ID_VIS_MENU: open_popup_menu(); break;
      }
      break;
    }
	}
	return DefWindowProc(hwnd,message,wParam,lParam);
}


void config_getinifn(struct winampVisModule *this_mod, char *ini_file)
{	// makes a .ini file in the winamp directory named "plugin.ini"
	char *p;
	GetModuleFileName(this_mod->hDllInstance,ini_file,MAX_PATH);
	p=ini_file+strlen(ini_file);
	while (p >= ini_file && *p != '\\') p--;
	if (++p >= ini_file) *p = 0;
	strcat(ini_file,"plugin.ini");
}


void config_read(struct winampVisModule *this_mod)
{
	char ini_file[MAX_PATH];
	config_getinifn(this_mod,ini_file);
	config_x = GetPrivateProfileInt(this_mod->description,"Screen_x",config_x,ini_file);
	config_y = GetPrivateProfileInt(this_mod->description,"Screen_y",config_y,ini_file);
}

void config_write(struct winampVisModule *this_mod)
{
	char string[32];
	char ini_file[MAX_PATH];

	config_getinifn(this_mod,ini_file);

	wsprintf(string,"%d",config_x);
	WritePrivateProfileString(this_mod->description,"Screen_x",string,ini_file);
	wsprintf(string,"%d",config_y);
	WritePrivateProfileString(this_mod->description,"Screen_y",string,ini_file);
}
