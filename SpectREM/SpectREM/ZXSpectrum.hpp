//
//  ZXSpectrum.hpp
//  z80test
//
//  Created by Michael Daley on 30/07/2017.
//  Copyright © 2017 Mike Daley. All rights reserved.
//

#ifndef ZXSpectrum_hpp
#define ZXSpectrum_hpp

#include <vector>
#include <iostream>
#include <fstream>
#include <string>

#include "Z80Core.h"
#include "MachineDetails.h"

using namespace std;

class ZXSpectrum
{

public:
    static const int        cBITMAP_ADDRESS = 16384;
    static const int        cBITMAP_SIZE = 6144;
    static const int        cATTR_SIZE = 768;
    static const int        cMEMORY_PAGE_SIZE = 16384;

private:
    enum
    {
        eDisplayBorder = 1,
        eDisplayPaper,
        eDisplayRetrace
    };
    
    enum
    {
        eFloatBusTypeValuePixel = 1,
        eFloatBusTypeValueAttribute
    };
    
    // Holds details of the host platforms key codes and how they map to the spectrum keyboard matrix
    typedef struct
    {
        int                 key;
        int                 mapEntry;
        int                 mapBit;
    } KEYBOARD_ENTRY;
    
public:
    // Holds the data returned when creating a Snapshot or Z80 snapshot
    struct snap {
        int length;
        unsigned char *data;
    };
    
    // Previous frames screen buffer contents
    struct ScreenBufferData {
        unsigned char pixels;
        unsigned char attribute;
        bool changed;
    };
    
    enum
    {
        eAYREGISTER_A_FINE = 0,
        eAYREGISTER_A_COARSE,
        eAYREGISTER_B_FINE,
        eAYREGISTER_B_COARSE,
        eAYREGISTER_C_FINE,
        eAYREGISTER_C_COARSE,
        eAYREGISTER_NOISEPER,
        eAYREGISTER_ENABLE,
        eAYREGISTER_A_VOL,
        eAYREGISTER_B_VOL,
        eAYREGISTER_C_VOL,
        eAYREGISTER_E_FINE,
        eAYREGISTER_E_COARSE,
        eAYREGISTER_E_SHAPE,
        eAYREGISTER_PORT_A,
        eAYREGISTER_PORT_B,
        
        // Used to emulate the odd floating behaviour of setting an AY register > 15. The value
        // written to registers > 15 decays over time and this is the value returned when reading
        // a register > 15
        eAYREGISTER_FLOATING,
        
        eAY_MAX_REGISTERS
    };
    
    enum
    {
        eENVFLAG_HOLD = 0x01,
        eENVFLAG_ALTERNATE = 0x02,
        eENVFLAG_ATTACK = 0x04,
        eENVFLAG_CONTINUE = 0x08
    };

public:
    ZXSpectrum();
    ~ZXSpectrum();

public:
    virtual void            initialise(string romPath);
    virtual void            resetMachine(bool hard = true);
    virtual void            release();

    virtual void            loadDefaultROM();

    void                    generateFrame();
    void                    displayFrameReset();
    void                    displayUpdateWithTs(int tStates);
    void                    keyboardKeyDown(unsigned short key);
    void                    keyboardKeyUp(unsigned short key);
    void                    keyboardFlagsChanged(unsigned long flags, unsigned short key);

    void                    ULAApplyIOContention(unsigned short address, bool contended);
    unsigned char           ULAFloatingBus();
    
    bool                    snapshotZ80LoadWithPath(const char *path);
    bool                    snapshotSNALoadWithPath(const char *path);
    int                     snapshotMachineInSnapshotWithPath(const char *path);
    snap                    snapshotCreateSNA();
    snap                    snapshotCreateZ80();
    
    void                    audioAYSetRegister(unsigned char reg);
    void                    audioAYWriteData(unsigned char data);
    unsigned char           audioAYReadData();
    void                    audioAYUpdate(int audioSteps);
    void                    audioReset();
    int                     audioQueueWrite(signed short *buffer, int count);
    int                     audioQueueRead(signed short *buffer, int count);
    int                     audioQueueBufferUsed();
    void                    audioUpdateWithTs(int tStates);
    
private:
    void                    displayBuildTsTable();
    void                    displayBuildLineAddressTable();
    void                    ULABuildContentionTable();
    void                    audioBuildAYVolumesTable();
    void                    keyboardCheckCapsLockStatus();
    void                    keyboardMapReset();
    string                  snapshotHardwareTypeForVersion(int version, int hardwareType);
    void                    snapshotExtractMemoryBlock(unsigned char *fileBytes, int memAddr, int fileOffset, bool isCompressed, int unpackedLength);
    void                    displaySetup();
    void                    audioSetup(float sampleRate, float fps);
    
    
    // Core memory/IO functions
    static unsigned char    zxSpectrumMemoryRead(unsigned short address, void *param);
    static void             zxSpectrumMemoryWrite(unsigned short address, unsigned char data, void *param);
    static void             zxSpectrumMemoryContention(unsigned short address, unsigned int tStates, void *param);
    static unsigned char    zxSpectrumDebugRead(unsigned int address, void *param, void *m);
    static void             zxSpectrumDebugWrite(unsigned int address, unsigned char byte, void *param, void *data);
    static unsigned char    zxSpectrumIORead(unsigned short address, void *param);
    static void             zxSpectrumIOWrite(unsigned short address, unsigned char data, void *param);

public:
    virtual unsigned char   coreMemoryRead(unsigned short address);
    virtual void            coreMemoryWrite(unsigned short address, unsigned char data);
    virtual void            coreMemoryContention(unsigned short address, unsigned int tStates);
    virtual unsigned char   coreDebugRead(unsigned int address, void *data);
    virtual void            coreDebugWrite(unsigned int address, unsigned char byte, void *data);
    virtual unsigned char   coreIORead(unsigned short address);
    virtual void            coreIOWrite(unsigned short address, unsigned char data);

    CZ80Core                z80Core;
    vector<char>            memoryRom;
    vector<char>            memoryRam;
    unsigned char           keyboardMap[8];
    static KEYBOARD_ENTRY   keyboardLookup[];
    int                     keyboardCapsLockFrames;
    
public:
    MachineInfo             machineInfo;

    // Display
    unsigned int            *displayBuffer;
    ScreenBufferData        *displayBufferCopy;
    unsigned int            displayBufferIndex;
    int                     screenWidth;
    int                     screenHeight;
    int                     screenBufferSize;
    int                     displayTstateTable[312][228];
    int                     displayLineAddrTable[192];
    static unsigned int     displayPalette[];
    int                     displayBorderColor;

    // Emulation
    int                     emuCurrentDisplayTs;
    int                     emuFrameCounter;
    bool                    emuPaused;
    int                     emuRAMPage;
    int                     emuROMPage;
    int                     emuDisplayPage;
    bool                    emuDisablePaging;
    string                  emuROMPath;
    
    // Audio
    int                     audioEarBit;
    int                     audioMicBit;
    short                   *audioBuffer;
    int                     audioBufferSize;
    int                     audioBufferIndex;
    int                     audioTsCounter;
    float                   audioTsStepCounter;

    float                   audioBeeperTsStep;
    float                   audioBeeperLeft;
    float                   audioBeeperRight;

    int                     audioAYChannelOutput[3];
    unsigned int            audioAYChannelCount[3];
    unsigned short          audioAYVolumes[16];
    unsigned int            audioAYrandom;
    unsigned int            audioAYOutput;
    unsigned int            audioAYNoiseCount;
    unsigned int            audioATaudioAYEnvelopeCount;
    int                     audioAYaudioAYEnvelopeStep;
    unsigned char           audioAYRegisters[ eAY_MAX_REGISTERS ];
    unsigned char           audioAYCurrentRegister;
    unsigned char           audioAYFloatingRegister;
    bool                    audioAYaudioAYaudioAYEnvelopeHolding;
    bool                    audioAYaudioAYEnvelopeHold;
    bool                    audioAYaudioAYEnvelopeAlt;
    bool                    audioAYEnvelope;
    unsigned int            audioAYAttackEndVol;
    float                   audioAYTsStep;
    int                     audioAYTs;
    
    short                   *audioQueueBuffer;
    int                     audioQueueBufferRead;
    int                     audioQueueBufferWritten;
    int                     audioQueueBufferCapacity;
    
    // Keyboard
    bool                    keyboardCapsLockPressed;
    
    // ULA
    unsigned int            ULAMemoryContentionTable[80000];
    unsigned int            ULAIOContentionTable[80000];
    static unsigned int     ULAConentionValues[];
    int                     ULAPortFFFDValue;

    // Floating bus
    static unsigned int     ULAFloatingBusValues[];
    

};

#endif /* ZXSpectrum_hpp */






