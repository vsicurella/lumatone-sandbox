/*
  ==============================================================================

    SandboxState.cpp
    Created: 4 Jun 2023 4:56:00pm
    Author:  Vincenzo

  ==============================================================================
*/

#include "SandboxState.h"


LumatoneSandboxState::LumatoneSandboxState(juce::ValueTree stateIn, juce::UndoManager* undoManagerIn) 
    :   state(stateIn),
        undoManager(undoManagerIn) 
{
    if (!state.isValid() || state.getType() != LumatoneSandboxProperty::StateTree)
    {
        state = juce::ValueTree(LumatoneSandboxProperty::StateTree);
    }
}

LumatoneSandboxState::LumatoneSandboxState(const LumatoneSandboxState& stateIn, juce::UndoManager* undoManagerIn)
    : state(stateIn.state),
    undoManager(undoManagerIn) {}

bool LumatoneSandboxState::writeToPropertiesFile()
{
    // todo
    return false;
}

bool LumatoneSandboxState::getBoolProperty(const juce::Identifier key, bool fallback) const
{
    auto prop = state.getProperty(key, juce::var((bool)fallback));
    return (bool)prop;
}

int LumatoneSandboxState::getIntProperty(const juce::Identifier key, int fallback) const
{
    auto prop = state.getProperty(key, juce::var((int)fallback));
    return (int)prop;
}

juce::String LumatoneSandboxState::getStringProperty(const juce::Identifier key, juce::String fallback) const
{
    auto prop = state.getProperty(key, juce::var(fallback));
    return prop.toString();
}

void LumatoneSandboxState::writeBoolProperty(const juce::Identifier key, bool value)
{
    state.setProperty(key, value, undoManager);
}

void LumatoneSandboxState::writeIntProperty(const juce::Identifier key, int value)
{
    state.setProperty(key, value, undoManager);
}

void LumatoneSandboxState::writeStringProperty(const juce::Identifier key, juce::String value)
{
    state.setProperty(key, value, undoManager);
}

