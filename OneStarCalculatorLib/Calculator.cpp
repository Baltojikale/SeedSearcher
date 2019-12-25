#include <iostream>
#include "Calculator.h"
#include "Const.h"
#include "XoroshiroState.h"

// ���������ݒ�
int g_Ivs[6];
int g_Ability;
int g_FixedIndex;

// �i�荞�ݏ����ݒ�
int g_Nature;
int g_NextIvs[6];
int g_NextAbility;
int g_NextFixedIndex;
int g_NextNature;

// V�m��p�Q��
const int* g_IvsRef[30] = {
	&g_Ivs[1], &g_Ivs[2], &g_Ivs[3], &g_Ivs[4], &g_Ivs[5],
	&g_Ivs[0], &g_Ivs[2], &g_Ivs[3], &g_Ivs[4], &g_Ivs[5],
	&g_Ivs[0], &g_Ivs[1], &g_Ivs[3], &g_Ivs[4], &g_Ivs[5],
	&g_Ivs[0], &g_Ivs[1], &g_Ivs[2], &g_Ivs[4], &g_Ivs[5],
	&g_Ivs[0], &g_Ivs[1], &g_Ivs[2], &g_Ivs[3], &g_Ivs[5],
	&g_Ivs[0], &g_Ivs[1], &g_Ivs[2], &g_Ivs[3], &g_Ivs[4]
};
const int* g_NextIvsRef[30] = {
	&g_NextIvs[1], &g_NextIvs[2], &g_NextIvs[3], &g_NextIvs[4], &g_NextIvs[5],
	&g_NextIvs[0], &g_NextIvs[2], &g_NextIvs[3], &g_NextIvs[4], &g_NextIvs[5],
	&g_NextIvs[0], &g_NextIvs[1], &g_NextIvs[3], &g_NextIvs[4], &g_NextIvs[5],
	&g_NextIvs[0], &g_NextIvs[1], &g_NextIvs[2], &g_NextIvs[4], &g_NextIvs[5],
	&g_NextIvs[0], &g_NextIvs[1], &g_NextIvs[2], &g_NextIvs[3], &g_NextIvs[5],
	&g_NextIvs[0], &g_NextIvs[1], &g_NextIvs[2], &g_NextIvs[3], &g_NextIvs[4]
};

// ���O�v�Z�f�[�^
_u64 g_CoefficientData[0x80];

inline _u64 GetSignature(_u64 value)
{
	unsigned int a = (unsigned int)(value ^ (value >> 32));
	a = a ^ (a >> 16);
	a = a ^ (a >> 8);
	a = a ^ (a >> 4);
	a = a ^ (a >> 2);
	return (a ^ (a >> 1)) & 1;
}

inline _u64 GetSignature7(unsigned short value)
{
	value = value ^ (value >> 4);
	value = value ^ (value >> 2);
	return (value ^ (value >> 1)) & 1;
}

void SetFirstCondition(int iv0, int iv1, int iv2, int iv3, int iv4, int iv5, int ability, int nature)
{
	g_Ivs[0] = iv0;
	g_Ivs[1] = iv1;
	g_Ivs[2] = iv2;
	g_Ivs[3] = iv3;
	g_Ivs[4] = iv4;
	g_Ivs[5] = iv5;
	g_FixedIndex = 0;
	for (int i = 0; i < 6; ++i)
	{
		if (g_Ivs[i] == 31)
		{
			g_FixedIndex = i;
		}
	}
	g_Ability = ability;
	g_Nature = nature;
}

void SetNextCondition(int iv0, int iv1, int iv2, int iv3, int iv4, int iv5, int ability, int nature)
{
	g_NextIvs[0] = iv0;
	g_NextIvs[1] = iv1;
	g_NextIvs[2] = iv2;
	g_NextIvs[3] = iv3;
	g_NextIvs[4] = iv4;
	g_NextIvs[5] = iv5;
	g_NextFixedIndex = 0;
	for (int i = 0; i < 6; ++i)
	{
		if (g_NextIvs[i] == 31)
		{
			g_NextFixedIndex = i;
		}
	}
	g_NextAbility = ability;
	g_NextNature = nature;
}

void Prepare()
{
	// �f�[�^�����
	for (unsigned short search = 0; search <= 0x7F; ++search)
	{
		g_CoefficientData[search] = 0;
		for (int i = 0; i < 57; ++i)
		{
			g_CoefficientData[search] |= (GetSignature7(Const::c_FormulaCoefficient[i] & search) << (56 - i));
		}
	}
}

int GetHiddenValue(int start, int end, _u64 seed)
{
	int value = 0;
	for (int i = start; i <= end; ++i)
	{
		if ((GetSignature(Const::c_Matrix[i] & seed) ^ Const::c_ConstList[i]) != 0)
		{
			value |= (1 << (end - i));
		}
	}
	return value;
}

_u64 Search(int ivs)
{
	XoroshiroState xoroshiro;
	
	_u64 ivs64 = (_u64)ivs;

	_u64 target = g_Ability;

	// ���3bit = V�ӏ�����
	target |= (ivs64 & 0xE000000ul) << 29; // fixedIndex0

	// ����25bit = �̒l
	target |= (ivs64 & 0x1F00000ul) << 26; // iv0_0
	target |= (ivs64 &   0xF8000ul) << 21; // iv1_0
	target |= (ivs64 &    0x7C00ul) << 16; // iv2_0
	target |= (ivs64 &     0x3E0ul) << 11; // iv3_0
	target |= (ivs64 &      0x1Ful) <<  6; // iv4_0

	// �B���ꂽ�l�𐄒�
	target |= ((8ul + g_FixedIndex - ((ivs64 & 0xE000000ul) >> 25)) & 7) << 51;

	target |= ((32ul + *g_IvsRef[g_FixedIndex * 5    ] - ((ivs64 & 0x1F00000ul) >> 20)) & 0x1F) << 41;
	target |= ((32ul + *g_IvsRef[g_FixedIndex * 5 + 1] - ((ivs64 &   0xF8000ul) >> 15)) & 0x1F) << 31;
	target |= ((32ul + *g_IvsRef[g_FixedIndex * 5 + 2] - ((ivs64 &    0x7C00ul) >> 10)) & 0x1F) << 21;
	target |= ((32ul + *g_IvsRef[g_FixedIndex * 5 + 3] - ((ivs64 &     0x3E0ul) >> 5)) & 0x1F) << 11;
	target |= ((32ul + *g_IvsRef[g_FixedIndex * 5 + 4] -  (ivs64 &      0x1Ful)) & 0x1F) << 1;

	// target�x�N�g�����͊���

	target ^= Const::c_Const;

	// 57bit���̌v�Z���ʃL���b�V��
	_u64 processedTarget = 0;
	for (int i = 0; i < 57; ++i)
	{
		processedTarget |= (GetSignature(Const::c_FormulaAnswerFlag[i] & target) << (56 - i));
	}

	// ����7bit�����߂�
	for (_u64 search = 0; search <= 0x7F; ++search)
	{
		_u64 seed = ((processedTarget ^ g_CoefficientData[search]) << 7) | search;

		// ��������i�荞��
		xoroshiro.SetSeed(seed);
		xoroshiro.Next(1); // EC
		xoroshiro.Next(1); // OTID
		xoroshiro.Next(1); // PID
		xoroshiro.Next(1); // V�ӏ�
		xoroshiro.Next(1); // �̒l1
		xoroshiro.Next(1); // �̒l2
		xoroshiro.Next(1); // �̒l3
		xoroshiro.Next(1); // �̒l4
		xoroshiro.Next(1); // �̒l5
		xoroshiro.Next(1); // ����
		xoroshiro.Next(1); // ���ʒl

		int nature = 0;
		do {
			nature = xoroshiro.Next(0x1F); // ���i
		} while (nature >= 25);

		if (nature != g_Nature)
		{
			continue;
		}

		// 2�C��
		_u64 nextSeed = seed + 0x82a2b175229d6a5bull;
		xoroshiro.SetSeed(nextSeed);
		xoroshiro.Next(1); // EC
		xoroshiro.Next(1); // OTID
		xoroshiro.Next(1); // PID
		int fixedIndex = 0;
		do {
			fixedIndex = xoroshiro.Next(7); // V�ӏ�
		} while (fixedIndex >= 6);

		if (g_NextFixedIndex != fixedIndex)
		{
			continue;
		}

		// �̒l
		bool isPassed = true;
		for (int i = 0; i < 5; ++i)
		{
			if ((*g_NextIvsRef[g_NextFixedIndex * 5 + i]) != xoroshiro.Next(0x1F))// �̒l1
			{
				isPassed = false;
				break;
			}
		}
		if (!isPassed)
		{
			continue;
		}
		
		// ����
		if (g_NextAbility != xoroshiro.Next(1))
		{
			continue;
		}

		// ���ʒl
		xoroshiro.Next(1);

		nature = 0;
		do {
			nature = xoroshiro.Next(0x1F); // ���i
		} while (nature >= 25);

		if (nature != g_NextNature)
		{
			continue;
		}

		return seed;
	}
	return 0;
}
