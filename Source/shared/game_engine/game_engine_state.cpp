/*

    File: game_engine_state.cpp
    Author: Vito
    Date: 2023/11/07

*/

#include "game_engine_state.h"

#include "../debug/LumatoneSandboxLogger.h"

juce::Array<juce::Identifier> LumatoneGameEngineState::GetGameEngineProperties()
{
    juce::Array<juce::Identifier> properties;

    properties.add(LumatoneGameEngineState::ID::GameEngineStateId);
    properties.add(LumatoneGameEngineState::ID::DefaultFps);
    properties.add(LumatoneGameEngineState::ID::RequestedFps);
    properties.add(LumatoneGameEngineState::ID::GameStatus);
    properties.add(LumatoneGameEngineState::ID::LogLevel);

    return properties;
}

LumatoneGameEngineState::LumatoneGameEngineState(juce::String nameIn, juce::ValueTree stateIn, juce::UndoManager *undoManager)
    : LumatoneStateBase(nameIn)
{
    if (stateIn.hasType(LumatoneGameEngineState::ID::GameEngineStateId))
    {
        state = stateIn;
        state.addListener(this);
    }
    else
    {
        state = stateIn.getOrCreateChildWithName(LumatoneGameEngineState::ID::GameEngineStateId, undoManager);
    }

    loadStateProperties(state);
}

void LumatoneGameEngineState::updateTimeIntervalMs() 
{
    timeIntervalMs = 1000.0 / runGameFps;
}

float LumatoneGameEngineState::getTimeIntervalMs() const
{
    return timeIntervalMs;
}

double LumatoneGameEngineState::getFps() const
{
    return runGameFps;
}

void LumatoneGameEngineState::forceFps(double fps)
{
    runGameFps = fps;
    writeStringProperty(LumatoneGameEngineState::ID::RequestedFps, juce::String(runGameFps), nullptr);
    updateTimeIntervalMs();
}

void LumatoneGameEngineState::setGameStatus(LumatoneGameEngineState::GameStatus newState)
{
    gameStatus = newState;
    writeStringProperty(LumatoneGameEngineState::ID::GameStatus, LumatoneGameEngineState::GameStatusToString(newState), nullptr);
}

void LumatoneGameEngineState::setGameName(LumatoneSandbox::GameName gameNameIn)
{
    gameName = gameNameIn;
    writeStringProperty(LumatoneGameEngineState::ID::GameName, LumatoneSandbox::GameNameToString(gameNameIn), nullptr);
}

void LumatoneGameEngineState::valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier &property)
{
    if (treeWhosePropertyHasChanged == state)
    {
        handleStatePropertyChange(state, property);
    }
}

juce::ValueTree LumatoneGameEngineState::loadStateProperties(juce::ValueTree stateIn)
{
    juce::ValueTree newState = (stateIn.hasType(LumatoneGameEngineState::ID::GameEngineStateId))
                             ? stateIn
                             : juce::ValueTree(LumatoneGameEngineState::ID::GameEngineStateId);

    for (auto property : GetGameEngineProperties())
    {
        if (newState.hasProperty(property))
            handleStatePropertyChange(newState, property);
    }

    return newState;
}

void LumatoneGameEngineState::handleStatePropertyChange(juce::ValueTree stateIn, const juce::Identifier &property)
{
    if (property == LumatoneGameEngineState::ID::DefaultFps)
    {
        defaultFps = static_cast<float>(stateIn.getProperty(property));
    }
    else if (property == LumatoneGameEngineState::ID::RequestedFps)
    {
        runGameFps = stateIn.getProperty(property).toString().getFloatValue();
        updateTimeIntervalMs();
    }
    else if (property == LumatoneGameEngineState::ID::GameName)
    {
        juce::String readName = stateIn.getProperty(property).toString();
        LumatoneSandbox::GameName name = LumatoneSandbox::GameNameFromString(readName);
        setGameName(name);
    }
    else if (property == LumatoneGameEngineState::ID::GameStatus)
    {
        juce::String readStatus = stateIn.getProperty(property).toString();
        LumatoneGameEngineState::GameStatus status = LumatoneGameEngineState::GameStatusFromString(readStatus);
        setGameStatus(status);
    }
}
