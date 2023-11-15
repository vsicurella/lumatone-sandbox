#include "hexagon_automata_game_state.h"

HexagonAutomata::State::State(std::shared_ptr<LumatoneLayout> layoutIn, juce::ValueTree engineStateIn)
    : LumatoneGameBaseState(LumatoneSandbox::GameName::HexagonAutomata, HexagonAutomata::ID::GameId, engineStateIn)
    , HexagonAutomata::BoardState(layoutIn)
{

}

void HexagonAutomata::State::setGameMode(GameMode modeIn)
{
    gameMode = modeIn;
    writeStringProperty(HexagonAutomata::ID::GameMode, GameModeToString(gameMode));
}

void HexagonAutomata::State::setGenerationMode(GenerationMode newMode)
{
    generationMode = newMode;
    writeStringProperty(HexagonAutomata::ID::GenerationMode, GenerationModeToString(generationMode));
}

void HexagonAutomata::State::setRulesMode(RulesMode newMode)
{
    rulesMode = newMode;
    writeStringProperty(HexagonAutomata::ID::RulesMode, RulesModeToString(rulesMode));
}

void HexagonAutomata::State::setAliveColour(juce::Colour newColour)
{
    aliveColour = newColour;
    writeStringProperty(HexagonAutomata::ID::AliveColour, aliveColour.toString());
}

void HexagonAutomata::State::setDeadColour(juce::Colour newColour)
{
    deadColour = newColour;
    writeStringProperty(HexagonAutomata::ID::DeadColour, deadColour.toString());
}

void HexagonAutomata::State::setBornSurviveRules(juce::String bornInputIn, juce::String surviveInputIn)
{
    bornRules = bornInputIn;
    writeStringProperty(HexagonAutomata::ID::BornRule, bornRules);

    surviveRules = surviveInputIn;
    writeStringProperty(HexagonAutomata::ID::SurviveRule, surviveRules);
}

void HexagonAutomata::State::setNeighborDistance(int distance)
{
    // TODO make this a hex shape pattern instead of static distance
    neighborsShape = juce::String(distance);
    writeStringProperty(HexagonAutomata::ID::NeighborShape, neighborsShape);
}

void HexagonAutomata::State::setGenerationMs(float msecValue)
{
    syncGenerationMs = msecValue;
    updateGenerationTickRate();
    writeStringProperty(HexagonAutomata::ID::SyncGenTime, juce::String(syncGenerationMs));
}

void HexagonAutomata::State::setGenerationBpm(float bpmValue)
{
    float ms = engineState.bpmToMsec(bpmValue);
    // logInfo("setGenerationBpm", "BPM of " + juce::String(bpmValue) + " converted to " + juce::String(ms) + "ms");
    setGenerationMs(ms);
}

// void HexagonAutomata::State::setTicksPerSyncGeneration(int ticks)
// {
//     ticksPerSyncGeneration = ticks;
//     writeIntProperty(HexagonAutomata::ID::SyncGenTime, ticksPerSyncGeneration);
// }

// void HexagonAutomata::State::setTicksPerAsyncGeneration(int ticks)
// {
//     ticksPerSyncGeneration = ticks;
//     writeIntProperty(HexagonAutomata::ID::AsyncGenTime, ticksPerAsyncGeneration);
// }

void HexagonAutomata::State::setOptions(GameOptions options)
{
    setGameMode(options.gameMode);
    setGenerationMode(options.generationMode);
    setAliveColour(options.aliveColour);
    setDeadColour(options.deadColour);
    setBornSurviveRules(options.bornRules, options.surviveRules);
    setNeighborDistance(options.neighborsShape.getIntValue());
    setGenerationMs(options.syncGenerationMs);
}

// Not sure why there's an EXC_BREAKPOINT here
// juce::ValueTree HexagonAutomata::State::loadStateProperties(juce::ValueTree stateIn)
// {
//     juce::ValueTree newState = (stateIn.hasType(gameId)) 
//                              ? stateIn
//                              : juce::ValueTree(gameId);

//     LumatoneGameBaseState::loadStateProperties(newState);

//     for (auto property : GetStateProperties())
//     {
//         handleStatePropertyChange(newState, property);
//     }
// }
void HexagonAutomata::State::updateGenerationTickRate()
{
    ticksPerGeneration = juce::roundToInt(engineState.msecToTicks(syncGenerationMs));
}

void HexagonAutomata::State::handleStatePropertyChange(juce::ValueTree stateIn, const juce::Identifier &property)
{
    if (property == LumatoneGameEngineState::ID::RequestedFps)
    {
        updateGenerationTickRate();
    }
    else if (property == HexagonAutomata::ID::AliveColour)
    {
        juce::Colour colour = juce::Colours::lightslategrey;

        if (stateIn.hasProperty(property))
        {
            juce::String colourStr = stateIn.getProperty(property).toString();
            colour = juce::Colour::fromString(colourStr);
        }

        setAliveColour(colour);        
    }
    else if (property == HexagonAutomata::ID::DeadColour)
    {
        if (stateIn.hasProperty(property))
        {
            juce::String colourStr = stateIn.getProperty(property).toString();
            deadColour = juce::Colour::fromString(colourStr);
        }
        else
        {
            deadColour = juce::Colours::black;
        }
    }
    else if (property == HexagonAutomata::ID::BornRule)
    {
        juce::String input = stateIn.getProperty(property).toString();
        
    }
}
