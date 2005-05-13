//======================================================================================
//======================================================================================
//
//
//			�r�b�g
//
//
//======================================================================================
//======================================================================================


//======================================================================================
//	include
//======================================================================================
#include "Bit.h"
#include "BitBlur.h"
#include "EnemyFx.h"
#include "Sound.h"


long CBIT::BitNum = 0;
long CBIT::BitCount = 0;

//======================================================================================
//
//	�R���X�g���N�^
//
//======================================================================================
CBIT::CBIT( float x, float y, long lv ) : CSPRITE( SORT_PLAYERSHOT )
{
	// ���x�����Ƃ̍ő唭�ˉ\��
	const long max[] = { 2, 4, 6, 8 };
	if ( ++BitNum > max[lv] )
	{
		Release();
		return;
	}

	SetHitCount( 5 );

	// �T�E���h�Đ�
	CSound::Play( SE_BIT );

	// ���˔ԍ����Ƃ̔��ˊp�x
	const long angle[] = { -128, 128, -256, 256, -384, 384, -512, 512 };

	Power = 50;
	Px = x;
	Py = y;
	bHit = true;
	Speed = 8;
	Angle = 2048+angle[BitCount];

	++BitCount %= max[lv];
}

//======================================================================================
//
//	����
//
//======================================================================================
bool CBIT::Action( void )
{
	RECT src = { 736, 446, 768, 478 };

	// ��ԋ߂��G����
	float x = Px;
	float y = Py;
	ReferenceNear( &x, &y, SORT_ENEMY );

	//------------------------------------------
	// ���W�v�Z
	//------------------------------------------

	// �ڕW�Ƃ̊p�x�Z�o
	long a = LunaMath::Atan( (long)(x-Px), (long)(y-Py) );

	// ���݂̊p�x�Ƃ̑��Ίp�x�Z�o
	a = Angle - a;
	a &= 4095;

	// �p�x�ɉ���������
	if ( a > 2048 )	{ Angle += 64; }
	else			{ Angle -= 64; }
	Angle &= 4095;

	// �p�x�ɉ����ăX�s�[�h����
	if ( a < 256 || a > 4096-256 )
	{
		if ( (Speed += 0.8f) > 12 ) Speed = 12;
	}
	else
	{
		if ( (Speed -= 1.2f) < 4 ) Speed = 4;
	}

	Px += LunaMath::Cos( (long)Angle, (long)Speed );
	Py += LunaMath::Sin( (long)Angle, (long)Speed );

	//-------------------------------------------
	// �`�揈��
	//-------------------------------------------
	RECT dest = { (long)Px-8, (long)Py-8, (long)Px+8, (long)Py+8 };

	Sprite[3][0]->Draw( &dest, D3DCOLOR_XRGB(255,255,255), &src );

	//-------------------------------------------
	// �����蔻��
	//-------------------------------------------
	lpHitPt[0].x = Px-8;	lpHitPt[0].y = Py-8;
	lpHitPt[1].x = Px+8;	lpHitPt[1].y = Py-8;
	lpHitPt[2].x = Px+8;	lpHitPt[2].y = Py+8;
	lpHitPt[3].x = Px-8;	lpHitPt[3].y = Py+8;
	lpHitPt[4] = lpHitPt[0];

	// �c��
	new CBITBLUR( Px, Py );

	return bDeath;
}

//======================================================================================
//
//	�_���[�W
//
//======================================================================================
void CBIT::Damage( long pow )
{
	CSound::Play( SE_BIT );
	bHit = false;
	bDeath = true;
	// �j�ЎU�炵
	for ( long i = 0; i < 16; i++ )
	{
		long angle = i * 4096 / 16;
		new CENEMYFX( Px, Py, 1, angle, D3DCOLOR_XRGB(255,255,255) );
	}
}