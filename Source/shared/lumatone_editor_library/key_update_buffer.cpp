#include "key_update_buffer.h"
#include "./lumatone_midi_driver/lumatone_midi_driver.h"

LumatoneKeyUpdateBuffer::LumatoneKeyUpdateBuffer(LumatoneFirmwareDriver& driverIn, LumatoneState state)
    : firmwareDriver(driverIn)
    , LumatoneState("LumatoneKeyUpdateBuffer", state)
{
    keysToUpdate.remapTable(280);
}

LumatoneKeyUpdateBuffer::~LumatoneKeyUpdateBuffer()
{
    
}

int LumatoneKeyUpdateBuffer::getKeyNum(int boardIndex, int keyIndex) const
{
    return boardIndex * getOctaveBoardSize() + keyIndex;
}

LumatoneKeyCoord LumatoneKeyUpdateBuffer::getKeyCoord(int keyNum) const
{
    return LumatoneKeyCoord(keyNum / getOctaveBoardSize(), keyNum % getOctaveBoardSize());
}

void LumatoneKeyUpdateBuffer::sendKeyConfig(int boardId, int keyIndex, const LumatoneKey& noteDataConfig, bool signalEditorListeners)
{
    updateKeyConfig(boardId - 1, keyIndex, noteDataConfig);
}

void LumatoneKeyUpdateBuffer::sendKeyColourConfig(int boardId, int keyIndex, juce::Colour colour, bool signalEditorListeners)
{
    updateKeyColour(boardId - 1, keyIndex, colour);
}

void LumatoneKeyUpdateBuffer::updateKeyConfig(int boardIndex, int keyIndex, const LumatoneKey& config)
{
    juce::ScopedLock l(lock);
    // while (!lock.tryEnter()) {}

    auto keyNum = getKeyNum(boardIndex, keyIndex);
    MappedLumatoneKey currentUpdateData = keysToUpdate[keyNum];

    auto currentKey = getKey(boardIndex, keyIndex);
    preUpdateLayout.getBoard(boardIndex)->theKeys[keyIndex] = *currentKey;

    if (*currentKey == config)
    {
        keysToUpdate.set(keyNum, MappedLumatoneKey());
    }
    else if (currentUpdateData.boardIndex < 0 || currentUpdateData.keyIndex < 0)
    {
        keysToUpdate.set(keyNum, MappedLumatoneKey(config.withColour(currentKey->colour), boardIndex, keyIndex));
    }
    else if (!currentUpdateData.configIsEqual(config))
    {
        keysToUpdate.set(keyNum, MappedLumatoneKey(config.withColour(currentUpdateData.colour), boardIndex, keyIndex));
    }

    
    startTimer(updateMs);
}

void LumatoneKeyUpdateBuffer::updateKeyColour(int boardIndex, int keyIndex, juce::Colour colour)
{
    juce::ScopedLock l(lock);
    // while (!lock.tryEnter()) {}

    auto keyNum = getKeyNum(boardIndex, keyIndex);
    MappedLumatoneKey currentUpdateData = keysToUpdate[keyNum];
    auto currentKey = getKey(boardIndex, keyIndex);
    preUpdateLayout.getBoard(boardIndex)->theKeys[keyIndex] = *currentKey;
    
    if (currentKey->colour == colour && currentKey->configIsEqual(currentUpdateData))
    {
        keysToUpdate.set(keyNum, MappedLumatoneKey());
    }
    else if (currentUpdateData.boardIndex < 0 || currentUpdateData.keyIndex < 0)
    {
        keysToUpdate.set(keyNum, MappedLumatoneKey(currentKey->withColour(colour), boardIndex, keyIndex));
    }
    else if (currentUpdateData.colour != colour)
    {
        keysToUpdate.set(keyNum, MappedLumatoneKey(currentUpdateData.withColour(colour), boardIndex, keyIndex));
    }

    
    startTimer(updateMs);
}

void LumatoneKeyUpdateBuffer::timerCallback()
{
    stopTimer();

    if (! lock.tryEnter())
        return;

    int keyNum = 0;
    for (int board = 0; board < getNumBoards(); board++)
    {
        int boardId = board + 1;
        for (int keyIndex = 0; keyIndex < getOctaveBoardSize(); keyIndex++)
        {
            auto keyUpdate = keysToUpdate[keyNum];
            if (keyUpdate.boardIndex >= 0 && keyUpdate.keyIndex >= 0)
            {
                // auto currentKey = getKey(board, keyIndex);
                auto currentKey = preUpdateLayout.getBoard(board)->theKeys[keyIndex];
                if (!currentKey.configIsEqual(keyUpdate))
                    firmwareDriver.sendKeyFunctionParameters(boardId, keyIndex, keyUpdate.noteNumber, keyUpdate.channelNumber, keyUpdate.keyType, keyUpdate.ccFaderDefault);
                
                if (!currentKey.colourIsEqual(keyUpdate))
                {
                    auto colour = keyUpdate.colour;
                    if (getLumatoneVersion() >= LumatoneFirmware::ReleaseVersion::VERSION_1_0_11)
                        firmwareDriver.sendKeyLightParameters(boardId, keyIndex, colour.getRed(), colour.getGreen(), colour.getBlue());
                    else
                        firmwareDriver.sendKeyLightParameters_Version_1_0_0(boardId, keyIndex, colour.getRed() * 0.5f, colour.getGreen() * 0.5f, colour.getBlue() * 0.5f);
                }
            }

            keysToUpdate.set(keyNum, MappedLumatoneKey());
            keyNum++;
        }
    }

    
}
