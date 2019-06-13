/*
 * Developed by Wang Xiao on 6/12/19 11:57 PM.
 * Last modified 6/12/19 2:10 PM.
 * Copyright (c) 2019. All rights reserved.
 */
 
 // miner.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"

bool ini();
bool clean();
BOOL AutoMine();
VOID LeftButtonClick(int row, int column);
VOID RightButtonClick(int row, int column);
int  IdentifyArea(int row, int column);			// 返回值为周围雷的数目，-1 未打开 -8 为自身为雷
bool Scan();
int  GetOpenCount(int row, int column);
int  GetUndefineCount(int row, int column);
int  GetMineCount(int row, int column);
int  GetTotalCount(int row, int column);

const int ROWS = 16;
const int COLUMNS = 30;
const int STARTX = 12;
const int STARTY = 55;
const int WIDTH = 16;
const int HEIGHT = 16;

struct Area
{
	int mineCount; // -8 确信是雷 -4 怀疑是雷 -2 有待进一步判断  -1 未知即还没有打开 负数代表是雷的可能性大小 
	int left;
	int top;
	float probability; // 是雷的概率
	int xPos;
	int yPos;
};

struct Operation
{
	int row;
	int column;
	int l1r2;
};

struct ProbabilitySet
{
	int mineCount;
	int setLength;
	Area *area[8];
};

int  Analyze(Operation oper[30 * 16]);
int  AnalyzeSet(Operation oper[30 * 16]);
int  AnalyzeCount(Operation oper[30 * 16]);
int  CompareSet(ProbabilitySet *lSet, ProbabilitySet *rSet);
int  GetSetDiff(ProbabilitySet *lSet, ProbabilitySet *rSet, int &operCount, Operation oper[30 * 16]);

ProbabilitySet probabilitySet[ROWS][COLUMNS];
Area mineField[ROWS][COLUMNS];
int probabilityRow;
int probabilityColumn;

int remainMines = 99;

HWND hWnd = NULL;
HDC  hDC  = NULL;
HBRUSH hRBrush = NULL;
HBRUSH hGBrush = NULL;
HBRUSH hYBrush = NULL;
HBRUSH hOldBrush = NULL;

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AutoMine, NULL, 0, 0);
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}

bool ini()
{
	hWnd = FindWindow(NULL, "扫雷");
	if (NULL == hWnd)
	{
		hWnd = FindWindow(NULL, "Minesweeper");
	}
	if (NULL != hWnd)
	{
		hDC = GetDC(hWnd);
		if (NULL == hDC)
		{
			MessageBox(NULL, "未找到图形区", "", 0);
		}
		else
		{
			hRBrush = CreateSolidBrush(RGB(255, 0, 0));
			hGBrush = CreateSolidBrush(RGB(0, 255, 0));
			hYBrush = CreateSolidBrush(RGB(255, 255, 0));
			hOldBrush = (HBRUSH)SelectObject(hDC, hRBrush);
			
			SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

			return true;
		}
	}
	else
	{
		MessageBox(NULL, "未找到窗口", "", 0);
	}

	return false;
}

bool clean()
{
	SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	
	SelectObject(hDC, hOldBrush);
	DeleteObject(hRBrush);
	DeleteObject(hGBrush);
	DeleteObject(hYBrush);
	ReleaseDC(hWnd, hDC);
	return true;
}

BOOL AutoMine()
{
	if (!ini())
	{
		return FALSE;
	}

	// 初始化高级雷区数据
	for (int i = 0; i < ROWS; i++)
	{
		for (int j = 0; j < COLUMNS; j++)
		{
			mineField[i][j].mineCount = -1;
			mineField[i][j].left = STARTX + WIDTH * j;
			mineField[i][j].top = STARTY + HEIGHT * i;
			mineField[i][j].probability = 99.0f / 480.0f;
			mineField[i][j].xPos = i;
			mineField[i][j].yPos = j;

			probabilitySet[i][j].mineCount = -1;
			probabilitySet[i][j].setLength = 0;
			for (int m = 0; m < 8; m++)
			{
				probabilitySet[i][j].area[m] = NULL;
			}
		}
	}

	remainMines = 99;

	int operCount = 0;
	Operation oper[30 * 16];

	while (remainMines > 0)
	{
		operCount = Analyze(oper);
		if (operCount == 0)
		{	
			SelectObject(hDC, hRBrush);
			LeftButtonClick(probabilityRow, probabilityColumn);
			if (!Scan())
			{
				return FALSE;
			}
		}
		else
		{
			for (int m = 0; m < operCount; m++)
			{
				if (oper[m].l1r2 == 1)
				{
					SelectObject(hDC, hGBrush);
					LeftButtonClick(oper[m].row, oper[m].column);
				}
				else if (oper[m].l1r2 == 2)
				{
					SelectObject(hDC, hYBrush);
					if (mineField[oper[m].row][oper[m].column].mineCount == -1)
					{
						RightButtonClick(oper[m].row, oper[m].column);
						mineField[oper[m].row][oper[m].column].mineCount = -8;
						remainMines--;
					}
				}
			}
			if (!Scan())
			{
				return FALSE;
			}
		}
	}

	if (remainMines == 0)
	{
		for (int i = 0; i < ROWS; i++)
		{
			for (int j = 0; j < COLUMNS; j++)
			{
				if (mineField[i][j].mineCount == -1)
				{
					SelectObject(hDC, hGBrush);
					LeftButtonClick(i, j);
				}
			}
		}
	}

	clean();

	return TRUE;
}

int Analyze(Operation oper[30 * 16])
{
	float probability = 1.0f;
	float MinProbability = 1.0f;
	int mineCount = 0;
	int openCount = 0;
	int undefined = 0;
	int operCount = 0;

	for (int i = 0; i < ROWS; i++)
	{
		for (int j = 0; j < COLUMNS; j++)
		{
			if (mineField[i][j].mineCount > 0)
			{
				mineCount = 0;
				undefined = GetUndefineCount(i, j);
				if (undefined == 0)
				{
					continue;
				}

				for (int m = i - 1; m <= i + 1; m++)
				{
					for (int n = j - 1; n <= j + 1; n++)
					{
						if (m >= 0 && n >= 0 && m < ROWS && n < COLUMNS && (m != i || n != j))
						{
							if (mineField[m][n].mineCount == -8)
							{
								mineCount++;
							}
							else if (mineField[m][n].mineCount == -1)
							{
								oper[operCount].row = m;
								oper[operCount].column = n;
								operCount++;
							}
							else
							{
								openCount++;
							}
						}
					}
				}

				if (mineField[i][j].mineCount == mineCount)
				{
					for (int k = operCount - undefined; k < operCount; k++)
					{
						oper[k].l1r2 = 1;
					}
				}
				else if ((mineField[i][j].mineCount == undefined + mineCount))
				{
					for (int k = operCount - undefined; k < operCount; k++)
					{
						oper[k].l1r2 = 2;
					}
				}
				else
				{
					operCount -= undefined;
				}
				
				if (operCount == 0)
				{
					probability = (float)(mineField[i][j].mineCount - mineCount) / (float)(undefined);
					for (int m = i - 1; m <= i + 1; m++)
					{
						for (int n = j - 1; n <= j + 1; n++)
						{
							if (m >= 0 && n >= 0 && m < ROWS && n < COLUMNS && (m != i || n != j))
							{
								if (mineField[m][n].mineCount == -1 && mineField[m][n].probability < probability)
								{
									mineField[m][n].probability = probability;
								}
							}
						}
					}
				}
			}
		}
	}

	if (operCount == 0)
	{
		operCount = AnalyzeSet(oper);
	}

	if (openCount == 0 && remainMines < 10)
	{
		openCount = AnalyzeCount(oper);
	}

	if (operCount == 0)
	{	
		undefined = 0;
		for (int m = 0; m < ROWS; m++)
		{
			for (int n = 0; n < COLUMNS; n++)
			{
				if (mineField[m][n].mineCount == -1)
				{
					undefined++;
				}
			}
		}
		
		if (undefined > 0)
		{
			probability = (float)(remainMines) / (float)(undefined);
			for (int m = 0; m < ROWS; m++)
			{
				for (int n = 0; n < COLUMNS; n++)
				{
					if (mineField[m][n].mineCount == -1 && GetTotalCount(m, n) == GetUndefineCount(m, n) && mineField[m][n].probability > probability)
					{
						mineField[m][n].probability = probability;
					}
					if (mineField[m][n].mineCount == -1 && mineField[m][n].probability < MinProbability)
					{
						MinProbability = mineField[m][n].probability;
						probabilityRow = m;
						probabilityColumn = n;
					}
					if ((probabilityRow == 0 || probabilityColumn == 0) && mineField[m][n].mineCount == -1
						&& mineField[m][n].probability == mineField[probabilityRow][probabilityColumn].probability)
					{
						probabilityRow = m;
						probabilityColumn = n;
					}
				}
			}
		}
	}
	return operCount;
}

int AnalyzeSet(Operation oper[30 * 16])
{
	int operCount = 0;
	int index = 0;

	// 生成集合列表
	for (int i = 0; i < ROWS; i++)
	{
		for (int j = 0; j < COLUMNS; j++)
		{
			probabilitySet[i][j].setLength = 0;
			if (mineField[i][j].mineCount > 0 && GetUndefineCount(i, j) > 0)
			{
				index = 0;
				probabilitySet[i][j].mineCount = mineField[i][j].mineCount - GetMineCount(i, j);
				for (int m = i - 1; m <= i + 1; m++)
				{
					for (int n = j - 1; n <= j + 1; n++)
					{
						if (m >= 0 && n >= 0 && m < ROWS && n < COLUMNS && mineField[m][n].mineCount == -1)
						{
							probabilitySet[i][j].area[index] = &mineField[m][n];
							index++;
						}
					}
				}
				probabilitySet[i][j].setLength = index;
			}
		}
	}

	// 比较集合列表
	for (i = 0; i < ROWS; i++)
	{
		for (int j = 0; j < COLUMNS; j++)
		{
			if (probabilitySet[i][j].setLength > 0)
			{
				for (int m = i; m <= i + 2; m++)
				{
					for (int n = j; n <= j + 2; n++)
					{
						if (m < ROWS && n < COLUMNS && probabilitySet[m][n].setLength > 0)
						{
							if (CompareSet(&probabilitySet[i][j], &probabilitySet[m][n]) == 0)
							{
								GetSetDiff(&probabilitySet[i][j], &probabilitySet[m][n], operCount, oper);
							}
						}
					}
				}
			}
		}
	}
	return operCount;
}

int CompareSet(ProbabilitySet *lSet, ProbabilitySet *rSet)
{
	int sameCount = 0;

	for (int i = 0; i < lSet->setLength; i++)
	{
		for (int j = 0; j < rSet->setLength; j++)
		{
			if (lSet->area[i] == rSet->area[j])
			{
				sameCount++;
				break;
			}
		}
	}

	if (lSet->setLength != rSet->setLength && (sameCount == lSet->setLength || sameCount == rSet->setLength))
	{
		return 0;
	}

	return 1;
}

int GetSetDiff(ProbabilitySet *lSet, ProbabilitySet *rSet, int &operCount, Operation oper[30 * 16])
{
	ProbabilitySet *shortSet = NULL;
	ProbabilitySet *longSet = NULL;
	bool find = false;
	int l1r2 = 0;

	if (lSet->setLength < rSet->setLength)
	{
		shortSet = lSet;
		longSet = rSet;
	}
	else
	{
		shortSet = rSet;
		longSet = lSet;
	}

	if (longSet->mineCount != shortSet->mineCount
		&& longSet->mineCount - shortSet->mineCount != longSet->setLength - shortSet->setLength)
	{
		return operCount;
	}
	else if (longSet->mineCount == shortSet->mineCount)
	{
		l1r2 = 1;
	}
	else
	{
		l1r2 = 2;
	}

	for (int i = 0; i < longSet->setLength; i++)
	{
		find = false;
		for (int j = 0; j < shortSet->setLength; j++)
		{
			if (longSet->area[i] == shortSet->area[j])
			{
				find = true;
				break;
			}
		}
		if (!find)
		{
			oper[operCount].row = longSet->area[i]->xPos;
			oper[operCount].column = longSet->area[i]->yPos;
			oper[operCount].l1r2 = l1r2;
			operCount++;
		}
	}

	return operCount;
}

int AnalyzeCount(Operation oper[30 * 16])
{
	int operCount = 0;
	int mineCount = 0;
	Area *tempSet[30 * 16];
	int tempSetLength = 0;
	bool find = false;
	bool intersect = false;
	float probability = 0.0f;
	int mystery = 0;

	for (int i = 0; i < ROWS; i++)
	{
		for (int j = 0; j < COLUMNS; j++)
		{
			intersect = false;
			for (int m = 0; m < probabilitySet[i][j].setLength; m++)
			{
				for (int n = 0; n < tempSetLength; n++)
				{
					if (probabilitySet[i][j].area[m] == tempSet[n])
					{
						intersect = true;
						break;
					}
				}
				if (intersect)
				{
					break;
				}
			}
			if (intersect)
			{
				continue;
			}

			for (m = 0; m < probabilitySet[i][j].setLength; m++)
			{
				mineCount += mineField[i][j].mineCount - GetMineCount(i, j);
				tempSet[tempSetLength] = probabilitySet[i][j].area[m];
				tempSetLength++;
			}
		}
	}

	for (i = 0; i < ROWS; i++)
	{
		for (int j = 0; j < COLUMNS; j++)
		{
			if (mineField[i][j].mineCount == -1)
			{
				mystery++;
			}
		}
	}

	probability = (float)(remainMines - mineCount) / (float)(mystery - tempSetLength);

	for (i = 0; i < ROWS; i++)
	{
		for (int j = 0; j < COLUMNS; j++)
		{
			if (mineField[i][j].mineCount == -1)
			{
				find = false;
				for (int m = 0; m < tempSetLength; m++)
				{
					if (tempSet[m] == &mineField[i][j])
					{
						find = true;
						break;
					}
				}
				if (!find)
				{
					if (remainMines == mineCount)
					{
						oper[operCount].row = i;
						oper[operCount].column = j;
						oper[operCount].l1r2 = 1;
						operCount++;
					}
					else
					{
						if (mineField[i][j].probability > probability)
						{
							mineField[i][j].probability = probability;
						}
					}
				}
			}
		}
	}
	return operCount;
}

VOID LeftButtonClick(int row, int column)
{
	if (IdentifyArea(row, column) != -1)
	{
		return;
	}

	Rectangle(hDC, mineField[row][column].left + 2, mineField[row][column].top + 2, mineField[row][column].left + 14, mineField[row][column].top + 14);
	Sleep(100);
	SendMessage(hWnd, WM_LBUTTONDOWN, 1, MAKELPARAM(mineField[row][column].left, mineField[row][column].top));
	SendMessage(hWnd, WM_LBUTTONUP, 0, MAKELPARAM(mineField[row][column].left, mineField[row][column].top));
}

VOID RightButtonClick(int row, int column)
{
	Rectangle(hDC, mineField[row][column].left + 2, mineField[row][column].top + 2, mineField[row][column].left + 14, mineField[row][column].top + 14);
	Sleep(100);
	SendMessage(hWnd, WM_RBUTTONDOWN, 2, MAKELPARAM(mineField[row][column].left, mineField[row][column].top));
	SendMessage(hWnd, WM_RBUTTONUP, 0, MAKELPARAM(mineField[row][column].left, mineField[row][column].top));
}

bool Scan()
{
	for (int i = 0; i < ROWS; i++)
	{
		for (int j = 0; j < COLUMNS; j++)
		{
			if (mineField[i][j].mineCount == -1)
			{
				mineField[i][j].mineCount = IdentifyArea(i, j);
				if (mineField[i][j].mineCount == -8)
				{
					return false;
				}
			}
		}
	}
	return true;
}

int IdentifyArea(int row, int column)
{
	COLORREF color;

	color = GetPixel(hDC, mineField[row][column].left + 8, mineField[row][column].top + 8);
	if (color == RGB(0, 0, 255))
	{
		return 1;
	}
	else if (color == RGB(0, 128, 0))
	{
		return 2;
	}
	else if (color == RGB(0, 0, 128))
	{
		return 4;
	}
	else if (color == RGB(128, 0, 0))
	{
		return 5;
	}
	else if (color == RGB(0, 128, 128))
	{
		return 6;
	}
	else if (color == RGB(0, 0, 0))
	{
		return -8;
	}

	color = GetPixel(hDC, mineField[row][column].left + 8, mineField[row][column].top + 12);
	if (color == RGB(255, 0, 0))
	{
		return 3;
	}

	color = GetPixel(hDC, mineField[row][column].left, mineField[row][column].top);
	if (color == RGB(255, 255, 255))
	{
		return -1;
	}

	color = GetPixel(hDC, mineField[row][column].left + 3, mineField[row][column].top + 3);
	if (color == RGB(0, 0, 0))
	{
		return 7;
	}
	return 0;
}

int GetOpenCount(int row, int column)
{
	int openCount = 0;

	for (int m = row - 1; m <= row + 1; m++)
	{
		for (int n = column - 1; n <= column + 1; n++)
		{
			if (m >= 0 && n >= 0 && m < ROWS && n < COLUMNS && (m != row || n != column))
			{
				if (mineField[m][n].mineCount != -1 && mineField[m][n].mineCount != -8)
				{
					openCount++;
				}
			}
		}
	}
	return openCount;
}

int GetUndefineCount(int row, int column)
{
	int undefineCount = 0;
	
	for (int m = row - 1; m <= row + 1; m++)
	{
		for (int n = column - 1; n <= column + 1; n++)
		{
			if (m >= 0 && n >= 0 && m < ROWS && n < COLUMNS && (m != row || n != column))
			{
				if (mineField[m][n].mineCount == -1)
				{
					undefineCount++;
				}
			}
		}
	}
	return undefineCount;
}

int GetMineCount(int row, int column)
{
	int mineCount = 0;
	
	for (int m = row - 1; m <= row + 1; m++)
	{
		for (int n = column - 1; n <= column + 1; n++)
		{
			if (m >= 0 && n >= 0 && m < ROWS && n < COLUMNS && (m != row || n != column))
			{
				if (mineField[m][n].mineCount == -8)
				{
					mineCount++;
				}
			}
		}
	}
	return mineCount;
}

int GetTotalCount(int row, int column)
{
	int totalCount = 0;
	
	for (int m = row - 1; m <= row + 1; m++)
	{
		for (int n = column - 1; n <= column + 1; n++)
		{
			if (m >= 0 && n >= 0 && m < ROWS && n < COLUMNS && (m != row || n != column))
			{
				
				totalCount++;
			}
		}
	}
	return totalCount;
}