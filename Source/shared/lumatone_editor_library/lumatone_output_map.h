/*
  ==============================================================================

    lumatone_output_map.h
    Created: 31 Jul 2023 9:28:35pm
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once
#include "./data/lumatone_layout.h"

class LumatoneOutputMap
{
    struct MidiCoord
    {
        MidiCoord(int channelNum = 0, int noteNum = -1)
            : channelNumber(channelNum)
            , noteNumber(noteNum) {}
        int channelNumber;
        int noteNumber;
    };

public:

    LumatoneOutputMap(LumatoneLayout* layout=nullptr);

    juce::Array<LumatoneKeyCoord> getKeysAssignedToNoteOn(int midiChannel, int noteNumber);

    void render(const LumatoneLayout& layout);

public:
    static void render(const LumatoneLayout& layout, juce::HashMap<juce::String, juce::Array<LumatoneKeyCoord>>& map);

private:

    juce::HashMap<juce::String, juce::Array<LumatoneKeyCoord>> midiMap;
};
