/*
  ==============================================================================

    LumatoneStateBase.cpp
    Created: 4 Jun 2023 4:56:00pm
    Author:  Vincenzo

  ==============================================================================
*/

#include "state_base.h"

void LumatoneStateBase::setStateProperty(const juce::Identifier &id, juce::var value)
{
    state.setPropertyExcludingListener(this, id, value, nullptr);
}

bool LumatoneStateBase::getBoolProperty(const juce::Identifier key, bool fallback) const
{
    auto prop = state.getProperty(key, juce::var((bool)fallback));
    return (bool)prop;
}

int LumatoneStateBase::getIntProperty(const juce::Identifier key, int fallback) const
{
    auto prop = state.getProperty(key, juce::var((int)fallback));
    return (int)prop;
}

juce::String LumatoneStateBase::getStringProperty(const juce::Identifier key, juce::String fallback) const
{
    auto prop = state.getProperty(key, juce::var(fallback));
    return prop.toString();
}

void LumatoneStateBase::valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier &property)
{
    handleStatePropertyChange(treeWhosePropertyHasChanged, property);
}
