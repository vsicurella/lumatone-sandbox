/*
  ==============================================================================

    lumatone_board.cpp
    Created: 6 Aug 2023 2:16:27pm
    Author:  Vincenzo

  ==============================================================================
*/

#include "lumatone_board.h"

/*
==============================================================================
TerpstraKeys class
==============================================================================
*/

LumatoneBoard::LumatoneBoard(LumatoneKeyType newKeyType, int numKeysIn, int boardIndex)
    : numKeys(numKeysIn)
    , board_idx(boardIndex)
{
    for (int i = 0; i < numKeys; i++)
        theKeys[i] = LumatoneKey(newKeyType);

    //key_idx = 0;
}

LumatoneBoard::LumatoneBoard(const LumatoneBoard& copyBoard)
{
    operator=(copyBoard);
}

void LumatoneBoard::operator=(const LumatoneBoard& copyBoard)
{
    numKeys = copyBoard.numKeys;
    board_idx = copyBoard.board_idx;

    for (int i = 0; i < numKeys; i++)
        theKeys[i] = LumatoneKey(copyBoard.theKeys[i]);
}

bool LumatoneBoard::isEmpty() const
{
    LumatoneKey emptyKeyData = LumatoneKey();

    for (int i = 0; i < numKeys; i++) 
        if (theKeys[i] != emptyKeyData) 
            return false;

    return true;
}

int LumatoneBoard::getNumKeys() const
{
    return numKeys;
}
