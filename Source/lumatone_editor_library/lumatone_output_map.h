/*
  ==============================================================================

    lumatone_output_map.h
    Created: 31 Jul 2023 9:28:35pm
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once
#include "lumatone_state.h"

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

    LumatoneOutputMap(LumatoneState stateIn);


    juce::Array<LumatoneKeyCoord> getKeysAssignedToNoteOn(int midiChannel, int noteNumber);

    void renderMap();

private:

    LumatoneState state;

    juce::HashMap<juce::String, juce::Array<LumatoneKeyCoord>> midiMap;
};
