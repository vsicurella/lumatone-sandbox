#include "./hexagon_automata.h"

juce::Array<HexagonAutomata::MappedHexState> HexagonAutomata::GameState::getNeighbors(const MappedHexState& cell, const juce::Array<Hex::Point>& vector) const
{
    juce::Array<MappedHexState> mappedCells;
    for (auto point : vector)
    {
        auto neighbor = (Hex::Point)cell + point;
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

juce::Array<HexagonAutomata::MappedHexState> HexagonAutomata::GameState::getAliveNeighbors(const HexagonAutomata::MappedHexState& cell, const juce::Array<Hex::Point>& vector) const
{
    auto cellCoord = (Hex::Point)cell;
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

void HexagonAutomata::GameState::remapLivingHexes()
{
    livingHexes.ensureStorageAllocated(numCells);
    livingHexes.clear();

    for (int i = 0; i < cells.size(); i++)
    {
        if (cells[i].isAlive())
        {
            auto keyCoord = layout->keyNumToKeyCoord(i);
            auto key = MappedLumatoneKey(*layout->readKey(i), keyCoord);
            livingHexes.add(MappedHexState(cells[i], key, hexMap.keyNumToHex(i)));
        }
    }
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
    if (rules.get() == nullptr)
    {
        rules.reset(new BornSurviveRule(2, 3, 4));
    }

    numCells = controller->getNumBoards() * controller->getOctaveBoardSize();
    
    cells.resize(numCells);

    neighborsVector = rules->getNeighborsVector(1);

    if (gfx.get() == nullptr)
        gfx.reset(new Renderer);
}

void HexagonAutomata::Game::reset(bool clearQueue)
{
    LumatoneSandboxGameBase::reset(clearQueue);

    random.setSeedRandomly();

    ticks = 0;

    if (clearQueue)
    {
        queueIdentityLayout(true);
    }
}

void HexagonAutomata::Game::nextTick()
{
    if (ticksToNextGeneration >= ticksPerGeneration)
    {
        ticksToNextGeneration = 0;
        updateNextGeneration();
    }
    
    updateGenerationAge();

    if (updatedCells.size() > 0)
        addToQueue(renderFrame());

    ticks++;
    ticksToNextGeneration++;
}

void HexagonAutomata::Game::pauseTick()
{
    addToQueue(renderFrame());
}

void HexagonAutomata::Game::setTicksPerGeneration(int ticks)
{
    if (ticks > 0)
    {
        ticksPerGeneration = ticks;
    }
}

juce::UndoableAction* HexagonAutomata::Game::renderFrame()
{
    if (updatedCells.size() > 0)
    {
        juce::Array<MappedLumatoneKey> keyUpdates;

        for (auto cell : updatedCells)
        {
            keyUpdates.add(gfx->renderCellKey(cell));

            auto cellNum = layout->keyCoordToKeyNum(cell.getKeyCoord());
            if (cellNum >= 0)
                cells.set(cellNum, cell);
        }

        updatedCells.clear();
        remapLivingHexes();

        return new LumatoneEditAction::MultiKeyAssignAction(controller, keyUpdates);
    }

    return nullptr;
}

void HexagonAutomata::Game::rerenderState()
{
    for (int c = 0; c < numCells; c++)
    {
        auto coord = layout->keyNumToKeyCoord(c);
        updatedCells.add(HexagonAutomata::MappedHexState(
            cells[c],
            layout->getMappedKey(coord.boardIndex, coord.keyIndex),
            hexMap.keyNumToHex(c)
        ));
    }

    addToQueue(renderFrame());
}

void HexagonAutomata::Game::addSeed(Hex::Point point)
{
    auto keyCoord = hexMap.hexToKeyCoords(point);
    HexState state(1.0f);
    auto newCell = HexagonAutomata::MappedHexState(
        state,
        layout->getMappedKey(keyCoord.boardIndex, keyCoord.keyIndex),
        point
    );

    updatedCells.add(newCell);
}

void HexagonAutomata::Game::addSeeds(juce::Array<Hex::Point> seedCoords)
{
    for (auto point : seedCoords)
    {
        addSeed(point);
    }
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
    gfx->setColour(newColour);
}

void HexagonAutomata::Game::updateNextGeneration()
{
    // // Check all cells?
    // for (int i = 0; i < numCells; i++)
    // {

    // }

    juce::Array<Hex::Point> emptyCells;
    juce::Array<MappedHexState> emptyNeighbors;

    for (int c = 0; c < livingHexes.size(); c++)
    {
        auto cell = livingHexes[c];

        // Check cell status
        auto neighbors = getNeighbors(cell, neighborsVector);
        juce::Array<MappedHexState> livingNeighbors;
        for (int n = 0; n < neighbors.size(); n++)
        {
            if (neighbors[n].isAlive())
                livingNeighbors.add(neighbors[n]);
            else if (emptyCells.addIfNotAlreadyThere((Hex::Point)neighbors[n]))
                emptyNeighbors.add(neighbors[n]);
        }

        float healthFactor = rules->getLifeFactor(cell, neighbors.getRawDataPointer(), livingNeighbors.size());
        cell.applyFactor(healthFactor);
        if (!cell.isAlive())
            updatedCells.add(cell);
    }

    for (int e = 0; e < emptyCells.size(); e++)
    {
        auto point = emptyCells[e];
        auto cell = emptyNeighbors[e];

        auto neighbors = getAliveNeighbors(cell, neighborsVector);
        if (rules->generateNewLife(cell, neighbors.getRawDataPointer(), neighbors.size()))
        {
            addSeed(point);
        }
    }
}

void HexagonAutomata::Game::updateGenerationAge()
{
    for (int c = 0; c < livingHexes.size(); c++)
    {
        auto cell = livingHexes[c];

        // Check cell status
        auto neighbors = getNeighbors(cell, neighborsVector);
        juce::Array<MappedHexState> livingNeighbors;

        float healthFactor = rules->getLifeFactor(cell, neighbors.getRawDataPointer(), livingNeighbors.size());
        cell.applyFactor(healthFactor);
        if (cell.isAlive())
        {
            updatedCells.add(cell);
            cell.age++;
        }
    }
}

void HexagonAutomata::Game::handleNoteOn(LumatoneMidiState* midiState, int midiChannel, int midiNote, juce::uint8 velocity)
{
    auto hexCoord = hexMap.keyCoordsToHex(midiChannel - 1, midiNote);
    addSeed(hexCoord);
}
