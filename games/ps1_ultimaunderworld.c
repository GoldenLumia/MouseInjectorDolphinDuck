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
// All Offsets (except base camera values) and PS1_ULTIMAUNDERWORLD_Inject were contributed by s-ilent on GitHub, thank you!
#include <stdint.h>
#include "../main.h"
#include "../memory.h"
#include "../mouse.h"
#include "game.h"

#define ULTIMAUNDERWORLD_CAMY 0x1D53D6
#define ULTIMAUNDERWORLD_CAMX 0x1D53D4
// Game State Management
#define ULTIMAUNDERWORLD_GAME_STATE 			  0x001FFF88 // Master state variable
#define ULTIMAUNDERWORLD_GAME_STATE_INGAME        0x0
#define ULTIMAUNDERWORLD_GAME_STATE_CURSOR        0x8
#define ULTIMAUNDERWORLD_GAME_STATE_MAP           0x10
#define ULTIMAUNDERWORLD_GAME_STATE_DIALOG_INV    0x20
#define ULTIMAUNDERWORLD_GAME_STATE_CHAR_SHEET    0x40
#define ULTIMAUNDERWORLD_GAME_STATE_SPELL_MENU    0x80
#define ULTIMAUNDERWORLD_GAME_STATE_START_MENU    0x100
// Addresses for cursor/search mode
// #define UU_IS_CURSOR_MODE_ACTIVE 0x0C9DE0 // This is 5 when cursor mode is on
#define ULTIMAUNDERWORLD_CURSOR_X 0x0C9DDC             // Cursor X position (word), range 4 to 312
#define ULTIMAUNDERWORLD_CURSOR_Y 0x0C9DE0             // Cursor Y position (word), range 16 to 230
#define ULTIMAUNDERWORLD_MAP_CURSOR_X 0x0
#define ULTIMAUNDERWORLD_MAP_CURSOR_Y 0x01ffc44

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
    if (xmouse == 0 && ymouse == 0)
        return;

    uint32_t gameState = PS1_MEM_ReadUInt(ULTIMAUNDERWORLD_GAME_STATE);


    switch (gameState)
    {
    case ULTIMAUNDERWORLD_GAME_STATE_INGAME:
    {
        uint16_t camX = PS1_MEM_ReadHalfword(ULTIMAUNDERWORLD_CAMX);
        int16_t camY = PS1_MEM_ReadInt16(ULTIMAUNDERWORLD_CAMY);

        float camXF = (float)camX;
        float camYF = (float)camY;

        const float looksensitivity = (float)sensitivity;

        float dx = (float)xmouse * looksensitivity;
        AccumulateAddRemainder(&camXF, &xAccumulator, xmouse, dx);

        float ym = (float)(invertpitch ? ymouse : -ymouse);
        float dy = ym * looksensitivity;
        AccumulateAddRemainder(&camYF, &yAccumulator, ym, dy);

        camYF = ClampFloat(camYF, -16384.f, 16384.f);

        PS1_MEM_WriteHalfword(ULTIMAUNDERWORLD_CAMX, (uint16_t)camXF);
        PS1_MEM_WriteInt16(ULTIMAUNDERWORLD_CAMY, (int16_t)camYF);
        break;
    }

    case ULTIMAUNDERWORLD_GAME_STATE_CURSOR:
    {
        uint32_t cursorX = PS1_MEM_ReadUInt(ULTIMAUNDERWORLD_CURSOR_X);
        uint32_t cursorY = PS1_MEM_ReadUInt(ULTIMAUNDERWORLD_CURSOR_Y);

        float cursorXF = (float)cursorX;
        float cursorYF = (float)cursorY;

        const float cursor_sensitivity = (float)sensitivity / 10.f;

        float dx = (float)xmouse * cursor_sensitivity;
        AccumulateAddRemainder(&cursorXF, &xAccumulator, xmouse, dx);

        float dy = (float)ymouse * cursor_sensitivity;
        AccumulateAddRemainder(&cursorYF, &yAccumulator, ymouse, dy);

        // Clamp cursor to the observed screen boundaries.
        cursorXF = ClampFloat(cursorXF, 4.f, 312.f);
        cursorYF = ClampFloat(cursorYF, 16.f, 230.f);

        PS1_MEM_WriteWord(ULTIMAUNDERWORLD_CURSOR_X, (uint32_t)cursorXF);
        PS1_MEM_WriteWord(ULTIMAUNDERWORLD_CURSOR_Y, (uint32_t)cursorYF);
        break;
    }

    case ULTIMAUNDERWORLD_GAME_STATE_MAP:
    {
        /*
        // TODO: Find the data type (likely word or halfword) and range for the map cursor.
        uint32_t mapCursorX = PS1_MEM_ReadWord(UU_MAP_CURSOR_X);
        uint32_t mapCursorY = PS1_MEM_ReadWord(UU_MAP_CURSOR_Y);

        float mapCursorXF = (float)mapCursorX;
        float mapCursorYF = (float)mapCursorY;

        const float map_sensitivity = (float)sensitivity / 1.f;

        mapCursorXF += (float)xmouse * map_sensitivity;
        mapCursorYF += (float)ymouse * map_sensitivity;

        // TODO: Find the min/max values for the map cursor and update the clamp.
        // mapCursorXF = ClampFloat(mapCursorXF, MAP_X_MIN, MAP_X_MAX);
        // mapCursorYF = ClampFloat(mapCursorYF, MAP_Y_MIN, MAP_Y_MAX);

        PS1_MEM_WriteWord(UU_MAP_CURSOR_X, (uint32_t)mapCursorXF);
        PS1_MEM_WriteWord(UU_MAP_CURSOR_Y, (uint32_t)mapCursorYF);
        break;
        */
        return;
    }

    // For all menus controlled by the D-pad, we do nothing.
    case ULTIMAUNDERWORLD_GAME_STATE_DIALOG_INV:
    case ULTIMAUNDERWORLD_GAME_STATE_CHAR_SHEET:
    case ULTIMAUNDERWORLD_GAME_STATE_SPELL_MENU:
    case ULTIMAUNDERWORLD_GAME_STATE_START_MENU:
        return;

    default:
        // If we're in an unknown state, it's safest to do nothing.
        return;
    }
}