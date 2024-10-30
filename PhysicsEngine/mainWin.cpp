#include "mainWin.h"

LRESULT MainWindow::OnCreate()
{
	GetClientRect(hWnd, &clientSize);

	if(!InitD2D()) return -1;

	rcCenter = D2D1::Point2F((clientSize.right - clientSize.left) / 2, 40);
	rcSize = D2D1::SizeF(40.f, 40.f);

	{
		ground = new D2D1_RECT_F[cMaxGround];

		ground[cGround].bottom = clientSize.bottom;
		ground[cGround].left = 0.f;
		ground[cGround].right = clientSize.right;
		ground[cGround].top = clientSize.bottom - 40.f;
		++cGround;
	}
	
	return 0;
}

LRESULT MainWindow::OnPaint()
{
	PAINTSTRUCT ps;
	pRT->BeginDraw();
	BeginPaint(hWnd, &ps);
	
	pRT->Clear(D2D1::ColorF(D2D1::ColorF::Black, 0.3f));

	pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::White));
	for (size_t i = 0; i < cGround; ++i) {
		pRT->FillRectangle(ground[i], pBrush);
	}

	if (isLButtonDown) {
		pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Gray));
		pRT->FillRectangle(D2D1::RectF(posLButtonDown.x, posLButtonDown.y, posMouseMove.x, posMouseMove.y), pBrush);
	}

	pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Red));
	pRT->FillRectangle(D2D1::RectF(rcCenter.x - rcSize.width / 2, rcCenter.y - rcSize.height / 2,
		rcCenter.x + rcSize.width / 2, rcCenter.y + rcSize.height / 2), pBrush);

	EndPaint(hWnd, &ps);
	return pRT->EndDraw();
}

LRESULT MainWindow::OnSize()
{
	GetClientRect(hWnd, &clientSize);

	pRT->Resize(D2D1::SizeU(clientSize.right - clientSize.left, 
		clientSize.bottom - clientSize.top));

	return 0;
}

LRESULT MainWindow::OnKeyDown(WPARAM key)
{
	keyState[key] = true;
	return 0;
}

LRESULT MainWindow::OnKeyUp(WPARAM key)
{
	keyState[key] = false;
	return 0;
}

LRESULT MainWindow::OnLButtonDown(D2D1_POINT_2F pos)
{
	posLButtonDown = pos;
	isLButtonDown = true;

	return 0;
}

LRESULT MainWindow::OnMouseMove(D2D1_POINT_2F pos)
{
	posMouseMove = pos;
	return 0;
}

LRESULT MainWindow::OnLButtonUp(D2D1_POINT_2F pos)
{
	D2D1_POINT_2F temp = pos;
	if (pos.x < posLButtonDown.x) {
		temp.x = posLButtonDown.x;
		posLButtonDown.x = pos.x;
		pos.x = temp.x;
	}
	if (pos.y < posLButtonDown.y) {
		temp.y = posLButtonDown.y;
		posLButtonDown.y = pos.y;
		pos.y = temp.y;
	}
	if (pos.x - posLButtonDown.x < 5.f || pos.y - posLButtonDown.y < 5.f) {}
	else if (isLButtonDown && cGround < cMaxGround) {
		ground[cGround++] = D2D1::RectF(posLButtonDown.x, posLButtonDown.y, pos.x, pos.y);

		InvalidateRect(hWnd, nullptr, false);
	}
	isLButtonDown = false;

	return 0;
}

LRESULT MainWindow::OnRButtonDown(D2D1_POINT_2F pos)
{
	
	for (int i = cGround-1; i > 0; --i) {
		if (ground[i].left < pos.x && pos.x < ground[i].right &&
			ground[i].top < pos.y && pos.y < ground[i].bottom) {
			--cGround;
			break;
		}
	}
	return 0;
}

bool MainWindow::InitD2D()
{
	HRESULT hr = S_OK;

	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory);

	if (!SUCCEEDED(hr)) return false;
	hr = pFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(
		hWnd, D2D1::SizeU(
			clientSize.right - clientSize.left, clientSize.bottom - clientSize.top
		), D2D1_PRESENT_OPTIONS_IMMEDIATELY
	), &pRT);

	if (!SUCCEEDED(hr)) return false;

	hr = pRT->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &pBrush);
	
	return true;
}

void MainWindow::Destroy()
{
	if (ground) {
		delete[] ground;
		ground = nullptr;
	}
}

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_CLOSE) PostQuitMessage(0);
	else if (uMsg == WM_DESTROY) {
		this->Destroy();
		DestroyWindow(hWnd);
	}

	switch (uMsg) {
		case WM_CREATE:
			{
				if (OnCreate()) throw;
				return 0;
			}
		case WM_PAINT:
			{
				return OnPaint();
			}
		case WM_SIZE:
			{
				return OnSize();
			}
		case WM_KEYDOWN:
			{
				return OnKeyDown(wParam);
			}
		case WM_KEYUP:
			{
				return OnKeyUp(wParam);
			}
		case WM_LBUTTONDOWN:
			{
				return OnLButtonDown(D2D1::Point2F(LOWORD(lParam), HIWORD(lParam)));
			}
		case WM_LBUTTONUP: 
			{
				return OnLButtonUp(D2D1::Point2F(LOWORD(lParam), HIWORD(lParam)));
			}
		case WM_MOUSEMOVE: 
			{
				return OnMouseMove(D2D1::Point2F(LOWORD(lParam), HIWORD(lParam)));
			}
		case WM_RBUTTONDOWN:
			{
				return OnRButtonDown(D2D1::Point2F(LOWORD(lParam), HIWORD(lParam)));
			}
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void MainWindow::Calculate()
{
	static float gravity = 4.3f;
	static float dropSpeed = 0.f;
	static float moveSpeed = 5.f;
	if (canJump && (keyState[VK_SPACE] || keyState[VK_UP] || keyState['W'])) {
		dropSpeed = -30.f;
		canJump = false;
	}
	else {
		dropSpeed += gravity;
	}

	if (keyState[VK_LEFT] + keyState['A'] == keyState[VK_RIGHT] + keyState['D']) moveSpeed = 0.f;
	else if (keyState[VK_LEFT] || keyState['A']) moveSpeed = -10.f;
	else if (keyState[VK_RIGHT] || keyState['D']) moveSpeed = 10.f;

	if (dropSpeed >= 0.f) {
		for (size_t i = 0; i < cGround; ++i) {
			if (!(ground[i].left < rcCenter.x + rcSize.width / 2.f &&
				rcCenter.x - rcSize.width / 2.f < ground[i].right)) continue;

			if (ground[i].top < rcCenter.y + rcSize.height / 2) continue;

			if (abs(ground[i].top - rcCenter.y - rcSize.height / 2) < 0.001f) {
				dropSpeed = 0.f;
				canJump = true;
				break;
			}

			if (rcCenter.y + rcSize.height / 2 + dropSpeed > ground[i].top) {
				float gap = ground[i].top - rcCenter.y - rcSize.height / 2.f;
				dropSpeed = gap;

			}

		}
	}
	else if (dropSpeed < 0.f) { // ��ü�� ���� �̵��� ��
		for (size_t i = 0; i < cGround; ++i) {
			// x�࿡�� ��ġ���� Ȯ��
			if (!(ground[i].left < rcCenter.x + rcSize.width / 2.f &&
				rcCenter.x - rcSize.width / 2.f < ground[i].right)) continue;

			// y�� ����: �ٴ��� �Ʒ����� ��ü�� ��ܺ��� �Ʒ��� �־�� ��
			if (ground[i].bottom > rcCenter.y - rcSize.height / 2) continue;

			// ��ü�� ��ܰ� �ٴ��� �Ʒ����� ���� �´�� ���
			if (abs(ground[i].bottom - rcCenter.y + rcSize.height / 2) < 0.001f) {
				dropSpeed = 0.f; // ���� �ӵ��� 0���� ����
				break;
			}

			// ��ü�� �ٴں��� �� ���� �̵��Ϸ��� �� ���
			if (rcCenter.y - rcSize.height / 2 + dropSpeed < ground[i].bottom) {
				float gap = ground[i].bottom - rcCenter.y + rcSize.height / 2.f;
				dropSpeed = gap; // �浹 ��ġ�� ����
			}
		}
	}

	if (moveSpeed < 0.f) {
		for (size_t i = 0; i < cGround; ++i) {
			if (!(ground[i].top < rcCenter.y + rcSize.height / 2.f &&
				rcCenter.y - rcSize.height / 2.f < ground[i].bottom)) continue;
			// y�� ��ġ�� ���� �ʴ� �ֵ� ����

			if (rcCenter.x - rcSize.width / 2.f < ground[i].right) continue;
			// ��ġ���� ������ �ִ°͵� ����

			if (abs(ground[i].right - rcCenter.x - rcSize.width / 2.f) < 0.001f) {
				// �ô��� ���� ������ �ߴ� �ߴ�
				moveSpeed = 0.f;
				break;
			}

			if (rcCenter.x - rcSize.width / 2.f + moveSpeed < ground[i].right) {
				// �̵��ӵ����� ���� ������ �� ���̸� �̵��ӵ��� ����
				float gap = ground[i].right - rcCenter.x + rcSize.width / 2.f;
				moveSpeed = gap;
			}
		}
	}
	else if (moveSpeed > 0.f) {
		for (size_t i = 0; i < cGround; ++i) {
			if (!(ground[i].top < rcCenter.y + rcSize.height / 2.f && rcCenter.y - rcSize.height / 2.f < ground[i].bottom)) continue;
			// y�� ��ġ�� ���� �ʴ� �ֵ� ����

			if (ground[i].left < rcCenter.x + rcSize.width / 2.f) continue;

			if (abs(rcCenter.x + rcSize.width / 2.f - ground[i].left) < 0.001f) {
				moveSpeed = 0.f;
				break;
			}

			if (ground[i].left < rcCenter.x + rcSize.width / 2.f + moveSpeed) {
				float gap = ground[i].left - (rcCenter.x + rcSize.width / 2.f);
				moveSpeed = gap;
			}

		}
	}

	rcCenter.x += moveSpeed;
	rcCenter.y += dropSpeed;
}