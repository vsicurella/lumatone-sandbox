#include "./hexagon_automata.h"
#include "hexagon_automata.h"

#include "../../lumatone_editor_library/LumatoneController.h"
#include "../../lumatone_editor_library/actions/edit_actions.h"

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
    , mode(HexagonAutomata::GameMode::Sequencer)
{
    initialize();
}

HexagonAutomata::Game::Game(LumatoneController* controller, const HexagonAutomata::GameState& stateIn)
    : LumatoneSandboxGameBase(controller, "Hexagon Automata") 
    , HexagonAutomata::GameState(stateIn)
    , mode(HexagonAutomata::GameMode::Sequencer)
{
    initialize();
}

void HexagonAutomata::Game::initialize()
{
    juce::ScopedLock l(lock);

    if (rules.get() == nullptr)
    {
        rules.reset(new BornSurviveRule(2, 3, 4));
    }

    numCells = controller->getNumBoards() * controller->getOctaveBoardSize();
    
    cells.resize(numCells);

    neighborsVector = rules->getNeighborsVector(1);

    if (render.get() == nullptr)
        render.reset(new Renderer);
}

void HexagonAutomata::Game::redoCensus()
{
    juce::ScopedLock l(lock);

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
}

void HexagonAutomata::Game::reset(bool clearQueue)
{
    juce::ScopedLock l(lock);

    LumatoneSandboxGameBase::reset(clearQueue);
    resetState();

    random.setSeedRandomly();

    ticks = 0;
    ticksToNextGeneration = 0;

    switch (mode)
    {
    case GameMode::Classic:
    {
        auto newLayout = getIdentityLayout(true);
        queueLayout(newLayout);
        break;
    }

    case GameMode::Sequencer:
    {
        auto newLayoutContext = getIdentityWithLayoutContext(false);
        newLayoutContext.transform([&](int boardIndex, int keyIndex, LumatoneKey& key) 
        { 
            AdjustLayoutColour::multiplyBrightness(render->emptyScalar, key);
        });

        queueLayout(newLayoutContext);
        controller->setContext(newLayoutContext);
        break;
    }

    default:
        break;
    }
}

void HexagonAutomata::Game::resetState()
{
    controller->allNotesOff();
    
    juce::ScopedLock l(lock);

    populatedCells.clear();
    newCells.clear();
    bornCells.clear();
    agingCells.clear();
    diedCells.clear();

    HexagonAutomata::GameState::resetState();    
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

    
}

void HexagonAutomata::Game::pauseTick()
{
    if (!lock.tryEnter())
        return;

    agingCells = juce::Array<MappedHexState>(populatedCells);
    addToQueue(renderFrame());

    
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

bool HexagonAutomata::Game::triggerCellMidi(const MappedHexState& cell)
{
    auto configKey = layoutBeforeStart.readKey(cell.boardIndex, cell.keyIndex);
    if ((configKey->keyType & 0x3) == LumatoneKeyType::disabledDefault)
        return false;

    if (cell.isAlive())
    {
        controller->sendKeyNoteOn(cell.boardIndex, cell.keyIndex, 0x70);
    }
    else
    {
        controller->sendKeyNoteOff(cell.boardIndex, cell.keyIndex, 0x0);
    }

    return true;
}

LumatoneAction* HexagonAutomata::Game::renderFrame()
{
    bool hasUpdates = bornCells.size() > 0 || agingCells.size() > 0 || newCells.size() > 0 || diedCells.size() > 0;
    if (!hasUpdates)
        return nullptr;

    juce::ScopedLock l(lock);

    juce::Array<MappedLumatoneKey> keyUpdates;
    juce::Array<MappedHexState> populated;
    
    for (auto cell : agingCells)
    {
        if (mode == GameMode::Classic)
            keyUpdates.add(render->renderCellKey(cell));
        else
            keyUpdates.add(render->renderSequencerKey(cell, layoutBeforeStart));

        auto cellNum = cell.boardIndex * layout->getOctaveBoardSize() + cell.keyIndex;
        auto lastCell = cells[cellNum];

        if (applyUpdatedCell(cell))
            populated.add(cell);

        if (mode == GameMode::Sequencer && cell.isDead() && lastCell.isAlive())
        {
            triggerCellMidi(cell);
        }

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
        if (mode == GameMode::Classic)
            keyUpdates.add(render->renderCellKey(cell));
        else
            keyUpdates.add(render->renderSequencerKey(cell, layoutBeforeStart));
            
        applyUpdatedCell(cell);

        if (verbose > 0)
            DBG("Cell " + cell.toString() + " is empty");
    }
    diedCells.clear();

    for (auto cell : bornCells)
    {
        if (mode == GameMode::Classic)
            keyUpdates.add(render->renderCellKey(cell));
        else
            keyUpdates.add(render->renderSequencerKey(cell, layoutBeforeStart));

        if (applyUpdatedCell(cell))
            populated.add(cell);

        if (mode == GameMode::Sequencer)
        {
            triggerCellMidi(cell);
        }

        if (verbose > 0)
            DBG("Cell " + cell.toString() + " was born");
    }
    bornCells.clear();

    for (auto cell : newCells)
    {
        if (mode == GameMode::Classic)
            keyUpdates.add(render->renderCellKey(cell));
        else
            keyUpdates.add(render->renderSequencerKey(cell, layoutBeforeStart));

        if (applyUpdatedCell(cell))
            populated.add(cell);

        if (verbose > 0)
            DBG("Cell " + cell.toString() + " was born");
    }
    newCells.clear();

    // updatedCells.clear();

    // TODO fix bug with managed version - loses living population after addSeed during pause
    populatedCells.swapWith(populated);

    // redoCensus();

    return new LumatoneEditAction::MultiKeyAssignAction(controller, keyUpdates, false);
}

void HexagonAutomata::Game::rerenderState()
{
	juce::ScopedLock l(lock);

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

	
}

void HexagonAutomata::Game::addSeed(Hex::Point point, bool triggerMidi)
{
	juce::ScopedLock l(lock);

    auto keyCoord = hexMap.hexToKeyCoords(point);
    HexState state(1.0f);
    auto newCell = HexagonAutomata::MappedHexState(
        state,
        layout->getMappedKey(keyCoord.boardIndex, keyCoord.keyIndex),
        point
    );

    newCell.HexState::colour = render->getAliveColour();

    if (triggerMidi && mode == GameMode::Sequencer)
    {
        triggerCellMidi(newCell);
    }

    newCells.add(newCell);
}

void HexagonAutomata::Game::addSeeds(juce::Array<Hex::Point> seedCoords, bool triggerMidi)
{
	juce::ScopedLock l(lock);

    for (auto point : seedCoords)
    {
        addSeed(point, triggerMidi);
    }
}

void HexagonAutomata::Game::addSeeds(int numSeeds, bool triggerMidi)
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
                addSeed(seed, triggerMidi);
            }
        }
    }
}

void HexagonAutomata::Game::setAliveColour(juce::Colour newColour)
{
    render->setColour(newColour);
}

juce::Colour HexagonAutomata::Game::getAliveColour() const
{
    return render->aliveColour;
}

void HexagonAutomata::Game::setDeadColour(juce::Colour newColour)
{
    render->setColour(render->aliveColour, newColour);
}

juce::Colour HexagonAutomata::Game::getDeadColour() const
{
    return render->deadColour;
}

void HexagonAutomata::Game::setBornSurviveRules(juce::Array<int> bornNums, juce::Array<int> surviveNums)
{
    juce::ScopedLock l(lock);

    rules.reset(new BornSurviveRule(bornNums, surviveNums));

    
}

void HexagonAutomata::Game::setBornSurviveRules(juce::String bornInput, juce::String surviveInput)
{
    juce::ScopedLock l(lock);

    rules.reset(new BornSurviveRule(bornInput, surviveInput));

    
}

void HexagonAutomata::Game::setNeighborDistance(int distance)
{
    juce::ScopedLock l(lock);

    auto vector = rules->getNeighborsVector(distance);
    neighborsVector.swapWith(vector);

    
}

void HexagonAutomata::Game::updateNextGeneration()
{
	juce::ScopedLock l(lock);

    juce::Array<MappedHexState> emptyCells;
    juce::HashMap<juce::String, MappedHexState> emptyNeighbors;

    for (int c = 0; c < populatedCells.size(); c++)
    {
        auto cell = populatedCells[c];

        // Check cell status
        auto neighbors = getNeighbors(cell, neighborsVector);
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
            // else if (emptyCells.addIfNotAlreadyThere((Hex::Point)neighbors[n]))
            //     emptyNeighbors.add(neighbors[n]);
        }

        if (cell.isAlive())
        {
            cell.age++;
            float healthFactor = rules->getLifeFactor(cell, neighbors.getRawDataPointer(), livingNeighbors.size());
            cell.applyFactor(healthFactor);
            agingCells.add(cell);
        }
        else if (cell.isDead())
        {
            cell.setEmpty();
            diedCells.add(cell);
        }

        // updatedCells.add(cell);
    }

    // for (int e = 0; e < emptyNeighbors.size(); e++)
    for (auto cell : emptyCells)
    {
        // auto point = emptyCells[e];
        auto neighbors = getAliveNeighbors(cell, neighborsVector);
        if (rules->generateNewLife(cell, neighbors.getRawDataPointer(), neighbors.size()))
        {
            cell.setBorn();
            cell.HexState::colour = render->renderNewbornColour(neighbors);
            bornCells.add(cell);
            // bool cellUpdated = false;
            // for (int u = 0; u < updatedCells.size(); u++)
            // {
            //     if ((Hex::Point)updatedCells[u] == (Hex::Point)cell)
            //     {
            //         updatedCells.set(u, cell);
            //         cellUpdated = true;
            //     }
            // }

            // if (!cellUpdated)
                // updatedCells.add(cell);
        }
    }

    // TODO fix above faster method - doesn't work after pausing

    // // This method is guaranteed to work
    // for (int i = 0; i < numCells; i++)
    // {
    //     auto cell = getMappedCell(i);
    //     auto neighbors = getAliveNeighbors(hexMap.keyNumToHex(i), neighborsVector);
        
    //     if (cell.isAlive())
    //     {
    //         auto healthFactor = rules->getLifeFactor(cell,neighbors.getRawDataPointer(), neighbors.size());
    //         cell.age++;
    //         cell.applyFactor(healthFactor);
    //         agingCells.add(cell);
    //     }
    //     else
    //     {
    //         if (rules->generateNewLife(cell, neighbors.getRawDataPointer(), neighbors.size()))
    //         {
    //             cell.setBorn();
    //             bornCells.add(cell);
    //         }
    //         else if (cell.isDead())
    //         {
    //             cell.age = 0;
    //             diedCells.add(cell);
    //         }
    //     }
    // }

	
}

void HexagonAutomata::Game::updateGenerationAge()
{
	juce::ScopedLock l(lock);

    for (int c = 0; c < populatedCells.size(); c++)
    // for (int i = 0; i < numCells; i++)
    {
        populatedCells.getReference(c).age++;
        agingCells.add(populatedCells[c]);
        // if (cells.getUnchecked(i).isAlive())
        // {
            // auto cell = getMappedCell(i);
            // cell.age++;
            // agingCells.add(cell);
        // }
    }

	
}

void HexagonAutomata::Game::handleAnyNoteOn(int midiChannel, int midiNote, juce::uint8 velocity)
{
    auto hexCoord = hexMap.keyCoordsToHex(midiChannel - 1, midiNote);
    addSeed(hexCoord, false);
}

void HexagonAutomata::Game::completeMappingLoaded(LumatoneLayout layout)
{
    // this is fine because we update via section update actions
    // so this should only be triggered by loading a new layout

    layoutBeforeStart = layout;
    reset(true);
}
