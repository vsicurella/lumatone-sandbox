/*
  ==============================================================================

    lumatone_key.cpp
    Created: 6 Aug 2023 2:16:18pm
    Author:  Vincenzo

  ==============================================================================
*/

#include "lumatone_key.h"

bool LumatoneKey::configIsEqual(const LumatoneKey& compare) const
{
    return keyType == compare.keyType 
        && channelNumber == compare.channelNumber
        && noteNumber == compare.noteNumber
        && ccFaderDefault == compare.ccFaderDefault;
}

bool LumatoneKey::colourIsEqual(const LumatoneKey& compare) const
{
    return colour == compare.colour;
}
