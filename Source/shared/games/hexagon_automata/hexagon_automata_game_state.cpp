#include "hexagon_automata_game_state.h"

juce::Array<juce::Identifier> HexagonAutomata::GetStateProperties()
{
    juce::Array<juce::Identifier> properties;
    properties.add(HexagonAutomata::ID::GameMode);
    properties.add(HexagonAutomata::ID::GenerationMode);
    properties.add(HexagonAutomata::ID::SyncGenTime);
    properties.add(HexagonAutomata::ID::AsyncGenTime);
    properties.add(HexagonAutomata::ID::BornRule);
    properties.add(HexagonAutomata::ID::SurviveRule);
    properties.add(HexagonAutomata::ID::NeighborShape);
    properties.add(HexagonAutomata::ID::AliveColour);
    properties.add(HexagonAutomata::ID::DeadColour);
    properties.add(HexagonAutomata::ID::BlendMode);
    properties.add(HexagonAutomata::ID::AliveBrightness);
    properties.add(HexagonAutomata::ID::DeadBrightness);
    return properties;
}

HexagonAutomata::State::State(std::shared_ptr<LumatoneLayout> layoutIn, juce::ValueTree engineStateIn)
    : LumatoneGameBaseState(LumatoneSandbox::GameName::HexagonAutomata, HexagonAutomata::ID::GameId, engineStateIn)
    , HexagonAutomata::BoardState(layoutIn)
{

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

void HexagonAutomata::State::setTicksPerSyncGeneration(int ticks)
{
    ticksPerSyncGeneration = ticks;
    writeIntProperty(HexagonAutomata::ID::SyncGenTime, ticksPerSyncGeneration);
}

void HexagonAutomata::State::setTicksPerAsyncGeneration(int ticks)
{
    ticksPerSyncGeneration = ticks;
    writeIntProperty(HexagonAutomata::ID::AsyncGenTime, ticksPerAsyncGeneration);
}

void HexagonAutomata::State::setOptions(GameOptions options)
{
}

juce::ValueTree HexagonAutomata::State::loadStateProperties(juce::ValueTree stateIn)
{
    juce::ValueTree newState = (stateIn.hasType(gameId)) 
                            ? stateIn
                            : juce::ValueTree(gameId);

    for (auto property : GetStateProperties())
    {
        handleStatePropertyChange(newState, property);
    }
}

void HexagonAutomata::State::handleStatePropertyChange(juce::ValueTree stateIn, const juce::Identifier &property)
{
    if (property == HexagonAutomata::ID::AliveColour)
    {
        if (stateIn.hasProperty(property))
        {
            juce::String colourStr = stateIn.getProperty(property).toString();
            aliveColour = juce::Colour::fromString(colourStr);
        }
        else
        {
            aliveColour = juce::Colours::lightslategrey;
        }
        
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
