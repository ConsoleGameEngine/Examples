#include "ConsoleGameEngine.h"

#include <time.h>

#define SCREEN_WIDTH 40
#define SCREEN_HEIGHT 30

#define MINES_COUNT 200

typedef struct Cell
{
	cgeBool bMine;
	cgeBool bOpen;
	cgeBool bFlag;
	int nAround;
} Cell, *PCell;

typedef struct World
{
	Cell aryMap[SCREEN_WIDTH * SCREEN_HEIGHT];

	int nTotalMines;
	int nClosedCells;
	cgeBool bFailed;
} World, *PWorld;

cgeBool IsInsideWorld(int x, int y)
{
	return x >= 0 && y >= 0 && x < SCREEN_WIDTH && y < SCREEN_HEIGHT;
}

PCell GetCell(PWorld pWorld, int x, int y)
{
	return &pWorld->aryMap[y * SCREEN_WIDTH + x];
}

void NewGame(PWorld pWorld)
{
	srand(time(NULL));

	memset(pWorld->aryMap, 0, sizeof(pWorld->aryMap));

	pWorld->nTotalMines = MINES_COUNT;
	pWorld->nClosedCells = SCREEN_WIDTH * SCREEN_HEIGHT;
	pWorld->bFailed = CGE_FALSE;

	for (int i = 0; i < MINES_COUNT; i++)
	{
		int x = rand() % SCREEN_WIDTH;
		int y = rand() % SCREEN_HEIGHT;

		Cell* pCell = GetCell(pWorld, x, y);

		if (pCell->bMine) i--;
		else
		{
			pCell->bMine = CGE_TRUE;

			for (int dx = -1; dx <= 1; dx++)
				for (int dy = -1; dy <= 1; dy++)
				{
					int ox = x + dx;
					int oy = y + dy;

					if (IsInsideWorld(ox, oy))
						GetCell(pWorld, ox, oy)->nAround++;
				}
		}
	}
}

void OpenFields(PWorld pWorld, int x, int y)
{
	Cell* pCell = GetCell(pWorld, x, y);

	if (!IsInsideWorld(x, y) || pCell->bOpen)
		return;

	pCell->bOpen = CGE_TRUE;
	pWorld->nClosedCells--;

	if (pCell->nAround == 0)
	{
		for (int dx = -1; dx <= 1; dx++)
			for (int dy = -1; dy <= 1; dy++)
				OpenFields(pWorld, x + dx, y + dy);
	}

	if (GetCell(pWorld, x, y)->bMine)
	{
		pWorld->bFailed = CGE_TRUE;

		int length = SCREEN_WIDTH * SCREEN_HEIGHT;

		for (int i = 0; i < length; i++)
			pWorld->aryMap[i].bOpen = CGE_TRUE;
	}
}

void cgeRun()
{
	World world;

	NewGame(&world);

	while (cgeUpdate())
	{
		if (cgeGetMouse(0).bReleased)
		{
			if (world.bFailed)
				NewGame(&world);
			else
			{
				int x = cgeGetMouseX();
				int y = cgeGetMouseY();

				if (IsInsideWorld(x, y) && !GetCell(&world, x, y)->bFlag)
					OpenFields(&world, x, y);
			}
		}

		if (cgeGetMouse(1).bReleased)
		{
			int x = cgeGetMouseX();
			int y = cgeGetMouseY();
			
			if (IsInsideWorld(x, y))
			{
				Cell* pCell = GetCell(&world, x, y);
				pCell->bFlag = !pCell->bFlag;
			}
		}

		if (world.nTotalMines == world.nClosedCells)
			NewGame(MINES_COUNT);

		for (int y = 0; y < SCREEN_HEIGHT; y++)
			for (int x = 0; x < SCREEN_WIDTH; x++)
			{
				Cell* pCell = GetCell(&world, x, y);

				if (pCell->bOpen)
				{
					if (pCell->bMine)
						cgeDraw(x, y, L'X', FG_BLACK | BG_DARK_GREY);
					else if (pCell->nAround > 0)
						cgeDraw(x, y, (wchar_t)(pCell->nAround + 48), FG_WHITE | BG_GREEN);
					else
						cgeDraw(x, y, PIXEL_SOLID, FG_GREEN);
				}
				else
				{
					if (pCell->bFlag)
						cgeDraw(x, y, L'?', FG_RED | BG_DARK_GREY);
					else
						cgeDraw(x, y, PIXEL_SOLID, FG_DARK_GREY);
				}
			}

		if (world.bFailed)
		{
			cgeDrawString(
				(float)SCREEN_WIDTH * 0.23f, SCREEN_HEIGHT / 2,
				L"PRESS LMB TO PLAY AGAIN!", FG_WHITE);
		}
	}
}

int main()
{
	if (cgeConstructConsole(SCREEN_WIDTH, SCREEN_HEIGHT, 16, 16, L"Minesweeper") == RC_OK)
		cgeRun();
}
