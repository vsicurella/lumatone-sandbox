#include "hexagon_automata_game_state.h"

HexagonAutomata::State::State(std::shared_ptr<LumatoneLayout> layoutIn, LumatoneGameEngineState& gameEngineStateIn)
    : LumatoneGameBaseState(LumatoneSandbox::GameName::HexagonAutomata, HexagonAutomata::ID::GameId, gameEngineStateIn)
    , HexagonAutomata::BoardState(layoutIn)
{

}

// HexagonAutomata::State::State(const State& copy, LumatoneGameEngineState& gameEngineStateIn)
//     : LumatoneGameBaseState(LumatoneSandbox::GameName::HexagonAutomata, HexagonAutomata::ID::GameId, engineStateIn)
//     , HexagonAutomata::BoardState(static_cast<const State&>(copy))
// {

// }

// HexagonAutomata::State::State(const State& copy, LumatoneGameEngineState& gameEngineStateIn)
//     : LumatoneGameBaseState(LumatoneSandbox::GameName::HexagonAutomata, HexagonAutomata::ID::GameId, engineStateIn)
//     , HexagonAutomata::BoardState(static_cast<const State&>(copy))
// {

// }

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

void HexagonAutomata::State::setClockMode(ClockMode newMode)
{
    clockMode = newMode;
    writeStringProperty(HexagonAutomata::ID::ClockMode, ClockModeToString(clockMode));

    if (clockMode == ClockMode::Engine)
        updateGenerationMsTicks();
    else if (clockMode == ClockMode::MidiClockClient)
        updateGenerationClockTime();
}

void HexagonAutomata::State::setNoSustainPassthrough(bool passThroughWithNoSustain)
{
    noSustainPassThrough = passThroughWithNoSustain;
    writeBoolProperty(HexagonAutomata::ID::NoSustainPassThrough, noSustainPassThrough);
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
    generationMs = msecValue;
    updateGenerationMsTicks();
    writeStringProperty(HexagonAutomata::ID::GenerationMs, juce::String(generationMs));
}

void HexagonAutomata::State::setGenerationBpm(float bpmValue)
{
    float ms = engineState.bpmToMsec(bpmValue);
    // logInfo("setGenerationBpm", "BPM of " + juce::String(bpmValue) + " converted to " + juce::String(ms) + "ms");
    setGenerationMs(ms);
}

void HexagonAutomata::State::setGenerationQN(float qnRatioIn)
{
    qnRatio = qnRatioIn;
    ticksPerGeneration = juce::roundToInt(ticksPerQuarterNote * qnRatio);
}

void HexagonAutomata::State::setOptions(GameOptions options)
{
    setGameMode(options.gameMode);
    setGenerationMode(options.generationMode);
    setAliveColour(options.aliveColour);
    setDeadColour(options.deadColour);
    setBornSurviveRules(options.bornRules, options.surviveRules);
    setNeighborDistance(options.neighborsShape.getIntValue());
    setGenerationMs(options.generationMs);
}

void HexagonAutomata::State::updateGenerationMsTicks()
{
    ticksPerGeneration = juce::roundToInt(engineState.msecToTicks(generationMs));
}

void HexagonAutomata::State::updateGenerationClockTime()
{
    ticksPerGeneration = juce::roundToInt(ticksPerQuarterNote * qnRatio);
}

juce::ValueTree HexagonAutomata::State::loadStateProperties(juce::ValueTree stateIn)
{
    for (auto property : HexagonAutomata::GetStateProperties())
    {
        handleStatePropertyChange(stateIn, property);
    }
    
    return stateIn;
}

void HexagonAutomata::State::handleStatePropertyChange(juce::ValueTree stateIn, const juce::Identifier &property)
{
    if (property == LumatoneGameEngineState::ID::RequestedFps)
    {
        if (clockMode == ClockMode::Engine)
            updateGenerationMsTicks();
        else if (clockMode == ClockMode::MidiClockClient)
            updateGenerationClockTime();
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
}
