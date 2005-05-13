//======================================================================================
//======================================================================================
//
//
//			�w�i
//
//
//======================================================================================
//======================================================================================


//======================================================================================
//	include
//======================================================================================
#include "BG.h"

//======================================================================================
//
//	�R���X�g���N�^
//
//======================================================================================
CBG::CBG( float x, float y, float u1, float v1, float u2, float v2, float scale ) : CSPRITE( SORT_NULL )
{
	x1 = x - (u2-u1)*scale/2;
	y1 = y - (v2-v1)*scale/2;
	x2 = x + (u2-u1)*scale/2;
	y2 = y + (v2-v1)*scale/2;
	x3 = u1;
	y3 = v1;
	x4 = u2;
	y4 = v2;
}

//======================================================================================
//
//	����
//
//======================================================================================
bool CBG::Action( void )
{
	RECT src = { (long)x3, (long)y3, (long)x4, (long)y4 };
	RECT dest = { (long)x1, (long)y1, (long)x2, (long)y2 };
	Sprite[0][0]->Draw( &dest, D3DCOLOR_XRGB(255,255,255), &src );

	return bDeath;
}