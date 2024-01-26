/*
==============================================================================

    LumatoneStateBase.h
    Created: 4 Jun 2023 4:56:00pm
    Author:  Vincenzo

==============================================================================
*/

#ifndef LUMATONE_STATE_BASE_H
#define LUMATONE_STATE_BASE_H

#include <JuceHeader.h>

class LumatoneStateBase : protected juce::ValueTree::Listener
{
public:
    LumatoneStateBase(juce::String nameIn) : name(nameIn) { }

    bool getBoolProperty(const juce::Identifier key, bool fallback) const;
    int getIntProperty(const juce::Identifier key, int fallback) const;
    juce::String getStringProperty(const juce::Identifier key, juce::String fallback = juce::String()) const;


protected:
    void setStateProperty(const juce::Identifier& id, juce::var value);

    virtual void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;

    virtual juce::ValueTree loadStateProperties(juce::ValueTree stateIn) = 0;
    virtual void handleStatePropertyChange(juce::ValueTree stateIn, const juce::Identifier& property) = 0;
    
    virtual void loadPropertiesFile(juce::PropertiesFile* properties) = 0;

protected:
    juce::ValueTree state;
    juce::String name;
};

#endif // LUMATONE_STATE_BASE_H
