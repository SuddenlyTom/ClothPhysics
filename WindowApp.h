#ifndef WINDOWAPP_H
#define WINDOWAP_H


#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC         ((USHORT) 0x01)
#endif
#ifndef HID_USAGE_GENERIC_MOUSE
#define HID_USAGE_GENERIC_MOUSE        ((USHORT) 0x02)
#endif

#include <Windows.h>
#include "Utility.h"
#include "Timer.h"
#include <string>


class WindowApp
{
public:
	WindowApp(HINSTANCE hInstance);
	virtual ~WindowApp();

	HINSTANCE getAppInst();
	HWND      getMainWnd();

	int run();

	// Framework methods.  Derived client class overrides these methods to 
	// implement specific application requirements.

	virtual void initApp();
	virtual void onResize();// reset projection/etc
	virtual void updateScene(float dt);
	virtual void drawScene(); 
	virtual void shutdown() = 0;
	virtual LRESULT msgProc(UINT msg, WPARAM wParam, LPARAM lParam);

protected:
	void initMainWindow();
	void initDirect3D();
	
protected:

	HINSTANCE mhAppInst;
	HWND      mhMainWnd;
	bool      mAppPaused;
	bool      mMinimized;
	bool      mMaximized;
	bool      mResizing;

	RAWINPUTDEVICE InputDevice;

	int		  mMouseX;
	int		  mMouseY;
	int		  mOldMouseX;
	int		  mOldMouseY;
	bool	  mLMouseDown;
	bool	  mRMouseDown;
	bool	  mMMouseDown;

	float	  mCameraRotationHorizontal;
	float	  mCameraRotationVertical;

	Timer mTimer;

	std::wstring mFrameStats;
 
	static ID3D10Device*    md3dDevice;
	IDXGISwapChain*  mSwapChain;
	ID3D10Texture2D* mDepthStencilBuffer;
	ID3D10RenderTargetView* mRenderTargetView;
	ID3D10DepthStencilView* mDepthStencilView;
	ID3DX10Font* mFont;
	ID3D10RasterizerState* mRasterState;

	// Derived class should set these in derived constructor to customize starting values.
	std::wstring mMainWndCaption;
	D3D10_DRIVER_TYPE md3dDriverType;
	D3DXCOLOR mClearColor;
	int mClientWidth;
	int mClientHeight;

	bool mbResetting;
private:

};




#endif