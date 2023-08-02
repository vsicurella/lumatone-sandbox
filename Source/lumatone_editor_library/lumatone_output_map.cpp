/*
  ==============================================================================

    lumatone_output_map.cpp
    Created: 31 Jul 2023 9:28:35pm
    Author:  Vincenzo

  ==============================================================================
*/

#include "lumatone_output_map.h"

LumatoneOutputMap::LumatoneOutputMap(LumatoneState stateIn)
    : state(stateIn),
      midiMap(280)
{
    renderMap();
}

juce::Array<LumatoneKeyCoord> LumatoneOutputMap::getKeysAssignedToNoteOn(int midiChannel, int noteNumber)
{
    juce::String hash = juce::String(midiChannel) + "," + juce::String(noteNumber);
    return midiMap[hash];
}

void LumatoneOutputMap::renderMap()
{
    for (int boardIndex = 0; boardIndex < state.getNumBoards(); boardIndex++)
    {
        for (int keyIndex = 0; keyIndex < state.getOctaveBoardSize(); keyIndex++)
        {
            auto key = state.getKey(boardIndex, keyIndex);
            
            juce::String midiHash = juce::String(key->channelNumber) + "," + juce::String(key->noteNumber);
            auto mappedKeys = midiMap[midiHash];
            mappedKeys.add(LumatoneKeyCoord(boardIndex, keyIndex));
            midiMap.set(midiHash, mappedKeys);
        }
    }
}


