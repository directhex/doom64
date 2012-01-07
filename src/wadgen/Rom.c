// Emacs style mode select	 -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id$
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// $Author$
// $Revision$
// $Date$
//
// DESCRIPTION: Rom handling stuff
//
//-----------------------------------------------------------------------------
#ifdef RCSID
static const char rcsid[] = "$Id$";
#endif

#include "WadGen.h"
#include "Files.h"
#include "Rom.h"
#include "Wad.h"

rom_t RomFile;

int	Rom_Verify(void);
void Rom_SwapBigEndian(int swaptype);
void Rom_GetIwad(void);

//**************************************************************
//**************************************************************
//	Rom_Open
//**************************************************************
//**************************************************************

void Rom_Open(void)
{
    int id = 0;
    
    RomFile.length = File_Read(wgenfile.filePath, &RomFile.data);
    if(RomFile.length <= 0)
        WGen_Complain("Rom_Open: Rom file length <= 0");
    
    id = Rom_Verify();
    
    if(!id)
        WGen_Complain("VGen_RomOpen: Not a valid n64 rom..");
    
    Rom_SwapBigEndian(id);

    memcpy(&RomFile.header, RomFile.data, ROMHEADERSIZE);
    strupr(RomFile.header.Name);
    
    Wad_GetIwad();
}

//**************************************************************
//**************************************************************
//	Rom_Close
//**************************************************************
//**************************************************************

void Rom_Close(void)
{
    Mem_Free((void**)&RomFile.data);
}

//**************************************************************
//**************************************************************
//	Rom_SwapBigEndian
//
//	Convert ROM into big endian format before processing iwad
//**************************************************************
//**************************************************************

void Rom_SwapBigEndian(int swaptype)
{
    uint len;
    
    // v64
    if(swaptype == 3)
    {
        short* swap;

        for(swap = (short*)RomFile.data, len = 0; len < RomFile.length / 2; len++)
            swap[len] = _SWAP16(swap[len]);
    }
    // n64
    else if(swaptype == 2)
    {
        int* swap;

        for(swap = (int*)RomFile.data, len = 0; len < RomFile.length / 4; len++)
            swap[len] = _SWAP32(swap[len]);
    }
    // z64 (do nothing)
}

//**************************************************************
//**************************************************************
//	Rom_Verify
//
//	Checks the beginning of a rom to verify if its a valid N64
//	rom and that its a Doom64 rom.
//**************************************************************
//**************************************************************

int Rom_Verify(void)
{
    if(strstr(wgenfile.filePath, ".z64"))   // big endian
        return 1;
    
    if(strstr(wgenfile.filePath, ".n64"))   // little endian
        return 2;
    
    if(strstr(wgenfile.filePath, ".v64"))   // byte swapped
        return 3;
    
    return 0;
}

//**************************************************************
//**************************************************************
//	Rom_VerifyRomCode
//**************************************************************
//**************************************************************

bool Rom_VerifyRomCode(const romLumpSpecial_t* l)
{
    if(RomFile.header.VersionID == 0x1)
    {
        if(strstr(l->countryID, "X") && RomFile.header.CountryID == 'E') return true;
    }
    if(strstr(l->countryID, &RomFile.header.CountryID)) return true;
    
    return false;
}
