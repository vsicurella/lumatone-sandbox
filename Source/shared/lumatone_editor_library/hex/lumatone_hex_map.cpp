/*
  ==============================================================================

    lumatone_hex_map.cpp
    Created: 31 Jul 2023 1:39:39am
    Author:  Vincenzo

  ==============================================================================
*/

#include "lumatone_hex_map.h"

LumatoneHexMap::LumatoneHexMap(std::shared_ptr<LumatoneLayout> layoutIn, Hex::Point originPointIn, int originBoardIndexIn, int originKeyIndexIn)
    : layout(layoutIn)
    , originPoint(originPointIn)
    , originBoardIndex(originBoardIndexIn)
    , originKeyIndex(originKeyIndexIn)
{
    map.reset(new juce::HashMap<juce::String, MappedKey>(280));
    renderMap();
}

LumatoneHexMap::~LumatoneHexMap()
{
    map = nullptr;
}

LumatoneKeyCoord LumatoneHexMap::hexToKeyCoords(Hex::Point point) const
{
    auto key = point.toString();
    return (*map)[key].key;
}

int LumatoneHexMap::hexToKeyNum(Hex::Point point) const
{
    auto coord = hexToKeyCoords(point);
    if (layout->isKeyCoordValid(coord))
        return coord.boardIndex * layout->getOctaveBoardSize() + coord.keyIndex;
    return -1;
}

Hex::Point LumatoneHexMap::keyCoordsToHex(int boardIndex, int keyIndex) const
{
    return boards[boardIndex].keys[keyIndex].point;
}

Hex::Point LumatoneHexMap::keyCoordsToHex(const LumatoneKeyCoord& keyCoord) const
{
    return keyCoordsToHex(keyCoord.boardIndex, keyCoord.keyIndex);
}

Hex::Point LumatoneHexMap::keyNumToHex(int keyNum) const
{
    return keyCoordsToHex(keyNum / layout->getOctaveBoardSize(), keyNum % layout->getOctaveBoardSize());
}

Hex::Point LumatoneHexMap::addBoardIndex(Hex::Point point, int numIndexes)
{
    return Hex::Point(point.q + numIndexes * 5, point.r + numIndexes * 2);
}

void LumatoneHexMap::renderMap()
{
    map->clear();

    for (int i = 0; i < layout->getNumBoards(); i++)
        boards[i] = MapBoard(layout->getOctaveBoardSize());

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

    int keyIndex = 0;
    int numRows = lumatoneGeometry.horizontalLineCount();

    for (int row = 0; row < numRows; row++)
    {
        Hex::Point rowPoint = zeroKeyPoint;

        switch (row)
        {
        default:
            rowPoint = zeroKeyPoint + Hex::Point(-(row / 2), row);
            break;

        case 9:
            rowPoint = zeroKeyPoint + Hex::Point(-3, row);
            break;

        case 10:
            rowPoint = zeroKeyPoint + Hex::Point(-1, row);
            break;

        }

        auto rowKeys = lumatoneGeometry.horizontalLine(row);
        int rowLength = rowKeys.size();

        for (int rowKeyIndex = 0; rowKeyIndex < rowLength; rowKeyIndex++)
        {
            for (int boardIndex = 0; boardIndex < layout->getNumBoards(); boardIndex++)
            {
                auto boordCoord = Hex::Point(rowPoint.q + rowKeyIndex, rowPoint.r);
                auto hexCoord = addBoardIndex(boordCoord, boardIndex);

                LumatoneKeyCoord keyCoord(boardIndex, keyIndex);

                MappedKey mappedKey = { keyCoord, hexCoord };
                boards[boardIndex].keys[keyIndex] = mappedKey;

                auto hexHash = hexCoord.toString();
                map->set(hexHash, mappedKey);
            }

            keyIndex++;
        }
    }
}

bool LumatoneHexMap::testIdentityMap()
{
    const int octaveSize = 56;
    auto layout = std::make_shared<LumatoneLayout>(LumatoneLayout::IdentityMapping(1, octaveSize));
    LumatoneHexMap hexMap(layout);

    auto board = layout->getBoard(0);

    Hex::Point expectedHexCoords[] =
    {
        Hex::Point(0, 0),
        Hex::Point(1, 0),
        Hex::Point(0, 1),
        Hex::Point(1, 1),
        Hex::Point(2, 1),
        Hex::Point(3, 1),
        Hex::Point(4, 1),
        Hex::Point(-1, 2),
        Hex::Point(0, 2),
        Hex::Point(1, 2),
        Hex::Point(2, 2),
        Hex::Point(3, 2),
        Hex::Point(4, 2),
        Hex::Point(-1, 3),
        Hex::Point(0, 3),
        Hex::Point(1, 3),
        Hex::Point(2, 3),
        Hex::Point(3, 3),
        Hex::Point(4, 3),
        Hex::Point(-2, 4),
        Hex::Point(-1, 4),
        Hex::Point(0, 4),
        Hex::Point(1, 4),
        Hex::Point(2, 4),
        Hex::Point(3, 4),
        Hex::Point(-2, 5),
        Hex::Point(-1, 5),
        Hex::Point(0, 5),
        Hex::Point(1, 5),
        Hex::Point(2, 5),
        Hex::Point(3, 5),
        Hex::Point(-3, 6),
        Hex::Point(-2, 6),
        Hex::Point(-1, 6),
        Hex::Point(0, 6),
        Hex::Point(1, 6),
        Hex::Point(2, 6),
        Hex::Point(-3, 7),
        Hex::Point(-2, 7),
        Hex::Point(-1, 7),
        Hex::Point(0, 7),
        Hex::Point(1, 7),
        Hex::Point(2, 7),
        Hex::Point(-4, 8),
        Hex::Point(-3, 8),
        Hex::Point(-2, 8),
        Hex::Point(-1, 8),
        Hex::Point(0, 8),
        Hex::Point(1, 8),
        Hex::Point(-3, 9),
        Hex::Point(-2, 9),
        Hex::Point(-1, 9),
        Hex::Point(0, 9),
        Hex::Point(1, 9),
        Hex::Point(-1, 10),
        Hex::Point(0, 10)
    };

    for (int keyIndex = 0; keyIndex < octaveSize; keyIndex++)
    {
        auto keyCoord = LumatoneKeyCoord(0, keyIndex);
        auto mapped = hexMap.keyCoordsToHex(keyCoord);

        if (expectedHexCoords[keyIndex] != mapped)
        {
            DBG("ERROR mapping " + keyCoord.toString() + ", expected " + expectedHexCoords[keyIndex].toString() + " got " + mapped.toString());
            return false;
        }
    }

    return true;
}
