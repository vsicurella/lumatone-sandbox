/*

    File: game_engine_state.cpp
    Author: Vito
    Date: 2023/11/07

*/

#include "game_engine_state.h"
#include "../games/game_base_state.h"

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
    state = stateIn.getOrCreateChildWithName(LumatoneGameEngineState::ID::GameEngineStateId, undoManager);
    loadStateProperties(state);
    state.addListener(this);
}


LumatoneGameEngineState::LumatoneGameEngineState(juce::String nameIn, const LumatoneGameEngineState& stateIn)
    : LumatoneStateBase(nameIn)
{
    state = loadStateProperties(stateIn.state);
    // state.addListener(this); // we lose callback order certainty with this
}

void LumatoneGameEngineState::setGameState(LumatoneGameBaseState* gameStateIn)
{
    if (gameStateIn == nullptr)
        return;

    juce::ValueTree gameState = gameStateIn->getNode();
    if (gameState.hasType(LumatoneGameEngineState::ID::GameStateId))
    {
        auto childState = state.getChildWithName(LumatoneGameEngineState::ID::GameStateId);
        if (childState.isValid())
            state.removeChild(childState, nullptr);

        state.addChild(gameState, 0, nullptr);
    }
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

bool LumatoneGameEngineState::operator==(const juce::ValueTree &compare) const
{
    return state == compare;
}

bool LumatoneGameEngineState::operator!=(const juce::ValueTree &compare) const
{
    return state != compare;
}

void LumatoneGameEngineState::addStateListener(LumatoneStateBase *stateIn)
{
    state.addListener(stateIn);
}

void LumatoneGameEngineState::setDefaultFps(double fps)
{
    setDefaultFps(fps, true);
}

void LumatoneGameEngineState::setDefaultFps(double fps, bool writeToState)
{
    defaultFps = fps;

    if (writeToState)
        writeIntProperty(LumatoneGameEngineState::ID::DefaultFps, defaultFps, nullptr);
}

void LumatoneGameEngineState::forceFps(double fps)
{
    forceFps(fps, true);
}

void LumatoneGameEngineState::forceFps(double fps, bool writeToState)
{
    runGameFps = fps;
    updateTimeIntervalMs();

    if (writeToState)
        writeStringProperty(LumatoneGameEngineState::ID::RequestedFps, juce::String(runGameFps), nullptr);
}

// juce::ValueTree LumatoneGameEngineState::getGameStateTree()
// {
//     return state;
//     // return state.getOrCreateChildWithName(LumatoneGameEngineState::ID::GameStateId, nullptr);
// }

void LumatoneGameEngineState::setGameStatus(LumatoneGameEngineState::GameStatus newState, bool writeToState)
{
    gameStatus = newState;
    if (writeToState)
        writeStringProperty(LumatoneGameEngineState::ID::GameStatus, LumatoneGameEngineState::GameStatusToString(newState), nullptr);
}

void LumatoneGameEngineState::setGameName(LumatoneSandbox::GameName gameNameIn, bool writeToState)
{
    gameName = gameNameIn;
    if (writeToState)
        writeStringProperty(LumatoneGameEngineState::ID::GameName, LumatoneSandbox::GameNameToString(gameNameIn), nullptr);
}

// void LumatoneGameEngineState::valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier &property)
// {
//     if (treeWhosePropertyHasChanged == state)
//     {
//         handleStatePropertyChange(state, property);
//     }
// }

juce::ValueTree LumatoneGameEngineState::loadStateProperties(juce::ValueTree stateIn)
{
    juce::ValueTree newState = (stateIn.hasType(LumatoneGameEngineState::ID::GameEngineStateId))
                             ? stateIn
                             : juce::ValueTree(LumatoneGameEngineState::ID::GameEngineStateId);

    for (auto property : GetGameEngineProperties())
    {
        // if (newState.hasProperty(property))
            handleStatePropertyChange(newState, property);
    }

    return newState;
}

void LumatoneGameEngineState::handleStatePropertyChange(juce::ValueTree stateIn, const juce::Identifier &property)
{
    if (property == LumatoneGameEngineState::ID::DefaultFps)
    {
        double readFps = static_cast<double>(stateIn.getProperty(property, 30.0));
        setDefaultFps(readFps, false);
    }
    else if (property == LumatoneGameEngineState::ID::RequestedFps)
    {
        double readFps = static_cast<double>(stateIn.getProperty(property, defaultFps));
        forceFps(readFps, false);
    }
    else if (property == LumatoneGameEngineState::ID::GameName)
    {
        juce::String readName = stateIn.getProperty(property, juce::var(LumatoneSandbox::GameNameToString(LumatoneSandbox::GameName::NoGame))).toString();
        LumatoneSandbox::GameName gameName = LumatoneSandbox::GameNameFromString(readName);
        setGameName(gameName, false);
    }
    else if (property == LumatoneGameEngineState::ID::GameStatus)
    {
        juce::String readStatus = stateIn.getProperty(property, juce::var(GameStatusToString(LumatoneGameEngineState::GameStatus::NoGame))).toString();
        LumatoneGameEngineState::GameStatus status = LumatoneGameEngineState::GameStatusFromString(readStatus);
        setGameStatus(status, false);
    }
}
