/*
  ==============================================================================

    LumatoneKeyboardComponent.cpp
    Created: 24 Jul 2023 9:52:28pm
    Author:  Vincenzo

  ==============================================================================
*/

#include "LumatoneKeyboardComponent.h"

LumatoneKeyboardComponent::LumatoneKeyboardComponent(LumatoneState stateIn)
    : LumatoneMidiState(stateIn)
    , lumatoneMidiMap()
{
    resetOctaveSize();
    completeMappingLoaded(*state.getMappingData());

    addMouseListener(this, this);

    addKeyListener(this);

    prepareHexTiling();

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

    // DBG("KEYBOARD COMPONENT REPAINT");

    // Draw a line under the selected sub board
    if (currentSetSelection >= 0 && currentSetSelection < state.getNumBoards())
    {
        juce::Path selectionMarkPath;
        selectionMarkPath.startNewSubPath(octaveBoards[currentSetSelection]->leftPos, octaveLineY);
        selectionMarkPath.lineTo(octaveBoards[currentSetSelection]->rightPos, octaveLineY);

        juce::Colour lineColour = juce::Colours::white;// findColour(LumatoneKeyEdit::outlineColourId);
        g.setColour(lineColour);
        g.strokePath(selectionMarkPath, juce::PathStrokeType(1.0f));
    }
}

void LumatoneKeyboardComponent::prepareHexTiling()
{
    // tilingGeometry.setColumnAngle(LUMATONEGRAPHICCOLUMNANGLE);
    // tilingGeometry.setRowAngle(LUMATONEGRAPHICROWANGLE);

    oct1Key1 = juce::Point<float>(oct1Key1X, oct1Key1Y);
    oct1Key56 = juce::Point<float>(oct1Key56X, oct1Key56Y);
    oct5Key7 = juce::Point<float>(oct5Key7X, oct5Key7Y);

    tilingGeometry.fitSkewedTiling(oct1Key1, oct1Key56, 10, oct5Key7, 24, true);
    keyCentres = tilingGeometry.getHexagonCentresSkewed(lumatoneGeometry, 0, state.getNumBoards());
}

void LumatoneKeyboardComponent::resized()
{
    graphicWidthUsed = getWidth();
    graphicHeightUsed = juce::roundToInt((float)getWidth() / imageAspect);

    if (graphicHeightUsed > getHeight())
    {
        graphicHeightUsed = getHeight();
        graphicWidthUsed = juce::roundToInt((float)graphicHeightUsed * imageAspect);
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

    int octaveIndex = 0;
    int octaveX = keyCentres[0].getX() * lumatoneBounds.getWidth() + lumatoneBounds.getX();
    octaveBoards[octaveIndex]->leftPos = octaveX - keyWidth * 0.5;

    // TODO get from state
    const int octaveBoardSize = 56;

    for (int keyIndex = 0; keyIndex < keyCentres.size(); keyIndex++)
    {
        int keyOctaveIndex = keyIndex % octaveBoardSize;

        // Apply rotational transform
        juce::Point<int> centre = juce::Point<int>(
            juce::roundToInt(keyCentres[keyIndex].x * lumatoneBounds.getWidth() + lumatoneBounds.getX()),
            juce::roundToInt(keyCentres[keyIndex].y * lumatoneBounds.getHeight() + lumatoneBounds.getY())
        );

        auto key = octaveBoards[octaveIndex]->keyMiniDisplay[keyOctaveIndex];
        key->setSize(keyWidth, keyHeight);
        key->setCentrePosition(centre);
        key->setKeyGraphics(keyShapeGraphic, keyShadowGraphic);

        if (keyOctaveIndex + 1 == octaveBoardSize)
        {
            octaveBoards[octaveIndex]->rightPos = key->getRight();
            octaveIndex++;

            if (octaveIndex < state.getNumBoards())
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

        for (int subBoardIndex = 0; subBoardIndex < state.getNumBoards(); subBoardIndex++)
        {
            OctaveBoard* board = octaveBoards.add(new OctaveBoard());

            for (int keyIndex = 0; keyIndex < octaveBoardSize; keyIndex++)
            {
                auto keyData = state.getKey(subBoardIndex, keyIndex);
                auto key = board->keyMiniDisplay.add(new LumatoneKeyDisplay(subBoardIndex, keyIndex, *keyData));
                addAndMakeVisible(key);
            }
        }

        currentOctaveSize = octaveBoardSize;
    }
}

void LumatoneKeyboardComponent::completeMappingLoaded(LumatoneLayout mappingData)
{
    auto colourModel = state.getColourModel();

    for (int boardIndex = 0; boardIndex < octaveBoards.size(); boardIndex++)
    {
        auto board = octaveBoards[boardIndex];

        for (int keyIndex = 0; keyIndex < board->keyMiniDisplay.size(); keyIndex++)
        {
            auto key = board->keyMiniDisplay[keyIndex];
            auto keyData = *mappingData.readKey(boardIndex, keyIndex);
            key->setLumatoneKey(keyData, boardIndex, keyIndex);

            auto modelColour = colourModel->getModelColour(keyData.colour);
            key->setDisplayColour(modelColour);

            // key->repaint();
        }
    }

    repaint();

    resetLayoutState();
}

void LumatoneKeyboardComponent::boardChanged(LumatoneBoard boardData)
{
    auto colourModel = state.getColourModel();

    auto board = octaveBoards[boardData.board_idx];

    for (int keyIndex = 0; keyIndex < state.getOctaveBoardSize(); keyIndex++)
    {
        auto key = board->keyMiniDisplay[keyIndex];

        auto keyData = boardData.theKeys[keyIndex];
        key->setLumatoneKey(keyData, boardData.board_idx, keyIndex);

        auto modelColour = colourModel->getModelColour(keyData.colour);
        key->setDisplayColour(modelColour);
        
        key->repaint();
    }

    resetLayoutState();
}

void LumatoneKeyboardComponent::keyChanged(int boardIndex, int keyIndex, LumatoneKey lumatoneKey)
{
    auto colourModel = state.getColourModel();

    auto key = octaveBoards[boardIndex]->keyMiniDisplay[keyIndex];

    LumatoneKey oldKey = *key->getKeyData();

    key->setLumatoneKey(lumatoneKey, boardIndex, keyIndex);
    
    auto modelColour = colourModel->getModelColour(lumatoneKey.colour);
    key->setDisplayColour(modelColour);

    key->repaint();

    if (   oldKey.keyType != lumatoneKey.keyType
        || oldKey.channelNumber != lumatoneKey.channelNumber
        || oldKey.noteNumber != lumatoneKey.noteNumber
        )
    {
        resetLayoutState();
    }
    
}

void LumatoneKeyboardComponent::resetLayoutState()
{
    lumatoneMidiMap.render(*state.getMappingData());
    LumatoneMidiState::reset();
}

void LumatoneKeyboardComponent::clearHeldNotes()
{
    for (auto key : keysOn)
    {
        if (key)
        {
            key->clearUiState();
        }
    }

    keysOn.clear();
}

void LumatoneKeyboardComponent::sustainStarted()
{

}

void LumatoneKeyboardComponent::sustainEnded()
{
    clearHeldNotes();
}

void LumatoneKeyboardComponent::lumatoneKeyDown(int boardIndex, int keyIndex)
{
    auto coord = LumatoneKeyCoord(boardIndex, keyIndex);
    if (state.isKeyCoordValid(coord))
    {
        auto key = octaveBoards[boardIndex]->keyMiniDisplay[keyIndex];
        jassert(key != nullptr);

        auto keyData = key->getKeyData();
        switch (keyData->keyType)
        {
        case LumatoneKeyType::noteOnNoteOff:
            noteOn(keyData->channelNumber, keyData->noteNumber, (juce::uint8)127);
            showKeyDown(boardIndex, keyIndex, true);
            break;
        default:
            break;
        }
    }
}

void LumatoneKeyboardComponent::lumatoneKeyUp(int boardIndex, int keyIndex)
{
    auto coord = LumatoneKeyCoord(boardIndex, keyIndex);
    if (state.isKeyCoordValid(coord))
    {
        auto key = octaveBoards[boardIndex]->keyMiniDisplay[keyIndex];
        jassert(key != nullptr);
        
        auto keyData = key->getKeyData();
        switch (keyData->keyType)
        {
        case LumatoneKeyType::noteOnNoteOff:
            noteOff(keyData->channelNumber, keyData->noteNumber, 0);
            showKeyDown(boardIndex, keyIndex, false);
            break;
        default:
            break;
        }
    }
}

void LumatoneKeyboardComponent::showKeyDown(int boardIndex, int keyIndex, bool keyIsDown)
{
    auto keyComponent = octaveBoards[boardIndex]->keyMiniDisplay[keyIndex];

    jassert(keyComponent != nullptr);
    if (keyComponent != nullptr)
    {
        if (keyIsDown)
            keyComponent->noteOn();
        else
            keyComponent->noteOff();
    }
}

juce::Image LumatoneKeyboardComponent::getResizedImage(LumatoneAssets::ID assetId, int targetWidth, int targetHeight)
{
    auto cachedImage = LumatoneAssets::getImage(assetId, targetHeight, targetWidth);
    return imageProcessor->resizeImage(cachedImage, targetWidth, targetHeight);
}

LumatoneKeyDisplay* LumatoneKeyboardComponent::getKeyFromMouseEvent(const juce::MouseEvent& e)
{
    juce::Point<float> position = e.position;

    if (e.eventComponent != this)
    {
        position = e.getEventRelativeTo(this).position;
    }

    auto child = getComponentAt(position);
    LumatoneKeyDisplay* key = nullptr;
    if (child && child->getParentComponent() == this)
        key = (LumatoneKeyDisplay*)child;

    return key;
}

void LumatoneKeyboardComponent::mouseMove(const juce::MouseEvent& e)
{
    const int mouseIndex = e.source.getIndex();
    auto lastOver = keysOverPerMouse[mouseIndex];

    LumatoneKeyCoord keyCoord;
    auto key = getKeyFromMouseEvent(e);
    if (key)
    {
        keyCoord = key->getCoord();
    }

    if (state.isKeyCoordValid(lastOver))
    {
        auto lastOverForMouse = octaveBoards[lastOver.boardIndex]->keyMiniDisplay[lastOver.keyIndex];
    }


    keysOverPerMouse.set(mouseIndex, keyCoord);
}

void LumatoneKeyboardComponent::mouseDown(const juce::MouseEvent& e)
{
    auto key = getKeyFromMouseEvent(e);
    if (key)
    {
        lumatoneKeyDown(key->getBoardIndex(), key->getKeyIndex());
        lastMouseKeyDown = key;

        keysDownPerMouse.set(e.source.getIndex(), key->getCoord());
        keysOn.addIfNotAlreadyThere(key);
    }
}

void LumatoneKeyboardComponent::mouseUp(const juce::MouseEvent& e)
{
    if (e.mods.isShiftDown())
    {
        
    }
    else if (lastMouseKeyDown)
    {
        lumatoneKeyUp(lastMouseKeyDown->getBoardIndex(), lastMouseKeyDown->getKeyIndex());
        keysOn.removeFirstMatchingValue(lastMouseKeyDown);
    }

    keysDownPerMouse.set(e.source.getIndex(), LumatoneKeyCoord());
}

void LumatoneKeyboardComponent::mouseDrag(const juce::MouseEvent& e)
{
    const int mouseIndex = e.source.getIndex();
    auto lastDownCoord = keysDownPerMouse[mouseIndex];

    LumatoneKeyDisplay* mouseKeyLastDown = nullptr;
    if (state.isKeyCoordValid(lastDownCoord))
        mouseKeyLastDown = octaveBoards[lastDownCoord.boardIndex]->keyMiniDisplay[lastDownCoord.keyIndex];


    bool onNewKey = false;

    auto key = getKeyFromMouseEvent(e);
    LumatoneKeyCoord keyCoord;
    if (key)
    {
        keyCoord = key->getCoord();
    }

    bool validKey = state.isKeyCoordValid(keyCoord);
    onNewKey = validKey && (mouseKeyLastDown == nullptr || lastDownCoord != keyCoord);

    bool setLastNoteOff = !e.mods.isShiftDown() && (onNewKey || !validKey);
    if (mouseKeyLastDown != nullptr && setLastNoteOff)
    {
        auto keyData = mouseKeyLastDown->getKeyData();
        switch (keyData->keyType)
        {
        case LumatoneKeyType::noteOnNoteOff:
            mouseKeyLastDown->endDrag();
            keysOn.removeFirstMatchingValue(mouseKeyLastDown);
            break;
        default:
            break;
        }
    }

    if (onNewKey)
    {
        keysDownPerMouse.set(mouseIndex, keyCoord);
        keysOverPerMouse.set(mouseIndex, keyCoord);

        auto keyData = key->getKeyData();
        switch (keyData->keyType)
        {
        case LumatoneKeyType::noteOnNoteOff:
            key->startDrag();
            keysOn.addIfNotAlreadyThere(key);
            break;
        default:
            break;
        }

        lastMouseKeyOver = key;
        lastMouseKeyDown = key;
    }
    else if (!e.mods.isShiftDown())
    {
        if (validKey)
        {
            
        }
        else
        {
            keysDownPerMouse.set(mouseIndex, LumatoneKeyCoord());
            keysOverPerMouse.set(mouseIndex, LumatoneKeyCoord());
        }
    }
}


bool LumatoneKeyboardComponent::keyStateChanged(bool isKeyDown)
{
    //if (!juce::KeyPress::isKeyCurrentlyDown(juce::KeyPress::upKey) && upHeld)
    //{
    //    upHeld = false;
    //    return true;
    //}

    //if (!juce::KeyPress::isKeyCurrentlyDown(juce::KeyPress::downKey) && downHeld)
    //{
    //    downHeld = false;
    //    return true;
    //}

    //if (!juce::KeyPress::isKeyCurrentlyDown(juce::KeyPress::leftKey) && leftHeld)
    //{
    //    leftHeld = false;
    //    return true;
    //}

    //if (!juce::KeyPress::isKeyCurrentlyDown(juce::KeyPress::rightKey) && rightHeld)
    //{
    //    rightHeld = false;
    //    return true;
    //}

    //if (!juce::KeyPress::isKeyCurrentlyDown(juce::KeyPress::spaceKey) && spaceHeld)
    //{
    //    spaceHeld = false;
    //    return true;
    //}

    return false;
}

bool LumatoneKeyboardComponent::keyPressed(const juce::KeyPress& key, Component* originatingComponent)
{
    return false;
}

void LumatoneKeyboardComponent::modifierKeysChanged(const juce::ModifierKeys& modifiers)
{
    //if (!rightMouseHeld && modifiers.isRightButtonDown())
    //{
    //    rightMouseHeld = true;
    //}
    //if (rightMouseHeld && !modifiers.isRightButtonDown())
    //{
    //    rightMouseHeld = false;
    //}

    if (!shiftHeld && modifiers.isShiftDown())
    {
        shiftHeld = true;
    }

    if (shiftHeld && !modifiers.isShiftDown())
    {
        shiftHeld = false;

        clearHeldNotes();

    }

    //if (!altHeld && modifiers.isAltDown())
    //{
    //    altHeld = true;
    //    if (uiModeSelected != UIMode::editMode)
    //    {
    //        isolateLastNote();
    //        repaint();
    //    }
    //}

    //else if (altHeld && !modifiers.isAltDown())
    //{
    //    altHeld = false;
    //}

    //if (!ctrlHeld && modifiers.isCtrlDown())
    //{
    //    ctrlHeld = true;
    //}

    //else if (ctrlHeld && !modifiers.isCtrlDown())
    //{
    //    ctrlHeld = false;
    //}
}

void LumatoneKeyboardComponent::noteOnInternal(const juce::MidiMessage& msg, int midiChannel, int midiNote, juce::uint8 velocity)
{
    LumatoneMidiState::noteOnInternal(msg, midiChannel, midiNote, velocity);
    handleNoteOn(nullptr, midiChannel, midiNote, velocity);
}

void LumatoneKeyboardComponent::noteOffInternal(const juce::MidiMessage& msg, int midiChannel, int midiNote, juce::uint8 velocity)
{
    LumatoneMidiState::noteOffInternal(msg, midiChannel, midiNote, velocity);
    handleNoteOff(nullptr, midiChannel, midiNote);
}

void LumatoneKeyboardComponent::handleNoteOn(LumatoneMidiState* midiState, int midiChannel, int midiNote, juce::uint8 velocity)
{
    auto mappedKeyCoords = lumatoneMidiMap.getKeysAssignedToNoteOn(midiChannel, midiNote);
    for (auto coord : mappedKeyCoords)
    {
        if (state.isKeyCoordValid(coord))
        {
            showKeyDown(coord.boardIndex, coord.keyIndex, true);
        }
    }
}

void LumatoneKeyboardComponent::handleNoteOff(LumatoneMidiState* midiState, int midiChannel, int midiNote)
{
    auto mappedKeyCoords = lumatoneMidiMap.getKeysAssignedToNoteOn(midiChannel, midiNote);
    for (auto coord : mappedKeyCoords)
    {
        if (state.isKeyCoordValid(coord))
        {
            showKeyDown(coord.boardIndex, coord.keyIndex, false);
        }
    }
}

void LumatoneKeyboardComponent::handleAftertouch(LumatoneMidiState* midiState, int midiChannel, int midiNote, juce::uint8 aftertouch)
{

}

void LumatoneKeyboardComponent::handleController(LumatoneMidiState* midiState, int midiChannel, int midiNote, juce::uint8 value)
{

}

