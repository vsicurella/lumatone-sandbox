#pragma once

#include "../lumatone_editor_library/LumatoneController.h"
#include "../lumatone_editor_library/palettes/ColourPaletteWindow.h"
#include "../lumatone_editor_library/palettes/palette_library.h"

class AdjustColourPanel : public juce::Component,
                          public LumatoneEditor::EditorListener
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

    private:
        juce::Colour colour;
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
    void reconfigureColours();

private:

    LumatoneController* controller;
    LumatonePaletteLibrary* paletteLibrary;

    juce::Array<juce::Colour> colours;

    juce::OwnedArray<Box> colourBoxes;

    std::unique_ptr<ColourPaletteWindow> palettePanel;
    std::unique_ptr<juce::CallOutBox> callout;


    // UI helpers
    juce::Rectangle<int> controlBounds;
    const float marginX = 0.1f;

    const float boxSizeX = 0.08f;
    const float boxMarginX = 0.01f;
};
