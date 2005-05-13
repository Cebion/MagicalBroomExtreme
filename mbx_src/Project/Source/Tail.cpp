//======================================================================================
//======================================================================================
//
//
//			尻尾
//
//
//======================================================================================
//======================================================================================


//======================================================================================
//	include
//======================================================================================
#include "Tail.h"


//======================================================================================
//
//	コンストラクタ
//
//======================================================================================
CTAIL::CTAIL( float x, float y ) : CSPRITE( SORT_NULL )
{
	Px = x+8;
	Py = y+42;
	x1 = F(RAND(-5,-2))/3.0f;
	y1 = F(RAND(-1,1))/3.0f;
	x2 = 0.0f;
	y2 = F(RAND(1,3))/100.0f;
}

//======================================================================================
//
//	処理
//
//======================================================================================
bool CTAIL::Action( void )
{
	Px += x1 += x2;
	Py += y1 += y2;

	Scale = LunaMath::FlatSpeed( 6L, 0L, 80, Time );

	Angle += 32;
	Angle &= 4095;

	long r = RAND( 0x30, 0x80 );
	long g = RAND( 0x30, 0x80 );
	long b = RAND( 0x30, 0x80 );

	RECT dest = { (long)Px-Scale, (long)Py-Scale, (long)Px+Scale, (long)Py+Scale };
	RECT src = { 520, 605, 552, 637 };
	Sprite[5][1]->DrawRotate( &dest, D3DCOLOR_XRGB(r,g,b), ANGLE2PAI(Angle), &src );

	return (++Time == 80);
}