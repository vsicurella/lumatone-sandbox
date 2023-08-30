#include "./hexagon_automata.h"

HexagonAutomata::MappedHexState HexagonAutomata::GameState::getMappedCell(int cellNum)
{
    auto hex = hexMap.keyNumToHex(cellNum);
    auto keyCoord = layout->keyNumToKeyCoord(cellNum);

    auto mappedKey = MappedLumatoneKey(*layout->readKey(keyCoord.boardIndex, keyCoord.keyIndex), keyCoord.boardIndex, keyCoord.keyIndex);
    return MappedHexState(cells[cellNum], mappedKey, hex);
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

HexagonAutomata::Game::Game(LumatoneController* controller)
    : LumatoneSandboxGameBase(controller, "Hexagon Automata")
    , HexagonAutomata::GameState(controller->shareMappingData())
{
    initialize();
}

HexagonAutomata::Game::Game(LumatoneController* controller, const HexagonAutomata::GameState& stateIn)
    : LumatoneSandboxGameBase(controller, "Hexagon Automata") 
    , HexagonAutomata::GameState(stateIn)
{
    initialize();
}

void HexagonAutomata::Game::initialize()
{
    lock.enter();

    if (rules.get() == nullptr)
    {
        rules.reset(new BornSurviveRule(2, 3, 4));
    }

    numCells = controller->getNumBoards() * controller->getOctaveBoardSize();
    
    cells.resize(numCells);

    neighborsVector = rules->getNeighborsVector(1);

    if (render.get() == nullptr)
        render.reset(new Renderer);

    lock.exit();
}

void HexagonAutomata::Game::redoCensus()
{
    lock.enter();

    populatedCells.clear();

    for (int i = 0; i < cells.size(); i++)
    {
        if (!cells[i].isEmpty())
        {
            auto keyCoord = layout->keyNumToKeyCoord(i);
            auto key = MappedLumatoneKey(*layout->readKey(i), keyCoord);
            populatedCells.add(MappedHexState(cells[i], key, hexMap.keyNumToHex(i)));
        }
    }

    lock.exit();
}

void HexagonAutomata::Game::reset(bool clearQueue)
{
    lock.enter();

    LumatoneSandboxGameBase::reset(clearQueue);
    resetState();

    random.setSeedRandomly();

    ticks = 0;
    ticksToNextGeneration = 0;

    if (clearQueue)
    {
        bornCells.clear();
        agingCells.clear();
        diedCells.clear();

        queueIdentityLayout(true);
    }

    lock.exit();
}

void HexagonAutomata::Game::resetState()
{
    lock.enter();

    HexagonAutomata::GameState::resetState();

    for (auto cell : populatedCells)
    {
        cell.health = 0;
        cell.age = 0;
        diedCells.add(cell);
    }

    lock.exit();
}

void HexagonAutomata::Game::nextTick()
{
    bool postpone = true;
    for (int tries = 0; tries < 5; tries++)
    {
        if (lock.tryEnter())
        {
            postpone = false;
            break;
        }
    }

    if (postpone)
        return;

    if (ticksToNextGeneration >= ticksPerGeneration)
    {
        ticksToNextGeneration = 0;
        updateNextGeneration();
    }
    else
    {
        updateGenerationAge();
    }

    addToQueue(renderFrame());

    ticks++;
    ticksToNextGeneration++;

    lock.exit();
}

void HexagonAutomata::Game::pauseTick()
{
    if (!lock.tryEnter())
        return;

    addToQueue(renderFrame());

    lock.exit();
}

void HexagonAutomata::Game::setTicksPerGeneration(int ticks)
{
    if (ticks > 0)
    {
        ticksPerGeneration = ticks;
        render->maxAge = ticksPerGeneration * 3;
    }
}

bool HexagonAutomata::Game::applyUpdatedCell(const HexagonAutomata::MappedHexState& cell)
{
    auto cellNum = layout->keyCoordToKeyNum(cell.getKeyCoord());
    if (cellNum >= 0)
    {
        cells.set(cellNum, cell);
        if (cell.isDead() || cell.isAlive())
            return true;
    }

    return false;
}

juce::UndoableAction* HexagonAutomata::Game::renderFrame()
{
    bool hasUpdates = bornCells.size() > 0 || agingCells.size() > 0 || diedCells.size() > 0;
    if (!hasUpdates)
        return nullptr;

    lock.enter();

    juce::Array<MappedLumatoneKey> keyUpdates;
    juce::Array<MappedHexState> populated;
    
    for (auto cell : agingCells)
    {
        keyUpdates.add(render->renderCellKey(cell));

        if (applyUpdatedCell(cell))
            populated.add(cell);

        if (verbose > 0)
        {
            if (cell.health > 0.0f)
                DBG("Cell " + cell.toString() + " has aged to " + juce::String(cell.age));
            else
                DBG("Cell " + cell.toString() + " has died aged " + juce::String(cell.age));
        }
    }
    agingCells.clear();

    for (auto cell : diedCells)
    {
        keyUpdates.add(render->renderCellKey(cell));
        applyUpdatedCell(cell);

        if (verbose > 0)
            DBG("Cell " + cell.toString() + " is empty");
    }
    diedCells.clear();

    for (auto cell : bornCells)
    {
        keyUpdates.add(render->renderCellKey(cell));

        if (applyUpdatedCell(cell))
            populated.add(cell);

        if (verbose > 0)
            DBG("Cell " + cell.toString() + " was born");
    }
    bornCells.clear();

    // updatedCells.clear();

    // TODO fix bug with managed version - loses living population after addSeed during pause
    // populatedCells.swapWith(populated);

    // redoCensus();

    lock.exit();

    return new LumatoneEditAction::MultiKeyAssignAction(controller, keyUpdates);
}

void HexagonAutomata::Game::rerenderState()
{
	lock.enter();

    juce::Array<MappedHexState> savedBorn;
    savedBorn.swapWith(bornCells);

    juce::Array<MappedHexState> savedAging;
    savedAging.swapWith(agingCells);

    juce::Array<MappedHexState> savedDied;
    savedDied.swapWith(diedCells);

    for (int c = 0; c < numCells; c++)
    {
        auto coord = layout->keyNumToKeyCoord(c);
        agingCells.add(HexagonAutomata::MappedHexState(
            cells[c],
            layout->getMappedKey(coord.boardIndex, coord.keyIndex),
            hexMap.keyNumToHex(c)
        ));
    }

    addToQueue(renderFrame());

    bornCells.swapWith(savedBorn);
    agingCells.swapWith(savedAging);
    diedCells.swapWith(savedDied);

	lock.exit();
}

void HexagonAutomata::Game::addSeed(Hex::Point point)
{
	lock.enter();

    auto keyCoord = hexMap.hexToKeyCoords(point);
    HexState state(1.0f);
    auto newCell = HexagonAutomata::MappedHexState(
        state,
        layout->getMappedKey(keyCoord.boardIndex, keyCoord.keyIndex),
        point
    );

    bornCells.add(newCell);

    // updatedCells.add(newCell);
	lock.exit();
}

void HexagonAutomata::Game::addSeeds(juce::Array<Hex::Point> seedCoords)
{
	lock.enter();

    for (auto point : seedCoords)
    {
        addSeed(point);
    }

	lock.exit();
}

void HexagonAutomata::Game::addSeeds(int numSeeds)
{
    float probablity = 0.5f;

    for (int i = 0; i < numSeeds; i++)
    {
        int keyNum = random.nextInt(numCells);

        auto hexCoord = hexMap.keyNumToHex(keyNum);
        
        juce::Array<Hex::Point> cellPoints;
        cellPoints.add(hexCoord);

        for (auto vec : neighborsVector)
        {
            auto point = hexCoord + vec;
            cellPoints.add(point);
        }

        for (auto seed : cellPoints)
        {
            float chance = random.nextFloat();
            if (chance <= probablity)
            {
                addSeed(seed);
            }
        }
    }
}

void HexagonAutomata::Game::setAliveColour(juce::Colour newColour)
{
    render->setColour(newColour);
}

void HexagonAutomata::Game::setBornSurviveRules(juce::Array<int> bornNums, juce::Array<int> surviveNums)
{
    lock.enter();

    rules.reset(new BornSurviveRule(bornNums, surviveNums));

    lock.exit();
}

void HexagonAutomata::Game::setBornSurviveRules(juce::String bornInput, juce::String surviveInput)
{
    lock.enter();

    rules.reset(new BornSurviveRule(bornInput, surviveInput));

    lock.exit();
}

void HexagonAutomata::Game::setNeighborDistance(int distance)
{
    lock.enter();

    neighborsVector.swapWith(rules->getNeighborsVector(distance));

    lock.exit();
}

void HexagonAutomata::Game::updateNextGeneration()
{
	lock.enter();

    juce::Array<Hex::Point> emptyCells;
    juce::Array<MappedHexState> emptyNeighbors;

    // for (int c = 0; c < populatedCells.size(); c++)
    // {
    //     auto cell = populatedCells[c];

    //     // Check cell status
    //     auto neighbors = getNeighbors(cell, neighborsVector);
    //     juce::Array<MappedHexState> livingNeighbors;
    //     for (int n = 0; n < neighbors.size(); n++)
    //     {
    //         if (neighbors[n].isAlive())
    //             livingNeighbors.add(neighbors[n]);
    //         else if (emptyCells.addIfNotAlreadyThere((Hex::Point)neighbors[n]))
    //             emptyNeighbors.add(neighbors[n]);
    //     }

    //     if (cell.isDead())
    //     {
    //         cell.age = 0;
    //         diedCells.add(cell);
    //     }
    //     else
    //     {
    //         float healthFactor = rules->getLifeFactor(cell, neighbors.getRawDataPointer(), livingNeighbors.size());
    //         cell.applyFactor(healthFactor);
    //         cell.age++;
    //         agingCells.add(cell);
    //     }

    //     // updatedCells.add(cell);
    // }

    // for (int e = 0; e < emptyCells.size(); e++)
    // {
    //     auto point = emptyCells[e];
    //     auto cell = emptyNeighbors[e];

    //     auto neighbors = getAliveNeighbors(cell, neighborsVector);
    //     if (rules->generateNewLife(cell, neighbors.getRawDataPointer(), neighbors.size()))
    //     {
    //         cell.setBorn();
    //         bornCells.add(cell);
    //         // bool cellUpdated = false;
    //         // for (int u = 0; u < updatedCells.size(); u++)
    //         // {
    //         //     if ((Hex::Point)updatedCells[u] == (Hex::Point)cell)
    //         //     {
    //         //         updatedCells.set(u, cell);
    //         //         cellUpdated = true;
    //         //     }
    //         // }

    //         // if (!cellUpdated)
    //             // updatedCells.add(cell);
    //     }
    // }

    for (int i = 0; i < numCells; i++)
    {
        auto cell = getMappedCell(i);
        auto neighbors = getAliveNeighbors(hexMap.keyNumToHex(i), neighborsVector);
                
        if (cell.isDead() || cell.HexState::isEmpty())
        {
            if (rules->generateNewLife(cell, neighbors.getRawDataPointer(), neighbors.size()))
            {
                cell.setBorn();
                bornCells.add(cell);
            }
        }

        else if (cell.isAlive())
        {
            auto healthFactor = rules->getLifeFactor(cell,neighbors.getRawDataPointer(), neighbors.size());
            cell.applyFactor(healthFactor);
            if (cell.isAlive())
                agingCells.add(cell);
            else
                diedCells.add(cell);
        }
    }

	lock.exit();
}

void HexagonAutomata::Game::updateGenerationAge()
{
	lock.enter();

    for (int c = 0; c < populatedCells.size(); c++)
    {
        auto cell = populatedCells[c];
        cell.age++;
        // updatedCells.add(cell);
        agingCells.add(cell);
    }

	lock.exit();
}

void HexagonAutomata::Game::handleNoteOn(LumatoneMidiState* midiState, int midiChannel, int midiNote, juce::uint8 velocity)
{
    auto hexCoord = hexMap.keyCoordsToHex(midiChannel - 1, midiNote);
    addSeed(hexCoord);
}
