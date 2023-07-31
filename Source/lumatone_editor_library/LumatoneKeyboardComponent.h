/*
  ==============================================================================

    LumatoneKeyboardComponent.h
    Created: 24 Jul 2023 9:52:28pm
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "lumatone_state.h"
#include "lumatone_assets.h"
#include "LumatoneKeyComponent.h"
#include "ApplicationListeners.h"


//==============================================================================
/*
*/
class LumatoneKeyboardComponent : public juce::Component,
                                  public LumatoneEditor::EditorListener
{
public:
    LumatoneKeyboardComponent(LumatoneState stateIn);
    ~LumatoneKeyboardComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void resetOctaveSize();

public:

    // LumatoneEditor::EditorListener Implementation
    void completeMappingLoaded(LumatoneLayout mappingData) override;
    void boardChanged(LumatoneBoard boardData) override;
    void keyChanged(int boardIndex, int keyIndex, LumatoneKey lumatoneKey) override;
    
private:

    juce::Image getResizedImage(LumatoneAssets::ID assetId, int targetWidth, int targetHeight);

private:

    LumatoneState state;

    struct OctaveBoard
    {
        juce::OwnedArray<LumatoneKeyDisplay> keyMiniDisplay;
        int leftPos;
        int rightPos;
    };

    juce::OwnedArray<OctaveBoard> octaveBoards;

    int currentOctaveSize = 0;
    int currentSetSelection;

    HexagonTilingGeometry tilingGeometry;

    int graphicWidthUsed = 0;
    int graphicHeightUsed = 0;

    juce::Image keyColourLayer;
    juce::Image keyShadowLayer;

    // std::unique_ptr<juce::Label> lblFirmwareVersion;

    //==============================================================================
    // Style helpers

    std::unique_ptr<ImageProcessor> imageProcessor;

    juce::Rectangle<int> lumatoneBounds;
    int octaveLineY = 0;

    juce::Image lumatoneGraphic;
    juce::Image keyShapeGraphic;
    juce::Image keyShadowGraphic;

    //==============================================================================
    // Position and sizing constants in reference to parent bounds

    const float imageAspect = 2.498233f;
    const float imageY      = 1.0f / 7.0f;
    const float imageHeight = 5.0f / 7.0f;

    //const float saveLoadH           = 0.0537634f;

    const float octaveLineYRatio    = 0.0236559f;

    // In reference to lumatoneBounds
    const float keybedX = 0.06908748f;

    const float keyW = 0.027352f;
    const float keyH = 0.07307f;

    const float oct1Key1X = 0.0839425f;
    const float oct1Key1Y = 0.335887f;

    const float oct1Key56X = 0.27304881f;
    const float oct1Key56Y = 0.8314673f;

    const float oct5Key7X = 0.878802f;
    const float oct5Key7Y = 0.356511491f;

    //===============================================================================

    juce::Point<float>  oct1Key1;
    juce::Point<float> oct1Key56;
    juce::Point<float>  oct5Key7;

    // Geometry settings
    TerpstraBoardGeometry    boardGeometry;

    //[/UserVariables]

    //==============================================================================
    std::unique_ptr<juce::TextButton> btnLoadFile;
    std::unique_ptr<juce::TextButton> btnSaveFile;
    std::unique_ptr<juce::TextButton> buttonReceive;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LumatoneKeyboardComponent)
};
