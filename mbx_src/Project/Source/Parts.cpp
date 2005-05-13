//======================================================================================
//======================================================================================
//
//
//			警告
//
//
//======================================================================================
//======================================================================================


//======================================================================================
//	include
//======================================================================================
#include "Parts.h"
#include "Application.h"


//======================================================================================
//
//	コンストラクタ
//
//======================================================================================
CPARTS::CPARTS( float x, float y, RECT *prect, LSURFACE p ) : CSPRITE( SORT_NULL )
{
	Px = x;
	Py = y;
	x2 = 0;
	y2 = 0.5f;
	x3 = F(prect->left);
	y3 = F(prect->top);
	x4 = F(prect->right);
	y4 = F(prect->bottom);
	long angle = RAND(2048+384,4096-384);
	x1 = F(LunaMath::Cos( angle )) *  5.0f / 65536.0f;
	y1 = F(LunaMath::Sin( angle )) * 10.0f / 65536.0f;
	pSurf = p;
	pSurf->AddRef();
}

//======================================================================================
//
//	処理
//
//======================================================================================
bool CPARTS::Action( void )
{
	Px += x1 += x2;
	Py += y1 += y2;

	if ( Py > 480 )
	{
		pSurf->Release();
		return true;
	}

	CApplication::AddEffect( this );

	return false;
}


//======================================================================================
//
//	描画
//
//======================================================================================
void CPARTS::Draw( void )
{
	pSurf->Blit( (long)Px, (long)Py, (long)x3, (long)y3, (long)x4, (long)y4 );
}