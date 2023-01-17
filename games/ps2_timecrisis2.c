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
#include <stdio.h>
#include <math.h>
#include "../main.h"
#include "../memory.h"
#include "../mouse.h"
#include "game.h"

#define TC2_cursorx 0x3AD240
#define TC2_cursory 0x3AD244

static uint8_t PS2_TC2_Status(void);
static void PS2_TC2_Inject(void);

static const GAMEDRIVER GAMEDRIVER_INTERFACE =
{
	"PS2 Time Crisis II",
	PS2_TC2_Status,
	PS2_TC2_Inject,
	1, // 1000 Hz tickrate
	0 // crosshair sway not supported for driver
};

const GAMEDRIVER *GAME_PS2_TIMECRISIS2 = &GAMEDRIVER_INTERFACE;

static float xAccumulator = 0.;
static float yAccumulator = 0.;

//==========================================================================
// Purpose: return 1 if game is detected
//==========================================================================
static uint8_t PS2_TC2_Status(void)
{
	return (PS2_MEM_ReadWord(0x00093390) == 0x534C5553U && PS2_MEM_ReadWord(0x00093394) == 0x5F323032U &&
			PS2_MEM_ReadWord(0x00093398) == 0x2E31393BU);
}
//==========================================================================
// Purpose: calculate mouse look and inject into current game
//==========================================================================
static void PS2_TC2_Inject(void)
{
	if(xmouse == 0 && ymouse == 0) // if mouse is idle
		return;

	const float looksensitivity = (float)sensitivity / 40.f;

	float cursorXInt = PS2_MEM_ReadUInt(TC2_cursorx);
	float cursorYInt = PS2_MEM_ReadUInt(TC2_cursory);
	float cursorX = (float)cursorXInt;
	float cursorY = (float)cursorYInt;

	float dx = (float)xmouse * looksensitivity;
	if (xmouse < 0)
		cursorX += ceil(dx);
	else
		cursorX += (uint16_t)dx;

	float r = fmod(dx, 1.f);

	if (abs(r + xAccumulator) >= 1)
	{
		if (xmouse > 0)
			cursorX += 1;
		else
			cursorX -= 1;
	}
	
	xAccumulator = fmod(r + xAccumulator, 1.f);

	int ym = (invertpitch ? -ymouse : ymouse);
	float dy = (float)ym * looksensitivity;
	// if (ymouse < 0)
	if (ym < 0)
		cursorY += ceil(dy);
	else
		cursorY += (uint16_t)dy;

	r = fmod(dy, 1.f);

	if (abs(r + yAccumulator) >= 1)
	{
		if (ym > 0)
			cursorY += 1;
		else
			cursorY -= 1;
	}
	
	yAccumulator = fmod(r + yAccumulator, 1.f);

	// cursorX += (float)xmouse * looksensitivity;
	// cursorY += (float)(invertpitch ? -ymouse : ymouse) * looksensitivity;

	PS2_MEM_WriteUInt(TC2_cursorx, (uint32_t)cursorX);
	PS2_MEM_WriteUInt(TC2_cursory, (uint32_t)cursorY);
}