/*
  ==============================================================================

    lumatone_hex_map.h
    Created: 31 Jul 2023 1:39:39am
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once
#include "./hex_field.h"
#include "../lumatone_geometry.h"
#include "../data/lumatone_layout.h"

class LumatoneHexMap
{
public:

    struct MappedKey
    {
        LumatoneKeyCoord key;
        Hex::Point point { 0, 0 };
    };

    struct MapBoard
    {
        int size = 0;
        MappedKey keys[MAXBOARDSIZE];

        MapBoard(int octaveBoardSize = 0)
        {
            size = octaveBoardSize;
            for (int i = 0; i < octaveBoardSize; i++)
                keys[i] = MappedKey();
        }
    };

public:
    LumatoneHexMap(std::shared_ptr<LumatoneLayout> layout, 
                   Hex::Point originPoint = Hex::Point(0, 0),
                   int originBoardIndex = 0, 
                   int originKeyIndex = 0);

    ~LumatoneHexMap();

    LumatoneKeyCoord hexToKeyCoords(Hex::Point point) const;
   
    Hex::Point keyCoordsToHex(int boardIndex, int keyIndex) const;
    Hex::Point keyCoordsToHex(const LumatoneKeyCoord& keyCoord) const;

private:

    std::unique_ptr<juce::HashMap<juce::String, MappedKey>> map;

    void renderMap();

    Hex::Point addBoardIndex(Hex::Point point, int numIndexes);

public:

    static bool testIdentityMap();

private:

    std::shared_ptr<LumatoneLayout> layout;

    LumatoneGeometry lumatoneGeometry;

    //juce::Array<MapBoard> boards;
    MapBoard boards[MAXNUMBOARDS];

    Hex::Point originPoint;
    int originBoardIndex = 0;
    int originKeyIndex = 0;
};