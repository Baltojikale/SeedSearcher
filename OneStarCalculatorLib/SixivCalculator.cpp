#include <iostream>
#include "Util.h"
#include "SixivCalculator.h"
#include "Const.h"
#include "XoroshiroState.h"
#include "Data.h"

// ���������ݒ�
static int g_Ivs[6];
static int g_FixedIndex1;
static int g_FixedIndex2;
static int g_Nature;
static int g_IvOffset;

#define LENGTH (61)

void SetSixCondition(int fixed1, int fixed2, int iv1, int iv2, int iv3, int iv4, int iv5, int iv6, int nature)
{
	g_FixedIndex1 = fixed1;
	g_FixedIndex2 = fixed2;
	g_Ivs[0] = iv1;
	g_Ivs[1] = iv2;
	g_Ivs[2] = iv3;
	g_Ivs[3] = iv4;
	g_Ivs[4] = iv5;
	g_Ivs[5] = iv6;
	g_Nature = nature;
}

void PrepareSix(int ivOffset)
{
	g_IvOffset = ivOffset;

	// �g�p����s��l���Z�b�g
	// �g�p����萔�x�N�g�����Z�b�g
	g_ConstantTermVector = 0;
	for (int i = 0; i < LENGTH - 1; ++i)
	{
		int index = 16 + 10 * ivOffset + i; // r[5+offset]����r[10+offset]�܂Ŏg��
		g_InputMatrix[i] = Const::c_Matrix[index];
		if (Const::c_ConstList[index] > 0)
		{
			g_ConstantTermVector |= (1ull << (LENGTH - 1 - i));
		}
	}
	// Ability��2�����k r[11+offset]
	int index = 80 + 10 * ivOffset;
	g_InputMatrix[LENGTH - 1] = Const::c_Matrix[index] ^ Const::c_Matrix[index + 5];
	if ((Const::c_ConstList[index] ^ Const::c_ConstList[index + 5]) != 0)
	{
		g_ConstantTermVector |= 1;
	}

	// �s��{�ό`�ŋ��߂�
	CalculateInverseMatrix(LENGTH);

	// ���O�f�[�^���v�Z
	CalculateCoefficientData(LENGTH);
}

_u64 SearchSix(int ivs)
{
	XoroshiroState xoroshiro;

	_u64 ivs64 = (_u64)ivs;

	_u64 target = 1; // ����2

	// ����30bit = �̒l
	target |= (ivs64 & 0x3E000000ul) << 31; // iv0_0
	target |= (ivs64 &  0x1F00000ul) << 26; // iv1_0
	target |= (ivs64 &    0xF8000ul) << 21; // iv2_0
	target |= (ivs64 &     0x7C00ul) << 16; // iv3_0
	target |= (ivs64 &      0x3E0ul) << 11; // iv4_0
	target |= (ivs64 &       0x1Ful) <<  6; // iv5_0

	// �B���ꂽ�l�𐄒�
	target |= ((32ul + g_Ivs[0] - ((ivs64 & 0x3E000000ul) >> 25)) & 0x1F) << 51;
	target |= ((32ul + g_Ivs[1] - ((ivs64 &  0x1F00000ul) >> 20)) & 0x1F) << 41;
	target |= ((32ul + g_Ivs[2] - ((ivs64 &    0xF8000ul) >> 15)) & 0x1F) << 31;
	target |= ((32ul + g_Ivs[3] - ((ivs64 &     0x7C00ul) >> 10)) & 0x1F) << 21;
	target |= ((32ul + g_Ivs[4] - ((ivs64 &      0x3E0ul) >> 5)) & 0x1F) << 11;
	target |= ((32ul + g_Ivs[5] - (ivs64 &        0x1Ful)) & 0x1F) << 1;

	// target�x�N�g�����͊���

	target ^= g_ConstantTermVector;

	// 60bit���̌v�Z���ʃL���b�V��
	_u64 processedTarget = 0;
	for (int i = 0; i < LENGTH; ++i)
	{
		processedTarget |= (GetSignature(g_AnswerFlag[i] & target) << (LENGTH - 1 - i));
	}

	// ���ʂ����߂�
	_u64 max = ((1 << (64 - LENGTH)) - 1);
	for (_u64 search = 0; search <= max; ++search)
	{
		_u64 seed = ((processedTarget ^ g_CoefficientData[search]) << (64 - LENGTH)) | search;

		// ��������i�荞��
		{
			xoroshiro.SetSeed(seed);
			xoroshiro.Next(); // EC
			xoroshiro.Next(); // OTID
			xoroshiro.Next(); // PID

			int ivs[6] = { -1, -1, -1, -1, -1, -1 };
			int fixedCount = 0;
			int offset = -2;
			do {
				int fixedIndex = 0;
				do {
					fixedIndex = xoroshiro.Next(7); // V�ӏ�
					++offset;
				} while (fixedIndex >= 6);

				if (ivs[fixedIndex] == -1)
				{
					ivs[fixedIndex] = 31;
					++fixedCount;
				}
			} while (fixedCount < 2);

			// reroll��
			if (offset != g_IvOffset)
			{
				continue;
			}

			// V�ӏ�
			if (ivs[g_FixedIndex1] != 31 || ivs[g_FixedIndex2] != 31)
			{
				continue;
			}

			// �̒l
			bool isPassed = true;
			for (int i = 0; i < 4; ++i)
			{
				if (g_Ivs[i] != xoroshiro.Next(0x1F))
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
			xoroshiro.Next();

			// ���ʒl
			int gender = 0;
			do {
				gender = xoroshiro.Next(0xFF); // ���ʒl
			} while (gender >= 253);

			int nature = 0;
			do {
				nature = xoroshiro.Next(0x1F); // ���i
			} while (nature >= 25);

			if (nature != g_Nature)
			{
				continue;
			}
		}

		{
			xoroshiro.SetSeed(seed);
			xoroshiro.Next(); // EC
			xoroshiro.Next(); // OTID
			xoroshiro.Next(); // PID

			int ivs[6] = { -1, -1, -1, -1, -1, -1 };
			int fixedCount = 0;
			do {
				int fixedIndex = 0;
				do {
					fixedIndex = xoroshiro.Next(7); // V�ӏ�
				} while (fixedIndex >= 6);

				if (ivs[fixedIndex] == -1)
				{
					ivs[fixedIndex] = 31;
					++fixedCount;
				}
			} while (fixedCount < 4);

			// �̒l
			xoroshiro.Next();
			xoroshiro.Next();

			// ����
			if (xoroshiro.Next(1) != 1)
			{
				continue;
			}

			// ���ʒl
			int gender = 0;
			do {
				gender = xoroshiro.Next(0xFF); // ���ʒl
			} while (gender >= 253);

			int nature = 0;
			do {
				nature = xoroshiro.Next(0x1F); // ���i
			} while (nature >= 25);

			if (nature != 18)
			{
				continue;
			}
		}

		// 2�C��
		_u64 nextSeed = seed + 0x82a2b175229d6a5bull;
		{
			xoroshiro.SetSeed(nextSeed);
			xoroshiro.Next(); // EC
			xoroshiro.Next(); // OTID
			xoroshiro.Next(); // PID

			int ivs[6] = { -1, -1, -1, -1, -1, -1 };
			int fixedCount = 0;
			do {
				int fixedIndex = 0;
				do {
					fixedIndex = xoroshiro.Next(7); // V�ӏ�
				} while (fixedIndex >= 6);

				if (ivs[fixedIndex] == -1)
				{
					ivs[fixedIndex] = 31;
					++fixedCount;
				}
			} while (fixedCount < 4);

			// �̒l
			bool isPassed = true;
			for (int i = 0; i < 6; ++i)
			{
				if (ivs[i] == 31)
				{
					if(i == 3 || i == 5)
					{
						isPassed = false;
						break;
					}
				}
			}
			if (!isPassed)
			{
				continue;
			}
			if (xoroshiro.Next(0x1F) != 1)
			{
				continue;
			}
			if (xoroshiro.Next(0x1F) != 27)
			{
				continue;
			}

			// ����
			xoroshiro.Next();

			// ���ʒl
			/*
			int gender = 0;
			do {
				gender = xoroshiro.Next(0xFF);
			} while (gender >= 253);
			*/

			// ���i
			int nature = 0;
			do {
				nature = xoroshiro.Next(0x1F);
			} while (nature >= 25);

			if (nature != 6)
			{
				continue;
			}
		}

		return seed;
	}
	return 0;
}
