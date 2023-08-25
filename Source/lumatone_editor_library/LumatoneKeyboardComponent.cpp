/*
  ==============================================================================

    LumatoneKeyboardComponent.cpp
    Created: 24 Jul 2023 9:52:28pm
    Author:  Vincenzo

  ==============================================================================
*/

#include "LumatoneKeyboardComponent.h"
#include "./color/colour_model.h"

LumatoneKeyboardComponent::LumatoneKeyboardComponent(LumatoneState stateIn)
    : LumatoneMidiState(stateIn)
    , lumatoneMidiMap()
    , lumatoneRender(stateIn)
{
    renderMode = LumatoneComponentRenderMode::GraphicInteractive;
    resetOctaveSize(false);
    completeMappingLoaded(*state.getMappingData());

    addMouseListener(this, this);
    addKeyListener(this);
}

LumatoneKeyboardComponent::~LumatoneKeyboardComponent()
{
}

void LumatoneKeyboardComponent::paint (juce::Graphics& g)
{
    switch (renderMode)
    {
    case LumatoneComponentRenderMode::Graphic:
    case LumatoneComponentRenderMode::GraphicInteractive:
        g.drawImageAt(lumatoneGraphic, lumatoneBounds.getX(), lumatoneBounds.getY());
        break;
    
    case LumatoneComponentRenderMode::MaxRes:
        g.drawImageAt(currentRender, lumatoneBounds.getX(), lumatoneBounds.getY());
        break;
    }

    // Draw a line under the selected sub board
    // if (currentSetSelection >= 0 && currentSetSelection < state.getNumBoards())
    // {
    //     juce::Path selectionMarkPath;
    //     selectionMarkPath.startNewSubPath(octaveBoards[currentSetSelection]->leftPos, octaveLineY);
    //     selectionMarkPath.lineTo(octaveBoards[currentSetSelection]->rightPos, octaveLineY);

    //     juce::Colour lineColour = juce::Colours::white;// findColour(LumatoneKeyEdit::outlineColourId);
    //     g.setColour(lineColour);
    //     g.strokePath(selectionMarkPath, juce::PathStrokeType(1.0f));
    // }
}

void LumatoneKeyboardComponent::resized()
{
    currentWidth = getWidth();
    currentHeight = juce::roundToInt((float)getWidth() / imageAspect);

    if (currentHeight > getHeight())
    {
        currentHeight = getHeight();
        currentWidth = juce::roundToInt((float)currentHeight * imageAspect);
    }

    // Prepare position helpers for graphics
    lumatoneBounds.setBounds(juce::roundToInt((getWidth() - currentWidth) * 0.5f),
                             juce::roundToInt((getHeight() - currentHeight) * 0.5f),
                             currentWidth, 
                             currentHeight);

    octaveLineY = lumatoneBounds.getBottom() + juce::roundToInt(getHeight() * octaveLineYRatio);

    // int btnHeight = juce::roundToInt(getHeight() * saveLoadH);
    //resizeLabelWithHeight(lblFirmwareVersion.get(), btnHeight * 0.6f);
    // lblFirmwareVersion->setTopLeftPosition(lumatoneBounds.getX(), lumatoneBounds.getY() - btnHeight * 0.6f);

    keyWidth = juce::roundToInt(lumatoneBounds.getWidth() * keyW);
    keyHeight = juce::roundToInt(lumatoneBounds.getHeight() * keyH);

    switch (renderMode)
    {
        case LumatoneComponentRenderMode::Graphic:
        case LumatoneComponentRenderMode::GraphicInteractive:
            lumatoneGraphic = lumatoneRender.getResizedAsset(LumatoneAssets::ID::LumatoneGraphic, lumatoneBounds.getWidth(), lumatoneBounds.getHeight());
            keyShapeGraphic = lumatoneRender.getResizedAsset(LumatoneAssets::ID::KeyShape, keyWidth, keyHeight);
            break;
        case LumatoneComponentRenderMode::MaxRes:
            currentRender = lumatoneRender.getResizedRender(lumatoneBounds.getWidth(), lumatoneBounds.getHeight());
            break;
    }

    keyShadowGraphic = lumatoneRender.getResizedAsset(LumatoneAssets::ID::KeyShadow, keyWidth, keyHeight, true);

    int octaveIndex = 0;
    int octaveX = keyCentres[0].getX() * lumatoneBounds.getWidth() + lumatoneBounds.getX();
    octaveBoards[octaveIndex]->leftPos = octaveX - keyWidth * 0.5;

    const int octaveBoardSize = state.getOctaveBoardSize();

    for (int keyIndex = 0; keyIndex < keyCentres.size(); keyIndex++)
    {
        int keyOctaveIndex = keyIndex % octaveBoardSize;

        juce::Point<int> keyPos = juce::Point<int>(
            juce::roundToInt(keyCentres[keyIndex].x * lumatoneBounds.getWidth() + lumatoneBounds.getX() - keyWidth * 0.5f),
            juce::roundToInt(keyCentres[keyIndex].y * lumatoneBounds.getHeight() + lumatoneBounds.getY() - keyHeight * 0.5f)
        );

        auto key = octaveBoards[octaveIndex]->keyMiniDisplay[keyOctaveIndex];
        key->setKeyGraphics(keyShapeGraphic, keyShadowGraphic);
        key->setBounds(keyPos.x, keyPos.y, keyWidth, keyHeight);

        if (keyOctaveIndex + 1 == octaveBoardSize)
        {
            octaveBoards[octaveIndex]->rightPos = key->getRight();
            octaveIndex++;

            if (octaveIndex < state.getNumBoards())
                octaveBoards[octaveIndex]->leftPos = key->getX();
        }
    }
}

void LumatoneKeyboardComponent::resetOctaveSize(bool resetState)
{
    const int octaveBoardSize = state.getOctaveBoardSize();
    if (currentOctaveSize != octaveBoardSize)
    {
        octaveBoards.clear();

        for (int subBoardIndex = 0; subBoardIndex < state.getNumBoards(); subBoardIndex++)
        {
            OctaveBoard* board = octaveBoards.add(new OctaveBoard());

            for (int keyIndex = 0; keyIndex < octaveBoardSize; keyIndex++)
            {
                auto keyData = state.getKey(subBoardIndex, keyIndex);
                auto key = board->keyMiniDisplay.add(new LumatoneKeyDisplay(subBoardIndex, keyIndex, *keyData));
                key->setRenderMode(renderMode);
                addAndMakeVisible(key);
            }
        }

        currentOctaveSize = octaveBoardSize;
    }

    lumatoneRender.resetOctaveSize();
    keyCentres = lumatoneRender.getKeyCentres();

    if (resetState)
        resetLayoutState();
}

void LumatoneKeyboardComponent::setRenderMode(LumatoneComponentRenderMode modeIn)
{
    renderMode = modeIn;
    for (int boardIndex = 0; boardIndex < octaveBoards.size(); boardIndex++)
    {
        auto board = octaveBoards[boardIndex];

        for (int keyIndex = 0; keyIndex < board->keyMiniDisplay.size(); keyIndex++)
        {
            auto key = board->keyMiniDisplay[keyIndex];
            key->setRenderMode(renderMode);
        }
    }

    mappingUpdateCallback();
}

void LumatoneKeyboardComponent::completeMappingLoaded(LumatoneLayout mappingData)
{
    for (int boardIndex = 0; boardIndex < octaveBoards.size(); boardIndex++)
    {
        auto board = octaveBoards[boardIndex];

        for (int keyIndex = 0; keyIndex < board->keyMiniDisplay.size(); keyIndex++)
        {
            auto keyData = *mappingData.readKey(boardIndex, keyIndex);
            keyUpdateCallback(boardIndex, keyIndex, keyData, false);
            
        }
    }

    resetLayoutState();
}

void LumatoneKeyboardComponent::boardChanged(LumatoneBoard boardData)
{
    auto board = octaveBoards[boardData.board_idx];

    for (int keyIndex = 0; keyIndex < state.getOctaveBoardSize(); keyIndex++)
    {
        auto key = board->keyMiniDisplay[keyIndex];

        auto keyData = boardData.theKeys[keyIndex];
        keyUpdateCallback(boardData.board_idx, keyIndex, keyData, false);
    }

    resetLayoutState();
}

void LumatoneKeyboardComponent::keyChanged(int boardIndex, int keyIndex, LumatoneKey lumatoneKey)
{
    keyUpdateCallback(boardIndex, keyIndex, lumatoneKey);
}

void LumatoneKeyboardComponent::keyConfigChanged(int boardIndex, int keyIndex, LumatoneKey keyData)
{
    keyUpdateCallback(boardIndex, keyIndex, keyData);
}

void LumatoneKeyboardComponent::keyColourChanged(int boardIndex, int keyIndex, juce::Colour keyColour)
{
    keyUpdateCallback(boardIndex, keyIndex, *state.getKey(boardIndex, keyIndex));
}

void LumatoneKeyboardComponent::selectionChanged(juce::Array<MappedLumatoneKey> selection)
{
    auto paintKey = renderMode != LumatoneComponentRenderMode::MaxRes;
    for (auto mappedKey : selection)
    {
        keyUpdateCallback(mappedKey.boardIndex, mappedKey.keyIndex, mappedKey, paintKey);
    }

    if (renderMode == LumatoneComponentRenderMode::MaxRes)
        rerender();
}

void LumatoneKeyboardComponent::keyUpdateCallback(int boardIndex, int keyIndex, const LumatoneKey& newKey, bool doRepaint)
{
    auto key = octaveBoards[boardIndex]->keyMiniDisplay[keyIndex];

    key->setLumatoneKey(newKey, boardIndex, keyIndex);
    updateKeyColour(boardIndex, keyIndex, newKey.colour);

    if (!doRepaint)
        return;

    if (   key->keyType != newKey.keyType
        || key->channelNumber != newKey.channelNumber
        || key->noteNumber != newKey.noteNumber
        )
    {
        resetLayoutState();
    }
    else if (key->colour != newKey.colour)
    {
        if (renderMode == LumatoneComponentRenderMode::MaxRes)
            rerender();
        else
        {
            key->repaint();
        }
    }
    
}

void LumatoneKeyboardComponent::mappingUpdateCallback()
{
    if (renderMode == LumatoneComponentRenderMode::MaxRes)
        lumatoneRender.render();

    if (currentWidth == 0 || currentHeight == 0)
        return;
    
    resized();
    repaint(lumatoneBounds);
}

void LumatoneKeyboardComponent::rerender()
{
    lumatoneRender.render();
    currentRender = lumatoneRender.getResizedRender(lumatoneBounds.getWidth(), lumatoneBounds.getHeight());
    repaint(lumatoneBounds);
}

void LumatoneKeyboardComponent::updateKeyColour(int boardIndex, int keyIndex, const juce::Colour& colour)
{
    auto colourModel = state.getColourModel();
    auto modelColour = colourModel->getModelColour(colour);
    auto key = octaveBoards[boardIndex]->keyMiniDisplay[keyIndex];
    key->setDisplayColour(modelColour);
}

void LumatoneKeyboardComponent::resetLayoutState()
{
    lumatoneMidiMap.render(*state.getMappingData());
    LumatoneMidiState::reset();
    mappingUpdateCallback();
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

        switch (key->keyType)
        {
        case LumatoneKeyType::noteOnNoteOff:
            noteOn(key->channelNumber, key->noteNumber, (juce::uint8)127);
            updateKeyState(boardIndex, keyIndex, true);
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
        
        switch (key->keyType)
        {
        case LumatoneKeyType::noteOnNoteOff:
            noteOff(key->channelNumber, key->noteNumber, 0);
            updateKeyState(boardIndex, keyIndex, false);
            break;
        default:
            break;
        }
    }
}

void LumatoneKeyboardComponent::updateKeyState(int boardIndex, int keyIndex, bool keyIsDown)
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

    // if (state.isKeyCoordValid(lastOver))
    // {
    //     auto lastOverForMouse = octaveBoards[lastOver.boardIndex]->keyMiniDisplay[lastOver.keyIndex];
    // }


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
    bool keyChanged = lastDownCoord != keyCoord;
    onNewKey = validKey && (mouseKeyLastDown == nullptr || keyChanged);
    
    bool setLastNoteOff = !e.mods.isShiftDown() && (onNewKey || !validKey);
    if (mouseKeyLastDown != nullptr && setLastNoteOff)
    {
        switch (mouseKeyLastDown->keyType)
        {
        case LumatoneKeyType::noteOnNoteOff:
            noteOff(mouseKeyLastDown->channelNumber, mouseKeyLastDown->noteNumber, 0);
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

        switch (key->keyType)
        {
        case LumatoneKeyType::noteOnNoteOff:
            noteOn(key->channelNumber, key->noteNumber, 100);
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
            updateKeyState(coord.boardIndex, coord.keyIndex, true);
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
            updateKeyState(coord.boardIndex, coord.keyIndex, false);
        }
    }
}

void LumatoneKeyboardComponent::handleAftertouch(LumatoneMidiState* midiState, int midiChannel, int midiNote, juce::uint8 aftertouch)
{

}

void LumatoneKeyboardComponent::handleController(LumatoneMidiState* midiState, int midiChannel, int midiNote, juce::uint8 value)
{

}

