/*
  ==============================================================================

    ColourSelectionPanels.h
    Created: 21 Dec 2020 9:48:08pm
    Author:  Vincenzo

  ==============================================================================
*/

#ifndef LUMATONE_COLOUR_PICKER_PANEL_H
#define LUMATONE_COLOUR_PICKER_PANEL_H

#include "./colour_selection_broadcaster.h"

//==============================================================================
/*
*   Full RGB colour selector panel
*/
class CustomPickerPanel : public juce::Component,
    public juce::ChangeListener,
    public ColourSelectionBroadcaster,
    public ColourSelectionListener
{
public:

    CustomPickerPanel()
    {
        colourPicker.reset(new juce::ColourSelector(
              juce::ColourSelector::ColourSelectorOptions::editableColour
            + juce::ColourSelector::ColourSelectorOptions::showColourAtTop
            + juce::ColourSelector::ColourSelectorOptions::showColourspace
        ));

        colourPicker->setName("ColourPicker");
        addAndMakeVisible(*colourPicker);
        colourPicker->addChangeListener(this);
    }

    ~CustomPickerPanel()
    {
        colourPicker = nullptr;
    }

    void paint(juce::Graphics& g) override {};

    void resized() override
    {
        colourPicker->setBounds(getLocalBounds());
    }

    void setCurrentColour(juce::Colour colourIn)
    {
        colourPicker->setCurrentColour(colourIn, juce::NotificationType::dontSendNotification);
    }

    void changeListenerCallback(juce::ChangeBroadcaster* source) override
    {
        selectorListeners.call(&ColourSelectionListener::colourChangedCallback, this, colourPicker->getCurrentColour());
    }

    void colourChangedCallback(ColourSelectionBroadcaster* source, juce::Colour newColour) override
    {
        if (this != source)
            colourPicker->setCurrentColour(newColour, juce::NotificationType::dontSendNotification);
    }

    //==============================================================================

    juce::Colour getSelectedColour() override
    {
        return colourPicker->getCurrentColour();
    }

    void deselectColour() override {};

private:

    std::unique_ptr<juce::ColourSelector> colourPicker;
};

#endif // LUMATONE_COLOUR_PICKER_PANEL_H
