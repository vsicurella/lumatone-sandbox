#pragma once

#include <JuceHeader.h>

class ColourEditPanel : juce::Component
{
private:
    class Box : public juce::Component
    {
    public:
        Box(juce::Colour colour);
        ~Box();

        void setColour(juce::Colour newColour);

        void paint(juce::Graphics& g) override;
        void resized() override;

    private:
        juce::Colour colour;
    };


public:

    ColourEditPanel();
    ColourEditPanel(const juce::Array<juce::Colour>& colourSet);
    
    ~ColourEditPanel();

    void setColours(const juce::Array<juce::Colour>& colourSet);

public:
    // juce::Component implementation
    void paint(juce::Graphics& g) override;
    void resized() override;

    void mouseMove(const juce::MouseEvent& e) override;
    void mouseDown(const juce::MouseEvent& e) override;

private:
    void reconfigureColours(const juce::Array<juce::Colour>& colourSet);

private:

    juce::Array<juce::Colour> colours;

    juce::OwnedArray<Box> colourBoxes;

    std::unique_ptr<juce::ColourSelector> selector;
    std::unique_ptr<juce::CallOutBox> selectorBox;


    // UI helpers
    juce::Rectangle<int> controlBounds;
    const float marginX = 0.1f;

    const float boxSizeX = 0.08f;
    const float boxMarginX = 0.01f;
};
