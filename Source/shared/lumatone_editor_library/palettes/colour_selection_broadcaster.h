/*
  ==============================================================================

    colour_selection_group.h
    Created: 20 Dec 2020 5:40:13pm
    Author:  Vincenzo

    Base classes for maintaining multiple objects that resolve to one or zero colours being selected.

  ==============================================================================
*/

#ifndef LUMATONE_COLOUR_SELECTION_BROADCASTER_H
#define LUMATONE_COLOUR_SELECTION_BROADCASTER_H

#include <JuceHeader.h>

class ColourSelectionBroadcaster;

class ColourSelectionListener
{
public:
    virtual ~ColourSelectionListener() {}
    virtual void colourChangedCallback(ColourSelectionBroadcaster* source, juce::Colour newColour) = 0;
};

class ColourSelectionBroadcaster
{
public:

    ColourSelectionBroadcaster() {};
    virtual ~ColourSelectionBroadcaster() {}

    virtual juce::Colour getSelectedColour() = 0;

    virtual void deselectColour() = 0;

    void    addColourSelectionListener(ColourSelectionListener* listenerIn) { selectorListeners.add(listenerIn); }
    void removeColourSelectionListener(ColourSelectionListener* listenerIn) { selectorListeners.remove(listenerIn); }

protected:

    juce::ListenerList<ColourSelectionListener> selectorListeners;
};

#endif // LUMATONE_COLOUR_SELECTION_BROADCASTER_H
