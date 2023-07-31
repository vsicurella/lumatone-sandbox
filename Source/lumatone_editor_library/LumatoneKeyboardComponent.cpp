/*
  ==============================================================================

    LumatoneKeyboardComponent.cpp
    Created: 24 Jul 2023 9:52:28pm
    Author:  Vincenzo

  ==============================================================================
*/

#include "LumatoneKeyboardComponent.h"

LumatoneKeyboardComponent::LumatoneKeyboardComponent(LumatoneState stateIn)
    : state(stateIn)
{
    resetOctaveSize();

    tilingGeometry.setColumnAngle(LUMATONEGRAPHICCOLUMNANGLE);
    tilingGeometry.setRowAngle(LUMATONEGRAPHICROWANGLE);

    LumatoneAssets::LoadAssets(LumatoneAssets::ID::LumatoneGraphic);
    //LumatoneAssets::LoadAssets(LumatoneAssets::ID::KeybedShadows);
    LumatoneAssets::LoadAssets(LumatoneAssets::ID::KeyShape);
    LumatoneAssets::LoadAssets(LumatoneAssets::ID::KeyShadow);
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

        juce::Colour lineColour = juce::Colours::white;// findColour(LumatoneKeyEdit::outlineColourId);
        g.setColour(lineColour);
        g.strokePath(selectionMarkPath, juce::PathStrokeType(1.0f));
    }
}

void LumatoneKeyboardComponent::resized()
{
    graphicWidthUsed = getWidth();
    graphicHeightUsed = getWidth() / imageAspect;

    if (graphicHeightUsed > getHeight())
    {
        graphicHeightUsed = getHeight();
        graphicWidthUsed = graphicHeightUsed * imageAspect;
    }

    // Prepare position helpers for graphics
    lumatoneBounds.setBounds(juce::roundToInt((getWidth() - graphicWidthUsed) * 0.5f),
                             juce::roundToInt((getHeight() - graphicHeightUsed) * 0.5f),
                             graphicWidthUsed, 
                             graphicHeightUsed);

    octaveLineY = lumatoneBounds.getBottom() + juce::roundToInt(getHeight() * octaveLineYRatio);

    // int btnHeight = juce::roundToInt(getHeight() * saveLoadH);
    //resizeLabelWithHeight(lblFirmwareVersion.get(), btnHeight * 0.6f);
    // lblFirmwareVersion->setTopLeftPosition(lumatoneBounds.getX(), lumatoneBounds.getY() - btnHeight * 0.6f);

    int keyWidth = juce::roundToInt(lumatoneBounds.getWidth() * keyW);
    int keyHeight = juce::roundToInt(lumatoneBounds.getHeight() * keyH);

    // Scale key graphics once
    lumatoneGraphic = getResizedImage(LumatoneAssets::ID::LumatoneGraphic, lumatoneBounds.getWidth(), lumatoneBounds.getHeight());
    keyShapeGraphic = getResizedImage(LumatoneAssets::ID::KeyShape, keyWidth, keyHeight);
    keyShadowGraphic = getResizedImage(LumatoneAssets::ID::KeyShadow, keyWidth, keyHeight);

    oct1Key1 = juce::Point<float>(oct1Key1X * lumatoneBounds.getWidth() + lumatoneBounds.getX(), oct1Key1Y * lumatoneBounds.getHeight() + lumatoneBounds.getY());
    oct1Key56 = juce::Point<float>(oct1Key56X * lumatoneBounds.getWidth() + lumatoneBounds.getX(), oct1Key56Y * lumatoneBounds.getHeight() + lumatoneBounds.getY());
    oct5Key7 = juce::Point<float>(oct5Key7X * lumatoneBounds.getWidth() + lumatoneBounds.getX(), oct5Key7Y * lumatoneBounds.getHeight() + lumatoneBounds.getY());

    tilingGeometry.fitSkewedTiling(oct1Key1, oct1Key56, 10, oct5Key7, 24, false);

    juce::Array<juce::Point<float>> keyCentres = tilingGeometry.getHexagonCentresSkewed(lumatoneGeometry, 0, NUMBEROFBOARDS);

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
    const int octaveBoardSize = state.getOctaveBoardSize();
    if (currentOctaveSize != octaveBoardSize)
    {
        lumatoneGeometry = LumatoneGeometry();
        octaveBoards.clear();

        for (int subBoardIndex = 0; subBoardIndex < NUMBEROFBOARDS; subBoardIndex++)
        {
            OctaveBoard* board = octaveBoards.add(new OctaveBoard());

            for (int keyIndex = 0; keyIndex < octaveBoardSize; keyIndex++)
            {
                auto keyData = state.getKey(subBoardIndex, keyIndex);
                auto key = board->keyMiniDisplay.add(new LumatoneKeyDisplay(subBoardIndex, keyIndex, *keyData));
                addAndMakeVisible(key);
            }

            jassert(board->keyMiniDisplay.size() == octaveBoardSize);
        }

        currentOctaveSize = octaveBoardSize;
    }

    jassert(octaveBoards.size() == NUMBEROFBOARDS);
}

void LumatoneKeyboardComponent::completeMappingLoaded(LumatoneLayout mappingData)
{
    for (int boardIndex = 0; boardIndex < octaveBoards.size(); boardIndex++)
    {
        auto board = octaveBoards[boardIndex];

        for (int keyIndex = 0; keyIndex < board->keyMiniDisplay.size(); keyIndex++)
        {
            auto key = board->keyMiniDisplay[keyIndex];
            key->setLumatoneKey(*mappingData.readKey(boardIndex, keyIndex));
            key->repaint();
        }
    }
}

void LumatoneKeyboardComponent::boardChanged(LumatoneBoard boardData)
{
    auto board = octaveBoards[boardData.board_idx];

    for (int keyIndex = 0; keyIndex < state.getOctaveBoardSize(); keyIndex++)
    {
        auto key = board->keyMiniDisplay[keyIndex];
        key->setLumatoneKey(boardData.theKeys[keyIndex]);
        key->repaint();
    }
}

void LumatoneKeyboardComponent::keyChanged(int boardIndex, int keyIndex, LumatoneKey lumatoneKey)
{
    auto key = octaveBoards[boardIndex]->keyMiniDisplay[keyIndex];
    key->setLumatoneKey(lumatoneKey);
    key->repaint();
}

juce::Image LumatoneKeyboardComponent::getResizedImage(LumatoneAssets::ID assetId, int targetWidth, int targetHeight)
{
    auto cachedImage = LumatoneAssets::getImage(assetId, targetHeight, targetWidth);
    return imageProcessor->resizeImage(cachedImage, targetWidth, targetHeight);
}
