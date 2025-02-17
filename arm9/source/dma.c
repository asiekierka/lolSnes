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

#include <nds.h>

#include "memory.h"


u8 DMA_Chans[8*16];
u8 DMA_HDMAFlag;

u8 HDMA_Pause[8];


u8 DMA_Read8(u32 addr)
{
	asm("stmdb sp!, {r12}");
	
	register u8 ret = (addr > 0x7F) ? 0 : DMA_Chans[addr];
	
	asm("ldmia sp!, {r12}");
	return ret;
}

u16 DMA_Read16(u32 addr)
{
	asm("stmdb sp!, {r12}");
	
	u16 ret = (addr > 0x7F) ? 0 : (DMA_Chans[addr] | (DMA_Chans[addr+1] << 8));

	asm("ldmia sp!, {r12}");
	return ret;
}

void DMA_Write8(u32 addr, u8 val)
{
	asm("stmdb sp!, {r12}");
	
	if (addr < 0x80)
		DMA_Chans[addr] = val;
	
	asm("ldmia sp!, {r12}");
}

void DMA_Write16(u32 addr, u16 val)
{
	asm("stmdb sp!, {r12}");
	
	if (addr < 0x80)
	{
		DMA_Chans[addr] = val & 0xFF;
		DMA_Chans[addr + 1] = val >> 8;
	}
	
	asm("ldmia sp!, {r12}");
}

void DMA_Enable(u8 flag)
{
	int c;
	for (c = 0; c < 8; c++)
	{
		if (!(flag & (1 << c)))
			continue;
		
		u8* chan = &DMA_Chans[c << 4];
		u8 params = chan[0];
		
		u16 maddrinc;
		switch (params & 0x18)
		{
			case 0x00: maddrinc = 1; break;
			case 0x10: maddrinc = -1; break;
			default: maddrinc = 0; break;
		}
		
		u8 paddrinc = params & 0x07;
		
		u8 ppuaddr = chan[1];
		u16 memaddr = chan[2] | (chan[3] << 8);
		u32 membank = chan[4] << 16;
		u16 bytecount = chan[5] | (chan[6] << 8);
		
		//printf("DMA%d %d %06X %s 21%02X | m:%d p:%d\n", c, bytecount, memaddr|membank, (params&0x80)?"<-":"->", ppuaddr, maddrinc, paddrinc);
		
		if (params & 0x80)
		{
			for (;;)
			{
				switch (paddrinc)
				{
					case 0:
						Mem_Write8(membank|memaddr, PPU_Read8(ppuaddr));
						memaddr += maddrinc; bytecount--;
						break;
					case 1:
						Mem_Write8(membank|memaddr, PPU_Read8(ppuaddr));
						memaddr += maddrinc; bytecount--; if (!bytecount) break;
						Mem_Write8(membank|memaddr, PPU_Read8(ppuaddr+1));
						memaddr += maddrinc; bytecount--;
						break;
					case 2:
					case 6:
						Mem_Write8(membank|memaddr, PPU_Read8(ppuaddr));
						memaddr += maddrinc; bytecount--; if (!bytecount) break;
						Mem_Write8(membank|memaddr, PPU_Read8(ppuaddr));
						memaddr += maddrinc; bytecount--;
						break;
					case 3:
					case 7:
						Mem_Write8(membank|memaddr, PPU_Read8(ppuaddr));
						memaddr += maddrinc; bytecount--; if (!bytecount) break;
						Mem_Write8(membank|memaddr, PPU_Read8(ppuaddr));
						memaddr += maddrinc; bytecount--; if (!bytecount) break;
						Mem_Write8(membank|memaddr, PPU_Read8(ppuaddr+1));
						memaddr += maddrinc; bytecount--; if (!bytecount) break;
						Mem_Write8(membank|memaddr, PPU_Read8(ppuaddr+1));
						memaddr += maddrinc; bytecount--;
						break;
					case 4:
						Mem_Write8(membank|memaddr, PPU_Read8(ppuaddr));
						memaddr += maddrinc; bytecount--; if (!bytecount) break;
						Mem_Write8(membank|memaddr, PPU_Read8(ppuaddr+1));
						memaddr += maddrinc; bytecount--; if (!bytecount) break;
						Mem_Write8(membank|memaddr, PPU_Read8(ppuaddr+2));
						memaddr += maddrinc; bytecount--; if (!bytecount) break;
						Mem_Write8(membank|memaddr, PPU_Read8(ppuaddr+3));
						memaddr += maddrinc; bytecount--;
						break;
					case 5:
						Mem_Write8(membank|memaddr, PPU_Read8(ppuaddr));
						memaddr += maddrinc; bytecount--; if (!bytecount) break;
						Mem_Write8(membank|memaddr, PPU_Read8(ppuaddr+1));
						memaddr += maddrinc; bytecount--; if (!bytecount) break;
						Mem_Write8(membank|memaddr, PPU_Read8(ppuaddr));
						memaddr += maddrinc; bytecount--; if (!bytecount) break;
						Mem_Write8(membank|memaddr, PPU_Read8(ppuaddr+1));
						memaddr += maddrinc; bytecount--;
						break;
				}
				
				if (!bytecount) break;
			}
		}
		else
		{
			for (;;)
			{
				switch (paddrinc)
				{
					case 0:
						PPU_Write8(ppuaddr, Mem_Read8(membank|memaddr));
						memaddr += maddrinc; bytecount--;
						break;
					case 1:
						PPU_Write8(ppuaddr, Mem_Read8(membank|memaddr));
						memaddr += maddrinc; bytecount--; if (!bytecount) break;
						PPU_Write8(ppuaddr+1, Mem_Read8(membank|memaddr));
						memaddr += maddrinc; bytecount--;
						break;
					case 2:
					case 6:
						PPU_Write8(ppuaddr, Mem_Read8(membank|memaddr));
						memaddr += maddrinc; bytecount--; if (!bytecount) break;
						PPU_Write8(ppuaddr, Mem_Read8(membank|memaddr));
						memaddr += maddrinc; bytecount--;
						break;
					case 3:
					case 7:
						PPU_Write8(ppuaddr, Mem_Read8(membank|memaddr));
						memaddr += maddrinc; bytecount--; if (!bytecount) break;
						PPU_Write8(ppuaddr, Mem_Read8(membank|memaddr));
						memaddr += maddrinc; bytecount--; if (!bytecount) break;
						PPU_Write8(ppuaddr+1, Mem_Read8(membank|memaddr));
						memaddr += maddrinc; bytecount--; if (!bytecount) break;
						PPU_Write8(ppuaddr+1, Mem_Read8(membank|memaddr));
						memaddr += maddrinc; bytecount--;
						break;
					case 4:
						PPU_Write8(ppuaddr, Mem_Read8(membank|memaddr));
						memaddr += maddrinc; bytecount--; if (!bytecount) break;
						PPU_Write8(ppuaddr+1, Mem_Read8(membank|memaddr));
						memaddr += maddrinc; bytecount--; if (!bytecount) break;
						PPU_Write8(ppuaddr+2, Mem_Read8(membank|memaddr));
						memaddr += maddrinc; bytecount--; if (!bytecount) break;
						PPU_Write8(ppuaddr+3, Mem_Read8(membank|memaddr));
						memaddr += maddrinc; bytecount--;
						break;
					case 5:
						PPU_Write8(ppuaddr, Mem_Read8(membank|memaddr));
						memaddr += maddrinc; bytecount--; if (!bytecount) break;
						PPU_Write8(ppuaddr+1, Mem_Read8(membank|memaddr));
						memaddr += maddrinc; bytecount--; if (!bytecount) break;
						PPU_Write8(ppuaddr, Mem_Read8(membank|memaddr));
						memaddr += maddrinc; bytecount--; if (!bytecount) break;
						PPU_Write8(ppuaddr+1, Mem_Read8(membank|memaddr));
						memaddr += maddrinc; bytecount--;
						break;
				}
				
				if (!bytecount) break;
			}
		}
		
		chan[2] = memaddr & 0xFF;
		chan[3] = memaddr >> 8;
		
		chan[5] = 0;
		chan[6] = 0;
	}
}

void DMA_ReloadHDMA()
{
	asm("stmdb sp!, {r12}");
	
	register u8 flag = DMA_HDMAFlag;
	if (flag)
	{
		int c;
		for (c = 0; c < 8; c++)
		{
			if (!(flag & (1 << c)))
				continue;
			
			u8* chan = &DMA_Chans[c << 4];
			
			// reload table address
			u16 tableaddr = chan[2] | (chan[3] << 8);
			u32 tablebank = chan[4] << 16;
			
			// load first repeatflag
			chan[10] = Mem_Read8(tablebank|tableaddr);
			tableaddr++;

			if (chan[0] & 0x40)
			{
				u16 memaddr = Mem_Read16(tableaddr|tablebank);
				chan[5] = memaddr & 0xFF;
				chan[6] = memaddr >> 8;
				
				tableaddr += 2;
			}
			
			chan[8] = tableaddr & 0xFF;
			chan[9] = tableaddr >> 8;
			
			HDMA_Pause[c] = 0;
		}
	}
	
	asm("ldmia sp!, {r12}");
}

extern u16 PPU_VCount;
const u8 hdma_sizes[8] = {1, 2, 2, 4, 4, 4, 2, 4};

void DMA_DoHDMA()
{
	register u8 flag = DMA_HDMAFlag;
	if (flag && (PPU_VCount < 224))
	{
		int c;
		for (c = 0; c < 8; c++)
		{
			if (!(flag & (1 << c)))
				continue;
			
			u8* chan = &DMA_Chans[c << 4];
			
			u16 tableaddr = chan[8] | (chan[9] << 8);
			u32 tablebank = chan[4] << 16;
			
			u8 repeatflag = chan[10];
			
			if (!HDMA_Pause[c])
			{
				if (repeatflag == 0)
					continue;
				
				u8 params = chan[0];
				
				u8 paddrinc = params & 0x07;
				
				u8 ppuaddr = chan[1];
				
				u16 memaddr;
				u32 membank;
				
				if (params & 0x40)
				{
					memaddr = chan[5] | (chan[6] << 8);
					membank = chan[7] << 16;
				}
				else
				{
					memaddr = tableaddr;
					membank = tablebank;
				}
				
				if (params & 0x80)
				{
					switch (paddrinc)
					{
						case 0:
							Mem_Write8(membank|memaddr, PPU_Read8(ppuaddr));
							break;
						case 1:
							Mem_Write8(membank|memaddr, PPU_Read8(ppuaddr));
							memaddr++;
							Mem_Write8(membank|memaddr, PPU_Read8(ppuaddr+1));
							break;
						case 2:
						case 6:
							Mem_Write8(membank|memaddr, PPU_Read8(ppuaddr));
							memaddr++;
							Mem_Write8(membank|memaddr, PPU_Read8(ppuaddr));
							break;
						case 3:
						case 7:
							Mem_Write8(membank|memaddr, PPU_Read8(ppuaddr));
							memaddr++;
							Mem_Write8(membank|memaddr, PPU_Read8(ppuaddr));
							memaddr++;
							Mem_Write8(membank|memaddr, PPU_Read8(ppuaddr+1));
							memaddr++;
							Mem_Write8(membank|memaddr, PPU_Read8(ppuaddr+1));
							break;
						case 4:
							Mem_Write8(membank|memaddr, PPU_Read8(ppuaddr));
							memaddr++;
							Mem_Write8(membank|memaddr, PPU_Read8(ppuaddr+1));
							memaddr++;
							Mem_Write8(membank|memaddr, PPU_Read8(ppuaddr+2));
							memaddr++;
							Mem_Write8(membank|memaddr, PPU_Read8(ppuaddr+3));
							break;
						case 5:
							Mem_Write8(membank|memaddr, PPU_Read8(ppuaddr));
							memaddr++;
							Mem_Write8(membank|memaddr, PPU_Read8(ppuaddr+1));
							memaddr++;
							Mem_Write8(membank|memaddr, PPU_Read8(ppuaddr));
							memaddr++;
							Mem_Write8(membank|memaddr, PPU_Read8(ppuaddr+1));
							break;
					}
				}
				else
				{
					switch (paddrinc)
					{
						case 0:
							PPU_Write8(ppuaddr, Mem_Read8(membank|memaddr));
							memaddr++;
							break;
						case 1:
							PPU_Write8(ppuaddr, Mem_Read8(membank|memaddr));
							memaddr++;
							PPU_Write8(ppuaddr+1, Mem_Read8(membank|memaddr));
							break;
						case 2:
						case 6:
							PPU_Write8(ppuaddr, Mem_Read8(membank|memaddr));
							memaddr++;
							PPU_Write8(ppuaddr, Mem_Read8(membank|memaddr));
							break;
						case 3:
						case 7:
							PPU_Write8(ppuaddr, Mem_Read8(membank|memaddr));
							memaddr++;
							PPU_Write8(ppuaddr, Mem_Read8(membank|memaddr));
							memaddr++;
							PPU_Write8(ppuaddr+1, Mem_Read8(membank|memaddr));
							memaddr++;
							PPU_Write8(ppuaddr+1, Mem_Read8(membank|memaddr));
							break;
						case 4:
							PPU_Write8(ppuaddr, Mem_Read8(membank|memaddr));
							memaddr++;
							PPU_Write8(ppuaddr+1, Mem_Read8(membank|memaddr));
							memaddr++;
							PPU_Write8(ppuaddr+2, Mem_Read8(membank|memaddr));
							memaddr++;
							PPU_Write8(ppuaddr+3, Mem_Read8(membank|memaddr));
							break;
						case 5:
							PPU_Write8(ppuaddr, Mem_Read8(membank|memaddr));
							memaddr++;
							PPU_Write8(ppuaddr+1, Mem_Read8(membank|memaddr));
							memaddr++;
							PPU_Write8(ppuaddr, Mem_Read8(membank|memaddr));
							memaddr++;
							PPU_Write8(ppuaddr+1, Mem_Read8(membank|memaddr));
							break;
					}
				}
				
				if (!(params & 0x40))
					tableaddr += hdma_sizes[paddrinc];
				else
				{
					memaddr++;
					chan[5] = memaddr & 0xFF;
					chan[6] = memaddr >> 8;
				}
			}
			
			repeatflag--;
			if (!(repeatflag & 0x80))
			{
				if (!repeatflag)
				{
					chan[10] = Mem_Read8(tablebank|tableaddr);
					tableaddr++;
					
					if (chan[0] & 0x40)
					{
						u16 maddr = Mem_Read16(tableaddr|tablebank);
						chan[5] = maddr & 0xFF;
						chan[6] = maddr >> 8;
						
						tableaddr += 2;
					}
					
					HDMA_Pause[c] = 0;
				}
				else
				{
					chan[10] = repeatflag;
					HDMA_Pause[c] = 1;
				}
			}
			else
			{
				if (repeatflag == 0x80)
				{
					chan[10] = Mem_Read8(tablebank|tableaddr);
					tableaddr++;
					
					if (chan[0] & 0x40)
					{
						u16 maddr = Mem_Read16(tableaddr|tablebank);
						chan[5] = maddr & 0xFF;
						chan[6] = maddr >> 8;
						
						tableaddr += 2;
					}
				}
				else
					chan[10] = repeatflag;
			}
			
			chan[8] = tableaddr & 0xFF;
			chan[9] = tableaddr >> 8;
		}
	}
}
