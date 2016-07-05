#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")


#include "StaticOverlay.h"
#include "WindowOverlay.h"
#include "OverlayManager.h"
#include "OverlayTexture.h"
#include "RenderEnvironment.h"
#include "VertexObject.h"
#include <openvr.h>
#include "resources.h"
#include "../UniversalVROverlay/resource.h"
#include <Psapi.h>
#include <PathCch.h>

#include <CommCtrl.h>



//Will Need to move to WindowManager class
#include <vector>
#include "../UniversalVROverlay/WindowDescriptor.h"


#pragma comment(lib, "Pathcch.lib")

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
HWND  hWnd;										// Main Window HWND
HFONT hFont;


HWND AddButton, DelButton, OverlayList;			//Controler HWNDs

HWND SettingsStatic;							
//Static Area Controllers
HWND TitleText, TitleBox;
HWND OverlayTypeText, OverlayTypeBox;
HWND KeybindText, KeybindBox;
HWND xRotate, yRotate, zRotate;
HWND xRotateText, yRotateText, zRotateText;
HWND xTranslate, yTranslate, zTranslate;
HWND xTranslateText, yTranslateText, zTranslateText;

WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HWND targethWnd;
HWND targethWnd2;

//D3D env
RenderEnvironment* env;

//OverlayMgr
OverlayManager* mgr;


std::vector<WindowDescriptor> wndVec;

												// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
BOOL    CALLBACK    EnumProc(HWND hWnd, LPARAM lParam);
INT_PTR	CALLBACK	DlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	//AllocConsole();
	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_WIN32PROJECT3, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);
	hFont = CreateFont(
		14, 0, 0, 0, FW_MEDIUM, FALSE, FALSE, FALSE, ANSI_CHARSET,
		OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, L"Arial");

	//Init d3d and manager
	//TODO: add VR hooking to it's own class
	


	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}
	
	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WIN32PROJECT3));

	MSG msg;

	
	//----VR Init-------------------------------------------------------
	vr::HmdError m_eLastHmdError;
	vr::IVRSystem *pVRSystem = vr::VR_Init(&m_eLastHmdError, vr::VRApplication_Overlay);

	if (m_eLastHmdError != vr::VRInitError_None)
	{

		return 1;
	}

	//----Overlay D3dInit-----------------------------------------------



	int32_t adapterIndex;
	pVRSystem->GetDXGIOutputInfo(&adapterIndex);

	env = new RenderEnvironment(adapterIndex);
	mgr = new OverlayManager();
	
	
	OverlayTexture* overlayTexture = new OverlayTexture(env);
	OverlayTexture* overlayTexture2 = new OverlayTexture(env);



	//Setup viewport
	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = static_cast<float>(1280);
	viewport.Height = static_cast<float>(720);
	viewport.MinDepth = D3D11_MIN_DEPTH;
	viewport.MaxDepth = D3D11_MAX_DEPTH;

	//D3D11
	env->getContext()->RSSetViewports(1, &viewport);

	

	//-----Overlay Config-----------------------------------------------	
	EnumWindows(EnumProc, 0);

	WindowOverlay* overlay = new WindowOverlay(overlayTexture);
	overlay->setHwnd(targethWnd);

	//WindowOverlay* overlay2 = new WindowOverlay(overlayTexture2);
	//overlay2->setHwnd(targethWnd2);

	
	mgr->addOverlay(overlay);
	//mgr->addOverlay(overlay2);

	vr::HmdMatrix34_t overlayDistanceMtx;
	memset(&overlayDistanceMtx, 0, sizeof(overlayDistanceMtx));
	overlayDistanceMtx.m[0][0] = overlayDistanceMtx.m[1][1] = overlayDistanceMtx.m[2][2] = 1.0f;
	overlayDistanceMtx.m[2][0] = 1.0f;
	overlayDistanceMtx.m[0][3] = 0.8f;
	overlayDistanceMtx.m[1][3] = -.5f;
	overlayDistanceMtx.m[2][3] = -1.0f;


	overlay->setOverlayMatrix(overlayDistanceMtx);
	overlay->ShowOverlay();

	vr::HmdMatrix34_t overlayDistanceMtx2;
	memset(&overlayDistanceMtx2, 0, sizeof(overlayDistanceMtx2));
	overlayDistanceMtx2.m[0][0] = overlayDistanceMtx2.m[1][1] = overlayDistanceMtx2.m[2][2] = 1.0f;
	overlayDistanceMtx2.m[2][0] = -1.0f;
	overlayDistanceMtx2.m[0][3] = -0.8f;
	overlayDistanceMtx2.m[1][3] = 0.5f;
	overlayDistanceMtx2.m[2][3] = -1.0f;

	//overlay2->setOverlayMatrix(overlayDistanceMtx2);
	//overlay2->ShowOverlay();


	//------------------------------------------------------------------
	// Main message loop:
	
	while (true)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;
			if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			
		}
		
		//TODO: Put in a thread to run dynamically
		//env->RenderFrame();

		//overlay->updateTexture();
		//overlay2->updateTexture();
		const std::vector<std::shared_ptr<Overlay>> vec = mgr->getOverlays();
		for (std::vector<std::shared_ptr<Overlay>>::const_iterator it = vec.begin(); it != vec.end(); ++it)
		{
			(*it)->updateTexture();
		}

		if (mgr && mgr->getOverlays().size() > 1)
		{

			mgr->getOverlays()[1]->setTrans(X_AXIS, SendMessage(xTranslate, TBM_GETPOS, 0, 0));
			mgr->getOverlays()[1]->setTrans(Y_AXIS, SendMessage(yTranslate, TBM_GETPOS, 0, 0));
			mgr->getOverlays()[1]->setTrans(Z_AXIS, SendMessage(zTranslate, TBM_GETPOS, 0, 0));

			mgr->getOverlays()[1]->setRotate(X_AXIS, SendMessage(xRotate, TBM_GETPOS, 0, 0));
			mgr->getOverlays()[1]->setRotate(Y_AXIS, SendMessage(yRotate, TBM_GETPOS, 0, 0));
			mgr->getOverlays()[1]->setRotate(Z_AXIS, SendMessage(zRotate, TBM_GETPOS, 0, 0));
		}
	}
	_CrtDumpMemoryLeaks();
	return (int)msg.wParam;
}

BOOL CALLBACK EnumProc(HWND hWnd, LPARAM lParam)
{
	TCHAR title[500];
	TCHAR wndTitle[500];
	ZeroMemory(title, sizeof(title));

	if (IsWindowVisible(hWnd))
	{
		//GetWindowText(hWnd, title, sizeof(title) / sizeof(title[0]));
		DWORD PID = NULL;
		GetWindowThreadProcessId(hWnd, &PID);
		HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, PID);
		GetModuleFileNameEx(hProcess, NULL, title, sizeof(title) / sizeof(title[0]));
		//GetModuleFileName()
		//GetWindowModuleFileName(hWnd, title, sizeof(title) / sizeof(title[0]));
		
		TCHAR *name = wcsrchr(title, L'\\');
		
		
		if (GetWindowText(hWnd, wndTitle, 500) > 0)
		{
			//--------------------------------------------------
			wndVec.push_back(WindowDescriptor(hWnd, std::wstring(wndTitle), std::wstring(name)));
			
			if (lstrcmpW(name, L"\\vlc.exe") == 0)
			{
				//MessageBox(NULL, name, name, MB_OK);
				targethWnd = hWnd;
			}
			if (lstrcmpW(name, L"\\Discord.exe") == 0)
			{
				//MessageBox(NULL, name, name, MB_OK);
				targethWnd2 = hWnd;
			}
		}

		//--------------------------------------------------

		

		CloseHandle(hProcess);
		
	}

	
	return TRUE;
}


//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WIN32PROJECT3));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_WIN32PROJECT3);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
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
	hInst = hInstance; // Store instance handle in our global variable

	/*hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 640 , 480, nullptr, nullptr, hInstance, nullptr);*/
	hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, 785,565, nullptr, nullptr, hInstance, nullptr);
	SendMessage(hWnd, WM_SETFONT, (WPARAM)hFont, TRUE);

	if (!hWnd)
	{
		return FALSE;
	}
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	
	switch (message)
	{
	case WM_CREATE:
	{
		//---Define Window Controls-----------------------------------------
		AddButton = CreateWindowEx(
			NULL,
			L"Button",
			L"+",
			WS_BORDER | WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			225, 0,
			30, 30,
			hWnd, NULL,
			hInst,
			NULL);
		SendMessage(AddButton, WM_SETFONT, (WPARAM)hFont, TRUE);
		OverlayList = CreateWindowEx(
			NULL,
			L"ListBox",
			L"test test test",
			WS_BORDER | WS_CHILD | WS_VISIBLE,
			5, 30,
			250, 500,
			hWnd, NULL,
			hInst,
			NULL);
		SendMessage(OverlayList, WM_SETFONT, (WPARAM)hFont, TRUE);
		SettingsStatic = CreateWindowEx(
			NULL,
			L"Button",
			L"Overlay Settings",
			WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
			265, 23,
			500, 500,
			hWnd, NULL,
			hInst,
			NULL);
		SendMessage(SettingsStatic, WM_SETFONT, (WPARAM)hFont, TRUE);

		//-----------------------------------------------

		/*
		HWND TitleText, TitleBox;
		HWND OverlayTypeText, OverlayTypeBox;
		HWND KeybindText, KeybindBox;
		
		*/

		TitleText = CreateWindowEx(
			NULL,
			L"Static",
			L"Title: ",
			WS_CHILD | WS_VISIBLE | SS_LEFT,
			10, 22,
			100, 18,
			SettingsStatic, NULL,
			hInst,
			NULL);
		SendMessage(TitleText, WM_SETFONT, (WPARAM)hFont, TRUE);

		TitleBox = CreateWindowEx(
			NULL,
			L"edit",
			L"Test",
			WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | ES_LEFT,
			85, 20,
			100, 18,
			SettingsStatic, NULL,
			hInst,
			NULL);
		SendMessage(TitleBox, WM_SETFONT, (WPARAM)hFont, TRUE);


		OverlayTypeText = CreateWindowEx(
			NULL,
			L"Static",
			L"Overlay Type: ",
			WS_CHILD | WS_VISIBLE | SS_LEFT,
			10, 47,
			100, 18,
			SettingsStatic, NULL,
			hInst,
			NULL);
		SendMessage(OverlayTypeText, WM_SETFONT, (WPARAM)hFont, TRUE);

		OverlayTypeBox = CreateWindowEx(
			NULL,
			L"edit",
			L"Test",
			WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | ES_LEFT,
			85, 45,
			100, 18,
			SettingsStatic, NULL,
			hInst,
			NULL);
		SendMessage(OverlayTypeBox, WM_SETFONT, (WPARAM)hFont, TRUE);

		KeybindText = CreateWindowEx(
			NULL,
			L"Static",
			L"Keybind: ",
			WS_CHILD | WS_VISIBLE | SS_LEFT,
			10, 72,
			100, 18,
			SettingsStatic, NULL,
			hInst,
			NULL);
		SendMessage(KeybindText, WM_SETFONT, (WPARAM)hFont, TRUE);

		KeybindBox = CreateWindowEx(
			NULL,
			L"edit",
			L"Test",
			WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | ES_LEFT,
			85, 70,
			100, 18,
			SettingsStatic, NULL,
			hInst,
			NULL);
		SendMessage(KeybindBox, WM_SETFONT, (WPARAM)hFont, TRUE);

		/*
		HWND xRotate, yRotate, zRotate;
		HWND xRotateText, yRotateText, zRotateText;
		HWND xTranslate, yTranslate, zTranslate;
		HWND xTranslateText, yTranslateText, zTranslateText;
		*/
		xRotate = CreateWindowEx(
			NULL,
			TRACKBAR_CLASSW,
			L"Test",
			WS_CHILD | WS_VISIBLE | WS_TABSTOP,
			10, 100,
			175, 18,
			SettingsStatic, NULL,
			hInst,
			NULL);
		SendMessage(xRotate, TBM_SETRANGE, TRUE, MAKELONG(-180, 180));

		yRotate = CreateWindowEx(
			NULL,
			TRACKBAR_CLASSW,
			L"Test",
			WS_CHILD | WS_VISIBLE | WS_TABSTOP,
			10, 130,
			175, 18,
			SettingsStatic, NULL,
			hInst,
			NULL);
		SendMessage(yRotate, TBM_SETRANGE, TRUE, MAKELONG(-180, 180));
		
		zRotate = CreateWindowEx(
			NULL,
			TRACKBAR_CLASSW,
			L"Test",
			WS_CHILD | WS_VISIBLE | WS_TABSTOP,
			10, 160,
			175, 18,
			SettingsStatic, NULL,
			hInst,
			NULL);
		SendMessage(zRotate, TBM_SETRANGE, TRUE, MAKELONG(-180, 180));



		//-----------------------------------------------

		xTranslate = CreateWindowEx(
			NULL,
			TRACKBAR_CLASSW,
			L"Test",
			WS_CHILD | WS_VISIBLE | WS_TABSTOP,
			10, 200,
			175, 18,
			SettingsStatic, NULL,
			hInst,
			NULL);
		SendMessage(xTranslate, TBM_SETRANGE, TRUE, MAKELONG(-100, 100));

		yTranslate = CreateWindowEx(
			NULL,
			TRACKBAR_CLASSW,
			L"Test",
			WS_CHILD | WS_VISIBLE | WS_TABSTOP,
			10, 230,
			175, 18,
			SettingsStatic, NULL,
			hInst,
			NULL);
		SendMessage(yTranslate, TBM_SETRANGE, TRUE, MAKELONG(-100, 100));

		zTranslate = CreateWindowEx(
			NULL,
			TRACKBAR_CLASSW,
			L"Test",
			WS_CHILD | WS_VISIBLE | WS_TABSTOP,
			10, 260,
			175, 18,
			SettingsStatic, NULL,
			hInst,
			NULL);
		SendMessage(zTranslate, TBM_SETRANGE, TRUE, MAKELONG(-100, 100));




		//TODO: Populate list from saved data;


		break;
	}
	case WM_GETMINMAXINFO:
	{
		MINMAXINFO* mmi = (MINMAXINFO*)lParam;
		mmi->ptMinTrackSize.x = 785;
		mmi->ptMinTrackSize.y = 565;
		return 0;
		break;
	}

	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Parse the menu selections:
		int hiWd = HIWORD(wParam);
		switch (hiWd)
		{
		case BN_CLICKED:
			/*TODO:
				*Spawn Dialog for window selection
				*Handle Dialog closing
				*If OK, create Overlay object, add it to list, add it to overlayManager
				*Destroy dialog
			*/
			DialogBox(hInst, MAKEINTRESOURCE(IDD_WNDSELECT), hWnd, DlgProc);
			//MessageBox(hWnd, L"Button clicked", L"OK", MB_OK);
		}
		switch (wmId)
		{

		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);

		// TODO: Add any drawing code that uses hdc here...
		
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK DlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND cBoxHwnd;
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		//TODO: Move this to a windowmanager.getList function
		cBoxHwnd = GetDlgItem(hwndDlg, IDC_COMBO1);
		wndVec.clear();
		EnumWindows(EnumProc, 0);
		for (std::vector<WindowDescriptor>::iterator it = wndVec.begin(); it != wndVec.end(); ++it)
		{
			SendMessage(cBoxHwnd, CB_ADDSTRING, 0, (LPARAM)(it->getTitle()).c_str());
		}
		
		
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_WNDREFRESH)
		{
			cBoxHwnd = GetDlgItem(hwndDlg, IDC_COMBO1);
			SendMessage(cBoxHwnd, CB_RESETCONTENT, 0, 0);
			wndVec.clear();
			EnumWindows(EnumProc, 0);
			for (std::vector<WindowDescriptor>::iterator it = wndVec.begin(); it != wndVec.end(); ++it)
			{
				SendMessage(cBoxHwnd, CB_ADDSTRING, 0, (LPARAM)(it->getTitle()).c_str());
			}
		}
		if (LOWORD(wParam) == IDYES )
		{
			cBoxHwnd = GetDlgItem(hwndDlg, IDC_COMBO1);
			int index = SendMessage(cBoxHwnd, CB_GETCURSEL, 0, 0);
			if (index != CB_ERR)
			{
				WindowOverlay* overlay = new WindowOverlay(new OverlayTexture(env));
				mgr->addOverlay(overlay);
				
				vr::HmdMatrix34_t overlayDistanceMtx2;
				memset(&overlayDistanceMtx2, 0, sizeof(overlayDistanceMtx2));
				overlay->setHwnd(wndVec[index].getHwnd());
				overlay->setOverlayMatrix(overlayDistanceMtx2);
				overlay->ShowOverlay();


			}
			
			EndDialog(hwndDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		if (LOWORD(wParam) == IDNO)
		{
			EndDialog(hwndDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;

}


