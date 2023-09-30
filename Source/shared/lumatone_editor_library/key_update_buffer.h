#pragma once

#include "./data/lumatone_state.h"

class LumatoneFirmwareDriver;

class LumatoneKeyUpdateBuffer : public LumatoneState,
                                private juce::Timer
{
public:

    LumatoneKeyUpdateBuffer(LumatoneFirmwareDriver& firmwareDriver, LumatoneState state);
    ~LumatoneKeyUpdateBuffer() override;

    void sendKeyConfig(int boardId, int keyIndex, const LumatoneKey& noteDataConfig, bool signalEditorListeners = true);
    void sendKeyColourConfig(int boardId, int keyIndex, juce::Colour colour, bool signalEditorListeners = true);

    void timerCallback() override;

private:

    int getKeyNum(int boardIndex, int keyIndex) const;
    LumatoneKeyCoord getKeyCoord(int keyNum) const;

    void updateKeyConfig(int boardIndex, int keyIndex, const LumatoneKey& config);
    void updateKeyColour(int boardIndex, int keyIndex, juce::Colour colour);

private:

    juce::CriticalSection lock;

    LumatoneFirmwareDriver& firmwareDriver;

    juce::HashMap<int, MappedLumatoneKey> keysToUpdate;

    LumatoneLayout preUpdateLayout;

    int updateMs = 300;
};
