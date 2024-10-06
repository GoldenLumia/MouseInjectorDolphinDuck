//===========================================================
// Mouse Injector for Dolphin
//==========================================================================
// Copyright (C) 2019-2020 Carnivorous
// All rights reserved.
//
// Mouse Injector is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, visit http://www.gnu.org/licenses/gpl-2.0.html
//==========================================================================
#include <stdint.h>
#include "../main.h"
#include "../memory.h"
#include "../mouse.h"
#include "game.h"

#define ACMOA_CAMY 0x453B0
#define ACMOA_CAMX 0x1E725A
#define ACMOA_ARENA_CAMX 0x1D8C1A
#define ACMOA_ARENA_CAMX_SANITY 0x1D8BAC
#define ACMOA_ARENA_CAMX_SANITY_VALUE 0x801D91F4
#define ACMOA_IS_NOT_BUSY 0x1BA72C
#define ACMOA_IS_NOT_PAUSED 0x3E720
#define ACMOA_IS_MAP_OPEN 0x15DD6B

static uint8_t PS1_ACMOA_Status(void);
static void PS1_ACMOA_Inject(void);

static const GAMEDRIVER GAMEDRIVER_INTERFACE =
{
	"Armored Core: Master of Arena",
	PS1_ACMOA_Status,
	PS1_ACMOA_Inject,
	1, // 1000 Hz tickrate
	0 // crosshair sway supported for driver
};

const GAMEDRIVER *GAME_PS1_ARMOREDCOREMOA = &GAMEDRIVER_INTERFACE;

static float xAccumulator = 0.f;
static float yAccumulator = 0.f;

//==========================================================================
// Purpose: return 1 if game is detected
//==========================================================================
static uint8_t PS1_ACMOA_Status(void)
{
	// SLUS_010.30
	return (PS1_MEM_ReadWord(0x928C) == 0x534C5553U &&
			PS1_MEM_ReadWord(0x9290) == 0x5F303130U &&
			PS1_MEM_ReadWord(0x9294) == 0x2E33303BU);
}
//==========================================================================
// Purpose: calculate mouse look and inject into current game
//==========================================================================
static void PS1_ACMOA_Inject(void)
{
	uint8_t isArena = 0;
	if (PS1_MEM_ReadUInt(ACMOA_ARENA_CAMX_SANITY) == ACMOA_ARENA_CAMX_SANITY_VALUE)
		isArena = 1;

	if (!PS1_MEM_ReadByte(ACMOA_IS_NOT_BUSY) && !isArena)
		return;

	if (!PS1_MEM_ReadByte(ACMOA_IS_NOT_PAUSED))
		return;

	// phantsmas map_open is primarly 0 and 9, but 1 in between certain load triggers (assumption)
	// looking for specifically 9 might be a bad idea but f@#k it we 9ball
	if (PS1_MEM_ReadByte(ACMOA_IS_MAP_OPEN) == 0x9 && !isArena)
		return;

	if(xmouse == 0 && ymouse == 0) // if mouse is idle
		return;
	
	uint16_t camX;
	if (isArena)
		camX = PS1_MEM_ReadHalfword(ACMOA_ARENA_CAMX);
	else
		camX = PS1_MEM_ReadHalfword(ACMOA_CAMX);
	uint16_t camY = PS1_MEM_ReadHalfword(ACMOA_CAMY);
	float camXF = (float)camX;
	float camYF = (float)camY;

	const float looksensitivity = (float)sensitivity / 20.f;
	const float scale = 1.f;

	float dx = -(float)xmouse * looksensitivity * scale;
	AccumulateAddRemainder(&camXF, &xAccumulator, -xmouse, dx);

	float ym = (float)(invertpitch ? -ymouse : ymouse);
	float dy = ym * looksensitivity * scale;
	AccumulateAddRemainder(&camYF, &yAccumulator, ym, dy);

	if (isArena)
		PS1_MEM_WriteHalfword(ACMOA_ARENA_CAMX, (uint16_t)camXF);
	else
		PS1_MEM_WriteHalfword(ACMOA_CAMX, (uint16_t)camXF);
	PS1_MEM_WriteHalfword(ACMOA_CAMY, (uint16_t)camYF);
}