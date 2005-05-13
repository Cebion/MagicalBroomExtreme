//======================================================================================
//======================================================================================
//
//
//			�Ռ��g
//
//
//======================================================================================
//======================================================================================

#pragma once

//======================================================================================
// INCLUDE
//======================================================================================
#include "Base.h"
#include "CharaBase.h"

//======================================================================================
// CLASS
//======================================================================================
typedef class CSONIC : public CSPRITE
{
protected:
	long EndTime;
	long EndSize;
	long Alpha;
	D3DCOLOR Color;
	float yaw, pitch, roll;
	long No;

public:

	// function
	CSONIC( float x, float y, long size, long time, bool rot, long no, D3DCOLOR color );
	virtual ~CSONIC(){}
	virtual bool Action( void );
	virtual void Draw( void );
}
CSONIC, *LPCSONIC;