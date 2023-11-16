/*

    File: hexagon_automata_cell_state.h
    Author: Vito
    Date: 2023/11/01

*/

#ifndef LUMATONE_HEX_AUTOMATA_CELL_STATE_H
#define LUMATONE_HEX_AUTOMATA_CELL_STATE_H

#include <JuceHeader.h>

#include "../../lumatone_editor_library/hex/hex_field.h"
#include "../../lumatone_editor_library/data/lumatone_key.h"
namespace HexagonAutomata
{
struct HexState
{
    float health;
    int age;

    juce::Colour colour;

    HexState()
        : health(0.0f), age(0), colour(juce::Colours::white) {}
    HexState(float healthIn, int ageIn=0, juce::Colour colourIn=juce::Colours::white)
        : health(healthIn), age(ageIn), colour(colourIn) {}

    void applyFactor(float lifeFactor) { health *= lifeFactor; }
    
    bool isAlive() const { return health > 0.0f; }

    bool isNewBorn() const { return health == 1.0f && age == 0; }

    bool isDead() const { return health <= 0.0f && age > 0; }
    
    bool isEmpty() const { return health <= 0.0f && age == 0; }

    void setBorn(float healthIn=1.0f) { health = healthIn; age = 0; }

    void setDead() { health = 0.0f; }

    void setEmpty() { health = 0.0f; age = 0; }
};


struct MappedHexState   : public HexState
                        , public MappedLumatoneKey
                        , public Hex::Point
{
    MappedHexState()
        : HexState()
        , MappedLumatoneKey(LumatoneKey(), LumatoneKeyCoord())
        , Hex::Point(0, 0)
    {}
    
    MappedHexState(const HexState& stateIn, const MappedLumatoneKey& key, Hex::Point hexCoord)
        : HexState(stateIn)
        , MappedLumatoneKey(key)
        , Hex::Point(hexCoord) {}


};

using CellStates = juce::Array<HexState>;
using MappedCellStates = juce::Array<MappedHexState>;

}

#endif // LUMATONE_HEX_AUTOMATA_CELL_STATE_H
