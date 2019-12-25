#include <iostream>
#include "Util.h"
#include "SixivCalculator.h"
#include "Const.h"
#include "XoroshiroState.h"
#include "Data.h"

// ���������ݒ�
extern int g_Ivs[6];

#define LENGTH (60)

void SetSixCondition(int fixed1, int fixed2, int iv1, int iv2, int iv3, int iv4, int iv5, int iv6, int nature)
{
	g_Ivs[0] = iv1;
	g_Ivs[1] = iv2;
	g_Ivs[2] = iv3;
	g_Ivs[3] = iv4;
	g_Ivs[4] = iv5;
	g_Ivs[5] = iv6;
}

void PrepareSix(int ivOffset)
{
	// �g�p����s��l���Z�b�g
	// �g�p����萔�x�N�g�����Z�b�g
	g_ConstantTermVector = 0;
	for (int i = 0; i < LENGTH; ++i)
	{
		int index = 16 + 10 * ivOffset; // r[5+offset]����r[10+offset]�܂Ŏg��
		g_InputMatrix[i] = Const::c_Matrix[index];
		if (Const::c_ConstList[index] > 0)
		{
			g_ConstantTermVector |= (1ull << (LENGTH - 1 - i));
		}
	}

	// �s��{�ό`�ŋ��߂�
	CalculateInverseMatrix(LENGTH);
}

_u64 SearchSix(int ivs)
{
	_u64 ivs64 = (_u64)ivs;

	_u64 target = 0;

	// ����30bit = �̒l
	target |= (ivs64 & 0x3E000000ul) << 30; // iv0_0
	target |= (ivs64 &  0x1F00000ul) << 25; // iv1_0
	target |= (ivs64 &    0xF8000ul) << 20; // iv2_0
	target |= (ivs64 &     0x7C00ul) << 15; // iv3_0
	target |= (ivs64 &      0x3E0ul) << 10; // iv4_0
	target |= (ivs64 &       0x1Ful) <<  5; // iv5_0

	// �B���ꂽ�l�𐄒�
	target |= ((32ul + g_Ivs[0] - ((ivs64 & 0x3E000000ul) >> 25)) & 0x1F) << 25;
	target |= ((32ul + g_Ivs[1] - ((ivs64 &  0x1F00000ul) >> 20)) & 0x1F) << 20;
	target |= ((32ul + g_Ivs[2] - ((ivs64 &    0xF8000ul) >> 15)) & 0x1F) << 15;
	target |= ((32ul + g_Ivs[3] - ((ivs64 &     0x7C00ul) >> 10)) & 0x1F) << 10;
	target |= ((32ul + g_Ivs[4] - ((ivs64 &      0x3E0ul) >> 5)) & 0x1F) << 5;
	target |= ((32ul + g_Ivs[5] - (ivs64 &        0x1Ful)) & 0x1F);

	// target�x�N�g�����͊���

	// 60bit���̌v�Z���ʃL���b�V��
	_u64 processedTarget = 0;
	for (int i = 0; i < 57; ++i)
	{
//		processedTarget |= (GetSignature(Const::c_FormulaAnswerFlag[i] & target) << (56 - i));
	}

	return 0;
}
