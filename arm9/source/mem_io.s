@ -----------------------------------------------------------------------------
@ Copyright 2013 Mega-Mario
@
@ This file is part of lolSnes.
@
@ lolSnes is free software: you can redistribute it and/or modify it under
@ the terms of the GNU General Public License as published by the Free
@ Software Foundation, either version 3 of the License, or (at your option)
@ any later version.
@
@ lolSnes is distributed in the hope that it will be useful, but WITHOUT ANY 
@ WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
@ FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
@
@ You should have received a copy of the GNU General Public License along 
@ with lolSnes. If not, see http://www.gnu.org/licenses/.
@ -----------------------------------------------------------------------------

.arm
.align 4

#include "cpu.inc"

.section    .itcm, "aw", %progbits

.align 4
.global Mem_IORead8
.global Mem_IORead16
.global Mem_IOWrite8
.global Mem_IOWrite16

EXPORT(Mem_IORead8)
	and r1, r0, #0xFF00
	and r0, r0, #0xFF
	
	cmp r1, #0x2100
	beq PPU_Read8
	
	cmp r1, #0x4200
	beq Mem_GIORead8
	
	cmp r1, #0x4300
	beq DMA_Read8
	
	cmp r1, #0x4000
	subeq snesCycles, snesCycles, #0x60000
	beq Mem_JoyRead8
	
	mov r0, #0
	bx lr
	
EXPORT(Mem_IORead16)
	and r1, r0, #0xFF00
	and r0, r0, #0xFF
	
	cmp r1, #0x2100
	beq PPU_Read16
	
	cmp r1, #0x4200
	beq Mem_GIORead16
	
	cmp r1, #0x4300
	beq DMA_Read16
	
	cmp r1, #0x4000
	subeq snesCycles, snesCycles, #0xC0000
	beq Mem_JoyRead16
	
	mov r0, #0
	bx lr
	
EXPORT(Mem_IOWrite8)
	and r2, r0, #0xFF00
	and r0, r0, #0xFF
	
	cmp r2, #0x2100
	beq PPU_Write8
	
	cmp r2, #0x4300
	beq DMA_Write8
	
	cmp r2, #0x4000
	subeq snesCycles, snesCycles, #0x60000
	beq Mem_JoyWrite8
	
	cmp r2, #0x4200
	bxne lr
	cmp r0, #0x00
	bne Mem_GIOWrite8
	tst r1, #0x80
	bicne snesP, snesP, #flagI2
	orreq snesP, snesP, #flagI2	
	b Mem_GIOWrite8
	
EXPORT(Mem_IOWrite16)
	and r2, r0, #0xFF00
	and r0, r0, #0xFF
	
	cmp r2, #0x2100
	beq PPU_Write16
	
	cmp r2, #0x4300
	beq DMA_Write16
	
	cmp r2, #0x4000
	subeq snesCycles, snesCycles, #0xC0000
	beq Mem_JoyWrite16
	
	cmp r2, #0x4200
	bxne lr
	cmp r0, #0x00
	bne Mem_GIOWrite16
	tst r1, #0x80
	bicne snesP, snesP, #flagI2
	orreq snesP, snesP, #flagI2
	b Mem_GIOWrite16
