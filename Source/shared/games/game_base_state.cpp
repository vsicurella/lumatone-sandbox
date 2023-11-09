/*

    File: game_base_state.h
    Author: Vito
    Date: 2023/11/07

*/

#include "./game_base_state.h"

juce::Array<juce::Identifier> LumatoneGameBaseState::GetLumatoneGameBaseProperties()
{
    juce::Array<juce::Identifier> properties;
    properties.add(LumatoneGameBaseState::ID::Ticks);
    properties.add(LumatoneGameBaseState::ID::MaxUpdatesPerFrame);
    properties.add(LumatoneGameBaseState::ID::LogLevel);
    return properties;
}

LumatoneGameBaseState::LumatoneGameBaseState(LumatoneSandbox::GameName name, juce::Identifier gameIdIn, juce::ValueTree stateIn)
    : LumatoneStateBase(LumatoneSandbox::GameNameToString(name))
    , gameId(gameIdIn)
    , engineStateTree(stateIn)
    , engineState(LumatoneSandbox::GameNameToString(name), stateIn)
{
    state = engineStateTree.getOrCreateChildWithName(gameId, nullptr);
    engineStateTree.addListener(this);
}

void LumatoneGameBaseState::valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier &property)
{
    handleStatePropertyChange(state, property);
}

juce::ValueTree LumatoneGameBaseState::loadStateProperties(juce::ValueTree stateIn)
{
    juce::ValueTree newState = (stateIn.hasType(gameId)) ? stateIn : juce::ValueTree(gameId);

    for (auto property : GetLumatoneGameBaseProperties())
    {
        if (newState.hasProperty(property))
            handleStatePropertyChange(newState, property);
    }

    return newState;
}

void LumatoneGameBaseState::handleStatePropertyChange(juce::ValueTree stateIn, const juce::Identifier &property)
{
    if (property == LumatoneGameEngineState::ID::RequestedFps)
    {

    }
    else if (property == LumatoneGameEngineState::ID::GameStatus)
    {

    }
}
