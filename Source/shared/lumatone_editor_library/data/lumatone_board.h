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

namespace LumatoneBoardProperty
{
    static const juce::Identifier State = juce::Identifier("LumatoneBoardState");
    static const juce::Identifier Id = juce::Identifier("Id");
    static const juce::Identifier Size = juce::Identifier("Size");
}

class LumatoneLayout;

// Subset of 56 or 55) keys
class LumatoneBoard
{
public:
    LumatoneBoard(LumatoneKeyType newKeyType = LumatoneKeyType::disabled, int numKeysIn=MAXBOARDSIZE, int boardIndex = 0);
    LumatoneBoard(const LumatoneBoard& copyBoard);
    LumatoneBoard(juce::ValueTree stateIn);

    void operator=(const LumatoneBoard& copyBoard);

    int getBoardIndex() const { return board_idx; }
    bool isEmpty() const;

    juce::ValueTree getState() const { return state; }

    int getNumKeys() const;
    const LumatoneKey& getKey(int index) const;

    void setKey(LumatoneKey key, int index);
    void setKey(const MappedLumatoneKey& mappedKey);
    void setKeyConfig(LumatoneKey key, int index);
    void setKeyColour(juce::Colour colour, int index);

    juce::Array<juce::Colour> getBoardColours() const;
    juce::Array<LumatoneKeyCoord> getKeysWithColour(const juce::Colour& c) const;

private:
    void updateState();
    void refreshFromState();

private:
    LumatoneKey        theKeys[MAXBOARDSIZE];
    int                board_idx;

    int numKeys;

    juce::ValueTree state;

    friend class LumatoneLayout;
};

#endif LUMATONE_BOARD_STATE_H
