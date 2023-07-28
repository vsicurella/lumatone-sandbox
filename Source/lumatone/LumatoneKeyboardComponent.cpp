/*
  ==============================================================================

    LumatoneKeyboardComponent.cpp
    Created: 24 Jul 2023 9:52:28pm
    Author:  Vincenzo

  ==============================================================================
*/

#include <JuceHeader.h>
#include "LumatoneKeyboardComponent.h"

LumatoneKeyboardComponent::LumatoneKeyboardComponent(LumatoneSandboxState stateIn)
	: state(stateIn)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
	tilingGeometry.setColumnAngle(LUMATONEGRAPHICCOLUMNANGLE);
	tilingGeometry.setRowAngle(LUMATONEGRAPHICROWANGLE);

	//TerpstraSysExApplication::getApp().getLumatoneController()->addStatusListener(this);
	//TerpstraSysExApplication::getApp().getLumatoneController()->addFirmwareListener(this);
}

LumatoneKeyboardComponent::~LumatoneKeyboardComponent()
{
    	imageProcessor = nullptr;
}

void LumatoneKeyboardComponent::paint (juce::Graphics& g)
{
	g.drawImageAt(lumatoneGraphic, lumatoneBounds.getX(), lumatoneBounds.getY());

	// Draw a line under the selected sub board
	if (currentSetSelection >= 0 && currentSetSelection < NUMBEROFBOARDS)
	{
		juce::Path selectionMarkPath;
		selectionMarkPath.startNewSubPath(octaveBoards[currentSetSelection]->leftPos, octaveLineY);
		selectionMarkPath.lineTo(octaveBoards[currentSetSelection]->rightPos, octaveLineY);

		juce::Colour lineColour = findColour(LumatoneKeyEdit::outlineColourId);
		g.setColour(lineColour);
		g.strokePath(selectionMarkPath, juce::PathStrokeType(1.0f));
	}
}

void LumatoneKeyboardComponent::resized()
{
	// Prepare position helpers for graphics
	int graphicHeight = juce::roundToInt(getHeight() * imageHeight);
	int graphicWidth = juce::roundToInt(imageAspect * graphicHeight);

	lumatoneBounds.setBounds(
		juce::roundToInt((getWidth() - graphicWidth) / 2.0f), juce::roundToInt(getHeight() * imageY),
		graphicWidth, graphicHeight
	);

	int btnHeight = juce::roundToInt(getHeight() * saveLoadH);
	//int btnMargin = juce::roundToInt(getWidth() * saveloadMarginW);
	//int saveLoadWidth = juce::roundToInt(getWidth() * saveLoadW);
	//int btnY = lumatoneBounds.getY() - juce::roundToInt(getHeight() * btnYFromImageTop);

	//int halfWidthX = juce::roundToInt(getWidth() * 0.5f);

	//btnLoadFile->setBounds(halfWidthX - btnMargin - saveLoadWidth, btnY, saveLoadWidth, btnHeight);
	//btnSaveFile->setBounds(halfWidthX + btnMargin, btnY, saveLoadWidth, btnHeight);

	octaveLineY = lumatoneBounds.getBottom() + juce::roundToInt(getHeight() * octaveLineYRatio);

	//int importY = lumatoneBounds.getY() - juce::roundToInt(getHeight() * importYFromImageTop);
	//int importWidth = juce::roundToInt(getWidth() * importW);
	//buttonReceive->setBounds(lumatoneBounds.getRight() - importWidth, importY, importWidth, btnHeight);

	//resizeLabelWithHeight(lblFirmwareVersion.get(), btnHeight * 0.6f);
	lblFirmwareVersion->setTopLeftPosition(lumatoneBounds.getX(), lumatoneBounds.getY() - btnHeight * 0.6f);

	int keyWidth = juce::roundToInt(lumatoneBounds.getWidth() * keyW);
	int keyHeight = juce::roundToInt(lumatoneBounds.getHeight() * keyH);

	// Scale key graphics once
	lumatoneGraphic = imageProcessor->resizeImage(juce::ImageCache::getFromHashCode(LumatoneEditorAssets::LumatoneGraphic), lumatoneBounds.getWidth(), lumatoneBounds.getHeight());
	keyShapeGraphic = imageProcessor->resizeImage(juce::ImageCache::getFromHashCode(LumatoneEditorAssets::KeyShape), keyWidth, keyHeight);
	keyShadowGraphic = imageProcessor->resizeImage(juce::ImageCache::getFromHashCode(LumatoneEditorAssets::KeyShadow), keyWidth, keyHeight);

	oct1Key1 = juce::Point<float>(oct1Key1X * lumatoneBounds.getWidth() + lumatoneBounds.getX(), oct1Key1Y * lumatoneBounds.getHeight() + lumatoneBounds.getY());
	oct1Key56 = juce::Point<float>(oct1Key56X * lumatoneBounds.getWidth() + lumatoneBounds.getX(), oct1Key56Y * lumatoneBounds.getHeight() + lumatoneBounds.getY());
	oct5Key7 = juce::Point<float>(oct5Key7X * lumatoneBounds.getWidth() + lumatoneBounds.getX(), oct5Key7Y * lumatoneBounds.getHeight() + lumatoneBounds.getY());

	tilingGeometry.fitSkewedTiling(oct1Key1, oct1Key56, 10, oct5Key7, 24, false);

	juce::Array<juce::Point<float>> keyCentres = tilingGeometry.getHexagonCentresSkewed(boardGeometry, 0, NUMBEROFBOARDS);
	//juce::jassert(keyCentres.size() == TerpstraSysExApplication::getApp().getOctaveBoardSize() * NUMBEROFBOARDS);

	int octaveIndex = 0;
	octaveBoards[octaveIndex]->leftPos = keyCentres[0].getX() - keyWidth * 0.5;

	// TODO get from state
	const int octaveBoardSize = 56;

	for (int keyIndex = 0; keyIndex < keyCentres.size(); keyIndex++)
	{
		int keyOctaveIndex = keyIndex % octaveBoardSize;

		// Apply rotational transform
		juce::Point<int> centre = keyCentres[keyIndex].roundToInt();

		auto key = octaveBoards[octaveIndex]->keyMiniDisplay[keyOctaveIndex];
		key->setSize(keyWidth, keyHeight);
		key->setCentrePosition(centre);
		key->setKeyGraphics(keyShapeGraphic, keyShadowGraphic);


		if (keyOctaveIndex + 1 == octaveBoardSize)
		{
			octaveBoards[octaveIndex]->rightPos = key->getRight();
			octaveIndex++;

			if (octaveIndex < NUMBEROFBOARDS)
				octaveBoards[octaveIndex]->leftPos = key->getX();
		}
	}
}

void LumatoneKeyboardComponent::resetOctaveSize()
{
	// TODO get from state
	const int octaveBoardSize = 56;
	if (currentOctaveSize != octaveBoardSize)
	{
		boardGeometry = TerpstraBoardGeometry();
		octaveBoards.clear();

		for (int subBoardIndex = 0; subBoardIndex < NUMBEROFBOARDS; subBoardIndex++)
		{
			OctaveBoard* board = octaveBoards.add(new OctaveBoard());

			for (int keyIndex = 0; keyIndex < octaveBoardSize; keyIndex++)
			{
				auto key = board->keyMiniDisplay.add(new LumatoneKeyDisplay(subBoardIndex, keyIndex));
				addAndMakeVisible(key);
			}

			jassert(board->keyMiniDisplay.size() == octaveBoardSize);
		}

		currentOctaveSize = octaveBoardSize;
	}

	jassert(octaveBoards.size() == NUMBEROFBOARDS);
}
