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

LumatoneGameBaseState::LumatoneGameBaseState(LumatoneSandbox::GameName nameIn, juce::Identifier gameIdIn, juce::ValueTree stateIn)
    : LumatoneStateBase(LumatoneSandbox::GameNameToString(nameIn))
    , gameId(gameIdIn)
    , engineState(LumatoneSandbox::GameNameToString(nameIn) + "EngineCopy", stateIn)
    // , LumatoneGameEngineState(LumatoneSandbox::GameNameToString(nameIn) + "EngineCopy", stateIn)
{
    state = engineState.getGameStateTree();
    if (state.hasProperty(LumatoneGameEngineState::ID::GameName))
    {
        if (state[LumatoneGameEngineState::ID::GameName].toString() != name)
        {
            state.removeAllProperties(nullptr);
            state.removeAllChildren(nullptr);
        }
    }

    writeStringProperty(LumatoneGameEngineState::ID::GameName, name);
    
    state.addListener(this);
    stateIn.addListener(this);
    // state.addListener(this);
    // state.getParent().addListener(this);
}

