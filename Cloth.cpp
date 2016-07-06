// 
// Bachelor of Software Engineering
// Media Design School
// Auckland
// New Zealand
//
// (c) 2005 - 2015 Media Design School
//
// File Name	: Cloth.cpp
// Description	: CCloth definition.
// Author		: Thomas OBrien.
// Mail			: thomas.obrien@mediadesignschool.com
//

//This Include
#include "Cloth.h"
//Local Includes
#include "Vertex.h"
//Library Includes 
#include <algorithm>
#include <limits>

CCloth::CCloth()
{
}


CCloth::~CCloth()
{
	shutdown();
}

void CCloth::init(ID3D10Device* pDevice, float fWidth, float fHeight, int iPartsWidth, int iPartsHeight, ERigidity eRigid, int iHooks) 
{
	//Store Member Variables
	md3dDevice = pDevice;
	miWidth = iPartsWidth;
	miHeight = iPartsHeight;
	bCanCut = true;
	meRigidity = eRigid;

	//Calculate Number of Vertices for Connecters
	int mNumParticles = (miWidth * miHeight) * 2;
	
	mParticles.resize(mNumParticles);

	//Create particles
	for (int x = 0; x < miWidth; x++)
	{
		for (int y = 0; y < miHeight; y++)
		{
			D3DXVECTOR3 vPosition = D3DXVECTOR3(
				fWidth * (x / static_cast<float>(miWidth)),
				-fHeight * (y / static_cast<float>(miHeight)),
				0
				);
			mParticles[(y * miWidth) + x] = CParticle(vPosition);
		}
	}

	//CParticle* part = &mParticles[mNumParticles * 0.5f];
	//part->setPos(D3DXVECTOR3(part->getPos().x, part->getPos().y, part->getPos().z + 100.0f));

	//Create constraints
	//Direct neighbour
	for (int x = 0; x<miWidth; x++)
	{
		for (int y = 0; y<miHeight; y++)
		{
			if (x < miWidth - 1)	// Right
				createConstraint(x, y, x + 1, y);

			if (y < miHeight - 1)	// Down
				createConstraint(x, y, x, y + 1);

			//F4: Tightly woven cloth structure
			if (eRigid == ERigidity::R_CROSSED)
			{
				if (x<miWidth - 1 && y<miHeight - 1)	//Cross Right
					createConstraint(x, y, x + 1, y + 1);
				
				if (x<miWidth - 1 && y<miHeight - 1)	//Cross Left
					createConstraint(x + 1, y, x, y + 1);
			}

		}
	}
	
	//Secondary neighbour
	//F4: Tightly woven cloth structure
	if (eRigid == ERigidity::R_INTERWEAVED)
	{
		for (int x = 0; x<miWidth; x++)
		{
			for (int y = 0; y<miHeight; y++)
			{
				if (x<miWidth - 2)	//Right
					createConstraint(x, y, x + 2, y);
		
				if (y<miHeight - 2)	//Down
					createConstraint(x, y, x, y + 2);
		
				if (x<miWidth - 2 && y<miHeight - 2)	//Cross Right
					createConstraint(x, y, x + 2, y + 2);
		
				if (x<miWidth - 2 && y<miHeight - 2)	//Cross Left
					createConstraint(x + 2, y, x, y + 2);
			}
		}
	}


	std::random_shuffle(mConstraints.begin(), mConstraints.end());


	//Fix the positions of the hooks
	getParticle(0, 0)->addPosition(D3DXVECTOR3(0.5f, 0.0f, 0.0f)); 
	getParticle(0, 0)->setMovable(false);

	getParticle(miWidth - 1, 0)->addPosition(D3DXVECTOR3(-0.5f, 0.0f, 0.0f)); 
	getParticle(miWidth - 1, 0)->setMovable(false);

	for (int i = 0; i < iHooks; i++)
	{
		int index = static_cast<int>(ceil(static_cast<double>(miWidth) / static_cast<double>(iHooks)) * (i + 1));
		getParticle(index, 0)->setMovable(false);

	}

	//Initialise DirectX elements
	buildFX();

	D3D10_RASTERIZER_DESC rdesc;
	ZeroMemory(&rdesc, sizeof(D3D10_RASTERIZER_DESC));
	rdesc.FillMode = D3D10_FILL_SOLID;
	rdesc.CullMode = D3D10_CULL_NONE;
	rdesc.DepthClipEnable = true;

	md3dDevice->CreateRasterizerState(&rdesc, &mRS);
	
	mNumVertices = mConstraints.size() * 2;

	//Generate Vertex structures
	mVertices.resize(mNumVertices);
	
	int iIndex = 0;
	for (unsigned int i = 0; i < mConstraints.size(); i++)
	{
		mVertices[iIndex].pos = mConstraints[i].getPart1()->getPos();
		mVertices[iIndex++].color = D3DXCOLOR(1.0f - mConstraints[i].getPart1()->getHealth(), 0.0f, mConstraints[i].getPart1()->getHealth(), 1.0f);

		mVertices[iIndex].pos = mConstraints[i].getPart2()->getPos();
		mVertices[iIndex++].color = D3DXCOLOR(1.0f - mConstraints[i].getPart2()->getHealth(), 0.0f, mConstraints[i].getPart2()->getHealth(), 1.0f);
	}


	//Generate Vertex buffer
	D3D10_BUFFER_DESC vbd;
	vbd.Usage = D3D10_USAGE_DYNAMIC;
	vbd.ByteWidth = sizeof(Vertex) * mNumVertices;
	vbd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
	vbd.MiscFlags = 0;
	D3D10_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &(mVertices.front());
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mVB));
}

void CCloth::update(float dt)
{
	int iIterations = 15;
	
	//Iterate over points and update
	std::vector<CParticle>::iterator p;
	for (p = mParticles.begin(); p != mParticles.end(); p++)
	{
		p->update(dt);
	}

	//Loop for each iteration
	for (int i = 0; i < iIterations; i++)
	{
		//For each constraint
		std::vector<CConstraint>::iterator c;
		int ci = 0;
		for (c = mConstraints.begin(), ci = 0; c != mConstraints.end(); c++, ci++)
		{
			//If it is alive
			if (c->isAlive())
			{
				//Make sure the constraint is valid
				bool result = c->fitConstraint();
				if (result == false)
				{
					//If its not, delete it
					//mConstraints[ci].setIsAlive(false);
					mPendingCut.push(ci);
				}
			}
		}
	}
	bCanCut = false;

	//Collision avoidance


	//Cleanup
	while(!mPendingCut.empty())
	{
		//We have a queue of the things needing to be cut.
		int index = (mPendingCut.front());
		mPendingCut.pop();

		//Nullify the constraint
		mConstraints[index].setIsAlive(false);
		mConstraints[index] = CConstraint(nullptr, nullptr);
	}
	bCanCut = true;

	//Update the vertex buffers to draw correctly
	UpdateBuffer();
}

void CCloth::predraw()
{
	md3dDevice->IASetInputLayout(mVertexLayout);
	md3dDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_LINELIST);
}


void CCloth::draw(D3DXMATRIX* pWVP)
{
	predraw();

	mfxWVPVar->SetMatrix((float*)pWVP);

	D3D10_TECHNIQUE_DESC techDesc;
	mTechnique->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		mTechnique->GetPassByIndex(p)->Apply(0);

		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		md3dDevice->IASetVertexBuffers(0, 1, &mVB, &stride, &offset);
		md3dDevice->Draw(mNumVertices, 0);
	}
}

void CCloth::applyForce(const D3DXVECTOR3 dir) {
	//Iterate over all particles
	std::vector<CParticle>::iterator particle;
	for (particle = mParticles.begin(); particle != mParticles.end(); particle++)
	{
		//Apply the specified force
		particle->addForce(dir);
	}
}

void CCloth::applyWind(const D3DXVECTOR3 dir)
{
	//For each of the particles
	for (int x = 0; x< miWidth - 1; x++)
	{
		for (int y = 0; y<miHeight - 1; y++)
		{
			//Add a find force int the specified triangle
			addWindForces(getParticle(x + 1, y), getParticle(x, y), getParticle(x, y + 1), dir);
			addWindForces(getParticle(x + 1, y + 1), getParticle(x + 1, y), getParticle(x, y + 1), dir);
		}
	}
}

void CCloth::collisionFloor(Terrain * pTerrain)
{
	//For each of the particles
	std::vector<CParticle>::iterator p;
	for (p = mParticles.begin(); p != mParticles.end(); p++)
	{
		//See if the particle is below the floor
		float floorHeight = pTerrain->GetPosition().y;
		float halfScale = pTerrain->GetScale().x * 0.5f;
		D3DXVECTOR3 terCentre = pTerrain->GetPosition();

		if (p->getPos().y < floorHeight )
		{
			//Push it up so that it is flat with the floor
			p->addPosition(D3DXVECTOR3(0.0f, (floorHeight - p->getPos().y), 0.0f));
		}
	}

}

void CCloth::collisionSphere(Sphere * pSphere)
{
	//For each of the particles
	std::vector<CParticle>::iterator p;
	for (p = mParticles.begin(); p != mParticles.end(); p++)
	{
		//Get distance between particle and sphere origin.
		D3DXVECTOR3 delta = p->getPos() - pSphere->getPos();
		float deltaRange = D3DXVec3Length(&delta);
		if (deltaRange < pSphere->getRadius())
		{
			//We are too close, push it out
			p->addPosition(delta * 0.1f);
		}
	}

}

void CCloth::collisionCapsule(Capsule * pCapsule)
{
	// Iterate over particles
	std::vector<CParticle>::iterator p;
	for (p = mParticles.begin(); p != mParticles.end(); p++)
	{
		float fDistance;
		D3DXVECTOR3 partPos = p->getPos();
		D3DXVECTOR3 v, w, capTop, capBot, delta;
		//Get positions of caps
		capTop = pCapsule->getPos() + D3DXVECTOR3(0, pCapsule->getHeight(), 0);
		capBot = pCapsule->getPos();
		v = capTop - capBot;
		w = partPos - capBot;
		float c1, c2;

		//Calculate position relative to the capsule
		if ((c1 = D3DXVec3Dot(&w, &v)) <= 0)
		{
			delta = partPos - capBot;
			fDistance = D3DXVec3Length(&delta);
		}
		else if ((c2 = D3DXVec3Dot(&v, &v)) <= c1)
		{
			delta = partPos - capTop;
			fDistance = D3DXVec3Length(&delta);
		}
		else
		{
			float b = c1 / c2;
			D3DXVECTOR3 Pb = capBot + b*v;
			delta = partPos - Pb;
			fDistance = D3DXVec3Length(&delta);
		}

		//Point is within the capsule
		if (fDistance < pCapsule->getRadius())
		{
			p->addPosition(delta * 0.1f);
		}
		
	}

}

void CCloth::collisionPyramid(Pyramid * pPyramid)
{
	D3DXVECTOR3* pNormals = pPyramid->getNormals();
	D3DXVECTOR3 vApex = pPyramid->getPos();
	float fScale = pPyramid->getScale();

	//Iterate over all particles
	std::vector<CParticle>::iterator p;
	for (p = mParticles.begin(); p != mParticles.end(); p++)
	{
		float fDistance;
		float fShortestDistance = static_cast<float>(INFINITE);
		D3DXVECTOR3 correction;
		D3DXVECTOR3 partPos = p->getPos();
		bool bInside = true;
		for (int i = 0; i < 4; i++)  //loop through each triangle face
		{
			float fPlaneDistance = -D3DXVec3Dot(&vApex, &pNormals[i]);
			fDistance = D3DXVec3Dot(&partPos, &pNormals[i]);
			fDistance += fPlaneDistance;

			if (fDistance < 0)
				bInside = false;
			else
			{
				if (abs(fDistance) < fShortestDistance)
				{
					fShortestDistance = abs(fDistance);
					correction = pNormals[i];
				}
			}
		}
		if (bInside)
		{
			//Particle is inside of the pyramid
			p->addPosition(correction * -(fShortestDistance));
		}
	}
}

void CCloth::collisionAvoidance()
{
	for (unsigned int i = 0; i < mParticles.size(); i++)
	{
		//Take the current particle and iterate over the rest of the particle
		for (unsigned int j = i + 1; j < mParticles.size(); j++)
		{
			float fThreshold = 0.01f;
			D3DXVECTOR3 vecDelta = mParticles[i].getPos() - mParticles[j].getPos();
			float fDelta = D3DXVec3Length(&vecDelta);

			if (fDelta <= fThreshold)
			{
				mParticles[i].addPosition(-vecDelta * 0.5f * fDelta);
				mParticles[j].addPosition(vecDelta * 0.5f * fDelta);

			}
		}
	}
}

void CCloth::releaseCloth()
{
	//For all particles
	std::vector<CParticle>::iterator p;
	for (p = mParticles.begin(); p != mParticles.end(); p++)
	{
		//Release them
		p->setMovable(true);
	}
}

void CCloth::closeHooks()
{
	D3DXVECTOR3 sum;
	ZeroMemory(&sum, sizeof(D3DXVECTOR3));
	int count = 0;
	
	//Get average positions of all hooks
	std::vector<CParticle>::iterator p;
	for (p = mParticles.begin(); p != mParticles.end(); p++)
	{
		if (!p->canMove())
		{
			sum += p->getPos();
			count++;
		}
	}

	//Move all of the points to the centre
	D3DXVECTOR3 centre = sum / static_cast<float>(count);
	for (p = mParticles.begin(); p != mParticles.end(); p++)
	{
		if (!p->canMove())
		{
			p->forceAddPosition((centre - p->getPos()) * 0.001f);
		}
	}
}

void CCloth::openHooks()
{
	D3DXVECTOR3 sum;
	ZeroMemory(&sum, sizeof(D3DXVECTOR3));
	int count = 0;

	//Get average position of all hooks
	std::vector<CParticle>::iterator p;
	for (p = mParticles.begin(); p != mParticles.end(); p++)
	{
		if (!p->canMove())
		{
			sum += p->getPos();
			count++;
		}
	}

	//Move them away from the centre
	D3DXVECTOR3 centre = sum / static_cast<float>(count);
	for (p = mParticles.begin(); p != mParticles.end(); p++)
	{
		if (!p->canMove())
		{
			p->forceAddPosition(-(centre - p->getPos()) * 0.001f);
		}
	}
}

void CCloth::burn(TRay & _rRay)
{
	//F8: Cloth can be ignited
	float fThreshold = 0.2f;
	//For all particles
	for (unsigned int i = 0; i < mParticles.size(); i++)
	{
		//Get distance between point and ray
		D3DXVECTOR3 partPos = mParticles[i].getPos();
		D3DXVECTOR3 delta = partPos - _rRay.pos;
		D3DXVECTOR3 cross;
		D3DXVec3Cross(&cross, &_rRay.dir, &delta);
		float fDistance = D3DXVec3Length(&cross);

		//See if ray has hit the particle
		if (fDistance < fThreshold)
		{
			//We've found a point that is close enough to our ray.
			//Burn it!
			mParticles[i].setOnFire(true);
		}
	}
}

void CCloth::cut(TRay& _rRay)
{
	std::vector<CConstraint>::iterator it;
	int index = 0;
	for (it = mConstraints.begin(), index = 0; it != mConstraints.end(); it++, index++)
	{
		if (it->isAlive())
		{
			float fThreshold = 0.1f;

			//Iterate over all constraints
			CLine line(it->getPart1()->getPos(), it->getPart2()->getPos());

			//See if distance from ray to constraint is less than the threshold
			if (rayToLineSegment(_rRay, line) <= fThreshold)
			{
				//If it is, cut the constraint
				mPendingCut.push(index);
			}
		}
	}
}

float CCloth::rayToLineSegment(const TRay& ray, CLine& line) {
	D3DXVECTOR3 u = ray.dir;
	D3DXVECTOR3 v = line.pos2 - line.pos1;
	D3DXVECTOR3 w = ray.pos - line.pos1;
	
	float a = D3DXVec3Dot(&u, &u);	// always >= 0
	float b = D3DXVec3Dot(&u, &v);
	float c = D3DXVec3Dot(&v, &v);	// always >= 0
	float d = D3DXVec3Dot(&u, &w);
	float e = D3DXVec3Dot(&v, &w);
	float D = a*c - b*b;	// always >= 0
	float sc, sN, sD = D;	// sc = sN / sD, default sD = D >= 0
	float tc, tN, tD = D;	// tc = tN / tD, default tD = D >= 0
	
							// compute the line parameters of the two closest points
	if (D < DBL_EPSILON) {	// the lines are almost parallel
		sN = 0.0;			// force using point P0 on segment S1
		sD = 1.0;			// to prevent possible division by 0.0 later
		tN = e;
		tD = c;
	}
	else {				// get the closest points on the infinite lines
		sN = (b*e - c*d);
		tN = (a*e - b*d);
		if (sN < 0.0) {	// sc < 0 => the s=0 edge is visible
			sN = 0.0;
			tN = e;
			tD = c;
		}
	}

	if (tN < 0.0) {		// tc < 0 => the t=0 edge is visible
		tN = 0.0;
		// recompute sc for this edge
		if (-d < 0.0)
			sN = 0.0;
		else {
			sN = -d;
			sD = a;
		}
	}
	else if (tN > tD) {	  // tc > 1 => the t=1 edge is visible
		tN = tD;
		// recompute sc for this edge
		if ((-d + b) < 0.0)
			sN = 0;
		else {
			sN = (-d + b);
			sD = a;
		}
	}
	// finally do the division to get sc and tc
	sc = (abs(sN) < DBL_EPSILON ? 0.0f : sN / sD);
	tc = (abs(tN) < DBL_EPSILON ? 0.0f : tN / tD);

	// get the difference of the two closest points
	D3DXVECTOR3 dP = w + (sc * u) - (tc * v);	// = S1(sc) - S2(tc)
	return D3DXVec3Length(&dP);	// return the closest distance
}

void CCloth::grab(TRay & _rRay)
{
	float fThreshold = 0.2f;
	mGrabbedParticles.clear();
	
	//Iterate over all particles, find those in the range
	for (unsigned int i = 0; i < mParticles.size(); i++)
	{
		D3DXVECTOR3 partPos = mParticles[i].getPos();
		D3DXVECTOR3 delta = partPos - _rRay.pos;
		D3DXVECTOR3 cross;
		D3DXVec3Cross(&cross, &_rRay.dir, &delta);
		float fDistance = D3DXVec3Length(&cross);

		if (fDistance < fThreshold)
		{
			//We've found a point that is close enough to our ray.
			//Grab it!
			mGrabbedParticles.push_back(&mParticles[i]);
		}
	}
}

void CCloth::drag(D3DXVECTOR3 screenDir, D3DXVECTOR3 cameraDir)
{
	float fDragScale = 0.3f;
	//Iterate over the grabbed particles and add to the position
	std::vector<CParticle*>::iterator it;
	for (it = mGrabbedParticles.begin(); it != mGrabbedParticles.end(); it++)
	{
		D3DXVECTOR3 delta = fDragScale * D3DXVECTOR3(cameraDir.x + screenDir.x, cameraDir.y + screenDir.y, cameraDir.z);
		//(*it)->addPosition(delta);
		(*it)->addForce(delta * 200.0f);
	}
}

void CCloth::releaseGrabbed()
{
	//Stop dragging any particles
	mGrabbedParticles.clear();
}


void CCloth::createConstraint(int p1x, int p1y, int p2x, int p2y) {
	//Make a constraint between 2 points
	CParticle* part1 = getParticle(p1x, p1y);
	CParticle* part2 = getParticle(p2x, p2y);
	mConstraints.push_back(CConstraint(part1, part2));

	//Tell the particles that they have a new connection
	part1->incrementConnectorCount();
	part2->incrementConnectorCount();
};

void CCloth::addWindForces(CParticle* p1, CParticle* p2, CParticle* p3, const D3DXVECTOR3 direction)
{
	//Calculate the normal to the triangle
	D3DXVECTOR3 normal = calcTriangleNormal(p1, p2, p3);
	D3DXVECTOR3 d;
	D3DXVec3Normalize(&d, &normal);
	D3DXVECTOR3 force = normal * (D3DXVec3Dot(&d, &direction));
	
	//Add the force in the correct directon
	p1->addForce(force);
	p2->addForce(force);
	p3->addForce(force);
}

D3DXVECTOR3 CCloth::calcTriangleNormal(CParticle* p1, CParticle* p2, CParticle* p3)
{
	//Get positions of specified particles
	D3DXVECTOR3 pos1 = p1->getPos();
	D3DXVECTOR3 pos2 = p2->getPos();
	D3DXVECTOR3 pos3 = p3->getPos();
	
	//Find the normal to these points
	D3DXVECTOR3 v1 = pos2 - pos1;
	D3DXVECTOR3 v2 = pos3 - pos1;
	D3DXVECTOR3 output; 
	D3DXVec3Cross(&output, &v1, &v2);
	
	return output; 
}

void CCloth::buildFX()
{
	DWORD shaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	shaderFlags |= D3D10_SHADER_DEBUG;
	shaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif

	ID3D10Blob* compilationErrors = 0;
	HRESULT hr = 0;
	hr = D3DX10CreateEffectFromFile(L"cloth.fx", 0, 0,
		"fx_4_0", shaderFlags, 0, md3dDevice, 0, 0, &mFX, &compilationErrors, 0);
	if (FAILED(hr))
	{
		if (compilationErrors)
		{
			MessageBoxA(0, (char*)compilationErrors->GetBufferPointer(), 0, 0);
			ReleaseCOM(compilationErrors);
		}
		DXTrace(__FILE__, (DWORD)__LINE__, hr, L"D3DX10CreateEffectFromFile", true);
	}

	mTechnique = mFX->GetTechniqueByName("ClothTech");
	mfxWVPVar = mFX->GetVariableByName("gWVP")->AsMatrix();

	buildVertexLayouts();
}

void CCloth::buildVertexLayouts()
{
	// Create the vertex input layout.

	D3D10_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D10_INPUT_PER_VERTEX_DATA, 0 },
	};

	// Create the input layout
	D3D10_PASS_DESC PassDesc;
	mTechnique->GetPassByIndex(0)->GetDesc(&PassDesc);
	HR(md3dDevice->CreateInputLayout(vertexDesc, 2, PassDesc.pIAInputSignature,
		PassDesc.IAInputSignatureSize, &mVertexLayout));
}

void CCloth::UpdateBuffer()
{
	int iIndex = 0;
	ZeroMemory(&mVertices[0], sizeof(Vertex) * mNumVertices);

	float fScale = 100.0f / mConstraints.size();

	//Iterate over all constraints
	for (unsigned int i = 0; i < mConstraints.size(); i++)
	{
		//Draw only if the constraint is alive
		if (mConstraints[i].isAlive())
		{
			//Get position 1
			mVertices[iIndex].pos = mConstraints[i].getPart1()->getPos();
			if (mConstraints[i].getPart1()->isOnFire())	//If its on fire, draw it red based on the health
				mVertices[iIndex++].color = D3DXCOLOR(2.0f * mConstraints[i].getPart1()->getHealth() * mConstraints[i].getPart1()->getBurnTimer(), 0.0f, mConstraints[i].getPart1()->getHealth(), 1.0f);

			else if(mConstraints[i].getPart1()->canMove())	//if its a normal part, draw blue based on the health.
				mVertices[iIndex++].color = D3DXCOLOR(1.0f - mConstraints[i].getPart1()->getHealth(), 0.0f, mConstraints[i].getPart1()->getHealth(), 1.0f);

			else	//Draw blue for hooks
				mVertices[iIndex++].color = D3DXCOLOR(0.0f, 0.0f, 1.0f, 1.0f);

			//Get position 2
			mVertices[iIndex].pos = mConstraints[i].getPart2()->getPos();
			if (mConstraints[i].getPart2()->isOnFire())  //If its on fire, draw it red based on the health
				mVertices[iIndex++].color = D3DXCOLOR(2.0f * mConstraints[i].getPart2()->getHealth() * mConstraints[i].getPart2()->getBurnTimer(), 0.0f, mConstraints[i].getPart2()->getHealth(), 1.0f);

			else if (mConstraints[i].getPart2()->canMove())  //if its a normal part, draw blue based on the health.
				mVertices[iIndex++].color = D3DXCOLOR(1.0f - mConstraints[i].getPart2()->getHealth(), 0.0f, mConstraints[i].getPart2()->getHealth(), 1.0f);

			else  //Draw blue for hooks
				mVertices[iIndex++].color = D3DXCOLOR(0.0f, 0.0f, 1.0f, 1.0f);

		}
	}

	//Update vertex buffer
	void* pVerts;
	mVB->Map(D3D10_MAP_WRITE_DISCARD, 0, &pVerts);
	ZeroMemory(pVerts, sizeof(Vertex) * mNumVertices);
	
	memcpy_s(pVerts, sizeof(Vertex) * mNumVertices, &mVertices[0], sizeof(Vertex) * mNumVertices);
	mVB->Unmap();
}

void CCloth::shutdown()
{
	//Release relevant objects
	ReleaseCOM(mVB);
	ReleaseCOM(mIB);
	ReleaseCOM(mFX);
	ReleaseCOM(mVertexLayout);
	ReleaseCOM(mRS);

	md3dDevice = nullptr;

}