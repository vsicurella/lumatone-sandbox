/*

    File: hexagon_automata_game_state.cpp
    Author: Vito
    Date: 2023/11/01

*/

#include "./hexagon_automata_board_state.h"

HexagonAutomata::BoardState::BoardState(std::shared_ptr<LumatoneLayout> layoutIn)
    : layout(layoutIn)
    , hexMap(layoutIn) 
{
    resetState();
}

HexagonAutomata::BoardState::BoardState(const HexagonAutomata::BoardState& copy)
    : layout(copy.layout)
    , hexMap(copy.layout)
    , numCells(copy.numCells)
    , cells(copy.cells) 
{

}

void HexagonAutomata::BoardState::resetState()
{
    numCells = layout->getOctaveBoardSize() * layout->getNumBoards();

    cells.resize(numCells);
    cells.fill(HexState());
}

HexagonAutomata::MappedHexState HexagonAutomata::BoardState::getMappedCell(int cellNum) const
{
    auto hex = hexMap.keyNumToHex(cellNum);
    auto keyCoord = layout->keyNumToKeyCoord(cellNum);

    auto mappedKey = MappedLumatoneKey(*layout->readKey(keyCoord.boardIndex, keyCoord.keyIndex), keyCoord.boardIndex, keyCoord.keyIndex);
    return HexagonAutomata::MappedHexState(cells[cellNum], mappedKey, hex);
}

juce::Array<HexagonAutomata::MappedHexState> HexagonAutomata::BoardState::getNeighbors(Hex::Point cellCoord, const juce::Array<Hex::Point, juce::CriticalSection>& vector) const
{
    juce::ScopedLock l(vector.getLock());

    juce::Array<MappedHexState> mappedCells;
    for (const Hex::Point& point : vector)
    {
        auto neighbor = cellCoord + point;
        auto keyCoord = hexMap.hexToKeyCoords(neighbor);
        int cellNum = layout->keyCoordToKeyNum(keyCoord);
        if (cellNum < 0)
            continue;

        auto mappedKey = MappedLumatoneKey(*layout->readKey(keyCoord.boardIndex, keyCoord.keyIndex), keyCoord.boardIndex, keyCoord.keyIndex);
        auto mappedCell = MappedHexState(cells[cellNum], mappedKey, neighbor);
        mappedCells.add(mappedCell);
    }

    return mappedCells;
}

juce::Array<HexagonAutomata::MappedHexState> HexagonAutomata::BoardState::getAliveNeighbors(Hex::Point cellCoord, const juce::Array<Hex::Point, juce::CriticalSection>& vector) const
{
    juce::ScopedLock l(vector.getLock());

    juce::Array<MappedHexState> mappedCells;
    for (const Hex::Point& point : vector)
    {
        auto neighbor = cellCoord + point;
        auto keyCoord = hexMap.hexToKeyCoords(neighbor);
        int cellNum = layout->keyCoordToKeyNum(keyCoord);
        if (cellNum < 0)
            continue;
        
        const HexState& cell = cells[cellNum];
        if (cell.isAlive())
        {
            auto mappedKey = MappedLumatoneKey(*layout->readKey(keyCoord.boardIndex, keyCoord.keyIndex), keyCoord.boardIndex, keyCoord.keyIndex);
            auto mappedCell = MappedHexState(cell, mappedKey, point);
            mappedCells.add(mappedCell);
        }
    }

    return mappedCells;
}
