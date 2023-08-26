#pragma once

#include "../lumatone_editor_library/LumatoneController.h"
#include "../lumatone_editor_library/palettes/ColourPaletteWindow.h"
#include "../lumatone_editor_library/palettes/palette_library.h"

#include "../lumatone_editor_library/actions/edit_actions.h"

#include "../lumatone_editor_library/color/adjust_layout_colour.h"

class AdjustColourPanel : public juce::Component,
                          public LumatoneEditor::EditorListener,
                          public ColourSelectionListener
{
private:
    class Box : public juce::Component
    {
    public:
        Box(juce::Colour colour);
        ~Box();

        void setColour(juce::Colour newColour);
        juce::Colour getColour() const;

        void paint(juce::Graphics& g) override;
        void resized() override;

        void setSelected(bool selected);

    private:
        juce::Colour colour;

        bool selected;
    };


public:

    AdjustColourPanel(LumatoneController* controller, LumatonePaletteLibrary* paletteLibrary);
    ~AdjustColourPanel();

public:
    // juce::Component implementation
    void paint(juce::Graphics& g) override;
    void resized() override;

    void mouseMove(const juce::MouseEvent& e) override;
    void mouseDown(const juce::MouseEvent& e) override;

private:
    void completeMappingLoaded(LumatoneLayout mappingData) override;
    void boardChanged(LumatoneBoard boardData) override;
    void keyChanged(int boardIndex, int keyIndex, LumatoneKey lumatoneKey) override;

private:
    void colourChangedCallback(ColourSelectionBroadcaster* source, juce::Colour newColour) override;

    void hueValueCallback();
    void saturationValueCallback();
    void brightnessValueCallback();
    void adjustWhiteValueCallback();

private:

    void reconfigureColours(bool doResize=true);

    void setSelectedBox(Box* box);
    void deselectBox();

private:

    LumatoneController* controller;
    LumatonePaletteLibrary* paletteLibrary;

    AdjustLayoutColour colourAdjuster;

    juce::Array<juce::Colour> colours;

    juce::OwnedArray<Box> colourBoxes;

    std::unique_ptr<juce::Slider> hueSlider;
    std::unique_ptr<juce::Slider> satSlider;
    std::unique_ptr<juce::Slider> brightnessSlider;
    std::unique_ptr<juce::Slider> kelvinSlider;

    std::unique_ptr<ColourPaletteWindow> palettePanel;
    std::unique_ptr<juce::CallOutBox> callout;

    int selectedBox = -1;
    juce::Array<LumatoneKeyCoord> keySelection;

    // UI helpers
    juce::Rectangle<int> controlBounds;
    const float marginX = 0.1f;

    const float boxSizeX = 0.08f;
    const float boxMarginX = 0.01f;
};
