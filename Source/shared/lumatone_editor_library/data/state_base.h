/*
  ==============================================================================

    LumatoneStateBase.h
    Created: 4 Jun 2023 4:56:00pm
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class LumatoneStateBase : protected juce::ValueTree::Listener
{
public:

    LumatoneStateBase(juce::String nameIn) : name(nameIn) { }
    LumatoneStateBase(const LumatoneStateBase& stateIn) : state(stateIn.state) { }

    bool getBoolProperty(const juce::Identifier key, bool fallback) const;
    int getIntProperty(const juce::Identifier key, int fallback) const;
    juce::String getStringProperty(const juce::Identifier key, juce::String fallback = juce::String()) const;

    virtual bool writeToPropertiesFile();

protected:

    juce::ValueTree state;

    juce::String name;

    void writeBoolProperty(const juce::Identifier key, bool value, juce::UndoManager* undo=nullptr);
    void writeIntProperty(const juce::Identifier key, int value, juce::UndoManager* undo=nullptr);
    void writeStringProperty(const juce::Identifier key, juce::String value, juce::UndoManager* undo=nullptr);
};
