/*
  ==============================================================================

    LumatoneKeyComponent.h
    Created: 26 Jul 2023 11:51:13pm
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "./data/lumatone_layout.h"
#include "ViewComponents.h"
#include "ImageResampling/ImageResampler.h"
#include "lumatone_tiling.h"

// Representation of a key inside the overview
class LumatoneKeyDisplay : public juce::Component
{
public:

    enum class UiMode
    {
        NoDisplay = -1,

        Shape = 0x000010,
        ShapeInteractive = 0x000011,

        Graphic = 0x000020,
        GraphicInteractive = 0x000021
    };

    enum class NoteOffModifier
    {
        None = 0,
        Sustain,
        Legato
    };

public:
    LumatoneKeyDisplay(int newBoardIndex, int newKeyIndex, LumatoneKey keyData=LumatoneKey());
    ~LumatoneKeyDisplay();

    LumatoneKeyDisplay::UiMode getUiMode() const { return uiMode; }
    void setUiMode(LumatoneKeyDisplay::UiMode uiModeIn);

    void paint(juce::Graphics&) override;
    void resized() override;

    void setKeyGraphics(juce::Image& colourGraphicIn, juce::Image& shadowGraphicIn);

    void setLumatoneKey(const LumatoneKey& lumatoneKey, int boardIdx, int keyIdx);

    void setDisplayColour(const juce::Colour& colour);

    const LumatoneKey* getKeyData() const;
    juce::Colour getKeyColour() const;

    int getBoardIndex() const { return boardIndex; }
    int getKeyIndex() const { return keyIndex; }
    LumatoneKeyCoord getCoord() const { return LumatoneKeyCoord(boardIndex, keyIndex); }

    void setSelected(bool selected);

    bool selected() const { return isSelected; }
    bool clicked() const { return isClicked; }
    bool mouseOver() const { return mouseIsOver; }

    void startDrag();
    void endDrag();
    void clearUiState();

    void noteOn();
    void noteOff();

private:

    void repaintIfInteractive();

public:

    void parentHierarchyChanged() override;

    void mouseEnter(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;
    void mouseMove(const juce::MouseEvent& e) override;
    
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LumatoneKeyDisplay)


    LumatoneKeyDisplay::UiMode uiMode;
    LumatoneKeyDisplay::NoteOffModifier noteOffMode;

    LumatoneKey keyData;

    int boardIndex = -1;
    int keyIndex = -1;

    bool mouseIsOver = false;
    bool isSelected = false;
    bool isClicked = false;
    bool isNoteOn = false;

    juce::Image* colourGraphic = nullptr;
    juce::Image* shadowGraphic = nullptr;

    //DEBUG
    juce::Colour keyColour;
};

