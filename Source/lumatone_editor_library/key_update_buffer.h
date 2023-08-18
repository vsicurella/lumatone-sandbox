#pragma once

#include "./data/lumatone_layout.h"
#include "./data/lumatone_state.h"

#include "./lumatone_midi_driver/lumatone_midi_driver.h"

class LumatoneKeyUpdateBuffer : public LumatoneState,
                                private juce::Timer
{
public:

    LumatoneKeyUpdateBuffer(TerpstraMidiDriver& midiDriver, juce::ValueTree lumatoneState);
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

    TerpstraMidiDriver& midiDriver;

    juce::HashMap<int, MappedLumatoneKey> keysToUpdate;

    int updateMs = 200;
};
