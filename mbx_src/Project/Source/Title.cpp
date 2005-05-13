//======================================================================================
//======================================================================================
//
//
//			タイトル
//
//
//======================================================================================
//======================================================================================


//======================================================================================
//	include
//======================================================================================
#include "Title.h"


//======================================================================================
//
//	コンストラクタ
//
//======================================================================================
CTITLE::CTITLE( float x, float y, long no ) : CSPRITE( SORT_NULL )
{
	Px = x;
	Py = y;
	No = no;
	Direction = 0;

	// 0番ははじめから選択状態
	bSelect = (no == 0);

	// いきなり揺れないため
	Phase = 2;
}

//======================================================================================
//
//	処理
//
//======================================================================================
bool CTITLE::Action( void )
{
	D3DCOLOR color;

	//------------------------------------------
	// 処理
	//------------------------------------------
	if ( bSelect )
	{
		const long dir[2] = { 384, -384 };
		switch ( Phase )
		{
	// -------------------------------------------------------- 初期化
		case 0:
			y1 = Py;
			Angle = 0;
			Scale = 30;
			Phase++;
			break;
	// -------------------------------------------------------- 上下揺れ
		case 1:
			y1 = Py + LunaMath::Sin( Angle, Scale/2 );
			Angle += dir[Direction];
			Scale -= 1;
			if ( Scale == 0 ) Phase++;
			break;
	// -------------------------------------------------------- 待機
		case 2:
			y1 = Py;
			break;
		}
		// 描画位置設定
		SetRect( &Dest, (long)Px-96, (long)y1-24, (long)Px+96, (long)y1+24 );
		SetRect( &Src, 0, 140+No*48, 192, 140+No*48+48 );
		color = D3DCOLOR_XRGB(192,192,192);
	}
	else
	{
		// 描画位置設定
		SetRect( &Dest, (long)Px-96, (long)Py-24, (long)Px+96, (long)Py+24 );
		SetRect( &Src, 0, 140+No*48, 192, 140+No*48+48 );
		color = D3DCOLOR_XRGB(48,48,48);
	}

	return bDeath;
}

//======================================================================================
//
//	転送矩形取得
//
//======================================================================================
void CTITLE::GetRect( RECT *pdest, RECT *psrc )
{
	*pdest = Dest;
	*psrc = Src;
}
