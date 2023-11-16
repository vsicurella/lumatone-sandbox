/*
  ==============================================================================

    hexagon_automata_rules.cpp
    Created: 2023/11/01
    Author:  Vincenzo

  ==============================================================================
*/

#include "./hexagon_automata_game_state.h"
#include "hexagon_automata_rules.h"

using namespace HexagonAutomata;

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

static NeighborsShape ParseShapeInput(juce::String shapeInput)
{
    NeighborsShape shapeDefinition;

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

static long Factorial(long num)
{
    if (num <= 1)
        return 1;
    return num * Factorial(num - 1);
}

static int Modulo(int n, int d)
{
    return ((n % d) + n) % d;
}

Rules::Rules()
{
    neighborsShape.addArray(getDefaultNeighborsShape());
}

Rules::Rules(juce::String neighborInput)
    : neighborsShape(ParseShapeInput(neighborInput))
{
}

Rules::Rules(NeighborsShapeTemp neighborsShapeIn)
{
    neighborsShape.addArray(neighborsShapeIn);
}

Rules::Rules(const State& gameStateIn)
    : neighborsShape(ParseShapeInput(gameStateIn.getNeighborShape()))
{
}

// MappedCellStates Rules::getPopulation(const State& state) const
// {
//     juce::ScopedLock l(state.cells.getLock());
//     MappedCellStates population;
//     for (int i = 0; i < state.numCells; i++)
//     {
//         auto cell = state.getMappedCell(i);
//         if (cell.isAlive())
//             population.add(cell);
//     }
//     return population;
// }

MappedCellStates Rules::getEmptyNeighbors(const State& state, const MappedCellStates& population) const
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

MappedCellStates BornSurviveRule::getNewCells(const State& state, const MappedCellStates& population)
{
    juce::ScopedLock thisLock(getLock());

    MappedCellStates newCells;
    int ticksPerGeneration = state.getTicksPerGeneration();

    if (state.getGenerationMode() == GenerationMode::Synchronous)
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

MappedCellStates BornSurviveRule::getUpdatedCells(const State& state, const MappedCellStates& population)
{
    juce::ScopedLock thisLock(getLock());

    MappedCellStates updatedCells;

    if (state.getGenerationMode() == GenerationMode::Synchronous 
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

    return updatedCells;
}

NeighborsShape Rules::createNeighborsShape(int distance) const 
{ 
    juce::ScopedLock l(lock);

    auto neighbors = Hex::Point().neighbors(distance);
    juce::Array<Hex::Point, juce::CriticalSection> neighborsOut;
    neighborsOut.addArray(neighbors);
    return neighborsOut;
}

BornSurviveRule::BornSurviveRule(int numBorn, int surviveLower, int surviveUpper)
    : Rules(getDefaultNeighborsShape())
{
    numsBorn.add(numBorn);

    for (int i = surviveLower; i <= surviveUpper; i++)
    {
        numsSurvive.add(i);    
    }
}

BornSurviveRule::BornSurviveRule(juce::Array<int> bornNums, juce::Array<int> surviveNums)
    : numsBorn(bornNums)
    , numsSurvive(surviveNums) 
{
}

BornSurviveRule::BornSurviveRule(juce::String bornString, juce::String surviveString)
{
    numsBorn = ParseListArgument(bornString);
    numsSurvive = ParseListArgument(surviveString);
}

float BornSurviveRule::getLifeFactor(const MappedHexState& origin, const MappedCellStates& neighbors)
{
    juce::ScopedLock l(lock);

    if (numsSurvive.contains(neighbors.size()))
        return 1.0f;
    return 0.0f;
}

bool BornSurviveRule::generateNewLife(const MappedHexState& origin, const MappedCellStates& neighbors)
{
    juce::ScopedLock l(lock);

    if (numsBorn.contains(neighbors.size()))
        return true;
    return false;
}

// BornMiddleSurviveRule::BornMiddleSurviveRule(int numForBorn, int maxBorn, int surviveLower, int surviveUpper)
// {
//     numsForBorn.add(numForBorn);
//     maxNumsBorn.add(maxBorn);
//     for (int i = surviveLower; i <= surviveUpper; i++)
//         numsSurvive.add(i);
// }

// BornMiddleSurviveRule::BornMiddleSurviveRule(juce::Array<int> numForBornIn, juce::Array<int> bornMaxes, juce::Array<int> surviveNums)
//     : numsForBorn(numsForBornIn)
//     , maxNumsForBorn(bornMaxes)
//     , numsSurvive(surviveNums)
// {
// }

// BornMiddleSurviveRule::BornMiddleSurviveRule(juce::String numForBorn, juce::String bornMaxes, juce::String survieNums)
// {
//     numsForBorn = ParseListArgument(numForBorn);
//     maxNumsBorn = ParseListArgument(bornMaxes);
//     numsSurvive = ParseListArgument(survieNums);
// }

// juce::Array<Hex::Point> BornMidNearSurviveRule::getNewCells(const MappedHexState &parentCell, const MappedHexState *neighbors, int numNeighbors)
// {
//     juce::Array<Hex::Point> newCells;

//     juce::Array<Hex::Point> aliveNeighbors;
//     for (int i = 0; i < numNeighbors; i++)
//     {

//     }

//     return newCells;
// }

// juce::Array<Hex::Point> BornMidFarSurviveRule::getNewCells(const MappedHexState &parentCell, const MappedHexState *neighbors, int numNeighbors)
// {
//     return juce::Array<Hex::Point>();
// }

void Rules::setNeighborsShape(juce::String shapeInputIn)
{
    auto readShape = ParseShapeInput(shapeInputIn);
    setNeighborsShape(readShape);
}

void Rules::setNeighborsShape(const NeighborsShape &shapeIn)
{
    neighborsShape = shapeIn;
}

NeighborsShapeTemp Rules::getNeighborsShape() const
{
    NeighborsShapeTemp shapeCopy;
    shapeCopy.addArray(neighborsShape);
    return shapeCopy;
}

NeighborsShapeTemp Rules::getDefaultNeighborsShape() const
{
    return Hex::Point().neighbors(1);
}

TotalisticRule::TotalisticRule(int numStatesIn, juce::String shapeInputIn, juce::String ruleString)
    : Rules(shapeInputIn)
    , numStates(numStatesIn)
{
    auto outputArray = parseRuleInput(ruleString);
    if (outputArray.size() == 0)
    {
        jassertfalse;
        return;
    }

    outputRuleString = ruleString;
    setupRules(outputArray);
}

TotalisticRule::TotalisticRule(int numStatesIn, NeighborsShapeTemp shapeIn, juce::String ruleString)
    : Rules(shapeIn)
    , numStates(numStatesIn)
{
    auto outputArray = parseRuleInput(ruleString);
    if (outputArray.size() == 0)
    {
        jassertfalse;
        return;
    }

    outputRuleString = ruleString;
    setupRules(outputArray);
}

MappedCellStates TotalisticRule::getNewCells(const State &state, const MappedCellStates &population)
{
    juce::ScopedLock thisLock(getLock());

    MappedCellStates newCells;
    int ticksPerGeneration = state.getTicksPerGeneration();

    if (state.getGenerationMode() == GenerationMode::Synchronous)
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
        for (auto cell : emptyCells)
        {
            auto neighbors = state.getNeighbors(cell, neighborsShape);
            float health = getLifeFactor(cell, neighbors);
            if (health > 0.0f)
            {
                cell.setBorn();
                cell.health = health;
                newCells.add(cell);
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
                    auto neighbors = state.getNeighbors(cell, neighborsShape);
                    float health = getLifeFactor(cell, neighbors);
                    if (health > 0.0f)
                    {
                        emptyCell.setBorn();
                        emptyCell.health = health;
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

MappedCellStates TotalisticRule::getUpdatedCells(const State &state, const MappedCellStates &population)
{
    juce::ScopedLock thisLock(getLock());

    MappedCellStates updatedCells;

    if (state.getGenerationMode() == GenerationMode::Synchronous 
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

        auto neighbors = state.getNeighbors(cell, neighborsShape);
        float newHealth = getLifeFactor(cell, neighbors);
        if (newHealth == 0.0f)
        {
            cell.setDead();
        }
        else
        {
            cell.health = newHealth;
        }

        updatedCells.add(cell);
    }

    return updatedCells;
}

NeighborsShapeTemp HexagonAutomata::TotalisticRule::getDefaultNeighborsShape() const
{
    NeighborsShapeTemp shape = Hex::Point().neighbors(1);
    shape.insert(0, Hex::Point());
    return shape;
}

float TotalisticRule::getLifeFactor(const MappedHexState &origin, const MappedCellStates &neighbors)
{
    int state = 0;
    int tallied = 0;
    stateCounts.fill(0);

    for (const MappedHexState& cell : neighbors)
    {
        state = healthToState(cell.health);
        stateCounts.set(state, stateCounts[state] + 1);
        tallied++;
    }

    // Factor in out of bounds
    while (tallied < neighborsShape.size())
    {
        stateCounts.set(0, stateCounts[0] + 1);
        tallied++;
    }

    #if JUCE_DEBUG
        int sum = 0;
        for (auto num : stateCounts)
            sum += num;
        jassert(sum == neighborsShape.size());
    #endif

    juce::String hash = getTableHash(stateCounts);
    return stateToHealthFactor(ruleTable[hash]);
}

int TotalisticRule::expectedRuleSize(int numStates, const NeighborsShape &shape)
{
    long inner = numStates * shape.size() - 1;
    long innerF = Factorial(inner);
    long rpl = Factorial(shape.size());
    long rpr = Factorial(numStates - 1);
    long denom = (Factorial(shape.size()) * Factorial(numStates - 1));
    return Factorial(numStates * shape.size() - 1L) / (Factorial(shape.size()) * Factorial(numStates - 1L));
}

juce::Array<int> TotalisticRule::parseRuleInput(juce::StringRef outputString)
{
    juce::Array<int> out;
    for (int i = 0; i < outputString.length(); i++)
    {
        juce::String tk = juce::String() + outputString[i];
        out.add(tk.getIntValue());
    }

    return out;
}

bool TotalisticRule::enumerateRule(const juce::Array<int> &outputRule)
{
    const int numNeighbors = neighborsShape.size();

    // Leading rule, first index, gets set after succeeding rules
    juce::Array<int> statePtrs;
    statePtrs.resize(numStates);
    statePtrs.fill(0);

    juce::Array<int> stateMax;
    stateMax.resize(numStates);
    stateMax.fill(numNeighbors);

    juce::Array<int> statesVec;
    statesVec.resize(numStates);

    // For debugging
    juce::StringArray rows;
    rows.ensureStorageAllocated(numStates+1);
    for (int s = 0; s <= numStates; s++)
    {
        rows.set(s, juce::String());
    }

    // Output rules are printed in index-descending order, so read in reverse
    int i = outputRule.size() - 1;
    for (int oi = 0; oi < outputRule.size(); oi++)
    {
        for (int s = 1; s < numStates; s++)
        {
            statesVec.set(s, statePtrs[s]);
        }

        if (statePtrs[1] < stateMax[1])
            statePtrs.set(1, statesVec[1] + 1);

        for (int s = 1; s < numStates; s++)
        {
            if (statePtrs[s] == stateMax[s])
            {
                stateMax.set(s, juce::jmax(stateMax[s] - 1, 0));
            }
            else if (statePtrs[s] > stateMax[s])
            {
                statePtrs.set(1, 0);
                
                if (s + 1 < numStates)
                    statePtrs.set(s + 1, statesVec[s + 1] + 1);
            }
        }

        int enumSum = 0;
        for (int s = 1; s < numStates; s++)
        {
            enumSum += statesVec[s];
            rows.set(s, juce::String(statesVec[s]) + rows[s]);
        }

        if (enumSum > numNeighbors)
        {
            jassertfalse;
            return false;
        }

        int leadingRule = juce::jmax(numNeighbors - enumSum, 0);
        statesVec.set(0, leadingRule);
        rows.set(0, juce::String(leadingRule) + rows[0]);
        if (leadingRule == 0)
            stateMax.set(0, stateMax[0] - 1);

        rows.set(numStates, rows[numStates] + juce::String(outputRule[oi]));
        for (int s = 0; s <= numStates; s++)
        {
            jassert(rows[s].length()-1 == oi);
        }

        juce::String hash = getTableHash(statesVec);
        ruleTable.set(hash, outputRule[i--]);

    //     switch (oi)
    //     {
    //     case 0:
    //         jassert(statesVec[0] == 7 && statesVec[1] == 0 && statesVec[2] == 0);
    //         break;
    //     case 1:
    //         jassert(statesVec[0] == 6 && statesVec[1] == 1 && statesVec[2] == 0);
    //         break;
    //     case 2:
    //         jassert(statesVec[0] == 5 && statesVec[1] == 2 && statesVec[2] == 0);
    //         break;
    //     case 3:
    //         jassert(statesVec[0] == 4 && statesVec[1] == 3 && statesVec[2] == 0);
    //         break;
    //     case 4:
    //         jassert(statesVec[0] == 3 && statesVec[1] == 4 && statesVec[2] == 0);
    //         break;
    //     case 5:
    //         jassert(statesVec[0] == 2 && statesVec[1] == 5 && statesVec[2] == 0);
    //         break;
    //     case 6:
    //         jassert(statesVec[0] == 1 && statesVec[1] == 6 && statesVec[2] == 0);
    //         break;
    //     case 7:
    //         jassert(statesVec[0] == 0 && statesVec[1] == 7 && statesVec[2] == 0);
    //         break;
    //     case 8:
    //         jassert(statesVec[0] == 6 && statesVec[1] == 0 && statesVec[2] == 1);
    //         break;
    //     }

    }

    DBG("Parsed Rule Table:");
    for (int s = numStates-1; s >= 0; s--)
    {
        DBG(rows[s]);
    }
    DBG(rows[numStates]);

    // juce::String testHash = "0,5,2";
    // DBG("Test this hash: " + testHash);
    // DBG(juce::String(ruleTable[testHash]));

    return true;
}

bool HexagonAutomata::TotalisticRule::setupRules(const juce::Array<int>& outputRule)
{
    // overflow issues
    // auto expected = expectedRuleSize(numStates, neighborsShape);
    // if (outputRule.size() != expectedRuleSize(numStates, neighborsShape))
    // {
    //     jassertfalse;
    //     return false;
    // }

    return enumerateRule(outputRule);
}

juce::String TotalisticRule::getTableHash(juce::Array<int> &stateVector)
{
    juce::String hash = "";
    for (int s = 0; s < stateVector.size(); s++)
    {
        hash += juce::String(stateVector[s]);
        if (s < stateVector.size() - 1)
            hash += ",";
     }

    return hash;
}

void HexagonAutomata::TotalisticRule::copyTableHash(const RuleTable &tableToCopy)
{
    // for (auto key : tableToCopy)()
    // {
    //     ruleTable.set(key, tableToCopy[key]);
    // }
}

float TotalisticRule::stateToHealthFactor(int stateNum) const
{
    float health = (float)stateNum / (float)numStates;
    return health;
}

int TotalisticRule::healthToState(float health) const
{
    auto state = juce::roundToInt(health * numStates);
    if (state < 0)
        return 0;
    if (state >= numStates)
        return numStates - 1;
    return state;
}

HexagonAutomata::SpiralRule::SpiralRule()
    : TotalisticRule(3, getDefaultNeighborsShape(), "000200120021220221200222122022221210")
{
}
