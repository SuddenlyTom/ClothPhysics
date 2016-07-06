//
//	Bachelor of Software Engineering (Game Programming)
//	Media Design School
//	Auckland
//	New Zealand
//
//	(c) 2015 Media Design School
//
//	File Name		:		CubeApp.cpp
//	Description		:		Implementation of the CubeApp class
//	Author			:		Tom O'Brien
//	Email			:		Thomas.OBrien@mediadesign.school.nz
//

//Local Includes
#include "WindowApp.h"
#include "Cloth.h"
#include "Camera.h"
#include "Light.h"
#include "Terrain.h"
#include "DebugLines.h"
#include "resource.h"

#include "Sphere.h"
#include "Capsule.h"
#include "Pyramid.h"

#include "Commctrl.h"
#include "Utility.h"

#define GRAVITY -0.9f




class MainApp : public WindowApp
{
public:
	MainApp(HINSTANCE hInstance);
	~MainApp();

	void initApp();
	void onResize();
	void updateScene(float dt);
	void drawScene();
	static void ResetCloth();

	virtual void shutdown();

	static BOOL CALLBACK OptionsDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);

	virtual LRESULT msgProc(UINT msg, WPARAM wParam, LPARAM lParam);


private:
	static Camera* mCamera;
	static Light* mLight;
	static CCloth* mCloth;
	Terrain* mTerrain;
	static DebugLines* mLines;
	Sphere* mSphere;
	Capsule* mCapsule;
	Pyramid* mPyramid;

	D3DXMATRIX mView;
	D3DXMATRIX mProj;
	D3DXMATRIX mWVP;

	int mSampleSize;
	double mScale;

	TRay CastRay();

	static HWND hOptions;

	//Cloth Settings
	static int mClothWidth;
	static int mClothHeight;
	static ERigidity mClothRigidity;
	static D3DXVECTOR3 mWindDirection;
	static float mWindForce;
	static float mGravityForce;
	static ECollidible mCollidibleObject;
	static int mClothHooks;
	static bool mCollisionAvoidance;
};

HWND MainApp::hOptions = NULL;
int MainApp::mClothWidth = 45;
int MainApp::mClothHeight = 45;
ERigidity MainApp::mClothRigidity = ERigidity::R_DEFAULT;
D3DXVECTOR3 MainApp::mWindDirection = D3DXVECTOR3(0.8f, 0.0f, 0.6f);
float MainApp::mWindForce = 1.0f;
float MainApp::mGravityForce = 0.9f;
ECollidible MainApp::mCollidibleObject = ECollidible::CL_NONE;
CCloth* MainApp::mCloth = nullptr;
Camera* MainApp::mCamera = nullptr;
Light* MainApp::mLight = nullptr;
DebugLines* MainApp::mLines = nullptr;
int MainApp::mClothHooks = 2;
bool MainApp::mCollisionAvoidance = false;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
	#if defined(DEBUG) | defined(_DEBUG)
		_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	#endif


	MainApp theApp(hInstance);
	
	theApp.initApp();

	return theApp.run();
}

//F3: Modularity to create the cloth in different sizes
BOOL CALLBACK MainApp::OptionsDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	static bool bShown = false;
	switch (Message)
	{


	case WM_INITDIALOG:
	{
		
	}
	break;

	case WM_WINDOWPOSCHANGED:
	{
		if ((((WINDOWPOS*)lParam)->flags & SWP_SHOWWINDOW) && !bShown)
		{
			bShown = true;
			PostMessage(hwnd, WM_APP, 0, 0);
		}
		break;
	}

	case WM_APP:
	{
		HWND hWidth = GetDlgItem(hOptions, IDC_CLOTHX);
		HWND hHeight = GetDlgItem(hOptions, IDC_CLOTHY);
		HWND hRigidity = GetDlgItem(hOptions, IDC_COMBO1);
		HWND hHooks = GetDlgItem(hOptions, IDC_CLOTH_HOOKS);

		HWND hWindX = GetDlgItem(hOptions, IDC_WINDX); 
		HWND hWindY = GetDlgItem(hOptions, IDC_WINDY);
		HWND hWindZ = GetDlgItem(hOptions, IDC_WINDZ);
		HWND hWindForce = GetDlgItem(hOptions, IDC_WIND_FORCE);

		HWND hGravityForce = GetDlgItem(hOptions, IDC_GRAVITY_FORCE);
		HWND hCollisionShape = GetDlgItem(hOptions, IDC_COL_COMBO);
		HWND hSelfCollisions = GetDlgItem(hOptions, IDC_SELF_COLLISION);

		//Cloth Width
		SendMessage(hWidth, TBM_SETRANGE, (WPARAM)1, (LPARAM)MAKELONG(20, 50));
		SendMessage(hWidth, TBM_SETPOS, (WPARAM)1, (LPARAM)mClothWidth);
		
		//Cloth Height
		SendMessage(hHeight, TBM_SETRANGE, (WPARAM)1, (LPARAM)MAKELONG(20, 50));
		SendMessage(hHeight, TBM_SETPOS, (WPARAM)1, (LPARAM)mClothHeight);

		//Cloth Rigidity
		SendMessage(hRigidity, CB_ADDSTRING, (WPARAM)1, (LPARAM)L"Default");
		SendMessage(hRigidity, CB_ADDSTRING, (WPARAM)1, (LPARAM)L"Crossed");
		SendMessage(hRigidity, CB_ADDSTRING, (WPARAM)1, (LPARAM)L"Interwoven");
		SendMessage(hRigidity, CB_SETCURSEL, (WPARAM)static_cast<int>(mClothRigidity), (LPARAM)0);

		//Cloth Height
		SendMessage(hHooks, TBM_SETRANGE, (WPARAM)1, (LPARAM)MAKELONG(0, 8));
		SendMessage(hHooks, TBM_SETPOS, (WPARAM)1, (LPARAM)mClothHooks);

		
		//Wind X
		SendMessage(hWindX, TBM_SETRANGE, (WPARAM)1, (LPARAM)MAKELONG(0, 100));
		SendMessage(hWindX, TBM_SETPOS, (WPARAM)1, (LPARAM)(mWindDirection.x * 100.0f));

		//Wind Y
		SendMessage(hWindY, TBM_SETRANGE, (WPARAM)1, (LPARAM)MAKELONG(0, 100));
		SendMessage(hWindY, TBM_SETPOS, (WPARAM)1, (LPARAM)(mWindDirection.y * 100.0f));

		//Wind Z
		SendMessage(hWindZ, TBM_SETRANGE, (WPARAM)1, (LPARAM)MAKELONG(0, 100));
		SendMessage(hWindZ, TBM_SETPOS, (WPARAM)1, (LPARAM)(mWindDirection.z * 100.0f));

		//Wind Force
		SendMessage(hWindForce, TBM_SETRANGE, (WPARAM)1, (LPARAM)MAKELONG(0, 100));
		SendMessage(hWindForce, TBM_SETPOS, (WPARAM)1, (LPARAM)(mWindForce * 100.0f));

		//Gravity Force
		SendMessage(hGravityForce, TBM_SETRANGE, (WPARAM)1, (LPARAM)MAKELONG(0, 100));
		SendMessage(hGravityForce, TBM_SETPOS, (WPARAM)1, (LPARAM)(mGravityForce * 100.0f));
		
		//CollisionShape
		SendMessage(hCollisionShape, CB_ADDSTRING, (WPARAM)1, (LPARAM)L"None");
		SendMessage(hCollisionShape, CB_ADDSTRING, (WPARAM)1, (LPARAM)L"Sphere");
		SendMessage(hCollisionShape, CB_ADDSTRING, (WPARAM)1, (LPARAM)L"Capsule");
		SendMessage(hCollisionShape, CB_ADDSTRING, (WPARAM)1, (LPARAM)L"Pyramid");
		SendMessage(hCollisionShape, CB_SETCURSEL, (WPARAM)static_cast<int>(mCollidibleObject), (LPARAM)0);

		//Self Collision
		SendMessage(hSelfCollisions, BM_SETCHECK, (WPARAM)static_cast<int>(mCollisionAvoidance), 0);
		
		break;
	}

	case WM_CLOSE:
	{
		EndDialog(hOptions, NULL);
		bShown = false;
		break;
	}

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_APPLY:
		{
			HWND hWidth = GetDlgItem(hOptions, IDC_CLOTHX);
			HWND hHeight = GetDlgItem(hOptions, IDC_CLOTHY);
			HWND hRigidity = GetDlgItem(hOptions, IDC_COMBO1);
			HWND hHooks = GetDlgItem(hOptions, IDC_CLOTH_HOOKS);

			HWND hWindX = GetDlgItem(hOptions, IDC_WINDX);
			HWND hWindY = GetDlgItem(hOptions, IDC_WINDY);
			HWND hWindZ = GetDlgItem(hOptions, IDC_WINDZ);
			HWND hWindForce = GetDlgItem(hOptions, IDC_WIND_FORCE);

			HWND hGravityForce = GetDlgItem(hOptions, IDC_GRAVITY_FORCE);
			HWND hCollisionShape = GetDlgItem(hOptions, IDC_COL_COMBO);
			HWND hSelfCollisions = GetDlgItem(hOptions, IDC_SELF_COLLISION);


			mClothWidth = SendMessage(hWidth, TBM_GETPOS, 0, 0);
			mClothHeight = SendMessage(hHeight, TBM_GETPOS, 0, 0);
			mClothRigidity = static_cast<ERigidity>(SendMessage(hRigidity, CB_GETCURSEL, 0, 0));
			mClothHooks = SendMessage(hHooks, TBM_GETPOS, 0, 0);

			mWindDirection.x = static_cast<float>(SendMessage(hWindX, TBM_GETPOS, 0, 0)) / 100.0f;
			mWindDirection.y = static_cast<float>(SendMessage(hWindY, TBM_GETPOS, 0, 0)) / 100.0f;
			mWindDirection.z = static_cast<float>(SendMessage(hWindZ, TBM_GETPOS, 0, 0)) / 100.0f;
			mWindForce = static_cast<float>(SendMessage(hWindForce, TBM_GETPOS, 0, 0)) / 100.0f;

			mGravityForce = static_cast<float>(SendMessage(hGravityForce, TBM_GETPOS, 0, 0)) / 100.0f;

			mCollidibleObject = static_cast<ECollidible>(SendMessage(hCollisionShape, CB_GETCURSEL, 0, 0));
			mCollisionAvoidance = static_cast<bool>(SendMessage(hSelfCollisions, BM_GETCHECK, 0, 0));

			EndDialog(hOptions, NULL);
			ResetCloth();
			bShown = false;

		}			
			break;	

		case IDC_CANCEL:
			EndDialog(hOptions, NULL);
			bShown = false;
			break;
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}


/***********************

* CubeApp: CubeApp Constructor
* @author: Thomas O'Brien
* @parameter: HINSTANCE - Application instance

********************/
MainApp::MainApp(HINSTANCE hInstance)
: WindowApp(hInstance)
{
	D3DXMatrixIdentity(&mView);
	D3DXMatrixIdentity(&mProj);
	D3DXMatrixIdentity(&mWVP); 
}
/***********************

* ~CubeApp: CubeApp Destructor
* @author: Thomas O'Brien

********************/
MainApp::~MainApp()
{
	shutdown();
}
/***********************

* initApp: Initialise the application
* @author: Thomas O'Brien
* @return: void

********************/
void MainApp::initApp()
{
	srand(static_cast<unsigned int>(timeGetTime()));
	WindowApp::initApp();

	//Init Cloth Settings
	mClothWidth = 45;
	mClothHeight = 45;
	mClothRigidity = ERigidity::R_DEFAULT;
	mWindDirection = D3DXVECTOR3(0.8f, 0.0f, 0.6f);
	mWindForce = 1.0f;
	mGravityForce = 0.9f;

	//Init Light
	mLight = new Light();
	mLight->dir = D3DXVECTOR3(0.57735f, -0.57735f, 0.57735f);
	mLight->ambient = D3DXCOLOR(0.4f, 0.4f, 0.4f, 1.0f);
	mLight->diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	mLight->specular = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);

	//Init Camera
	mCamera = new Camera();
	mCamera->Initialise(D3DXVECTOR3(-12.101674f, 11.198098f, -29.331566f), D3DXVECTOR3(0.0f, 1.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 1.0f));
	mCamera->GetView(&mView);
	
	mTerrain = new Terrain();
	mTerrain->init(md3dDevice, D3DXVECTOR3(-300.0f, -30.0f, -300.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(25.0f, 1.0f, 25.0f), 26);
	
	mLines = new DebugLines();
	mLines->init(md3dDevice);

	mSphere = new Sphere();
	mSphere->init(md3dDevice);
	
	mCapsule = new Capsule();
	mCapsule->init(md3dDevice);

	mPyramid = new Pyramid();
	mPyramid->init(md3dDevice);

	//Init Cloth
	ResetCloth();
}
/***********************

* onResize: Handle Window Resize
* @author: Thomas O'Brien
* @return: void

********************/
void MainApp::onResize()
{
	WindowApp::onResize();

	float aspect = float(mClientWidth)/mClientHeight;
	D3DXMatrixPerspectiveFovLH(&mProj, 0.25f*PI, aspect, 1.0f, 1000.0f);
}

/***********************

* updateScene: Update the game scene
* @author: Thomas O'Brien
* @parameter: float - delta tick
* @return: void

********************/
void MainApp::updateScene(float dt)
{
	
	WindowApp::updateScene(dt);


	mCamera->Update(dt);
	mCamera->GetView(&mView);

	if (GetAsyncKeyState('R'))
		ResetCloth();

	if (GetAsyncKeyState('T'))
		mCloth->releaseCloth();

	if (GetAsyncKeyState('O'))
		mCloth->openHooks();

	if (GetAsyncKeyState('P'))
		mCloth->closeHooks();

	if (GetAsyncKeyState('C'))
		mLines->clear();

	//Update Collidible Object Position
	if (GetAsyncKeyState(VK_NUMPAD8))
	{
		D3DXVECTOR3 transform(0.0f, 0.0f, 1.0f);
		mSphere->transform(transform * dt);
		mCapsule->transform(transform * dt);
		mPyramid->transform(transform * dt);
	}

	if (GetAsyncKeyState(VK_NUMPAD5))
	{
		D3DXVECTOR3 transform(0.0f, 0.0f, -1.0f);
		mSphere->transform(transform * dt);
		mCapsule->transform(transform * dt);
		mPyramid->transform(transform * dt);
	}

	if (GetAsyncKeyState(VK_NUMPAD4))
	{
		D3DXVECTOR3 transform(-1.0f, 0.0f, 0.0f);
		mSphere->transform(transform * dt);
		mCapsule->transform(transform * dt);
		mPyramid->transform(transform * dt);
	}

	if (GetAsyncKeyState(VK_NUMPAD6))
	{
		D3DXVECTOR3 transform(1.0f, 0.0f, 0.0f);
		mSphere->transform(transform * dt);
		mCapsule->transform(transform * dt);
		mPyramid->transform(transform * dt);
	}

	if (GetAsyncKeyState(VK_NUMPAD7))
	{
		D3DXVECTOR3 transform(0.0f, -1.0f, 0.0f);
		mSphere->transform(transform * dt);
		mCapsule->transform(transform * dt);
		mPyramid->transform(transform * dt);
	}

	if (GetAsyncKeyState(VK_NUMPAD9))
	{
		D3DXVECTOR3 transform(0.0f, 1.0f, 0.0f);
		mSphere->transform(transform * dt);
		mCapsule->transform(transform * dt);
		mPyramid->transform(transform * dt);
	}

	//Update
	if (mCloth != nullptr)
	{
		mCloth->applyForce(D3DXVECTOR3(0, -mGravityForce, 0)* dt); // add gravity each frame, pointing down

		//F6: Fan that creates wind in different speeds
		mCloth->applyWind(mWindDirection * mWindForce *dt); // generate some wind each frame

		//F7: Collision with self
		//WARNING: massive frame rate reduction - use smaller cloth size
		if (mCollisionAvoidance)
			mCloth->collisionAvoidance();

		mCloth->update(dt);
		mCloth->collisionFloor(mTerrain);

		//Collisions with objects
		//F5: Collision with another object
		switch (mCollidibleObject)
		{
		case ECollidible::CL_SPHERE:
		{
			mSphere->update();
			mCloth->collisionSphere(mSphere);
			break;
		}

		case ECollidible::CL_CAPSULE:
		{
			mCapsule->update();
			mCloth->collisionCapsule(mCapsule);
			break;
		}

		case ECollidible::CL_PYRAMID:
		{
			mPyramid->update();
			mCloth->collisionPyramid(mPyramid);
			break;
		}
		};

	}

	if (mTerrain != nullptr)
	{
		mTerrain->update(mView * mProj);
	}
}

/***********************

* drawScene: Handle the drawing of the game scene
* @author: Thomas O'Brien
* @return: void

********************/
void MainApp::drawScene()
{
	WindowApp::drawScene();

	// Restore default states, input layout and primitive topology 
	// because mFont->DrawText changes them.  Note that we can 
	// restore the default states by passing null.
	md3dDevice->OMSetDepthStencilState(0, 0);
	float blendFactors[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	md3dDevice->OMSetBlendState(0, blendFactors, 0xffffffff);

	D3DXMATRIX VP = mView*mProj;


	mCloth->draw(&VP);

	mTerrain->draw();

	switch (mCollidibleObject)
	{
	case ECollidible::CL_SPHERE:
	{
		mSphere->draw(VP);
		break;
	}
	case ECollidible::CL_CAPSULE:
	{
		mCapsule->draw(VP);
		break;
	}
	case ECollidible::CL_PYRAMID:
	{
		mPyramid->draw(VP);
		break;
	}
	}
	
	//Uncomment to show the raycasts
	//mLines->draw(&VP);

	// We specify DT_NOCLIP, so we do not care about width/height of the rect.
	RECT R = {5, 5, 0, 0};
	mFont->DrawText(0, mFrameStats.c_str(), -1, &R, DT_NOCLIP, YELLOW);

	mSwapChain->Present(0, 0);
}

LRESULT MainApp::msgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		// WM_ACTIVATE is sent when the window is activated or deactivated.  
		// We pause the game when the window is deactivated and unpause it 
		// when it becomes active.  
		case WM_ACTIVATE:
			if (LOWORD(wParam) == WA_INACTIVE)
			{
				mAppPaused = true;
				mTimer.stop();
			}
			else
			{
				mAppPaused = false;
				mTimer.start();
			}
			return 0;

			// WM_SIZE is sent when the user resizes the window.  
		case WM_SIZE:
			// Save the new client area dimensions.
			mClientWidth = LOWORD(lParam);
			mClientHeight = HIWORD(lParam);
			if (md3dDevice)
			{
				if (wParam == SIZE_MINIMIZED)
				{
					mAppPaused = true;
					mMinimized = true;
					mMaximized = false;
				}
				else if (wParam == SIZE_MAXIMIZED)
				{
					mAppPaused = false;
					mMinimized = false;
					mMaximized = true;
					onResize();
				}
				else if (wParam == SIZE_RESTORED)
				{

					// Restoring from minimized state?
					if (mMinimized)
					{
						mAppPaused = false;
						mMinimized = false;
						onResize();
					}

					// Restoring from maximized state?
					else if (mMaximized)
					{
						mAppPaused = false;
						mMaximized = false;
						onResize();
					}
					else if (mResizing)
					{
						// If user is dragging the resize bars, we do not resize 
						// the buffers here because as the user continuously 
						// drags the resize bars, a stream of WM_SIZE messages are
						// sent to the window, and it would be pointless (and slow)
						// to resize for each WM_SIZE message received from dragging
						// the resize bars.  So instead, we reset after the user is 
						// done resizing the window and releases the resize bars, which 
						// sends a WM_EXITSIZEMOVE message.
					}
					else // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
					{
						onResize();
					}
				}
			}
			return 0;

			// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
		case WM_ENTERSIZEMOVE:
			mAppPaused = true;
			mResizing = true;
			mTimer.stop();
			return 0;

			// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
			// Here we reset everything based on the new window dimensions.
		case WM_EXITSIZEMOVE:
			mAppPaused = false;
			mResizing = false;
			mTimer.start();
			onResize();
			return 0;

			// WM_DESTROY is sent when the window is being destroyed.
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

			// The WM_MENUCHAR message is sent when a menu is active and the user presses 
			// a key that does not correspond to any mnemonic or accelerator key. 
		case WM_MENUCHAR:
			// Don't beep when we alt-enter.
			return MAKELRESULT(0, MNC_CLOSE);

			// Catch this message so to prevent the window from becoming too small.
		case WM_GETMINMAXINFO:
			((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
			((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
			return 0;

		case WM_KEYDOWN:
		{
			if (LOWORD(wParam) == VK_F1)
			{
					hOptions = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG1), mhMainWnd, (DLGPROC)MainApp::OptionsDlgProc);
					ShowWindow(hOptions, SW_SHOW);
			}
			return 0;
		}

		//Cut
		case WM_LBUTTONDOWN:
		{
			this->mLMouseDown = true;
			this->mMouseX = LOWORD(lParam);
			this->mMouseY = HIWORD(lParam);
			SetCapture(mhMainWnd);
			return 0;
		}

		case WM_LBUTTONUP:
		{
			this->mLMouseDown = false;
			this->mMouseX = LOWORD(lParam);
			this->mMouseY = HIWORD(lParam);
			ReleaseCapture();
			return 0;
		}

		//RotateCamera
		case WM_MBUTTONDOWN:
		{
			this->mMMouseDown = true;
			this->mMouseX = LOWORD(lParam);
			this->mMouseY = HIWORD(lParam);
			SetCapture(mhMainWnd);
			return 0;
		}

		case WM_MBUTTONUP:
		{
			this->mMMouseDown = false;
			this->mMouseX = LOWORD(lParam);
			this->mMouseY = HIWORD(lParam);
			ReleaseCapture();
			return 0;
		}

		//Grab
		case WM_RBUTTONDOWN:
		{

			TRay ray = CastRay();
			mCloth->grab(ray);

			this->mMouseX = LOWORD(lParam);
			this->mMouseY = HIWORD(lParam);
			this->mRMouseDown = true;
			SetCapture(mhMainWnd);
			return 0;
		}

		case WM_RBUTTONUP:
		{			
			this->mRMouseDown = false;
			this->mMouseX = LOWORD(lParam);
			this->mMouseY = HIWORD(lParam);
			ReleaseCapture();
			return 0;
		}



		case WM_INPUT:
		{
			UINT dwSize = 40;
			static BYTE lpb[40];

			GetRawInputData((HRAWINPUT)lParam, RID_INPUT,
				lpb, &dwSize, sizeof(RAWINPUTHEADER));

			RAWINPUT* raw = (RAWINPUT*)lpb;

			if (raw->header.dwType == RIM_TYPEMOUSE)
			{
				//Rotate Camera
				if (this->mMMouseDown == true)
				{
					int xPosRelative = raw->data.mouse.lLastX;
					int yPosRelative = -(raw->data.mouse.lLastY);

					mCamera->RotateCamera(static_cast<float>(xPosRelative), static_cast<float>(yPosRelative));
					RECT window;
					RECT client;
					GetWindowRect(mhMainWnd, &window);
					GetClientRect(mhMainWnd, &client);

					SetPhysicalCursorPos(window.left + static_cast<int>(((window.right - window.left) - client.right) / 2.0f) + mMouseX, window.top + (((window.bottom - window.top) - client.bottom)) - static_cast<int>(((window.right - window.left) - client.right) / 2.0f) + mMouseY);
				}
				//Cut
				if (this->mLMouseDown == true)
				{
					if (GetAsyncKeyState(VK_LSHIFT))
					{
						TRay ray = CastRay();
						mCloth->burn(ray);
					}
					else
					{
						if (mCloth->canCut())
						{
							TRay ray = CastRay();
							mCloth->cut(ray);

						}
					}

				}
				//Grab
				//F1: Forces
				if (this->mRMouseDown == true)
				{
					int xPosRelative = raw->data.mouse.lLastX;
					int yPosRelative = -(raw->data.mouse.lLastY);
					D3DXVECTOR3 delta(static_cast<float>(xPosRelative), static_cast<float>(yPosRelative), 0.0f);
					D3DXMATRIX matVP = mView * mProj;
					D3DXVECTOR3 screenToWorld;
					D3DXMatrixInverse(&matVP, NULL, &matVP);
					D3DXVec3TransformNormal(&screenToWorld, &delta, &matVP);
					mCloth->drag(screenToWorld, mCamera->GetForward());

				}
			}
			break;
		}

	}

	return DefWindowProc(mhMainWnd, msg, wParam, lParam);
}

void MainApp::shutdown()
{
	if (mCamera != nullptr)
	{
		delete mCamera;
		mCamera = nullptr;
	}

	if (mLight != nullptr)
	{
		delete mLight;
		mLight = nullptr;
	} 

	if (mTerrain != nullptr)
	{
		delete mTerrain;
		mTerrain = nullptr;
	}

	if (mCloth != nullptr)
	{
		delete mCloth;
		mCloth = nullptr;
	}

	if (mLines != nullptr)
	{
		delete mLines;
		mLines = nullptr;
	}

	if (mSphere != nullptr)
	{
		delete mSphere;
		mSphere = nullptr;
	}

	if (mCapsule != nullptr)
	{
		delete mCapsule;
		mCapsule = nullptr;
	}

	if (mPyramid != nullptr)
	{
		delete mPyramid;
		mPyramid = nullptr;
	}

	if (md3dDevice)
		md3dDevice->ClearState();

}

void MainApp::ResetCloth()
{
	float fWidth = 20.0f;		  // Width between particles
	float fHeight = 20.0f;		  // Height between particles

	if (mCloth != nullptr)
	{
		delete mCloth;
	}

	//F3: Modularity to create the cloth in different sizes
	mCloth = new CCloth();
	mCloth->init(md3dDevice, fWidth, fHeight, mClothWidth, mClothHeight, mClothRigidity, mClothHooks);

	mCamera->SetPosition(D3DXVECTOR3(-12.101674f, 11.198098f, -29.331566f));
	mCamera->SetUp(D3DXVECTOR3(0.0f, 1.0f, 0.0f));
	mCamera->SetForward(D3DXVECTOR3(0.0f, 0.0f, 1.0f));
	
	mCamera->SetHeading(0.560178f); 
	mCamera->SetPitch(0.508796f);

	mLines->clear();
}

TRay MainApp::CastRay()
{
	POINT pos;
	GetCursorPos(&pos);
	ScreenToClient( mhMainWnd, &pos );

	float x = (2.0f * pos.x) / mClientWidth - 1.0f;
	float y = 1.0f - (2.0f * pos.y) / mClientHeight;
	float z = 1.0f;
	D3DXVECTOR3 ray_nds = D3DXVECTOR3(x, y, z);

	D3DXVECTOR4 ray_clip = D3DXVECTOR4(ray_nds.x, ray_nds.y, 1.0, 1.0);

	D3DXMATRIX invProj;
	D3DXMatrixInverse(&invProj, 0, &mProj);
	D3DXVECTOR4 ray_eye;
	D3DXVec4Transform(&ray_eye, &ray_clip, &invProj);

	ray_eye = D3DXVECTOR4(ray_eye.x, ray_eye.y, 1.0, 0.0);

	D3DXVECTOR4 ray_wor_temp;
	D3DXVECTOR3 ray_wor;
	D3DXMATRIX currView;
	D3DXMATRIX invView;
	mCamera->GetView(&currView);
	D3DXMatrixInverse(&invView, 0, &(currView));

	D3DXVec4Transform(&ray_wor_temp, &ray_eye, &invView);
	ray_wor = D3DXVECTOR3(ray_wor_temp.x, ray_wor_temp.y, ray_wor_temp.z);
	
	// don't forget to normalise the vector
	D3DXVec3Normalize(&ray_wor, &ray_wor);

	TRay ray;
	ray.pos = mCamera->GetPosition();
	ray.dir = ray_wor;

	CLine newLine;
	newLine.pos1 = mCamera->GetPosition();
	newLine.pos2 = newLine.pos1 + (ray_wor * 1000.0f);
	
	

	mLines->AddLine(newLine);

	return ray;
}
