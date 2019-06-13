/*
 * Developed by Wang Xiao on 6/12/19 11:57 PM.
 * Last modified 6/12/19 2:10 PM.
 * Copyright (c) 2019. All rights reserved.
 */
 
// AutoMine.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "resource.h"
#include <tlhelp32.h>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];								// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];								// The title bar text
DWORD dwPID;

// Foward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
DWORD GetPID();
BOOL StartWinmine();
BOOL InjectDll();

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_AUTOMINE, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_AUTOMINE);

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage is only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_AUTOMINE);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCSTR)IDC_AUTOMINE;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HANDLE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   MoveWindow(hWnd, 800, 360, 320, 240, TRUE);
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	TCHAR szHello[MAX_LOADSTRING];
	LoadString(hInst, IDS_HELLO, szHello, MAX_LOADSTRING);

	switch (message) 
	{
		case WM_COMMAND:
			wmId    = LOWORD(wParam); 
			wmEvent = HIWORD(wParam); 
			// Parse the menu selections:
			switch (wmId)
			{
				case IDM_ABOUT:
				   DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
				   break;
				case IDM_EXIT:
				   DestroyWindow(hWnd);
				   break;
				case IDM_WINMINE:
					// WinExec("c:\\windows\\system32\\winmine.exe", TRUE);
					StartWinmine();
					break;
				case IDM_AUTOMINE:
					InjectDll();
					break;
				default:
				   return DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;
		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			// TODO: Add any drawing code here...
			RECT rt;
			GetClientRect(hWnd, &rt);
			DrawText(hdc, szHello, strlen(szHello), &rt, DT_CENTER);
			EndPaint(hWnd, &ps);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

// Mesage handler for about box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
				return TRUE;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
			{
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}
			break;
	}
    return FALSE;
}

DWORD GetPID()
{
	HANDLE hSnapshot = NULL;
	PROCESSENTRY32 pe;
	BOOL bMore = FALSE;

	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
  
	ZeroMemory(&pe, sizeof(pe));
	pe.dwSize = sizeof(pe);

	bMore = Process32First(hSnapshot, &pe);	
	
	while(bMore)
	{	
		if(strcmp(pe.szExeFile,"winmine.exe") == 0)	
		{			
			CloseHandle(hSnapshot);
			return pe.th32ProcessID;
		}
		bMore = Process32Next(hSnapshot, &pe);
	}

	CloseHandle(hSnapshot);
	return 0;
}

BOOL StartWinmine()
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	
	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	ZeroMemory( &pi, sizeof(pi) );
	
	if( !CreateProcess( NULL,						// No module name (use command line). 
		TEXT("c:\\windows\\system32\\winmine.exe"), // Command line. 
		NULL,										// Process handle not inheritable. 
		NULL,										// Thread handle not inheritable. 
		FALSE,										// Set handle inheritance to FALSE. 
		0,											// No creation flags. 
		NULL,										// Use parent's environment block. 
		NULL,										// Use parent's starting directory. 
		&si,										// Pointer to STARTUPINFO structure.
		&pi )										// Pointer to PROCESS_INFORMATION structure.
		) 
	{
		return FALSE;
	}
	
	dwPID = pi.dwProcessId;
	// WaitForSingleObject( pi.hProcess, INFINITE );

	CloseHandle( pi.hProcess );
	CloseHandle( pi.hThread );
	return TRUE;
}

BOOL InjectDll()
{
	if (dwPID == 0)
	{
		//return FALSE;
	}

	BOOL   bResult          = FALSE; 
	HANDLE hProcess         = NULL;
	HANDLE hThread          = NULL;
	char*  pszLibFileRemote = NULL;
	LPTSTR lpszLibName		= "E:\\develop\\vc++6.0\\miner\\Debug\\miner.dll";
	//LPTSTR lpszLibName		= "G:\\miner\\Debug\\miner.dll";
	
	__try 
	{
		//hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID);        
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetPID());    
		if (hProcess == NULL)
		{
			DWORD err = GetLastError();
			__leave;
		}
		
		int cch = 1 + strlen(lpszLibName);
		
		pszLibFileRemote = (PSTR)VirtualAllocEx(hProcess, NULL, cch, MEM_COMMIT, PAGE_READWRITE);		
		if (pszLibFileRemote == NULL) 
		{
			__leave;
		}
		
		if (!WriteProcessMemory(hProcess, (PVOID)pszLibFileRemote, (PVOID)lpszLibName, cch, NULL))
		{
			__leave;
		}
		
		PTHREAD_START_ROUTINE pfnThreadRtn = (PTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle("Kernel32"), "LoadLibraryA");		
		if (pfnThreadRtn == NULL) 
		{
			__leave;
		}
		
		hThread = CreateRemoteThread(hProcess, NULL, 0, pfnThreadRtn, (PVOID)pszLibFileRemote, 0, NULL);
		if (hThread == NULL) 
		{
			__leave;
		}
		
		WaitForSingleObject(hThread, INFINITE);
		
		bResult = TRUE; 
	}
	__finally 
	{ 
		if (pszLibFileRemote != NULL) 
		{
			VirtualFreeEx(hProcess, (PVOID)pszLibFileRemote, 0, MEM_RELEASE);
		}
		
		if (hThread != NULL)
		{
			CloseHandle(hThread);
		}
		
		if (hProcess != NULL) 
		{
			CloseHandle(hProcess);
		}
	}
	return bResult;
}