/*
  ==============================================================================

    colour_selection_group.h
    Created: 20 Dec 2020 5:40:13pm
    Author:  Vincenzo

    Base classes for maintaining multiple objects that resolve to one or zero colours being selected.

  ==============================================================================
*/

#ifndef LUMATONE_COLOUR_SELECTION_GROUP_H
#define LUMATONE_COLOUR_SELECTION_GROUP_H

#include "./colour_selection_broadcaster.h"

class ColourSelectionGroup :  public ColourSelectionBroadcaster,
                              private ColourSelectionListener
{
public:

    ~ColourSelectionGroup()
    {
        for (auto selector : colourSelectors)
            selector->removeColourSelectionListener(this);
    }

    /// <summary>
    /// Adds a ColourPaletteComponent and returns the palette's group index
    /// </summary>
    /// <param name="paletteComponentToAdd"></param>
    /// <returns></returns>
    int addSelector(ColourSelectionBroadcaster* selectorToAdd)
    {
        bool newSelector = colourSelectors.addIfNotAlreadyThere(selectorToAdd);

        if (newSelector)
            selectorToAdd->addColourSelectionListener(this);

        return colourSelectors.indexOf(selectorToAdd);
    }

    void removeSelector(ColourSelectionBroadcaster* selectorToRemove)
    {
        int removed = colourSelectors.removeAllInstancesOf(selectorToRemove);

        if (removed > 0)
            selectorToRemove->removeColourSelectionListener(this);
    }

    /// <summary>
    /// Removes the selected status from the selected palette swatch
    /// </summary>
    void deselectColour() override
    {
        if (selectedBroadcasterIndex >= 0)
            colourSelectors[selectedBroadcasterIndex]->deselectColour();

        selectedBroadcasterIndex = -1;
    }

    juce::Colour getSelectedColour() override
    {
        if (selectedBroadcasterIndex >= 0)
            return colourSelectors[selectedBroadcasterIndex]->getSelectedColour();
        
        return juce::Colour();
    }

    ColourSelectionBroadcaster* getSelectedBroadcaster() const
    {
        if (selectedBroadcasterIndex >= 0)
            return colourSelectors[selectedBroadcasterIndex];

        return nullptr;
    }

    int getSelectedBroadcasterIndex() const
    {
        return selectedBroadcasterIndex;
    }

    int getIndexOfSelector(ColourSelectionBroadcaster* selectorIn) const
    {
        return colourSelectors.indexOf(selectorIn);
    }

    void setCurrentSelector(ColourSelectionBroadcaster* selector)
    {
        auto index = colourSelectors.indexOf(selector);

        if (index >= 0 && index < colourSelectors.size())
        {
            if (selectedBroadcasterIndex >= 0 && getSelectedBroadcaster() != selector)
            {
                getSelectedBroadcaster()->deselectColour();
            }

            selectedBroadcasterIndex = index;

            selectorListeners.call(&ColourSelectionListener::colourChangedCallback, selector, selector->getSelectedColour());
        }
    }

    //=========================================================================

    /// <summary>
    /// Use this callback to determine the selected colour among all colour selectors
    /// </summary>
    /// <param name="source"></param>
    /// <param name="newColour"></param>
    void colourChangedCallback(ColourSelectionBroadcaster* source, juce::Colour newColour) override
    {
        setCurrentSelector(source);
    }

private:

    juce::Array<ColourSelectionBroadcaster*> colourSelectors;

    int selectedBroadcasterIndex = -1;
};

#endif // LUMATONE_COLOUR_SELECTION_GROUP_H