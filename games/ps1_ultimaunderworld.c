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

#define ULTIMAUNDERWORLD_CAMY 0x1D53D6
#define ULTIMAUNDERWORLD_CAMX 0x1D53D4

static uint8_t PS1_ULTIMAUNDERWORLD_Status(void);
static void PS1_ULTIMAUNDERWORLD_Inject(void);

static const GAMEDRIVER GAMEDRIVER_INTERFACE =
{
	"Ultima Underworld: The Stygian Abyss",
	PS1_ULTIMAUNDERWORLD_Status,
	PS1_ULTIMAUNDERWORLD_Inject,
	1, // 1000 Hz tickrate
	0 // crosshair sway supported for driver
};

const GAMEDRIVER *GAME_PS1_ULTIMAUNDERWORLD = &GAMEDRIVER_INTERFACE;

static float xAccumulator = 0.f;
static float yAccumulator = 0.f;

//==========================================================================
// Purpose: return 1 if game is detected
//==========================================================================
// 53 4C 50 53 5F 30 30 37 2E 34 32 3B
// 0x9244
static uint8_t PS1_ULTIMAUNDERWORLD_Status(void)
{
	return (PS1_MEM_ReadWord(0x9244) == 0x534C5053U && // SLPS_007.42
			PS1_MEM_ReadWord(0x9248) == 0x5F303037U &&
			PS1_MEM_ReadWord(0x924C) == 0x2E34323BU);
}
//==========================================================================
// Purpose: calculate mouse look and inject into current game
//==========================================================================
static void PS1_ULTIMAUNDERWORLD_Inject(void)
{
	if(xmouse == 0 && ymouse == 0) // if mouse is idle
		return;

	uint16_t camX = PS1_MEM_ReadHalfword(ULTIMAUNDERWORLD_CAMX);
	uint16_t camY = PS1_MEM_ReadHalfword(ULTIMAUNDERWORLD_CAMY);
	float camXF = (float)camX;
	float camYF = (float)camY;

	const float looksensitivity = (float)sensitivity;

	float dx = (float)xmouse * looksensitivity;
	AccumulateAddRemainder(&camXF, &xAccumulator, xmouse, dx);

	float ym = (float)(invertpitch ? ymouse : -ymouse);
	float dy = ym * looksensitivity;
	AccumulateAddRemainder(&camYF, &yAccumulator, ym, dy);

	PS1_MEM_WriteHalfword(ULTIMAUNDERWORLD_CAMX, (uint16_t)camXF);
	PS1_MEM_WriteHalfword(ULTIMAUNDERWORLD_CAMY, (uint16_t)camYF);
}