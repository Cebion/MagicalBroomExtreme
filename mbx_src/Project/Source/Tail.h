//======================================================================================
//======================================================================================
//
//
//			�K��
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
typedef class CTAIL : public CSPRITE
{
public:

	// function
	CTAIL( float x, float y );
	virtual ~CTAIL(){}
	virtual bool Action( void );
}
CTAIL, *LPCTAIL;