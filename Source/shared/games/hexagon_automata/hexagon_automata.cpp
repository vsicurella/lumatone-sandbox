#include "hexagon_automata.h"

#include "./hexagon_automata_renderer.h"
#include "./hexagon_automata_rules.h"
#include "./hexagon_automata_launcher.h"

#include "../../lumatone_editor_library/LumatoneController.h"
#include "../../lumatone_editor_library/actions/edit_actions.h"

#include "../../lumatone_editor_library/color/adjust_layout_colour.h"

HexagonAutomata::Game::Game(LumatoneGameEngineState& gameEngineStateIn, LumatoneController* controller)
    : LumatoneSandboxGameBase(controller, "Hexagon Automata")
    , HexagonAutomata::State(controller->shareMappingData(), gameEngineStateIn)
{
    initialize();
}

HexagonAutomata::Game::~Game()
{
    clearAllCells(true);
}

void HexagonAutomata::Game::initialize()
{
    if (rules.get() == nullptr)
    {
        setBornSurviveRules(bornRules, surviveRules);
    }

    numCells = controller->getNumBoards() * controller->getOctaveBoardSize();
    
    cells.resize(numCells);

    if (render.get() == nullptr)
    {
        render.reset(new Renderer);
    }
}

bool HexagonAutomata::Game::reset(bool clearQueue)
{
    loadStateProperties(state);

    LumatoneSandboxGameBase::reset(clearQueue);
    resetState();

    random.setSeedRandomly();

    ticks = 0;

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
        return false;
    }

    return true;
}

void HexagonAutomata::Game::resetState()
{
    clearAllCells();

    ticks = 0;

    {
        juce::ScopedLock il(inputLock);
        newCells.clear();
    }

    HexagonAutomata::BoardState::resetState();    
}

bool HexagonAutomata::Game::nextTick()
{
    if (quitGame)
    {
        quit();
        return false;
    }

    juce::ScopedTryLock fl(frameLock);
    if (!fl.isLocked())
    {
        logSkippedFrame("nextFrame");
        return true;
    }

    numUpdatesInFrame = maxUpdatesPerFrame > 0 
                      ? juce::jmin(maxUpdatesPerFrame, currentFrameCells.size())
                      : currentFrameCells.size();

    if (!noSustainPassThrough || sustainIsOn)
    {
        if (clockMode == HexagonAutomata::ClockMode::Engine || clockFlag)
        {
            updateCellStates();
            addFramesToQueue();
            clockFlag = false;
        }
        else
        {
            updateUserInputCells();
            addFramesToQueue();
        }
    }
    else
    {
        updateUserInputCells();
        addFramesToQueue();
    }

    ticks++;
    return true;
}

bool HexagonAutomata::Game::pauseTick()
{
    if (quitGame)
    {
        quit();
        return false;
    }

    juce::ScopedTryLock fl(frameLock);
    if (!fl.isLocked())
    {
        logInfo("pauseTick", "Skipping frame, failed to acquire lock.");
        return true;
    }

    updateUserInputCells();

    addFramesToQueue();

    return true;
}

bool HexagonAutomata::Game::applyUpdatedCell(const HexagonAutomata::MappedHexState& cell)
{
    juce::ScopedLock sl(stateLock);
    
    auto cellNum = layout->keyCoordToKeyNum(cell.getKeyCoord());
    if (cellNum >= 0)
    {
        // juce::String cellNumStr;
        // if (cell.isDead())
        //     cellNumStr = "d";
        // cellNumStr += juce::String(cellNum);

        // if (cell.health != cells[cellNum].health)
        //     logInfo("applyUpdatedCell", "Updated cell: " + cellNumStr + " (" + cell.toString() + ")");

        cells.set(cellNum, cell);
        if (cell.isDead() || cell.isAlive())
            return true;
    }
    else
    {
        logWarning("applyUpdatedCell", "Ignoring cell update: " + cell.toString());
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
    juce::ScopedTryLock fl(frameLock);
    if (!fl.isLocked())
    {
        logSkippedFrame("renderFrame");
        return nullptr;
    }

    juce::Array<MappedLumatoneKey> keyUpdates;
    // int limit = juce::jmin(maxUpdatesPerFrame, currentFrameCells.size());
    // int limit = maxUpdatesPerFrame > 0 ? juce::jmin(maxUpdatesPerFrame, currentFrameCells.size()) : currentFrameCells.size();

    for (int i = 0; i < numUpdatesInFrame; i++)
    {
        const MappedHexState& update = currentFrameCells.getReference(i);
        MappedLumatoneKey renderedKey;

        switch (gameMode)
        {
        case GameMode::Classic:
            renderedKey = render->renderCellKey(update);
            break;

        case GameMode::Sequencer:
            renderedKey = render->renderSequencerKey(update, layoutBeforeStart);
            break;

        case GameMode::None:
        default:
            jassertfalse;
        }

        if (!renderedKey.isEmpty())
        {
            keyUpdates.add(renderedKey);
        }
    }

    if (keyUpdates.size())
        return new LumatoneEditAction::MultiKeyAssignAction(controller, keyUpdates, false);

    return nullptr;
}

void HexagonAutomata::Game::addSeed(Hex::Point point, float healthIn, bool triggerMidi)
{
    auto keyCoord = hexMap.hexToKeyCoords(point);
    HexState state(1.0f);
    auto newCell = HexagonAutomata::MappedHexState(
        state,
        point,
        layout->getMappedKey(keyCoord.boardIndex, keyCoord.keyIndex),
        hexMap
    );

    newCell.health = healthIn;

    if (triggerMidi && gameMode == GameMode::Sequencer)
    {
        // logInfo("addSeed", "triggerCellMidi");
        triggerCellMidi(newCell);
    }

    {
        juce::ScopedLock il(inputLock);
        newCells.add(newCell);
    }
}

void HexagonAutomata::Game::addSeeds(juce::Array<Hex::Point> seedCoords, bool triggerMidi)
{
	juce::ScopedLock l(inputLock);

    for (auto point : seedCoords)
    {
        addSeed(point, 1.0f, triggerMidi);
    }
}

void HexagonAutomata::Game::addSeeds(int numSeeds, bool triggerMidi)
{
    juce::Array<Hex::Point> neighborVecCopy;
    {
        juce::ScopedLock fl(frameLock);
        neighborVecCopy = rules->getNeighborsShape();
    }

    juce::ScopedLock il(inputLock);

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
                float health = random.nextFloat();
                addSeed(seed, health, triggerMidi);
            }
        }
    }
}

void HexagonAutomata::Game::clearCell(const MappedHexState &cell, bool triggerMidi)
{
    juce::ScopedLock il(inputLock);

    auto deadCell = cell;
    deadCell.setDead();
    clearedCells.add(deadCell);

    if (triggerMidi)
    {
        // logInfo("clearCell", "triggerCellMidi");
        triggerCellMidi(deadCell);
    }
}

void HexagonAutomata::Game::clearAllCells(bool triggerMidi)
{
    juce::ScopedLock il(inputLock);
    for (const MappedHexState& cell : populatedCells)
    {
        auto deadCell = cell;
        deadCell.setDead();
        clearedCells.add(deadCell);

        if (triggerMidi)
        {
            // logInfo("clearAllCells", "triggerCellMidi");
            triggerCellMidi(deadCell);
        }
    }
}

void HexagonAutomata::Game::setGameMode(GameMode modeIn)
{
    if (gameMode == modeIn)
        return;

    HexagonAutomata::State::setGameMode(modeIn);
    quitGame = true;
}

void HexagonAutomata::Game::setGenerationMode(GenerationMode newMode)
{
    if (generationMode == newMode)
        return;

    HexagonAutomata::State::setGenerationMode(newMode);
}

void HexagonAutomata::Game::setRulesMode(RulesMode modeIn)
{
    HexagonAutomata::State::setRulesMode(modeIn);

    switch (rulesMode)
    {
    default:
    case HexagonAutomata::RulesMode::BornSurvive:
        setBornSurviveRules(bornRules, surviveRules);
        break;
    case HexagonAutomata::RulesMode::SpiralRule:
        setSpiralRule();
        break;
    case HexagonAutomata::RulesMode::BzReactionRule:
        setReactionRule(64, 2, 3, 5);
        break;
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

    if (rulesMode == HexagonAutomata::RulesMode::BornSurvive)
    {
        if (rules != nullptr && (bornInput != bornRules || surviveInput != surviveRules))
            {
            juce::ScopedLock l(rules->getLock());
            rules.reset(new HexagonAutomata::BornSurviveRule(bornRules, surviveRules));
            return;
            }

        rules = std::make_unique<BornSurviveRule>(bornRules, surviveRules);
    }
}

void HexagonAutomata::Game::setSpiralRule()
{
    logInfo("setSpiralRule", "");

    // HexagonAutomata::Game::setRulesMode(HexagonAutomata::RulesMode::SpiralRule);
    
    if (rules != nullptr)
    {
        juce::ScopedLock l(rules->getLock());
        rules.reset(new HexagonAutomata::SpiralRule());
        return;
    }

    rules = std::make_unique<HexagonAutomata::SpiralRule>();
}

void HexagonAutomata::Game::setReactionRule(int numStates, int k1, int k2, int speed)
{
    logInfo("setReactionRule", "");
    
    if (rules != nullptr)
    {
        juce::ScopedLock l(rules->getLock());
        rules.reset(new HexagonAutomata::BzReactionRule(numStates, k1, k2, speed));
        return;
    }

    rules = std::make_unique<HexagonAutomata::BzReactionRule>(numStates, k1, k2, speed);
}

void HexagonAutomata::Game::logSkippedFrame(juce::String method) const
{
    logWarning(method, "Skipping frame, failed to acquire lock.");
}

void HexagonAutomata::Game::logCellState(juce::String method, juce::String message, const MappedCellStates &states) const
{
    juce::String stateString = message + ": ";
    for (const MappedHexState& cell : states)
    {
        auto cellNum = hexMap.hexToKeyNum(cell);
        juce::String cellStr;
        if (cell.isDead())
            cellStr = "d";
        cellStr = juce::String(cellNum);

        stateString += cellStr + ",";
    }

    logInfo(method, stateString);
}

// juce::ValueTree HexagonAutomata::Game::loadStateProperties(juce::ValueTree stateIn)
// {
//     juce::ValueTree newState = (stateIn.hasType(gameId)) 
//                              ? stateIn
//                              : juce::ValueTree(gameId);

//     LumatoneGameBaseState::loadStateProperties(newState);

//     for (auto property : HexagonAutomata::GetStateProperties())
//     {
//         handleStatePropertyChange(newState, property);
//     }
    
//     return newState;
// }

void HexagonAutomata::Game::handleStatePropertyChange(juce::ValueTree stateIn, const juce::Identifier &property)
{
    if (property == HexagonAutomata::ID::GameMode)
    {
        auto value = state.getProperty(property, juce::var(GameModeToString(GameMode::Classic)));
        auto readMode = GameModeFromString(value.toString());
        setGameMode(readMode);
    }
    else if (property == HexagonAutomata::ID::GenerationMode)
    {
        auto value = state.getProperty(property, juce::var(GenerationModeToString(GenerationMode::Synchronous)));
        auto readMode = GenerationModeFromString(value.toString());
        setGenerationMode(readMode);
    }
    else if (property == HexagonAutomata::ID::GenerationMs)
    {
        setGenerationMs(static_cast<float>(state.getProperty(property, 1000.0f)));
    }
    else if (property == HexagonAutomata::ID::RulesMode)
    {
        auto value = state.getProperty(property, juce::var(RulesModeToString(RulesMode::BornSurvive)));
        RulesMode rulesMode = RulesModeFromString(value.toString());
        setRulesMode(rulesMode);
    }
    else if (property == HexagonAutomata::ID::BornRule)
    {
        auto bornRulesInput = state.getProperty(HexagonAutomata::ID::BornRule, "2");
        auto surviveRulesInput = state.getProperty(HexagonAutomata::ID::SurviveRule, "3,4");
        setBornSurviveRules(bornRulesInput, surviveRulesInput);
    }
    else if (property == HexagonAutomata::ID::SurviveRule)
    {
        auto bornRulesInput = state.getProperty(HexagonAutomata::ID::BornRule, "2");
        auto surviveRulesInput = state.getProperty(HexagonAutomata::ID::SurviveRule, "3,4");
        setBornSurviveRules(bornRulesInput, surviveRulesInput);
    }
    else
    {
        HexagonAutomata::State::handleStatePropertyChange(stateIn, property);

        if (property == HexagonAutomata::ID::AliveColour)
        {
            render->setColour(aliveColour, deadColour);
        }
        else if (property == HexagonAutomata::ID::DeadColour)
        {
            render->setColour(aliveColour, deadColour);
        }
    }
}

void HexagonAutomata::Game::addToPopulation(MappedHexState &cell, MappedCellStates& population)
{
    render->renderCellColour(cell);
    population.add(cell);
}

bool HexagonAutomata::Game::removeFromPopulation(MappedHexState& cell, MappedCellStates& population)
{
    for (int i = 0; i < population.size(); i++)
    {
        const MappedHexState& compare = population.getReference(i);
        if (compare.boardIndex == cell.boardIndex
         && compare.keyIndex == cell.keyIndex
            )
        {
            render->renderCellColour(cell);
            population.remove(i);
            return true;
        }
    }

    return false;
}

void HexagonAutomata::Game::updateUserInputCells()
{
    juce::ScopedTryLock il(inputLock);
    if (!il.isLocked())
        return;

    int numNewCells = newCells.size();
    for (int i = 0; i < numNewCells; i++)
    {
        MappedHexState& cell = newCells.getReference(i); 
        addToPopulation(cell, populatedCells);
        applyUpdatedCell(cell);
        currentFrameCells.add(cell);
    }
    newCells.removeRange(0, numCells);

    int numClearedCells = clearedCells.size();
    for (int i = 0; i < numClearedCells; i++)
    {
        auto cell = clearedCells.getReference(i);
        if (removeFromPopulation(cell, populatedCells))
        {
            applyUpdatedCell(cell);
            currentFrameCells.add(cell);
        }
    }
    clearedCells.removeRange(0, numClearedCells);
}

void HexagonAutomata::Game::updateCellStates()
{
    nextPopulation.clearQuick();

    {
        juce::ScopedLock sl(stateLock);
        for (MappedHexState& cell : populatedCells)
        {
            cell.age++;
            // applyUpdatedCell(cell);
        }
    }

    updateUserInputCells();

    juce::ScopedLock l(rules->getLock());

    auto bornCells = rules->getNewCells(*static_cast<State*>(this), populatedCells);
    for (MappedHexState& newCell : bornCells)
    {
        if (gameMode == GameMode::Sequencer)
        {
            // logInfo("updateCellStates (bornCells)", "triggerCellMidi");
            triggerCellMidi(newCell);
        }

        addToPopulation(newCell, nextPopulation);
        currentFrameCells.add(newCell);
    }
    // if (bornCells.size())
    // {
    //     logCellState("updateCellStates", "Added born cells", bornCells);
    // }

    auto updatedCells = rules->getUpdatedCells(*static_cast<State*>(this), populatedCells);
    for (MappedHexState& update : updatedCells)
    {
        if (update.isAlive())
        {
            addToPopulation(update, nextPopulation);
        }

        const HexState& current = cells[update.cellNum];
        if (current.age != update.age || current.health != update.health)
        {
            render->renderCellColour(update);
            applyUpdatedCell(update);

            if (update.cellColor != current.cellColor)
                currentFrameCells.add(update);

            if (update.isDead())
            {
                if (gameMode == GameMode::Sequencer)
                {
                    // logInfo("updateCellStates (updatedCells)", "triggerCellMidi");
                    triggerCellMidi(update);
                }

            }
        }

        // if (update.isAlive())
        // {
        //     addToPopulation(update, nextPopulation);

        //     auto cellNum = hexMap.hexToKeyNum(update);
        //     const HexState& current = cells[cellNum];
            
        //     if (rulesMode == RulesMode::BzReactionRule && current != update)
        //     {
        //         render->renderCellColour(update);
        //         currentFrameCells.add(update);
        //         applyUpdatedCell(update);
        //     }
        // }
        // else
        // {
        //     if (gameMode == GameMode::Sequencer)
        //     {
        //         // logInfo("updateCellStates (updatedCells)", "triggerCellMidi");
        //         triggerCellMidi(update);
        //     }

        //     // move these below this scope for render effects
        //     render->renderCellColour(update);
        //     currentFrameCells.add(update);
        //     applyUpdatedCell(update);
        // }
    }
    // if (updatedCells.size())
    // {
    //     logCellState("updateCellStates", "Updated cells",  updatedCells);
    // }

    juce::ScopedLock sl(stateLock);

    // Apply updated states
    for (MappedHexState& cell : nextPopulation)
    {
        applyUpdatedCell(cell);
    }

    populatedCells.swapWith(nextPopulation);
}

void HexagonAutomata::Game::addFramesToQueue()
{
    if (currentFrameCells.size() > 0)
    {
        addToQueue(renderFrame());
        // logInfo("nextTick", "added " + juce::String(juce::jmin(currentFrameCells.size(), maxUpdatesPerFrame)) + " cell updates to queue;");
    
        currentFrameCells.removeRange(0, numUpdatesInFrame);
    }
}

void HexagonAutomata::Game::handleAnyNoteOn(int midiChannel, int midiNote, juce::uint8 velocity)
{
    auto hexCoord = hexMap.keyCoordsToHex(midiChannel - 1, midiNote);
    int cellNum = hexMap.hexToKeyNum(hexCoord);

    juce::ScopedLock sl(stateLock);
    MappedHexState cell = getMappedCell(cellNum);

    logInfo("handleAnyNoteOn", "Note on " + juce::String(midiChannel) + "," + juce::String(midiNote) + " triggering cell " + juce::String(cellNum));

    float velocityFloat = (float)velocity / 127.0f;
    if (rulesMode == RulesMode::SpiralRule)
        velocity = powf(velocity, 3);

    if (cell.isAlive())
    {
        // clearCell(cell);
        cell.setBorn(velocityFloat);

        if (gameMode == HexagonAutomata::GameMode::Sequencer)
        {
            triggerCellMidi(cell);
        }
        
        applyUpdatedCell(cell);
    }
    else
        addSeed(hexCoord, velocityFloat, true);
}

void HexagonAutomata::Game::handleAnyNoteOff(int midiChannel, int midiNote)
{
    if (noSustainPassThrough && !sustainIsOn)
    {
        auto hexCoord = hexMap.keyCoordsToHex(midiChannel - 1, midiNote);
        int cellNum = hexMap.hexToKeyNum(hexCoord);

        juce::ScopedLock sl(stateLock);
        MappedHexState cell = getMappedCell(cellNum);

        clearCell(cell, true);
    }
}

void HexagonAutomata::Game::handleAnyController(int channel, int ccNum, juce::uint8 value)
{
    if (ccNum == 64)
    {
        handleSustain(value >= 0x40);
    }
}

void HexagonAutomata::Game::handleMidiClock(int quarterNoteInterval)
{
    if (clockMode != HexagonAutomata::ClockMode::MidiClockClient)
        return;

    juce::ScopedTryLock fl(frameLock);
    if (!fl.isLocked())
    {
        logWarning("handleMidiClock", "Skipped because frame lock could not be acquired");
        return;
    }

    if (quarterNoteInterval != ticksPerQuarterNote)
    {
        ticksPerQuarterNote = quarterNoteInterval;
        updateGenerationClockTime();
    }

    // logInfo("handleMidiClock", "");
    // sendClockSignal();
    clockFlag = true;
}

void HexagonAutomata::Game::handleSustain(bool toggled)
{
    sustainIsOn = toggled;

    if (noSustainPassThrough && !sustainIsOn)
    {
        clearAllCells();
    }
}

void HexagonAutomata::Game::completeMappingLoaded(LumatoneLayout layout)
{
    LumatoneSandboxGameBase::completeMappingLoaded(layout);
    quitGame = true;
}

LumatoneSandboxGameComponent* HexagonAutomata::Game::createController()
{
    return new HexagonAutomata::Component(this);
}
