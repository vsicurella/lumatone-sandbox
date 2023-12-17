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
    // LumatoneStateBase(const LumatoneStateBase& stateIn) 
    //     : state(stateIn.state) 
    //     , name(stateIn.name) { }

    bool getBoolProperty(const juce::Identifier key, bool fallback) const;
    int getIntProperty(const juce::Identifier key, int fallback) const;
    juce::String getStringProperty(const juce::Identifier key, juce::String fallback = juce::String()) const;

    virtual bool writeToPropertiesFile();

protected:
    void writeBoolProperty(const juce::Identifier key, bool value, juce::UndoManager* undo=nullptr);
    void writeIntProperty(const juce::Identifier key, int value, juce::UndoManager* undo=nullptr);
    void writeStringProperty(const juce::Identifier key, juce::String value, juce::UndoManager* undo=nullptr);

protected:
    virtual void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;

    virtual juce::ValueTree loadStateProperties(juce::ValueTree stateIn) = 0;
    virtual void handleStatePropertyChange(juce::ValueTree stateIn, const juce::Identifier& property) = 0;

protected:
    juce::ValueTree state;
    juce::String name;
};
