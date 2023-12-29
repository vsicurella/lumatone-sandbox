/*

    File: hexagon_automata_game_state.h
    Author: Vito
    Date: 2023/11/01

*/

#ifndef LUMATONE_HEX_AUTOMATA_BOARD_STATE_H
#define LUMATONE_HEX_AUTOMATA_BOARD_STATE_H

#include "./hexagon_automata_cell_state.h"

#include "../../lumatone_editor_library/mapping/lumatone_hex_map.h"

namespace HexagonAutomata
{
class BoardState
{
public:
    std::shared_ptr<LumatoneLayout> layout;
    LumatoneHexMap hexMap;

    int numCells = 0;
    CellStates cells;

    BoardState(std::shared_ptr<LumatoneLayout> layoutIn);
    BoardState(const BoardState& copy);

    virtual void resetState();

    MappedHexState getMappedCell(int cellNum) const;

    virtual juce::Array<MappedHexState> getNeighbors(Hex::Point cellCoord, const juce::Array<Hex::Point, juce::CriticalSection>& vector) const;
    virtual juce::Array<MappedHexState> getAliveNeighbors(Hex::Point cellCoord, const juce::Array<Hex::Point, juce::CriticalSection>& vector) const;
};
    
}

#endif // LUMATONE_HEX_AUTOMATA_BOARD_STATE_H
