#include "Camera.h"



#include <string>

Camera::Camera()
{
}


Camera::~Camera()
{
}

bool Camera::Initialise(D3DXVECTOR3 pos, D3DXVECTOR3 up, D3DXVECTOR3 forward)
{
	mPosition = pos;
	mUp = up;
	mForward = forward;
	Update(0.0f);
	return true;
}

void Camera::Update(float dt)
{
	D3DXVec3Cross(&mRight, &mUp, &mForward);

	bool bBoost = (GetAsyncKeyState(VK_LSHIFT)) ? true : false;
	float fSpeedMult = (bBoost) ? 3.0f : 2.0f;

	if (GetAsyncKeyState('W'))
		mPosition += mForward * dt * fSpeedMult;
	if (GetAsyncKeyState('A'))
		mPosition -= mRight * dt * fSpeedMult;
	if (GetAsyncKeyState('D'))
		mPosition += mRight * dt * fSpeedMult;
	if (GetAsyncKeyState('S'))
		mPosition -= mForward * dt * fSpeedMult;

	if (GetAsyncKeyState(VK_SPACE))
		mPosition += (D3DXVECTOR3(0.0f, 1.0f, 0.0f) * dt * fSpeedMult);

	if (GetAsyncKeyState(VK_LCONTROL))
		mPosition -= (D3DXVECTOR3(0.0f, 1.0f, 0.0f) * dt * fSpeedMult);

	//OutputDebugString((std::to_wstring(fHeading) + L" " + std::to_wstring(fPitch) + L"\n").c_str());
	//OutputDebugString((std::to_wstring(mPosition.x) + L" " + std::to_wstring(mPosition.y) + L" " + std::to_wstring(mPosition.z) + L"\n\n").c_str());

}

void Camera::GetView(D3DXMATRIX* view)
{
	D3DXMATRIX matRotate;
	D3DXMatrixRotationYawPitchRoll(&matRotate, fHeading, fPitch, 0.0f);

	D3DXVECTOR3 vecLookAt;
	D3DXVECTOR3 dUp = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	D3DXVECTOR3 dForward = D3DXVECTOR3(0.0f, 0.0f, 1.0f);

	D3DXVec3TransformCoord(&mUp, &dUp, &matRotate);
	D3DXVec3TransformCoord(&vecLookAt, &dForward, &matRotate);

	D3DXVec3Normalize(&mForward, &vecLookAt);
	D3DXVec3Cross(&mRight, &mUp, &vecLookAt);
	D3DXVec3Normalize(&mRight, &mRight);

	mForward = vecLookAt;
	vecLookAt = mPosition + vecLookAt;

	D3DXMatrixLookAtLH(view, &mPosition, &vecLookAt, &mUp);
}

void Camera::ResetRotation()
{
	fHeading = 0.0f;
	fPitch = 0.0f;
}

void Camera::RotateCamera(float dx, float dy)
{
	float fRotationSpeed = 0.001f;
	fHeading += fRotationSpeed * dx;
	fPitch -= fRotationSpeed * dy;
	
	if (fHeading > PI * 2.0f) {
		fHeading -= (PI * 2.0f);
	}
	else if (fHeading < 0) {
		fHeading = (PI * 2.0f) + fHeading;
	}

	if (fPitch > (PI * 0.5f)) {
		fPitch = (PI * 0.5f);
	}
	else if (fPitch < -(PI * 0.5f)) {
		fPitch = -(PI * 0.5f);
	}


}

