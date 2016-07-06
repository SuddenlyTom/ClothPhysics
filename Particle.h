// 
// Bachelor of Software Engineering
// Media Design School
// Auckland
// New Zealand
//
// (c) 2005 - 2015 Media Design School
//
// File Name	: Particle.h
// Description	: CParticle and CConstraint declatations.
// Author		: Thomas OBrien.
// Mail			: thomas.obrien@mediadesignschool.com
//

#pragma once

#include "Utility.h"
#include <string>

class CParticle
{
public:
	CParticle();
	CParticle(D3DXVECTOR3 pos) :vecPos(pos), vecOldPos(pos), vecAcceleration(D3DXVECTOR3(0, 0, 0)),
		fMass(1.0f), fHealth(1.0f), bCanMove(true), bOnFire(false), fBurnTimer(0), iConnectionCount(0){};
	~CParticle();
	
	void update(float dt);
	
	void setOnFire(bool b) { bOnFire = b; };
	bool isOnFire() { return bOnFire; };

	void addBurnTime(float f) { fBurnTimer = f; };
	float getBurnTimer() { return fBurnTimer; };


	/** Add force to particle
		@remarks
			uses the mass of the particle to make an addition to the acceleration
			used in verlat integration
		@author Tom O'Brien	
	*/
	void addForce(D3DXVECTOR3 force) {
		vecAcceleration += (force / fMass);
	}

	/** Add health to particle
		@author Tom O'Brien
	*/
	void addHealth(float h){ 
		if ( fHealth + h <= 1.0f && fHealth + h > 0.0f )
			fHealth += h; 
	};

	/** Increment Connector Count
		@remarks
			The value used to determine how many connections remain on this particle			
		@author Tom O'Brien
	*/
	void incrementConnectorCount() { iConnectionCount++; };

	/** Decrement Connector Count
		@remarks
			The value used to determine how many connections remain on this particle
			Only reduce if above 0, kill particle if 0 connections
		@author Tom O'Brien
	*/
	void decrementConnectorCount() { 
		if( iConnectionCount - 1 >= 0)
			iConnectionCount--;
		if (iConnectionCount <= 0)
		{
			fHealth = 0.0f;
			setMovable(false);
		}
	};
	int getConnectorCount() { return iConnectionCount; };

	D3DXVECTOR3& getPos(){ 
		return vecPos;
	};


	void setPos(D3DXVECTOR3& _rPos) {
		vecPos = _rPos;
	};
	
	void zeroAcceleration() { 
		vecAcceleration = D3DXVECTOR3(0, 0, 0); 
	};
	
	void addPosition(const D3DXVECTOR3 offset) {
		if (bCanMove) {
			vecPos += offset;
		}
	};

	void forceAddPosition(const D3DXVECTOR3 offset) {
			vecPos += offset;
			vecOldPos = vecPos;
	};

	float getHealth(){
		return fHealth;
	}

	void setMovable(bool state) {
		bCanMove = state;
	}

	static void setDamping(float f) {
		fDamping = f;
	}

	bool canMove() { return bCanMove; };


private:
	static float fDamping;

	bool bOnFire;
	float fBurnTimer;

	bool bCanMove;
	float fMass;
	D3DXVECTOR3 vecPos;
	D3DXVECTOR3 vecOldPos;
	D3DXVECTOR3 vecAcceleration;
	float fHealth;
	int iConnectionCount;

};

class CConstraint
{
public:
	//Member Functions
	CConstraint(CParticle* p1, CParticle* p2);

	void setIsMarkedForDeath(bool b) { bMarkedForDeath = b;	};
	bool isMarkedForDeath() { return bMarkedForDeath; };

	void setIsAlive(bool b);
	bool isAlive() { return bIsAlive; };

	float getRestDistance() { return fRestDistance; };

	/** fitConstraint
		@remarks
			make sure particles are the correct distance away
		@author Tom O'Brien
	*/
	bool fitConstraint();
	
	CParticle* getPart1() {	return pPart1; };
	CParticle* getPart2() {return pPart2;};

private:
	//Member Variables
	CParticle* pPart1;
	CParticle* pPart2;

	float fBurnTimer;
	bool bMarkedForDeath;
	bool bIsAlive;
	float fRestDistance;
};

