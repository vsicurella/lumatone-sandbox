/*
  ==============================================================================

    hexagon_automata_rules.cpp
    Created: 2023/11/01
    Author:  Vincenzo

  ==============================================================================
*/

#include "./hexagon_automata_game_state.h"
#include "hexagon_automata_rules.h"

static juce::Array<int> ParseListArgument(juce::String numberList)
{
    juce::Array<int> list;
    juce::String dbgStr;

    auto tks = juce::StringArray::fromTokens(numberList, juce::String(","), juce::String());
    for (auto tk : tks)
    { 
        if (tk.trim().isNotEmpty())
        {
            auto num = tk.getIntValue();
            if (num > 0 && num < 280)
            {
                list.addIfNotAlreadyThere(num);
                dbgStr += juce::String(num) + ",";
            }
        }
    }

    DBG("BornSurviveRule::ParseListArgument: " + dbgStr);

    return list;
}

static HexagonAutomata::NeighborsShape ParseShapeInput(juce::String shapeInput)
{
    HexagonAutomata::NeighborsShape shapeDefinition;

    auto tks = juce::StringArray::fromTokens(shapeInput, juce::String("|"), juce::String());
    for (auto tk : tks)
    {
        if (tk.trim().isNotEmpty())
        {
            auto point = Hex::Point::FromString(tk);
            shapeDefinition.add(point);
        }
    }
    return shapeDefinition;
}

HexagonAutomata::Rules::Rules()
{
    neighborsShape.addArray(getDefaultNeighborsShape());
}

HexagonAutomata::Rules::Rules(juce::String neighborInput)
    : neighborsShape(ParseShapeInput(neighborInput))
{
}

HexagonAutomata::Rules::Rules(NeighborsShapeTemp neighborsShapeIn)
{
    neighborsShape.addArray(neighborsShapeIn);
}

HexagonAutomata::Rules::Rules(const HexagonAutomata::State& gameStateIn)
    : neighborsShape(ParseShapeInput(gameStateIn.getNeighborShape()))
{
}

// HexagonAutomata::MappedCellStates HexagonAutomata::Rules::getPopulation(const HexagonAutomata::State& state) const
// {
//     juce::ScopedLock l(state.cells.getLock());
//     HexagonAutomata::MappedCellStates population;
//     for (int i = 0; i < state.numCells; i++)
//     {
//         auto cell = state.getMappedCell(i);
//         if (cell.isAlive())
//             population.add(cell);
//     }
//     return population;
// }

HexagonAutomata::MappedCellStates HexagonAutomata::Rules::getEmptyNeighbors(const HexagonAutomata::State& state, const MappedCellStates& population) const
{
    juce::Array<MappedHexState> emptyCells;
    juce::HashMap<juce::String, MappedHexState> emptyNeighbors;

    for (int c = 0; c < population.size(); c++)
    {
        const MappedHexState& cell = population.getReference(c);

        // Check cell status
        auto neighbors = state.getNeighbors(cell, neighborsShape);
        juce::Array<MappedHexState> livingNeighbors;
        for (int n = 0; n < neighbors.size(); n++)
        {
            if (neighbors[n].isAlive())
                livingNeighbors.add(neighbors[n]);
            else
            {
                auto cellHash = neighbors[n].toString();
                if (emptyNeighbors[cellHash].boardIndex < 0)
                {
                    emptyCells.add(neighbors[n]);
                    emptyNeighbors.set(cellHash, neighbors[n]);
                }
            }
        }
    }

    return emptyCells;
}

HexagonAutomata::MappedCellStates HexagonAutomata::BornSurviveRule::getNewCells(const HexagonAutomata::State& state, const MappedCellStates& population) const
{
    juce::ScopedLock thisLock(getLock());

    MappedCellStates newCells;
    int ticksPerGeneration = state.getTicksPerGeneration();

    if (state.getGenerationMode() == HexagonAutomata::GenerationMode::Synchronous)
    {
        if ((state.getNumTicks() < ticksPerGeneration) || (state.getNumTicks() % ticksPerGeneration != 0))
        {
            return newCells;
        }
    }

    juce::ScopedLock shapeLock(neighborsShape.getLock());
    MappedCellStates emptyCells = getEmptyNeighbors(state, population);

    switch (state.generationMode)
    {
    default:
    case GenerationMode::None:
    case GenerationMode::Synchronous:
        for (const MappedHexState& cell : emptyCells)
        {
            auto neighbors = state.getAliveNeighbors(cell, neighborsShape);
            if (generateNewLife(cell, neighbors))
            {
                auto update = MappedHexState(cell);
                update.setBorn();
                newCells.add(update);
            }
        }
        break;

    case GenerationMode::Asynchronous:
    {
        juce::HashMap<juce::String, MappedHexState> bornNeighbors;
        for (const MappedHexState& cell : population)    
        {
            if (cell.age < ticksPerGeneration || cell.age % ticksPerGeneration != 0)
                continue;

            // Find neighbors that can be born
            for (auto emptyCell : emptyCells)
            {
                auto cellHash = emptyCell.toString();
                if (bornNeighbors[cellHash].boardIndex >= 0)
                    continue;

                if (cell.distanceTo(emptyCell) == 1)
                {
                    auto neighbors = state.getAliveNeighbors(emptyCell, neighborsShape);
                    if (generateNewLife(emptyCell, neighbors))
                    {
                        emptyCell.setBorn();
                        bornNeighbors.set(cellHash, emptyCell);
                        newCells.add(emptyCell);
                    }
                }
            }       
        }
        break;
    }
    }

    return newCells;
}

HexagonAutomata::MappedCellStates HexagonAutomata::BornSurviveRule::getUpdatedCells(const HexagonAutomata::State& state, const MappedCellStates& population) const
{
    juce::ScopedLock thisLock(getLock());

    MappedCellStates updatedCells;

    if (state.getGenerationMode() == HexagonAutomata::GenerationMode::Synchronous 
      && (state.getNumTicks() < state.getTicksPerGeneration() 
       || state.getNumTicks() % state.getTicksPerGeneration() != 0
        )
       ) 
    {
        updatedCells = population;
    }
    else for (auto cell : population)
    {
        if (state.getGenerationMode() == GenerationMode::Asynchronous)
        {
            if (cell.age < state.getTicksPerGeneration() || cell.age % state.getTicksPerGeneration() != 0)
            {
                updatedCells.add(cell);
                continue;
            }
        }

        auto neighbors = state.getAliveNeighbors(cell, neighborsShape);
        float healthMult = getLifeFactor(cell, neighbors);
        if (healthMult == 0.0f)
        {
            cell.setDead();
        }
        else
        {
            cell.applyFactor(healthMult);
        }

        updatedCells.add(cell);
    }

    // for (MappedHexState& cell : updatedCells)
    // {
    //     cell.age++;
    // }

    return updatedCells;
}

HexagonAutomata::NeighborsShape HexagonAutomata::Rules::createNeighborsShape(int distance) const 
{ 
    juce::ScopedLock l(lock);

    auto neighbors = Hex::Point().neighbors(distance);
    juce::Array<Hex::Point, juce::CriticalSection> neighborsOut;
    neighborsOut.addArray(neighbors);
    return neighborsOut;
}

HexagonAutomata::BornSurviveRule::BornSurviveRule(int numBorn, int surviveLower, int surviveUpper)
    : Rules(getDefaultNeighborsShape())
{
    numsBorn.add(numBorn);

    for (int i = surviveLower; i <= surviveUpper; i++)
    {
        numsSurvive.add(i);    
    }
}

HexagonAutomata::BornSurviveRule::BornSurviveRule(juce::Array<int> bornNums, juce::Array<int> surviveNums)
    : numsBorn(bornNums)
    , numsSurvive(surviveNums) 
{
}

HexagonAutomata::BornSurviveRule::BornSurviveRule(juce::String bornString, juce::String surviveString)
{
    numsBorn = ParseListArgument(bornString);
    numsSurvive = ParseListArgument(surviveString);
}

float HexagonAutomata::BornSurviveRule::getLifeFactor(const HexagonAutomata::MappedHexState& origin, const HexagonAutomata::MappedCellStates& neighbors) const
{
    juce::ScopedLock l(lock);

    if (numsSurvive.contains(neighbors.size()))
        return 1.0f;
    return 0.0f;
}

bool HexagonAutomata::BornSurviveRule::generateNewLife(const HexagonAutomata::MappedHexState& origin, const HexagonAutomata::MappedCellStates& neighbors) const
{
    juce::ScopedLock l(lock);

    if (numsBorn.contains(neighbors.size()))
        return true;
    return false;
}

// HexagonAutomata::BornMiddleSurviveRule::BornMiddleSurviveRule(int numForBorn, int maxBorn, int surviveLower, int surviveUpper)
// {
//     numsForBorn.add(numForBorn);
//     maxNumsBorn.add(maxBorn);
//     for (int i = surviveLower; i <= surviveUpper; i++)
//         numsSurvive.add(i);
// }

// HexagonAutomata::BornMiddleSurviveRule::BornMiddleSurviveRule(juce::Array<int> numForBornIn, juce::Array<int> bornMaxes, juce::Array<int> surviveNums)
//     : numsForBorn(numsForBornIn)
//     , maxNumsForBorn(bornMaxes)
//     , numsSurvive(surviveNums)
// {
// }

// HexagonAutomata::BornMiddleSurviveRule::BornMiddleSurviveRule(juce::String numForBorn, juce::String bornMaxes, juce::String survieNums)
// {
//     numsForBorn = ParseListArgument(numForBorn);
//     maxNumsBorn = ParseListArgument(bornMaxes);
//     numsSurvive = ParseListArgument(survieNums);
// }

// juce::Array<Hex::Point> HexagonAutomata::BornMidNearSurviveRule::getNewCells(const MappedHexState &parentCell, const MappedHexState *neighbors, int numNeighbors)
// {
//     juce::Array<Hex::Point> newCells;

//     juce::Array<Hex::Point> aliveNeighbors;
//     for (int i = 0; i < numNeighbors; i++)
//     {

//     }

//     return newCells;
// }

// juce::Array<Hex::Point> HexagonAutomata::BornMidFarSurviveRule::getNewCells(const MappedHexState &parentCell, const MappedHexState *neighbors, int numNeighbors)
// {
//     return juce::Array<Hex::Point>();
// }

void HexagonAutomata::Rules::setNeighborsShape(juce::String shapeInputIn)
{
    auto readShape = ParseShapeInput(shapeInputIn);
    setNeighborsShape(readShape);
}

void HexagonAutomata::Rules::setNeighborsShape(const NeighborsShape &shapeIn)
{
    juce::ScopedLock l(neighborsShape.getLock());
    neighborsShape = shapeIn;
}

HexagonAutomata::NeighborsShapeTemp HexagonAutomata::Rules::getNeighborsShape() const
{
    NeighborsShapeTemp shapeCopy;
    juce::ScopedLock l(neighborsShape.getLock());
    shapeCopy.addArray(neighborsShape);
    return shapeCopy;
}

HexagonAutomata::NeighborsShapeTemp HexagonAutomata::Rules::getDefaultNeighborsShape() const
{
    return Hex::Point().neighbors(1);
}