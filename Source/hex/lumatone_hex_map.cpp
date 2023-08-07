/*
  ==============================================================================

    lumatone_hex_map.cpp
    Created: 31 Jul 2023 1:39:39am
    Author:  Vincenzo

  ==============================================================================
*/

#include "lumatone_hex_map.h"

LumatoneHexMap::LumatoneHexMap(LumatoneLayout layoutIn, Hex::Point originPointIn, int originBoardIndexIn, int originKeyIndexIn)
    : layout(layoutIn)
    , originPoint(originPointIn)
    , originBoardIndex(originBoardIndexIn)
    , originKeyIndex(originKeyIndexIn)
    , map(juce::HashMap<juce::String, MappedKey>(280))
{
    renderMap();
}

LumatoneKeyCoord LumatoneHexMap::hexToKeyCoords(Hex::Point point) const
{
    auto key = point.toString();
    return map[key].key;
}

Hex::Point LumatoneHexMap::keyCoordsToHex(int boardIndex, int keyIndex) const
{
    return boards[boardIndex].keys[keyIndex].point;
}

Hex::Point LumatoneHexMap::addBoardIndex(Hex::Point point, int numIndexes)
{
    return Hex::Point(point.q + numIndexes * 7, point.r - numIndexes * 5);
}

void LumatoneHexMap::renderMap()
{
    map.clear();
    //boards.clear();

    for (int i = 0; i < layout.getNumBoards(); i++)
        //boards.add(MapBoard(layout.getOctaveBoardSize()));
        boards[i] = MapBoard(layout.getOctaveBoardSize());

    // use lumatoneGeometry.getVerticalOriginLineIndexForRow to find zeroth key from origin

    int originRowIndex = 0;
    int lineToZeroDiff = 0;

    for (int row = 0; row < lumatoneGeometry.horizontalLineCount(); row++)
    {
        if (originKeyIndex < lumatoneGeometry.getLastIndexForRow(row))
        {
            originRowIndex = row;
            lineToZeroDiff = originKeyIndex - lumatoneGeometry.getVerticalOriginLineIndexForRow(row);
            break;
        }
    }
    
    Hex::Point zeroKeyPoint = addBoardIndex(
                                Hex::Point(originPoint.q + lineToZeroDiff, originPoint.r - originRowIndex),
                                -originBoardIndex);

    int lineIndex = 0;
    int keyIndex = 0;
    int rowKeyIndex = 0;

    for (int row = 0; row < lumatoneGeometry.horizontalLineCount(); row++)
    {
        Hex::Point rowPoint = zeroKeyPoint;

        switch (row)
        {
        case 0:
            rowPoint = zeroKeyPoint;
            break;

        case 2:
        case 4:
        case 6:
        case 8:
            rowPoint = zeroKeyPoint + Hex::Point(-row, row * 2);
            break;

        case 1:
        case 3:
        case 5:
        case 7:
            rowPoint = zeroKeyPoint + Hex::Point(-row - 1, row * 2 + 1);
            break;

        case 9:
            rowPoint = zeroKeyPoint + Hex::Point(-1, 10);
            break;

        case 10:
            rowPoint = zeroKeyPoint + Hex::Point(-1, 10);
            break;
        }

        while (keyIndex < lumatoneGeometry.getLastIndexForRow(lineIndex))
        {
            for (int boardIndex = 0; boardIndex < layout.getNumBoards(); boardIndex++)
            {
                auto mappedBoardKey = addBoardIndex(rowPoint + Hex::Point(rowKeyIndex, 0), boardIndex);

                LumatoneKeyCoord coords(boardIndex, keyIndex);

                MappedKey mappedKey = { coords, mappedBoardKey };
                //boards.getReference(boardIndex).keys[keyIndex] = mappedKey;
                boards[boardIndex].keys[keyIndex] = mappedKey;

                auto key = mappedBoardKey.toString();
                map.set(key, mappedKey);

                keyIndex++;
            }
        }

        lineIndex++;
        rowKeyIndex = 0;
    }
}
