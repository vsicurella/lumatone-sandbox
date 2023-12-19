/*

    File: game_base_state.h
    Author: Vito
    Date: 2023/11/07

*/

#include "game_base_state.h"

juce::Array<juce::Identifier> LumatoneGameBaseState::GetLumatoneGameBaseProperties()
{
    juce::Array<juce::Identifier> properties;
    properties.add(LumatoneGameBaseState::ID::Ticks);
    properties.add(LumatoneGameBaseState::ID::MaxUpdatesPerFrame);
    properties.add(LumatoneGameBaseState::ID::LogLevel);
    return properties;
}

LumatoneGameBaseState::LumatoneGameBaseState(LumatoneSandbox::GameName nameIn, juce::Identifier gameIdIn, LumatoneGameEngineState& gameEngineStateIn)
    : LumatoneStateBase(LumatoneSandbox::GameNameToString(nameIn))
    , gameId(gameIdIn)
    , engineState(LumatoneSandbox::GameNameToString(nameIn) + "EngineCopy", gameEngineStateIn)
{
    state = juce::ValueTree(LumatoneGameEngineState::ID::GameStateId);
    writeStringProperty(LumatoneGameEngineState::ID::GameName, name);
    
    engineState.addStateListener(this);
}

void LumatoneGameBaseState::handleStatePropertyChange(juce::ValueTree stateIn, const juce::Identifier &property)
{
    if (engineState == stateIn)
    {
        engineState.handleStatePropertyChange(stateIn, property);
    }
}
