// 
// Bachelor of Software Engineering
// Media Design School
// Auckland
// New Zealand
//
// (c) 2005 - 2015 Media Design School
//
// File Name	: Particle.cpp
// Description	: CParticle and CConstraint definitions.
// Author		: Thomas OBrien.
// Mail			: thomas.obrien@mediadesignschool.com
//

#include "Particle.h"

float CParticle::fDamping = 0.01f;


CParticle::CParticle()
{
}


CParticle::~CParticle()
{
}

void CParticle::update(float dt)
{
	//See if particle is connected to anything
	if (iConnectionCount > 0)
	{
		if (bCanMove)
		{
			if (bOnFire)
			{
				//Increment burn timer, damage particle and add upward force
				fBurnTimer += 0.001f;
				addForce(D3DXVECTOR3(0, 0.015f, 0));
				addHealth(-0.0006f);
			}

			//Verlet Integration
			D3DXVECTOR3 tempPos = vecPos;
			vecPos = vecPos + (vecPos - vecOldPos) * (1.0f - fDamping) + (vecAcceleration * dt);
			vecOldPos = tempPos;
			zeroAcceleration();
		}
	}
	else
	{
		this->setMovable(false);
	}
}

CConstraint::CConstraint(CParticle * p1, CParticle * p2) :pPart1(p1), pPart2(p2),
bIsAlive(true), fBurnTimer(0.0f), bMarkedForDeath(false)
{
	if (p1 != nullptr && p2 != nullptr)
	{
		D3DXVECTOR3 vec = p1->getPos() - p2->getPos();
		fRestDistance = D3DXVec3Length(&vec);
	}
	else
	{
		bIsAlive = false;
	}
}

void CConstraint::setIsAlive(bool b)
{
	bIsAlive = b;
	if (b == false)
	{
		if (pPart1 != nullptr && pPart2 != nullptr)
		{
			this->getPart1()->decrementConnectorCount();
			this->getPart2()->decrementConnectorCount();
		}
	}
}

bool CConstraint::fitConstraint()
{
	if (pPart1 != nullptr && pPart2 != nullptr)
	{
		//Get vec between particles
		D3DXVECTOR3 vecParticles = pPart1->getPos() - pPart2->getPos();
		float vecLength = D3DXVec3Length(&vecParticles);

		//Spread the flame if it has been burning long enough
		if (pPart1->isOnFire())
		{
			if (pPart1->getBurnTimer() >= 0.3f + static_cast<float>(rand() % 200) / 100.0f)
			{
				pPart2->setOnFire(true);
			}
		}
		if (pPart2->isOnFire())
		{
			if (pPart2->getBurnTimer() >= 0.3f + static_cast<float>(rand() % 200) / 100.0f)
			{
				pPart1->setOnFire(true);
			}
		}

		if (vecLength > 1.05f * fRestDistance)
		{
			//F2: Cloth can be torn under some amount of force
			//It's hurting.
			pPart1->addHealth(-0.0002f * vecLength);
			pPart2->addHealth(-0.0002f * vecLength);
		}
		else
		{

			//It's healing, as long as it's not burning.
			if (!pPart1->isOnFire())
				pPart1->addHealth(0.0001f);
			if (!pPart2->isOnFire())
				pPart2->addHealth(0.0001f);

		}

		//One or more of our particles is dead. We should die too.
		if (pPart1->getHealth() < 0.001f || pPart2->getHealth() < 0.001f)
			return false;

		//If, after all of that, we are still alive, lets constrain ourselves
		D3DXVECTOR3 vecCorrection = vecParticles * (1.0f - fRestDistance / vecLength);
		//Push particles to their appropriate position
		D3DXVECTOR3 vecHalf = vecCorrection * 0.5f;
		pPart1->addPosition(-vecHalf);
		pPart2->addPosition(vecHalf);
		return true;
	}
	return true;
};

