#include "key_update_buffer.h"

LumatoneKeyUpdateBuffer::LumatoneKeyUpdateBuffer(TerpstraMidiDriver& driverIn, juce::ValueTree stateIn)
    : midiDriver(driverIn)
    , LumatoneState(stateIn)
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
    lock.enter();

    auto keyNum = getKeyNum(boardIndex, keyIndex);
    MappedLumatoneKey currentUpdateData = keysToUpdate[keyNum];

    auto currentKey = getKey(boardIndex, keyIndex);

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

    lock.exit();
    startTimer(updateMs);
}

void LumatoneKeyUpdateBuffer::updateKeyColour(int boardIndex, int keyIndex, juce::Colour colour)
{
    lock.enter();

    auto keyNum = getKeyNum(boardIndex, keyIndex);
    MappedLumatoneKey currentUpdateData = keysToUpdate[keyNum];
    auto currentKey = getKey(boardIndex, keyIndex);
    
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

    auto test = keysToUpdate[keyNum];
    lock.exit();
    startTimer(updateMs);
}

void LumatoneKeyUpdateBuffer::timerCallback()
{
    if (! lock.tryEnter())
        return;

    stopTimer();

    int keyNum = 0;
    for (int board = 0; board < getNumBoards(); board++)
    {
        int boardId = board + 1;
        for (int keyIndex = 0; keyIndex < getOctaveBoardSize(); keyIndex++)
        {
            auto keyUpdate = keysToUpdate[keyNum];
            if (keyUpdate.boardIndex >= 0 && keyUpdate.keyIndex >= 0)
            {
                auto currentKey = getKey(board, keyIndex);
                // if (!currentKey->configIsEqual(keyUpdate))
                    midiDriver.sendKeyFunctionParameters(boardId, keyIndex, keyUpdate.noteNumber, keyUpdate.channelNumber, keyUpdate.keyType, keyUpdate.ccFaderDefault);
                
                // if (!currentKey->colourIsEqual(keyUpdate))
                // {
                    auto colour = keyUpdate.colour;
                    if (getLumatoneVersion() >= LumatoneFirmwareVersion::VERSION_1_0_11)
                        midiDriver.sendKeyLightParameters(boardId, keyIndex, colour.getRed(), colour.getGreen(), colour.getBlue());
                    else
                        midiDriver.sendKeyLightParameters_Version_1_0_0(boardId, keyIndex, colour.getRed() * 0.5f, colour.getGreen() * 0.5f, colour.getBlue() * 0.5f);
                // }
            }

            keysToUpdate.set(keyNum, MappedLumatoneKey());
            keyNum++;
        }
    }

    lock.exit();
}
