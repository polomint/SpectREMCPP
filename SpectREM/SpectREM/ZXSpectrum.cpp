//
//  ZXSpectrum48.cpp
//  z80test
//
//  Created by Michael Daley on 30/07/2017.
//  Copyright © 2017 Mike Daley. All rights reserved.
//

#include "ZXSpectrum.hpp"

#pragma mark - Constructor/Deconstructor

ZXSpectrum::ZXSpectrum()
{
    cout << "ZXSpectrum::Constructor" << endl;
}

ZXSpectrum::~ZXSpectrum()
{
    cout << "ZXSpectrum::Destructor" << endl;
    release();
}

#pragma mark - Initialise

void ZXSpectrum::initialise(char *romPath)
{
    cout << "ZXSpectrum::initialise(char *romPath)" << endl;
    
    z80Core.Initialise(zxSpectrumMemoryRead,
                       zxSpectrumMemoryWrite,
                       zxSpectrumIORead,
                       zxSpectrumIOWrite,
                       zxSpectrumMemoryContention,
                       zxSpectrumDebugRead,
                       zxSpectrumDebugWrite,
                       this);
    
    screenWidth = machineInfo.pxEmuBorder + machineInfo.pxHorizontalDisplay + machineInfo.pxEmuBorder;
    screenHeight = machineInfo.pxEmuBorder + machineInfo.pxVerticalDisplay + machineInfo.pxEmuBorder;
    screenBufferSize = screenHeight * screenWidth;
    
    memoryRom.resize( machineInfo.romSize );
    memoryRam.resize( machineInfo.ramSize );
    
    displayBuffer = new unsigned int[ screenBufferSize ];
    displayBufferCopy = new ScreenBufferData[ machineInfo.tsPerFrame ];

    buildScreenLineAddressTable();
    buildDisplayTstateTable();
    buildContentionTable();
    loadRomWithPath(romPath);
    reset();
    
}

void ZXSpectrum::loadRomWithPath(char *romPath)
{
    ifstream romFile(romPath, ios::binary|ios::ate);
    romFile.seekg(0, ios::beg);
    romFile.read(memoryRom.data(), memoryRom.size());
}

#pragma mark - Generate a frame

void ZXSpectrum::runFrame()
{
    int currentFrameTstates = machineInfo.tsPerFrame;
    
    while (currentFrameTstates > 0 && !paused)
    {
        int tStates = z80Core.Execute(1, machineInfo.intLength);
        currentFrameTstates -= tStates;
        
        if (z80Core.GetTStates() >= machineInfo.tsPerFrame)
        {
            z80Core.ResetTStates( machineInfo.tsPerFrame );
            z80Core.SignalInterrupt();
            updateScreenWithTstates(machineInfo.tsPerFrame - currentDisplayTstates);
            frameCounter++;
            currentFrameTstates = 0;
            resetFrame();
            checkCapsLockStatus();
        }
    }
}

#pragma mark - Memory Access

unsigned char ZXSpectrum::zxSpectrumMemoryRead(unsigned short address, void *param)
{
    return ((ZXSpectrum *) param)->coreMemoryRead(address);
}

void ZXSpectrum::zxSpectrumMemoryWrite(unsigned short address, unsigned char data, void *param)
{
    ((ZXSpectrum *) param)->coreMemoryWrite(address, data);
}

void ZXSpectrum::zxSpectrumMemoryContention(unsigned short address, unsigned int tStates, void *param)
{
    ((ZXSpectrum *) param)->coreMemoryContention(address, tStates);
}

unsigned char ZXSpectrum::zxSpectrumDebugRead(unsigned int address, void *param, void *data)
{
    return ((ZXSpectrum *) param)->coreDebugRead(address, data);
}

void ZXSpectrum::zxSpectrumDebugWrite(unsigned int address, unsigned char byte, void *param, void *data)
{
    ((ZXSpectrum *) param)->coreDebugWrite(address, byte, data);
}

unsigned char ZXSpectrum::coreMemoryRead(unsigned short address)
{
    if (address < machineInfo.romSize)
    {
        return memoryRom[address];
    }
    
    return memoryRam[address - machineInfo.romSize];
}

void ZXSpectrum::coreMemoryWrite(unsigned short address, unsigned char data)
{
    if (address < machineInfo.romSize)
    {
        return;
    }
    
    if (address >= machineInfo.romSize && address < cBITMAP_ADDRESS + cBITMAP_SIZE + cATTR_SIZE){
        updateScreenWithTstates((z80Core.GetTStates() - currentDisplayTstates) + machineInfo.paperDrawingOffset);
    }

    memoryRam[address - machineInfo.romSize] = data;
}

void ZXSpectrum::coreMemoryContention(unsigned short address, unsigned int tStates)
{
    // Nothing to see here
}

unsigned char ZXSpectrum::coreDebugRead(unsigned int address, void *data)
{
    if (address < machineInfo.romSize)
    {
        return memoryRom[address];
    }
    
    return memoryRam[address - machineInfo.romSize];
}

void ZXSpectrum::coreDebugWrite(unsigned int address, unsigned char byte, void *data)
{
    if (address < machineInfo.romSize)
    {
        memoryRom[address] = byte;
    }
    else
    {
        memoryRam[address - machineInfo.romSize] = byte;
    }
}

#pragma mark - IO Access

unsigned char ZXSpectrum::zxSpectrumIORead(unsigned short address, void *param)
{
    return ((ZXSpectrum *) param)->coreIORead(address);
}

void ZXSpectrum::zxSpectrumIOWrite(unsigned short address, unsigned char data, void *param)
{
    ((ZXSpectrum *) param)->coreIOWrite(address, data);
}

unsigned char ZXSpectrum::coreIORead(unsigned short address)
{
    // Check to see if the keyboard is being read and if so return any keys currently pressed
    unsigned char result = 0xff;
    if (address & 0xfe)
    {
        for (int i = 0; i < 8; i++)
        {
            if (!(address & (0x100 << i)))
            {
                result &= keyboardMap[i];
            }
        }
    }
    
    return result;
}

void ZXSpectrum::coreIOWrite(unsigned short address, unsigned char data)
{
    // Nothing to see here
}

#pragma mark - Reset

void ZXSpectrum::reset()
{
    z80Core.Reset();
    resetKeyboardMap();
    resetFrame();
    frameCounter = 0;
}

#pragma mark - Release

void ZXSpectrum::release()
{
    delete displayBuffer;
    delete displayBufferCopy;
}




