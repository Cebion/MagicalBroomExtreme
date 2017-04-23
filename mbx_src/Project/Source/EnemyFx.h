/* encoding = CP932 */
//======================================================================================
//======================================================================================
//
//
//			�G�L�������j
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
typedef class CENEMYFX : public CSPRITE
{
protected:
	D3DCOLOR Color;

public:

	// function
	CENEMYFX( float x, float y, float speed, long angle, D3DCOLOR color );
	virtual ~CENEMYFX(){}
	virtual bool Action( void );
}
CENEMYFX, *LPCENEMYFX;