/*
  ==============================================================================

    LumatoneKeyboardComponent.h
    Created: 24 Jul 2023 9:52:28pm
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "LumatoneKeyComponent.h"
#include "lumatone_state.h"

// Hash codes for use with ImageCache::getFromHashCode()
enum LumatoneEditorAssets
{
	//LoadIcon            = 0x0002000,
	//SaveIcon            = 0x0002001,
	//ImportIcon          = 0x0002002,
	LumatoneGraphic = 0x0002100,
	KeybedShadows = 0x0002101,
	KeyShape = 0x0002200,
	KeyShadow = 0x0002201,
	TickBox = 0x0003100,
	SavePalette = 0x0005000,
	CancelPalette = 0x0005001,
	TrashCanIcon = 0x0005002,
	CloneIcon = 0x0005003
};

//==============================================================================
/*
*/
class LumatoneKeyboardComponent : public juce::Component
{
public:
    LumatoneKeyboardComponent(LumatoneState stateIn);
    ~LumatoneKeyboardComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void resetOctaveSize();

private:

	LumatoneState state;

	struct OctaveBoard
	{
		juce::OwnedArray<LumatoneKeyDisplay> keyMiniDisplay;
		int leftPos;
		int rightPos;
	};

	juce::OwnedArray<OctaveBoard> octaveBoards;

	int	currentOctaveSize = 0;
	int currentSetSelection;

	HexagonTilingGeometry tilingGeometry;

	juce::Image keyColourLayer;
	juce::Image keyShadowLayer;

    std::unique_ptr<juce::Label> lblFirmwareVersion;

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

	//const float importYFromImageTop = 0.0752688f;
	//const float importH             = 0.0526882f;
	//const float importW             = 0.132f;

	//const float btnYFromImageTop    = 0.04172043f;
	//const float saveLoadW           = 0.07416f;
	const float saveLoadH           = 0.0537634f;
	//const float saveloadMarginW     = 0.0034f;

	//const float filenameBaselineY   = 0.1221505f;

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
	TerpstraBoardGeometry	boardGeometry;

    //[/UserVariables]

    //==============================================================================
    std::unique_ptr<juce::TextButton> btnLoadFile;
    std::unique_ptr<juce::TextButton> btnSaveFile;
    std::unique_ptr<juce::TextButton> buttonReceive;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LumatoneKeyboardComponent)
};
