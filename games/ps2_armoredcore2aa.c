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

#define PI 3.14159265f // AC2 Previous Value: 0x40490FDB
#define TAU 6.2831853f // AC2 Previous Value: 0x40C90FDB

#define AC2AA_ROTY 0x000000 // AC2 Previous Value: 0x2BB920
#define AC2AA_ROTX 0x000000 // AC2 Previous Value: 0x2BB954

#define AC2AA_IS_PAUSED 0x000000 // AC2 Previous Value: 0x2B6900
#define AC2AA_IS_PAUSED_TRUE 0x00000000 // AC2 Previous Value: 0xFF010000
#define AC2AA_IS_IN_GAME_CUTSCENE 0x000000 // AC2 Previous Value: 0x2B68EC
#define AC2AA_IS_MAP_DISPLAYED 0x0000000 // AC2 Previous Value: 0x1C7D624
#define AC2AA_IS_NOT_IN_MENU 0x000000 // AC2 Previous Value: 0x2D4D00

static uint8_t PS2_AC2AA_Status(void);
static void PS2_AC2AA_Inject(void);

static const GAMEDRIVER GAMEDRIVER_INTERFACE =
{
    "Armored Core 2",
    PS2_AC2AA_Status,
    PS2_AC2AA_Inject,
    1, // 1000 Hz tickrate
    0 // crosshair sway not supported for driver
};

const GAMEDRIVER *GAME_PS2_ARMOREDCORE2AA = &GAMEDRIVER_INTERFACE;

//==========================================================================
// Purpose: return 1 if game is detected
//==========================================================================
static uint8_t PS2_AC2AA_Status(void)
{
    // 0x00093390 == 0x534C5553U
    // 0x00093394 == 0x5F323030U
    // 0x00093398 == 0x2E31343BU
    return (PS2_MEM_ReadWord(0x00000000) == 0x00000000U &&
            PS2_MEM_ReadWord(0x00000000) == 0x00000000U &&
            PS2_MEM_ReadWord(0x00000000) == 0x00000000U);
}
//==========================================================================
// Purpose: calculate mouse look and inject into current game
//==========================================================================
static void PS2_AC2AA_Inject(void)
{
    if(xmouse == 0 && ymouse == 0) // if mouse is idle
        return;

    float looksensitivity = (float)sensitivity / 40.f;
    float scale = 400.f;

    if (!PS2_MEM_ReadUInt(AC2AA_IS_NOT_IN_MENU))
        return;

    // paused
    if (PS2_MEM_ReadWord(AC2AA_IS_PAUSED) == AC2AA_IS_PAUSED_TRUE)
        return;

    if (PS2_MEM_ReadWord(AC2AA_IS_IN_GAME_CUTSCENE))
        return;

    if (PS2_MEM_ReadUInt(AC2AA_IS_MAP_DISPLAYED))
        return;

    float rotX = PS2_MEM_ReadFloat(AC2AA_ROTX);
    float rotY = PS2_MEM_ReadFloat(AC2AA_ROTY);

    rotX += (float)xmouse * looksensitivity / scale;
    rotY -= (float)(invertpitch ? -ymouse : ymouse) * looksensitivity / scale;

    rotY = ClampFloat(rotY, -1.17809999f, 1.17809999);

    while (rotX > PI)
        rotX -= TAU;
    while (rotX < -PI)
        rotX += TAU;

    PS2_MEM_WriteFloat(AC2AA_ROTX, (float)rotX);
    PS2_MEM_WriteFloat(AC2AA_ROTY, (float)rotY);
}