/*
  ==============================================================================

    lumatone_board.h
    Created: 6 Aug 2023 2:16:27pm
    Author:  Vincenzo

  ==============================================================================
*/

#ifndef LUMATONE_BOARD_STATE_H
#define LUMATONE_BOARD_STATE_H

#include "../data/lumatone_editor_common.h"
#include "lumatone_key.h"

#define MAXBOARDSIZE 56

// Subset of 56 or 55) keys
class LumatoneBoard 
{
public:
    LumatoneKey        theKeys[MAXBOARDSIZE];
    int                board_idx;

    LumatoneBoard(LumatoneKeyType newKeyType = LumatoneKeyType::disabled, int numKeysIn=MAXBOARDSIZE, int boardIndex = 0);
    LumatoneBoard(const LumatoneBoard& copyBoard);

    void operator=(const LumatoneBoard& copyBoard);

    bool isEmpty() const;

    int getNumKeys() const;

    juce::Array<juce::Colour> getBoardColours() const;
    juce::Array<LumatoneKeyCoord> getKeysWithColour(const juce::Colour& c) const;

private:

    int numKeys;

};

#endif LUMATONE_BOARD_STATE_H
