/*
  ==============================================================================

    lumatone_hex_map.h
    Created: 31 Jul 2023 1:39:39am
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once
#include "./hex_field.h"
#include "../lumatone_editor_library/lumatone_geometry.h"
#include "../lumatone_editor_library/lumatone_state.h"

class LumatoneHexMap
{
public:

    struct KeyCoords
    {
        int boardIndex = 0;
        int keyIndex = 0;
    };

    struct MappedKey
    {
        KeyCoords key{ -1, -1 };
        Hex::Point point { 0, 0 };
    };

    struct MapBoard
    {
        int size = 0;
        juce::Array<MappedKey> keys;

        MapBoard(int octaveBoardSize = 0)
        {
            size = octaveBoardSize;
            for (int i = 0; i < octaveBoardSize; i++)
                keys.add(MappedKey());
        }
    };

public:
    LumatoneHexMap(LumatoneState stateIn, 
                   Hex::Point originPoint = Hex::Point(0, 0), 
                   int originBoardIndex = 0, 
                   int originKeyIndex = 0);

    KeyCoords hexToKeyCoords(Hex::Point point) const;
   
    Hex::Point keyCoordsToHex(int boardIndex, int keyIndex) const;

private:

    juce::HashMap<juce::String, MappedKey> map;

    void renderMap();

    Hex::Point addBoardIndex(Hex::Point point, int numIndexes);

private:

    LumatoneState state;

    LumatoneGeometry lumatoneGeometry;

    juce::Array<MapBoard> boards;

    Hex::Point originPoint;
    int originBoardIndex = 0;
    int originKeyIndex = 0;
};