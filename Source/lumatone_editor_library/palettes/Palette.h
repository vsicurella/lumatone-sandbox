/*
  ==============================================================================

    PolygonPalette.h
    Created: 4 Feb 2021 11:42:00pm
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class Palette : public juce::Component, protected juce::ChangeListener
{
public:
    Palette(int numSwatchesIn)
        : numSwatches(numSwatchesIn)
    {
        palette.resize(numSwatches);
        
        // Setup paths needed to draw swatches
    }

    virtual ~Palette() {}

    //======================================================================
    // Implementation of juce::Component

    virtual void mouseDown(const juce::MouseEvent& e) override    
    {
        if (isEnabled())
        {
            for (auto s : swatchPaths)
            {
                if (s.contains(e.position))
                {
                    setSelectedSwatchNumber(swatchPaths.indexOf(s));
                    return;
                }
            }

            selectedSwatch = -1;
            repaint();
        }
    }

    //======================================================================
    // Getters

    int getNumberOfSwatches() const
    {
        return numSwatches;
    }

    // Returns index of selected swatch, -1 if none
    int getSelectedSwatchNumber() const
    {
        return selectedSwatch;
    } 

    juce::Colour getSelectedSwatchColour() const
    {
        if (selectedSwatch >= 0 && selectedSwatch < numSwatches)
            return palette[selectedSwatch];

        return juce::Colour();
    }

    juce::Colour getSwatchColour(int swatchIndex) const
    {
        return palette[swatchIndex];
    }

    juce::Array<juce::Colour> getColourPalette() const
    {
        return palette;
    }

    //======================================================================
    // Setters

    virtual void setSelectedSwatchNumber(int swatchIndex)
    {
        selectedSwatch = swatchIndex;
        if (selector)
        {
            selector->setCurrentColour(palette[selectedSwatch], juce::NotificationType::dontSendNotification);
        }
        repaint();
    }

    virtual void setColourPalette(juce::Array<juce::Colour> colourPaletteIn)
    {
        palette = colourPaletteIn;
        repaint();
    }

    virtual void setSwatchColour(int swatchNumber, juce::Colour newColour)
    {
        if (swatchNumber >= 0 && swatchNumber < numSwatches)
            palette.set(swatchNumber, newColour);
    }

    virtual void attachColourSelector(juce::ColourSelector* selectorIn)
    {
        selector = selectorIn;
        
        if (selector)
        {
            if (selectedSwatch > -1)
            {
                selector->setCurrentColour(palette[selectedSwatch], juce::NotificationType::dontSendNotification);
                //selector->setFocusserCallback([&](Component* c, var data) {
                //    palette.set(focussedSwatch, Colour::fromString(data.toString()));
                //    repaint();
                //});
            }

            selector->addChangeListener(this);
        }
    }

    //======================================================================
    // Implementation of juce::ChangeListener

    void changeListenerCallback(juce::ChangeBroadcaster* source) override
    {
        if (source == selector && selectedSwatch > -1)
        {
            setSwatchColour(selectedSwatch, selector->getCurrentColour());
            onlyRepaintSelectedSwatch = true;
            repaint();
        }
    }

protected:

    juce::Array<juce::Path> swatchPaths;

    int selectedSwatch = -1;
    bool onlyRepaintSelectedSwatch = false;

private:

    const int numSwatches;

    juce::ColourSelector* selector = nullptr;

    juce::Array<juce::Colour> palette =
    {
        juce::Colours::orange,
        juce::Colours::red,
        juce::Colours::springgreen,
        juce::Colours::lightsteelblue,
        juce::Colours::yellowgreen,
        juce::Colours::rebeccapurple
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Palette)
};
