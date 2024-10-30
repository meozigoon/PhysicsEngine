#pragma once
#include "baseWin.h"

#include <cmath>
#include <d2d1.h>
#pragma comment(lib, "d2d1")

class MainWindow : public BaseWindow<MainWindow> {
private:
	LRESULT OnCreate();
	LRESULT OnPaint();
	LRESULT OnSize();
	LRESULT OnKeyDown(WPARAM key);
	LRESULT OnKeyUp(WPARAM key);
	LRESULT OnLButtonDown(D2D1_POINT_2F pos);
	LRESULT OnMouseMove(D2D1_POINT_2F pos);
	LRESULT OnLButtonUp(D2D1_POINT_2F pos);
	LRESULT OnRButtonDown(D2D1_POINT_2F pos);
private:
	bool InitD2D();
	void Destroy();
private:
	RECT clientSize;

	ID2D1Factory* pFactory;
	ID2D1HwndRenderTarget* pRT;
	ID2D1SolidColorBrush* pBrush;

	D2D1_POINT_2F rcCenter;
	D2D1_SIZE_F rcSize;

	
	size_t cGround;
	size_t const cMaxGround = 1000;
	D2D1_RECT_F* ground;

	bool keyState[256];
	bool canJump = false;
#pragma region 마우스 통제
	bool isLButtonDown;
	D2D1_POINT_2F posMouseMove;
	D2D1_POINT_2F posLButtonDown;

#pragma endregion

public:
	MainWindow() :clientSize{}, pFactory{}, pRT{}, pBrush{}, rcCenter{}, rcSize{}, cGround{}, ground{}, keyState{}, canJump{}, isLButtonDown{}, posMouseMove{}, posLButtonDown{}
	{}
	virtual LPCWSTR ClassName() const { return L"Physics Engine"; }
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

	void Calculate();
};