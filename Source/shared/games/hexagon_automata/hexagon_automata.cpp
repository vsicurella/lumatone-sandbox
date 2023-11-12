#include "hexagon_automata.h"

#include "./hexagon_automata_renderer.h"
#include "./hexagon_automata_rules.h"
#include "./hexagon_automata_launcher.h"

#include "../../lumatone_editor_library/LumatoneController.h"
#include "../../lumatone_editor_library/actions/edit_actions.h"

#include "../../lumatone_editor_library/color/adjust_layout_colour.h"

HexagonAutomata::Game::Game(juce::ValueTree engineStateIn, LumatoneController* controller)
    : LumatoneSandboxGameBase(controller, "Hexagon Automata")
    , HexagonAutomata::State(controller->shareMappingData(), engineStateIn)
    , LumatoneSandboxLogger("Hexagon Automata")
{
    initialize();
}

void HexagonAutomata::Game::initialize()
{
    if (rules.get() == nullptr)
    {
        setBornSurviveRules(bornRules, surviveRules);
    }

    numCells = controller->getNumBoards() * controller->getOctaveBoardSize();
    
    cells.resize(numCells);

    neighborsVector = rules->getNeighborsVector(1);

    if (render.get() == nullptr)
    {
        render.reset(new Renderer);
    }
}

void HexagonAutomata::Game::reset(bool clearQueue)
{
    loadStateProperties(state);

    LumatoneSandboxGameBase::reset(clearQueue);
    resetState();

    random.setSeedRandomly();

    ticks = 0;
    ticksToNextCellUpdate = 0;

    switch (gameMode)
    {
    case GameMode::Classic:
    {
        auto newLayout = getIdentityLayout(true, getDeadColour());
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

    case GameMode::None:
    default:
        jassertfalse;
    }
}

void HexagonAutomata::Game::resetState()
{
    clearAllCells();

    ticks = 0;
    ticksToNextSyncCellUpdate = 0;
    ticksToNextCellUpdate = 0;
    
    populatedCells.clear();
    newCells.clear();

    HexagonAutomata::BoardState::resetState();    
}

void HexagonAutomata::Game::nextTick()
{
    juce::ScopedTryLock l(currentFrameCells.getLock());

    if (!l.isLocked())
    {
        return;
    }

    // if (ticksToNextCellUpdate >= ticksPerCellUpdate)
    // {       
        // ticksToNextCellUpdate = 0;
        // DBG("Update cell states on tick: " + juce::String(ticks));
        updateCellStates();
    // }
    // else
    // {
    //     ticksToNextCellUpdate++; 
    // }

    if (currentFrameCells.size() > 0)
    {
        addToQueue(renderFrame());
        // logInfo("nextTick", "added " + juce::String(juce::jmin(currentFrameCells.size(), maxUpdatesPerFrame)) + " cell updates to queue;");
        currentFrameCells.removeRange(0, maxUpdatesPerFrame);
    }

    ticks++;
}

void HexagonAutomata::Game::pauseTick()
{
    juce::ScopedTryLock l(currentFrameCells.getLock());
    if (!l.isLocked())
        return;

    updateNewCells();
    addToQueue(renderFrame());
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

LumatoneAction* HexagonAutomata::Game::renderFrame() const
{
    juce::ScopedTryLock l(currentFrameCells.getLock());
    if (!l.isLocked())
        return nullptr;

    juce::Array<MappedLumatoneKey> keyUpdates;
    int limit = juce::jmin(maxUpdatesPerFrame, currentFrameCells.size());

    for (int i = 0; i < limit; i++)
    {
        const MappedHexState& update = currentFrameCells.getReference(i);

        switch (gameMode)
        {
        case GameMode::Classic:
            keyUpdates.add(render->renderCellKey(update));
            break;

        case GameMode::Sequencer:
            keyUpdates.add(render->renderSequencerKey(update, layoutBeforeStart));
            break;

        case GameMode::None:
        default:
            jassertfalse;
        }
    }

    if (keyUpdates.size())
        return new LumatoneEditAction::MultiKeyAssignAction(controller, keyUpdates, false);

    return nullptr;
}

void HexagonAutomata::Game::addSeed(Hex::Point point, bool triggerMidi)
{
    auto keyCoord = hexMap.hexToKeyCoords(point);
    HexState state(1.0f);
    auto newCell = HexagonAutomata::MappedHexState(
        state,
        layout->getMappedKey(keyCoord.boardIndex, keyCoord.keyIndex),
        point
    );

    if (gameMode == GameMode::Classic)
        newCell.HexState::colour = aliveColour;

    if (triggerMidi && gameMode == GameMode::Sequencer)
    {
        triggerCellMidi(newCell);
    }

    newCells.add(newCell);
}

void HexagonAutomata::Game::addSeeds(juce::Array<Hex::Point> seedCoords, bool triggerMidi)
{
	juce::ScopedLock l(newCells.getLock());

    for (auto point : seedCoords)
    {
        addSeed(point, triggerMidi);
    }
}

void HexagonAutomata::Game::addSeeds(int numSeeds, bool triggerMidi)
{
    juce::Array<Hex::Point> neighborVecCopy;
    {
        juce::ScopedLock l(neighborsVector.getLock());
        neighborVecCopy.addArray(neighborsVector);
    }

	juce::ScopedLock l(newCells.getLock());

    float probablity = 0.5f;

    for (int i = 0; i < numSeeds; i++)
    {
        int keyNum = random.nextInt(numCells);

        auto hexCoord = hexMap.keyNumToHex(keyNum);
        
        juce::Array<Hex::Point> cellPoints;
        cellPoints.add(hexCoord);

        for (const Hex::Point& vec : neighborVecCopy)
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

void HexagonAutomata::Game::clearCell(Hex::Point coord, bool triggerMidi)
{
    auto cellNum = hexMap.hexToKeyNum(coord);
    auto cell = getMappedCell(cellNum);
    clearCell(cell, triggerMidi);
}

void HexagonAutomata::Game::clearCell(MappedHexState &cell, bool triggerMidi)
{
    cell.setDead();
    applyUpdatedCell(cell);
    triggerCellMidi(cell);

    juce::ScopedLock l(populatedCells.getLock());

    for (int i = 0; i < populatedCells.size(); i++)
    {
        if (static_cast<Hex::Point>(populatedCells[i]) == static_cast<Hex::Point>(cell))
        {
            populatedCells.remove(i);
            break;
        }
    }

    currentFrameCells.add(cell);
}

void HexagonAutomata::Game::clearAllCells(bool triggerMidi)
{
    juce::ScopedLock cellLock(populatedCells.getLock());
    juce::ScopedLock frameLock(currentFrameCells.getLock());

    for (int i = 0; i < populatedCells.size(); i++)
    {
        auto cell = populatedCells.removeAndReturn(0);
        cell.setDead();

        applyUpdatedCell(cell);
        
        if (triggerMidi)
            triggerCellMidi(cell);
        
        currentFrameCells.add(cell);
    }
}

void HexagonAutomata::Game::setGameMode(GameMode modeIn)
{
    if (gameMode == modeIn)
        return;

    HexagonAutomata::State::setGameMode(modeIn);
    reset(true);
}

void HexagonAutomata::Game::setGenerationMode(GenerationMode newMode)
{
    if (generationMode == newMode)
        return;

    HexagonAutomata::State::setGenerationMode(newMode);

    if (newMode == HexagonAutomata::GenerationMode::Synchronous)
    {
        ticksToNextSyncCellUpdate = 0;
    }
}

void HexagonAutomata::Game::setAliveColour(juce::Colour newColour)
{
    HexagonAutomata::State::setAliveColour(newColour);
    render->setColour(aliveColour, deadColour);
}

void HexagonAutomata::Game::setDeadColour(juce::Colour newColour)
{
    HexagonAutomata::State::setDeadColour(deadColour);
    render->setColour(aliveColour, deadColour);
}

void HexagonAutomata::Game::setBornSurviveRules(juce::String bornInput, juce::String surviveInput)
{
    HexagonAutomata::State::setBornSurviveRules(bornInput, surviveInput);

    if (rules != nullptr)
        {
        juce::ScopedLock l(rules->getLock());
        rules.reset(new HexagonAutomata::BornSurviveRule(bornRules, surviveRules));
        return;
        }

    rules = std::make_unique<BornSurviveRule>(bornRules, surviveRules);
}

// void HexagonAutomata::Game::setNeighborDistance(int distance)
// {
//     HexagonAutomata::State::setNeighborDistance(distance);
// }

juce::ValueTree HexagonAutomata::Game::loadStateProperties(juce::ValueTree stateIn)
{
    juce::ValueTree newState = (stateIn.hasType(gameId)) 
                             ? stateIn
                             : juce::ValueTree(gameId);

    LumatoneGameBaseState::loadStateProperties(newState);

    for (auto property : HexagonAutomata::GetStateProperties())
    {
        handleStatePropertyChange(newState, property);
    }
    
    return newState;
}

void HexagonAutomata::Game::handleStatePropertyChange(juce::ValueTree stateIn, const juce::Identifier &property)
{
    HexagonAutomata::State::handleStatePropertyChange(stateIn, property);
    
    if (property == HexagonAutomata::ID::GameMode)
    {
        setGameMode(gameMode);
    }
    else if (property == HexagonAutomata::ID::GenerationMode)
    {
        setGenerationMode(generationMode);
    }
    else if (property == HexagonAutomata::ID::SyncGenTime)
    {
        render->setMaxAge(ticksPerSyncGeneration * 6);
    }
    else if (property == HexagonAutomata::ID::AsyncGenTime)
    {
        render->setMaxAge(ticksPerSyncGeneration * 6);
    }
    else if (property == HexagonAutomata::ID::AliveColour)
    {
        render->setColour(aliveColour, deadColour);
    }
    else if (property == HexagonAutomata::ID::DeadColour)
    {
        render->setColour(aliveColour, deadColour);
    }
    else if (property == HexagonAutomata::ID::BornRule)
    {
        juce::ScopedLock l(rules->getLock());
        rules.reset(new BornSurviveRule(bornRules, surviveRules));
    }
    else if (property == HexagonAutomata::ID::SurviveRule)
    {
        juce::ScopedLock l(rules->getLock());
        rules.reset(new BornSurviveRule(bornRules, surviveRules));
    }
    else if (property == HexagonAutomata::ID::NeighborShape)
    {
        
    }
}

void HexagonAutomata::Game::updateNewCells()
{
    for (int i = 0; i < newCells.size(); i++)
    {
        const MappedHexState& cell = newCells.getReference(i); 
        populatedCells.add(cell);
        currentFrameCells.add(cell);
    }
    newCells.clear();
}

void HexagonAutomata::Game::updateCellStates()
{
    updateNewCells();

    juce::ScopedLock l(rules->getLock());

    juce::Array<MappedHexState> cellsToUpdate;

    // First, make references to populated cells and empty cells

    // Keep track of cells so we don't add duplicates
    juce::Array<MappedHexState> emptyCells;
    juce::HashMap<juce::String, MappedHexState> emptyNeighbors;

    for (int c = 0; c < populatedCells.size(); c++)
    {
        MappedHexState& cell = populatedCells.getReference(c);

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
        }

        // Also advance age of populated cells
        // In Asynchronous mode we will later mark ones past maxAge as dead
        // Can also be used for render effects
        cell.age++;
        applyUpdatedCell(cell);
    }

    // Synchronous mode will ignore cell ages and perform
    // logic on the cell states altogether 
    if (generationMode == GenerationMode::Synchronous)
    {
        if (ticksToNextSyncCellUpdate >= ticksPerSyncGeneration)
        {
            ticksToNextSyncCellUpdate = 0;    
        }
        else
        {
            ticksToNextSyncCellUpdate++;
            return;
        }

        // Determine cells that should be born
        for (auto cell : emptyCells)
        {
            auto neighbors = getAliveNeighbors(cell, neighborsVector);
            if (rules->generateNewLife(cell, neighbors.getRawDataPointer(), neighbors.size()))
            {
                cell.setBorn();
                cell.HexState::colour = render->renderNewbornColour(neighbors);
                cellsToUpdate.add(cell);
            }
        }
    }

    juce::Array<MappedHexState> nextGenCells; // For Async mode

    // Now check for surviving cells
    float healthMult;
    for (auto cell : populatedCells)
    {
        if (generationMode == GenerationMode::Asynchronous)
        {
            if (cell.age < ticksPerSyncGeneration)
                continue;

            else if (cell.age % ticksPerSyncGeneration == 0)
            {
                nextGenCells.add(cell);
            }
            else
                continue;
        }

        auto neighbors = getAliveNeighbors(cell, neighborsVector);

        healthMult = rules->getLifeFactor(cell, neighbors.getRawDataPointer(), neighbors.size());
        if (healthMult == 0.0f)
        {
            cell.setDead();
            cellsToUpdate.add(cell);
        }
        else
        {
            cell.applyFactor(healthMult);
        }
    }

    if (generationMode == GenerationMode::Asynchronous)
    {
        // Keep track of cells so we don't add duplicates
        juce::HashMap<juce::String, MappedHexState> bornNeighbors;

        for (auto genCell : nextGenCells)
        {
            // Find neighbors that can be born
            for (auto emptyCell : emptyCells)
            {
                auto cellHash = emptyCell.toString();
                if (bornNeighbors[cellHash].boardIndex >= 0)
                    continue;

                if (genCell.distanceTo(emptyCell) == 1)
                {
                    auto neighbors = getAliveNeighbors(emptyCell, neighborsVector);
                    if (rules->generateNewLife(emptyCell, neighbors.getRawDataPointer(), neighbors.size()))
                    {
                        emptyCell.setBorn();
                        emptyCell.HexState::colour = render->renderNewbornColour(neighbors);
                        cellsToUpdate.add(emptyCell);
                        bornNeighbors.set(cellHash, emptyCell);
                    }
                }
            }
        }
    }

    for (auto cell : cellsToUpdate)
    {
        applyUpdatedCell(cell);

        // Add born cells to population cache
        if (cell.isAlive() && cell.age == 0)
            populatedCells.add(cell);

        // Remove dead cells from population cache
        else if (cell.isDead()) for (int i = 0; i < populatedCells.size(); i++)
        {
            const MappedHexState& compare = populatedCells.getReference(i);
            if (compare.boardIndex == cell.boardIndex
             && compare.keyIndex == cell.keyIndex
            //  && compare.colourIsEqual(static_cast<LumatoneKey>(cell))
            )
            {
                populatedCells.remove(i);
                break;
            }
        }

        if (gameMode == GameMode::Sequencer)
        {
            triggerCellMidi(cell);
        }
    }

    currentFrameCells.addArray(cellsToUpdate);
}

void HexagonAutomata::Game::handleAnyNoteOn(int midiChannel, int midiNote, juce::uint8 velocity)
{
    auto hexCoord = hexMap.keyCoordsToHex(midiChannel - 1, midiNote);
    int cellNum = hexMap.hexToKeyNum(hexCoord);
    auto cell = getMappedCell(cellNum);

    if (cell.isAlive())
    {
        // clearCell(cell);
        cell.setBorn();
        triggerCellMidi(cell);
        applyUpdatedCell(cell);
    }
    else
        addSeed(hexCoord, true);
}

void HexagonAutomata::Game::completeMappingLoaded(LumatoneLayout layout)
{
    // this is fine because we update via section update actions
    // so this should only be triggered by loading a new layout

    layoutBeforeStart = layout;
    reset(true);
}

LumatoneSandboxGameComponent* HexagonAutomata::Game::createController()
{
    return new HexagonAutomata::Component(this);
}
