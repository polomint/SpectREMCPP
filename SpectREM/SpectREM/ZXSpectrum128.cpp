//
//  ZXSpectrum128.cpp
//  SpectREM
//
//  Created by Mike Daley on 04/09/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#include "ZXSpectrum128.hpp"

#include <iostream>
#include <fstream>

#pragma mark - Constants

const int cROM_SIZE = 16384;

#pragma mark - Constructor/Destructor

ZXSpectrum128::ZXSpectrum128() : ZXSpectrum()
{
    cout << "ZXSpectrum128::Constructor" << endl;
}

ZXSpectrum128::~ZXSpectrum128()
{
    cout << "ZXSpectrum128::Destructor" << endl;
    release();
}

#pragma mark - Initialise

void ZXSpectrum128::initialise(string romPath)
{
    cout << "ZXSpectrum128::initialise(char *rom)" << endl;
    
    machineInfo = machines[ eZXSpectrum128 ];
    
    ZXSpectrum::initialise(romPath);
    loadDefaultROM();
    
    emuROMPage = 0;
    emuRAMPage = 0;
    emuDisplayPage = 5;
    emuDisablePaging = false;
    ULAPortFFFDValue = 0;

}

void ZXSpectrum128::loadDefaultROM()
{
    string romPath = emuROMPath;
    romPath.append("/Contents/Resources/128-0.ROM");
    
    ifstream romFile0(romPath, ios::binary|ios::ate);
    romFile0.seekg(0, ios::beg);
    romFile0.read(memoryRom.data(), cROM_SIZE);
    romFile0.close();
    
    string romPath1 = emuROMPath;
    romPath1.append("/Contents/Resources/128-1.ROM");
    
    ifstream romFile1(romPath1, ios::binary|ios::ate);
    romFile1.seekg(0, ios::beg);
    romFile1.read(memoryRom.data() + cROM_SIZE, cROM_SIZE);
    romFile1.close();
}

#pragma mark - ULA

unsigned char ZXSpectrum128::coreIORead(unsigned short address)
{
    bool contended = false;
    int memoryPage = address / cMEMORY_PAGE_SIZE;
    
    if (machineInfo.hasPaging &&
        (memoryPage == 1 ||
         (memoryPage == 3 && (emuRAMPage == 1 ||
                              emuRAMPage == 3 ||
                              emuRAMPage == 5 ||
                              emuRAMPage == 7))))
    {
        contended = true;
    }
    
    ZXSpectrum::ULAApplyIOContention(address, contended);
    
    // ULA Un-owned ports
    if (address & 0x01)
    {
        // Add Kemptston joystick support. Until then return 0. Byte returned by a Kempston joystick is in the
        // format: 000FDULR. F = Fire, D = Down, U = Up, L = Left, R = Right
        // Joystick is read first as it takes priority if you read from a port that activates the keyboard as well on a
        // real machine.
        if ((address & 0xff) == 0x1f)
        {
            return 0x0;
        }
        
        // AY-3-8912 ports
        else if ((address & 0xc002) == 0xc000 && machineInfo.hasAY)
        {
            return audioAYReadData();
        }
        
        
        // Getting here means that nothing has handled that port read so based on a real Spectrum
        // return the floating bus value
        return ULAFloatingBus();
    }
    
    // The base classes virtual function deals with owned ULA ports such as the keyboard ports
    unsigned char result = ZXSpectrum::coreIORead(address);
    
    return (result & 191);
}

void ZXSpectrum128::coreIOWrite(unsigned short address, unsigned char data)
{
    bool contended = false;
    int memoryPage = address / cMEMORY_PAGE_SIZE;
    
    if (machineInfo.hasPaging &&
        (memoryPage == 1 ||
         (memoryPage == 3 && (emuRAMPage == 1 ||
                              emuRAMPage == 3 ||
                              emuRAMPage == 5 ||
                              emuRAMPage == 7))))
    {
        contended = true;
    }

    ZXSpectrum::ULAApplyIOContention(address, contended);
    
    // Port: 0xFE
    //   7   6   5   4   3   2   1   0
    // +---+---+---+---+---+-----------+
    // |   |   |   | E | M |  BORDER   |
    // +---+---+---+---+---+-----------+
    if (!(address & 0x01))
    {
        displayUpdateWithTs((z80Core.GetTStates() - emuCurrentDisplayTs) + machineInfo.borderDrawingOffset);
        audioEarBit = (data & 0x10) >> 4;
        audioMicBit = (data & 0x08) >> 3;
        displayBorderColor = data & 0x07;
    }
    
    // AY-3-8912 ports
    if(address == 0xfffd && machineInfo.hasAY)
    {
        ULAPortFFFDValue = data;
        audioAYSetRegister(data);
    }
    
    if ((address & 0xc002) == 0x8000 && machineInfo.hasAY)
    {
        audioAYWriteData(data);
    }
    
    // Memory paging port
    if ( address == 0x7ffd && emuDisablePaging != true)
    {
        // Save the last byte set, used when generating a Z80 snapshot
        ULAPortFFFDValue = data;
        
        if (emuDisplayPage != ((data & 0x08) == 0x08) ? 7 : 5)
        {
            displayUpdateWithTs((z80Core.GetTStates() - emuCurrentDisplayTs) + machineInfo.borderDrawingOffset);
        }
        
        // You should only be able to disable paging once. To enable paging again then a reset is necessary.
        if (data & 0x20 && emuDisablePaging == false)
        {
            emuDisablePaging = true;
        }
        emuROMPage = ((data & 0x10) == 0x10) ? 1 : 0;
        emuRAMPage = (data & 0x07);
        emuDisplayPage = ((data & 0x08) == 0x08) ? 7 : 5;
    }
}

#pragma mark - Memory Read/Write

void ZXSpectrum128::coreMemoryWrite(unsigned short address, unsigned char data)
{
    int memoryPage = address / cMEMORY_PAGE_SIZE;
    address &= 16383;

    if (memoryPage == 0)
    {
        return;
    }
    else if (memoryPage == 1)
    {
        memoryRam[(5 * cMEMORY_PAGE_SIZE) + address] = data;
    }
    else if (memoryPage == 2)
    {
        memoryRam[(2 * cMEMORY_PAGE_SIZE) + address] = data;
    }
    else if (memoryPage == 3)
    {
        memoryRam[(emuRAMPage * cMEMORY_PAGE_SIZE) + address] = data;
    }
    
    // Only update screen if display memory has been written too
    if (address >= 16384 && address < cBITMAP_ADDRESS + cBITMAP_SIZE + cATTR_SIZE){
        displayUpdateWithTs((z80Core.GetTStates() - emuCurrentDisplayTs) + machineInfo.paperDrawingOffset);
    }
}

unsigned char ZXSpectrum128::coreMemoryRead(unsigned short address)
{
    int page = address / cMEMORY_PAGE_SIZE;
    address &= 16383;

    if (page == 0)
    {
        return (memoryRom[(emuROMPage * cMEMORY_PAGE_SIZE) + address]);
    }
    else if (page == 1)
    {
        return (memoryRam[(5 * cMEMORY_PAGE_SIZE) + address]);
    }
    else if (page == 2)
    {
        return (memoryRam[(2 * cMEMORY_PAGE_SIZE) + address]);
    }
    else if (page == 3)
    {
        return (memoryRam[(emuRAMPage * cMEMORY_PAGE_SIZE) + address]);
    }
    
    return 0;
}

#pragma mark - Memory Contention

void ZXSpectrum128::coreMemoryContention(unsigned short address, unsigned int tStates)
{
    int memoryPage = address / cMEMORY_PAGE_SIZE;
    
    if (memoryPage == 1 ||
        (memoryPage == 3 &&
          (emuRAMPage == 1 || emuRAMPage == 3 || emuRAMPage == 5 || emuRAMPage == 7)))
    {
        z80Core.AddContentionTStates( ULAMemoryContentionTable[z80Core.GetTStates() % machineInfo.tsPerFrame] );
    }
}

#pragma mark - Release/Reset

void ZXSpectrum128::release()
{
    ZXSpectrum::release();
}

void ZXSpectrum128::resetMachine(bool hard)
{
    emuROMPage = 0;
    emuRAMPage = 0;
    emuDisplayPage = 5;
    emuDisablePaging = false;
    ULAPortFFFDValue = 0;
    ZXSpectrum::resetMachine(hard);
}




