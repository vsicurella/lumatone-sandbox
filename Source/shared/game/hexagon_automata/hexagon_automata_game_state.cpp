/*

    File: hexagon_automata_game_state.cpp
    Author: Vito
    Date: 2023/11/01

*/

#include "./hexagon_automata_game_state.h"

HexagonAutomata::GameState::GameState(std::shared_ptr<LumatoneLayout> layoutIn)
    : layout(layoutIn)
    , hexMap(layoutIn) 
{
    resetState();
}

HexagonAutomata::GameState::GameState(const HexagonAutomata::GameState& copy)
    : layout(copy.layout)
    , hexMap(copy.layout)
    , numCells(copy.numCells)
    , cells(copy.cells) 
{

}

void HexagonAutomata::GameState::resetState()
{
    numCells = layout->getOctaveBoardSize() * layout->getNumBoards();

    cells.resize(numCells);
    cells.fill(HexState());
}

HexagonAutomata::MappedHexState HexagonAutomata::GameState::getMappedCell(int cellNum)
{
    auto hex = hexMap.keyNumToHex(cellNum);
    auto keyCoord = layout->keyNumToKeyCoord(cellNum);

    auto mappedKey = MappedLumatoneKey(*layout->readKey(keyCoord.boardIndex, keyCoord.keyIndex), keyCoord.boardIndex, keyCoord.keyIndex);
    return HexagonAutomata::MappedHexState(cells[cellNum], mappedKey, hex);
}

juce::Array<HexagonAutomata::MappedHexState> HexagonAutomata::GameState::getNeighbors(Hex::Point cellCoord, const juce::Array<Hex::Point>& vector) const
{
    juce::Array<MappedHexState> mappedCells;
    for (auto point : vector)
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

juce::Array<HexagonAutomata::MappedHexState> HexagonAutomata::GameState::getAliveNeighbors(Hex::Point cellCoord, const juce::Array<Hex::Point>& vector) const
{
    juce::Array<MappedHexState> mappedCells;
    for (auto point : vector)
    {
        auto neighbor = cellCoord + point;
        auto keyCoord = hexMap.hexToKeyCoords(neighbor);
        int cellNum = layout->keyCoordToKeyNum(keyCoord);
        if (cellNum < 0)
            continue;
        
        if (cells[cellNum].isAlive())
        {
            auto mappedKey = MappedLumatoneKey(*layout->readKey(keyCoord.boardIndex, keyCoord.keyIndex), keyCoord.boardIndex, keyCoord.keyIndex);
            auto mappedCell = MappedHexState(cells[cellNum], mappedKey, point);
            mappedCells.add(mappedCell);
        }
    }

    return mappedCells;
}