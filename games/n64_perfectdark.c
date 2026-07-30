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
#include "../main.h"
#include "../memory.h"
#include "../mouse.h"
#include "game.h"

#define TAU 6.2831853f // 0x40C90FDB

// OFFSET addresses, requires a base address to use
#define PD_camx          0x144  // f32 vv_theta   — horizontal look angle (degrees)
#define PD_camy          0x154  // f32 vv_verta    — vertical look angle (degrees)
#define PD_speedverta    0x15C  // f32 speedverta  — angular velocity (zero to prevent overwrite)

#define PD_lookahead     0x110  // bool lookaheadcentreenabled
#define PD_hoverbike     0x1A6C // struct prop *hoverbike
#define PD_bondmovemode  0x1B0  // s32 bondmovemode — 0=walk, 3=bike, 4=grab, 5=cutscene
#define PD_MOVEMODE_BIKE 3

// STATIC addresses
// g_Vars at 0x80099FC0; g_Vars+0x2A0 = struct player *bond (Joanna)
#define PD_playerbase    0x8009A260

static uint8_t N64_PD_Status(void);
static uint8_t N64_PD_DetectPlayer(void);
static void N64_PD_Inject(void);

static const GAMEDRIVER GAMEDRIVER_INTERFACE =
{
	"Perfect Dark (USA) (Rev 1)",
	N64_PD_Status,
	N64_PD_Inject,
	1, // 1000 Hz tickrate
	0  // crosshair sway not supported
};

const GAMEDRIVER *GAME_N64_PERFECTDARK = &GAMEDRIVER_INTERFACE;

static uint32_t playerbase = 0;

//==========================================================================
// Purpose: return 1 if game is detected
//==========================================================================
static uint8_t N64_PD_Status(void)
{
	return (N64_MEM_ReadUInt(0x80000000) == 0x3C1A7000
	     && N64_MEM_ReadUInt(0x80000004) == 0x275A3500);
}

//==========================================================================
// Purpose: determines if there is a player
//==========================================================================
static uint8_t N64_PD_DetectPlayer(void)
{
	const uint32_t tempplayerbase = N64_MEM_ReadUInt(PD_playerbase);

	if (!N64WITHINMEMRANGE(tempplayerbase))
		return 0;

	const uint32_t prop = N64_MEM_ReadUInt(tempplayerbase + 0xBC); // struct prop *prop (0x00BC in struct player)
	if (prop && !N64WITHINMEMRANGE(prop))
		return 0;

	playerbase = tempplayerbase;
	return 1;
}

//==========================================================================
// Inject mouse look each tick
//==========================================================================
static void N64_PD_Inject(void)
{
	// 1. Detect player — bail early if invalid
	if (!(N64_PD_DetectPlayer()))
		return;

	// 2. Disable look-ahead (auto-centering)
	N64_MEM_WriteUInt(playerbase + PD_lookahead, 0x0);

	// 2b. Zero vertical angular velocity — prevents the game from overwriting
	//     vv_verta each frame based on controller input
	N64_MEM_WriteFloat(playerbase + PD_speedverta, 0.0f);

	// 3. Skip if mouse is idle
	if (xmouse == 0 && ymouse == 0)
		return;

	const float looksensitivity = (float)sensitivity / 40.f;

	const float fov = 0.8f; // just an arbitrary  fov value

	// 4. Horizontal camera
	const uint32_t movemode = N64_MEM_ReadUInt(playerbase + PD_bondmovemode);
	const uint32_t hoverbike = N64_MEM_ReadUInt(playerbase + PD_hoverbike);

	if (movemode == PD_MOVEMODE_BIKE && hoverbike)
	{
		const uint32_t bikeobj = N64_MEM_ReadUInt(hoverbike + 0x04);
		if (N64WITHINMEMRANGE(bikeobj))
		{
			const uint32_t yrot_addr = bikeobj + 0x5C + 0x10; // defaultobj + hov.yrot
			float yrot = N64_MEM_ReadFloat(yrot_addr);
			yrot -= (float)xmouse / 400.f * looksensitivity;
			N64_MEM_WriteFloat(yrot_addr, yrot);
		}
	}
	else
	{
		float camx = N64_MEM_ReadFloat(playerbase + PD_camx);
		camx /= 360.f;
		camx += (float)xmouse / 10.f * looksensitivity / (360.f / TAU) / (1.2f / fov);
		camx *= 360.f;
		N64_MEM_WriteFloat(playerbase + PD_camx, camx);
	}

	float camy = N64_MEM_ReadFloat(playerbase + PD_camy);
	camy -= (float)ymouse / 10.f * looksensitivity / (360.f / TAU) / (1.2f / fov) * 360.f;
	if (camy > 90.f)  camy = 90.f;
	if (camy < -90.f) camy = -90.f;
	N64_MEM_WriteFloat(playerbase + PD_camy, camy);
}
