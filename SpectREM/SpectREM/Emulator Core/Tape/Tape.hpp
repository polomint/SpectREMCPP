//
//  Tape1.hpp
//  SpectREM
//
//  Created by Mike Daley on 11/09/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#ifndef Tape_hpp
#define Tape_hpp

#include <vector>
#include <iostream>
#include <fstream>

using namespace std;

#pragma mark - TypeDefs


typedef void (*TapeStatusCallback)(int blockIndex, int bytes);


#pragma mark - Tape Block


class TapeBlock
{
public:
    virtual ~TapeBlock();
    
public:
    virtual unsigned char   getFlag();
    virtual unsigned char   getDataType();
    virtual unsigned short  getDataLength();
    virtual unsigned char   getChecksum();
    virtual unsigned short  getAutoStartLine();
    virtual unsigned short  getStartAddress();
    virtual string          getBlockName() = 0;
    virtual string          getFilename();
    
public:
    unsigned short          blockLength = 0;
    unsigned char           *blockData = nullptr;
    int                     blockType = 0;
    int                     currentByte = 0;
};


#pragma mark - Tape Program Header Block


class ProgramHeader : public TapeBlock
{
public:
    virtual unsigned short  getAutoStartLine();
    virtual unsigned short  getProgramLength();
    virtual unsigned short  getDataLength();
    virtual unsigned char   getChecksum();
    virtual string          getBlockName();
};


#pragma mark - Tape Numeric Data Header Block


class NumericDataHeader : public TapeBlock
{
public:
    virtual string          getBlockName();
};


#pragma mark - Tape Alphanumeric Data Header Block


class AlphanumericDataHeader : public TapeBlock
{
public:
    virtual string          getBlockName();
};


#pragma mark - Tape Byte Header Block Block


class ByteHeader : public TapeBlock
{
public:
    unsigned short          getStartAddress();
    virtual unsigned char   getChecksum();
    virtual string          getBlockName();
};


#pragma mark - Tape Data Block Block


class DataBlock : public TapeBlock
{
public:
    unsigned char           *getDataBlock();
    virtual unsigned char   getDataType();
    virtual unsigned char   getChecksum();
    virtual string          getBlockName();
};


#pragma mark - Main Tape Processing Class


class Tape
{
    // TAPE block types
    enum
    {
        ePROGRAM_HEADER = 0,
        eNUMERIC_DATA_HEADER,
        eALPHANUMERIC_DATA_HEADER,
        eBYTE_HEADER,
        eDATA_BLOCK,
        eFRAGMENTED_DATA_BLOCK,
        eUNKNOWN_BLOCK = 99
    };
    
    // TAP Processing states
    enum
    {
        eNO_TAPE = 0,
        eHEADER_PILOT,
        eSYNC1,
        eSYNC2,
        eDATA_PILOT,
        eBLOCK_PAUSE,
        eDATA_STREAM,
        eHEADER_DATA_STREAM,
        eDATA_BIT
    };

public:
    Tape(TapeStatusCallback callback);
    virtual ~Tape();
    
public:
    // Load a TAP file
    bool                    loadWithPath(const char *);
    
    // Loads/Saves the block controlled by performing a ROM load or save
    void                    loadBlock(void *m);
    void                    saveBlock(void *m);
    
    // Updates the tape to generate the tape output. Tstates passed in should be the tStates used in each opcode executed
    void                    updateWithTs(int tStates);
    
    // Functions used to control the state of the currently loaded tape
    void                    startPlaying();
    void                    stopPlaying();
    void                    rewindTape();
    void                    rewindBlock();
    void                    eject();
    
    // Functions used to get details of the loaded tape that can then be used in a UI to display those details
    void                    updateStatus(); // Called when the internal status of the current tape changes
    unsigned long           numberOfTapeBlocks();
    void                    setSelectedBlock(int blockIndex);
    
    // Returns a vector that contains the current tape data ready to write to disk
    vector<unsigned char>   getTapeData();
    
private:
    void                    reset(bool clearBlocks);
    bool                    processData(unsigned char *fileBytes, long size);
    void                    generateHeaderPilotWithTs(int tStates);
    void                    generateSync1WithTs(int tStates);
    void                    generateSync2WithTs(int tStates);
    void                    generateDataPilotWithTs(int tStates);
    void                    tapeGenerateDataStreamWithTs(int tStates);
    void                    generateHeaderDataStreamWithTs(int tStates);
    void                    generateDataBitWithTs(int tStates);
    void                    tapeBlockPauseWithTs(int tStates);
    
public:
    bool                    loaded = false;
    bool                    playing = false;
    int                     currentBytePtr = 0;
    int                     currentBlockIndex = 0;
    bool                    newBlock = false;
    vector<TapeBlock *>     blocks;
    int                     inputBit = 0;
    
private:
    int                     pilotPulseTStates = 0;          // How many Ts have passed since the start of the pilot pulses
    int                     pilotPulses = 0;                // How many pilot pulses have been generated
    int                     syncPulseTStates = 0;           // Sync pulse tStates
    int                     dataPulseTStates = 0;           // How many Ts have passed since the start of the data pulse
    bool                    flipTapeBit = 0;                // Should the tape bit be flipped
    int                     processingState = 0;            // Current processing state e.g. generating pilot, streaming data
    int                     nextProcessingState = 0;        // Next processing state to be used
    int                     currentDataBit = 0;             // Which bit of the current byte in the data stream is being processed
    int                     blockPauseTStates = 0;          // How many tStates have passed since starting the pause between data blocks
    int                     dataBitTStates = 0;             // How many tStates to pause when processing data bit pulses
    int                     dataPulseCount = 0;             // How many pulses have been generated for the current data bit;
    TapeBlock               *tapeCurrentBlock = nullptr;    // Current tape block
    
    // Function called whenever the status of the tape changes e.g. new block, rewind, stop etc
    TapeStatusCallback      updateStatusCallback = NULL;
};

#endif /* Tape1_hpp */
