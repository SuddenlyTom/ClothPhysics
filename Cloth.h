// 
// Bachelor of Software Engineering
// Media Design School
// Auckland
// New Zealand
//
// (c) 2005 - 2015 Media Design School
//
// File Name	: Cloth.h
// Description	: CCloth declaration.
// Author		: Thomas OBrien.
// Mail			: thomas.obrien@mediadesignschool.com
//

#pragma once

//Local Includes
#include "Utility.h"
#include "Vertex.h"
#include "Particle.h"
#include "Terrain.h"
#include "Sphere.h"
#include "Capsule.h"
#include "Pyramid.h"

//Library Includes
#include <vector>
#include <queue>

class CCloth
{
	//Member Functions
public:
	CCloth();
	~CCloth();


	/** Initialise the cloth
	@remarks
		uses the passed parameters to determine the various settings for the cloth
	@author Tom O'Brien
	*/
	void init(ID3D10Device* pDevice, float fWidth, float fHeight, int iPartsWidth, int iPartsHeight, ERigidity eRigid, int iHooks);
	

	/** Update the various elements of the cloth
	@author Tom O'Brien
	*/
	void update(float dt);


	/** Draw the cloth
	@author Tom O'Brien
	*/
	void draw(D3DXMATRIX* pWVP);

	/** Shutdown and release the cloth
	@author Tom O'Brien
	*/
	void shutdown();

	/** Apply a force to each particle
	@author Tom O'Brien
	*/
	void applyForce(const D3DXVECTOR3 dir);

	/** Apply a wind force to the particles
	@author Tom O'Brien
	*/
	void applyWind(const D3DXVECTOR3 dir);
	
	//Collisions

	/** Handle floor collisions
	@author Tom O'Brien
	*/
	void collisionFloor(Terrain* pTerrain);

	/** Handle collisions with sphere
	@author Tom O'Brien
	*/
	void collisionSphere(Sphere* pSphere);

	/** Handle collisions with capsule
	@author Tom O'Brien
	*/
	void collisionCapsule(Capsule * pCapsule);

	/** Handle collisions with pyramid
	@author Tom O'Brien
	*/
	void collisionPyramid(Pyramid* pPyramid);

	/** Avoid collision with self
	@remarks
		Is extremely taxing, so use with smaller cloth size
	@author Tom O'Brien
	*/
	void collisionAvoidance();


	/** Set all particles to movable, dropping from hooks
	@author Tom O'Brien
	*/
	void releaseCloth();

	/** Move cloth hooks closer together
	@author Tom O'Brien
	*/
	void closeHooks();

	/** Move cloth hooks further apart
	@author Tom O'Brien
	*/
	void openHooks();

	/** Distance from ray to line segment
	@remarks
		Used in ray casts to calculate clicked lines
	@author Tom O'Brien
	*/
	float rayToLineSegment(const TRay& ray, CLine& line);

	/** Burn particles affected by the casted ray
	@author Tom O'Brien
	*/
	void burn(TRay& _rRay);

	/** Cut constraints affected by the ray
	@author Tom O'Brien
	*/
	void cut(TRay& _rRay);

	/** Grab particles affected by the ray
	@remarks
		Grabbed particles are added to the vector mGrabbedParticles
		to be used later on
	@author Tom O'Brien
	*/
	void grab(TRay& _rRay);

	/** Release the grabbed particles
	@remarks
	uses the mass of the particle to make an addition to the acceleration
	used in verlat integration
	@author Tom O'Brien
	*/
	void releaseGrabbed();

	/** Move grabbed particles
	@remarks
		Takes the screen-space movement and camera direction to determine movement
	@author Tom O'Brien
	*/
	void drag(D3DXVECTOR3 screenDir, D3DXVECTOR3 cameraDir);

	/** See if we are able to cut
	@author Tom O'Brien
	*/
	bool canCut() {
		return bCanCut;
	};

private:
	void predraw();
	void buildFX();
	void buildVertexLayouts();
	void UpdateBuffer();

	/** Return particle within the vector
	@author Tom O'Brien
	*/
	CParticle* getParticle(int xPos, int yPos) {
		return &mParticles[(yPos * miWidth) + xPos];
	};

	/** Create a constraint between the defined particles
	@author Tom O'Brien
	*/
	void createConstraint(int p1x, int p1y, int p2x, int p2y);
	
	/** Apply wind to the triangle defined by the points
	@author Tom O'Brien
	*/
	void addWindForces(CParticle* p1, CParticle* p2, CParticle* p3, const D3DXVECTOR3 direction);
	
	/** Calculate the normal to the defined triangle
	@author Tom O'Brien
	*/
	D3DXVECTOR3 calcTriangleNormal(CParticle* p1, CParticle* p2, CParticle* p3);

	//Member Variables
private:
	//Cloth dependancies
	std::vector<CParticle> mParticles;
	std::vector<CConstraint> mConstraints;
	std::vector<CParticle*> mGrabbedParticles;

	std::queue<int> mPendingCut;

	std::vector<Vertex> mVertices;
	std::vector<DWORD> mIndices;

	int miWidth;
	int miHeight;
	ERigidity meRigidity;
	bool bCanCut;

	//DX10 dependancies
	DWORD mNumVertices;
	DWORD mNumIndices;

	ID3D10Device* md3dDevice;//Device
	ID3D10Buffer* mVB; // Vertex buffer
	ID3D10Buffer* mIB; // Index  buffer

	ID3D10Effect* mFX;
	ID3D10EffectTechnique* mTechnique;
	ID3D10InputLayout* mVertexLayout;
	ID3D10EffectMatrixVariable* mfxWVPVar;
	ID3D10RasterizerState* mRS;

	

};

