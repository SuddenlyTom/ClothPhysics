#pragma once
#include "Utility.h"

class Camera
{
public:
	Camera();
	~Camera();

	bool Initialise(D3DXVECTOR3 pos, D3DXVECTOR3 up, D3DXVECTOR3 forward);
	void Update(float dt);
	void GetView(D3DXMATRIX* view);
	void ResetRotation();
	void RotateCamera(float dx, float dy);

	D3DXVECTOR3 GetPosition(){ return mPosition; }
	D3DXVECTOR3 GetForward(){ return mForward; }
	D3DXVECTOR3 GetUp(){ return mUp; }

	float GetPitch() { return fPitch; };
	float GetHeading() { return fHeading; };

	void SetHeading(float h) { fHeading = h; };
	void SetPitch(float p) { fPitch = p; };

	void SetPosition(D3DXVECTOR3& rPosition) { mPosition = rPosition; }
	void SetForward(D3DXVECTOR3& rForward) { mForward = rForward; }
	void SetUp(D3DXVECTOR3& rUp) { mUp = rUp; }

private:
	D3DXVECTOR3 mPosition;
	D3DXVECTOR3 mForward;
	D3DXVECTOR3 mUp;
	D3DXVECTOR3 mRight;

	float fHeading;
	float fPitch;
};

