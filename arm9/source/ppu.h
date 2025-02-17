/*
    Copyright 2013 Mega-Mario

    This file is part of lolSnes.

    lolSnes is free software: you can redistribute it and/or modify it under
    the terms of the GNU General Public License as published by the Free
    Software Foundation, either version 3 of the License, or (at your option)
    any later version.

    lolSnes is distributed in the hope that it will be useful, but WITHOUT ANY 
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
    FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along 
    with lolSnes. If not, see http://www.gnu.org/licenses/.
*/

#ifndef PPU_H
#define PPU_H

void PPU_Reset();

ITCM_CODE void PPU_SNESVBlank();

ITCM_CODE void PPU_VBlank();
ITCM_CODE void PPU_HBlank();

u8 PPU_Read8(u32 addr);
void PPU_Write8(u32 addr, u8 val);

#endif
