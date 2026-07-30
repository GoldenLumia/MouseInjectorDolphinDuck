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
// EARLY RELEASE FOR TESTERS
// TO DO: Cutscene offset (are there any in AC3?), Map offset. Finding something like IsBusy in previous games is proving to be troublesome.
// Anything else is mostly extra and requires more testing.
#include <stdint.h>
#include "../main.h"
#include "../memory.h"
#include "../mouse.h"
#include "game.h"

#define PI 3.14159265f
#define TAU 6.2831853f

#define AC3_ROTY 0x4F50D0
#define AC3_ROTX 0x4F5104

#define AC3_IS_PAUSED 0x6FE614

// (commentfromac2aa) Needs more testing, other offsets to test if this fails:
// (commentfromac2aa) 0x459CD4 (this is probably more closely related to the
// (commentfromac2aa) text box or similar since it flips at the start and end of training too)
//#define AC2AA_IS_IN_GAME_CUTSCENE 0x2FD40C

// (commentfromac2aa) UPDATE: all the other offsets flip when the AC boosts or smth, this is the only stable offset lol
//#define AC2AA_IS_MAP_DISPLAYED 0x1C14A14

//#define AC2AA_IS_NOT_IN_MENU 0x1D4DB38

static uint8_t PS2_AC3_Status(void);
static void PS2_AC3_Inject(void);

static const GAMEDRIVER GAMEDRIVER_INTERFACE =
{
    "Armored Core 3",
    PS2_AC3_Status,
    PS2_AC3_Inject,
    1, // 1000 Hz tickrate
    0 // crosshair sway not supported for driver
};

const GAMEDRIVER *GAME_PS2_ARMOREDCORE3 = &GAMEDRIVER_INTERFACE;

//==========================================================================
// Purpose: return 1 if game is detected
//==========================================================================
static uint8_t PS2_AC3_Status(void)
{
    // SLUS_204.35
    return (PS2_MEM_ReadWord(0x00093390) == 0x534C5553U &&
            PS2_MEM_ReadWord(0x00093394) == 0x5F323034U &&
            PS2_MEM_ReadWord(0x00093398) == 0x2E33353BU);
}
//==========================================================================
// Purpose: calculate mouse look and inject into current game
//==========================================================================
static void PS2_AC3_Inject(void)
{
    if(xmouse == 0 && ymouse == 0) // if mouse is idle
        return;

    float looksensitivity = (float)sensitivity / 40.f;
    float scale = 400.f;

    //if (!PS2_MEM_ReadUInt(AC3_IS_NOT_IN_MENU))
    //    return;

    // paused
    if (PS2_MEM_ReadWord(AC3_IS_PAUSED))
        return;

    //if (PS2_MEM_ReadWord(AC3_IS_IN_GAME_CUTSCENE))
    //    return;

    //if (PS2_MEM_ReadWord(AC3_IS_MAP_DISPLAYED))
    //    return;

    float rotX = PS2_MEM_ReadFloat(AC3_ROTX);
    float rotY = PS2_MEM_ReadFloat(AC3_ROTY);

    rotX += (float)xmouse * looksensitivity / scale;
    rotY -= (float)(invertpitch ? -ymouse : ymouse) * looksensitivity / scale;

    rotY = ClampFloat(rotY, -1.17809999f, 1.17809999);

    while (rotX > PI)
        rotX -= TAU;
    while (rotX < -PI)
        rotX += TAU;

    PS2_MEM_WriteFloat(AC3_ROTX, (float)rotX);
    PS2_MEM_WriteFloat(AC3_ROTY, (float)rotY);
}