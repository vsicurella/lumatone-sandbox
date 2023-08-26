#include "adjust_layout_colour.h"

AdjustLayoutColour::AdjustLayoutColour(LumatoneController* controllerIn)
    : controller(controllerIn)
    , hexMap(controllerIn->shareMappingData())
{
    endAction();
}

AdjustLayoutColour::~AdjustLayoutColour()
{
}

void AdjustLayoutColour::replaceColour(juce::Colour oldColour, juce::Colour newColour, bool sendUpdate)
{
    auto keyCoords = layoutBeforeAdjust.getKeysWithColour(oldColour);
    juce::Array<MappedLumatoneKey> keyUpdates;

    for (auto coord : keyCoords)
    {
        auto key = &currentLayout.getBoard(coord.boardIndex)->theKeys[coord.keyIndex];
        keyUpdates.add(MappedLumatoneKey(key->withColour(newColour), coord));
    }

    if (sendUpdate)
        sendSelectionUpdate(keyUpdates, true);
}

bool AdjustLayoutColour::rotateHue(float change, LumatoneKey& key) const
{
    if  (   key.colour.isTransparent()
        ||  (   (key.colour.getRed() == key.colour.getGreen())
            &&  (key.colour.getRed() == key.colour.getBlue())
            )
        )
        return false;
    
    key.colour = key.colour.withRotatedHue(change);
    return true;
}

void AdjustLayoutColour::rotateHue(float change, bool sendUpdate)
{
    auto coords = currentLayout.getAllKeyCoords();
    
    hueRotateValue = change;
    auto updatedKeys = updateAdjustedColoursState(coords);
    
    // rotateHue(change, coords, false);
    if (sendUpdate)
        sendSelectionUpdate(updatedKeys, true);
}

void AdjustLayoutColour::rotateHue(float change, const juce::Array<LumatoneKeyCoord>& selection, bool sendUpdate)
{
    if (currentAction != AdjustLayoutColour::Type::ROTATEHUE)
    {
        beginAction(AdjustLayoutColour::Type::ROTATEHUE);
        currentLayout = *controller->getMappingData();
    }

    juce::Array<MappedLumatoneKey> updateKeys;
    for (auto coord : selection)
    {
        LumatoneKey key = *currentLayout.readKey(coord.boardIndex, coord.keyIndex);
        key.colour = (&layoutBeforeAdjust.getBoard(coord.boardIndex)->theKeys[coord.keyIndex])->colour;
        if (rotateHue(change, key))
            updateKeys.add(MappedLumatoneKey(key, coord));
    }

    if (sendUpdate)
        sendSelectionUpdate(updateKeys, true);
}

bool AdjustLayoutColour::multiplyBrightness(float change, LumatoneKey& key) const
{
    if (key.colour.isTransparent())
        return false;

     key.colour = key.colour.withMultipliedBrightness(change);
     return true;
}

void AdjustLayoutColour::multiplyBrightness(float change, bool sendUpdate)
{
    auto coords = currentLayout.getAllKeyCoords();
    
    multiplyBrightnessValue = change;
    auto updatedKeys = updateAdjustedColoursState(coords);

    // multiplyBrightness(change, coords, false);
    if (sendUpdate)
        sendSelectionUpdate(updatedKeys, true);
}

void AdjustLayoutColour::multiplyBrightness(float change, const juce::Array<LumatoneKeyCoord>& selection, bool sendUpdate)
{
    if (currentAction != AdjustLayoutColour::Type::ADJUSTBRIGHTNESS)
    {
        beginAction(AdjustLayoutColour::Type::ADJUSTBRIGHTNESS);
        currentLayout = *controller->getMappingData();
    }

    juce::Array<MappedLumatoneKey> updateKeys;
    for (auto coord : selection)
    {
        auto key = *currentLayout.readKey(coord.boardIndex, coord.keyIndex);
        key.colour = (&layoutBeforeAdjust.getBoard(coord.boardIndex)->theKeys[coord.keyIndex])->colour;
        if (multiplyBrightness(change, key))
            updateKeys.add(MappedLumatoneKey(key, coord));
    }

    if (sendUpdate)
        sendSelectionUpdate(updateKeys, true);
}

bool AdjustLayoutColour::multiplySaturation(float change, LumatoneKey& key) const
{
    if (key.colour.isTransparent())
        return false;

    key.colour = key.colour.withMultipliedSaturation(change);
    return true;
}

void AdjustLayoutColour::multiplySaturation(float change, bool sendUpdate)
{
    auto coords = currentLayout.getAllKeyCoords();
    
    multiplySaturationValue = change;
    auto updatedKeys = updateAdjustedColoursState(coords);
    
    // multiplySaturation(change, coords, false);
    if (sendUpdate)
        sendSelectionUpdate(updatedKeys, true);
}

void AdjustLayoutColour::multiplySaturation(float change, const juce::Array<LumatoneKeyCoord>& selection, bool sendUpdate)
{
    if (currentAction != AdjustLayoutColour::Type::ADJUSTSATURATION)
    {
        beginAction(AdjustLayoutColour::Type::ADJUSTSATURATION);
        currentLayout= *controller->getMappingData();
    }

    juce::Array<MappedLumatoneKey> updateKeys;
    for (auto coord : selection)
    {
        auto key = *currentLayout.readKey(coord.boardIndex, coord.keyIndex);
        key.colour = (&layoutBeforeAdjust.getBoard(coord.boardIndex)->theKeys[coord.keyIndex])->colour;

        if (multiplySaturation(change, key))
            updateKeys.add(MappedLumatoneKey(key, coord));
    }

    if (sendUpdate)
        sendSelectionUpdate(updateKeys, true);
}

void AdjustLayoutColour::setGradient(SetGradientOptions options)
{
    float originColumn = 0;
    float originRow = 0;

    float furthestColumn = 0;
    float furthestRow = 0;

    juce::Array<int> presentColumns;
    juce::Array<int> presentRows;

    juce::Array<Hex::Point> updateHexCoords;
    for (auto coord : options.selection)
    {
        auto hex = hexMap.keyCoordsToHex(coord);
        updateHexCoords.add(hex);

        if (hex.q < originColumn)
            originColumn = hex.q;
        if (hex.r < originRow)
            originRow = hex.r;

        if (hex.q > furthestColumn)
            furthestColumn = hex.q;
        if (hex.r > furthestRow)
            furthestRow = hex.r;

        presentColumns.addIfNotAlreadyThere(hex.q);
        presentRows.addIfNotAlreadyThere(hex.r);
    }

    presentColumns.sort();
    presentRows.sort();

    auto selectionOrigin = Hex::Point(originColumn, originRow);
    auto furthestPoint = Hex::Point(furthestColumn, furthestRow);
    int selectionDistance = furthestPoint.distanceTo(selectionOrigin);

    auto boardOrigin = Hex::Point(0, 0);
    int maxBoardDistance = 35;

    juce::Array<MappedLumatoneKey> keyUpdates;

    float maxGradientDistance = 1.0f;
    if (options.selectionOrigin)
    {
        if (options.fillRelative)
            maxGradientDistance = presentColumns.size();
        else
            maxGradientDistance = selectionDistance;
    }
    else
    {
        maxGradientDistance = maxBoardDistance;
    }

    float keyGradientDistance = 0.0f;
    for (int i = 0; i < options.selection.size(); i++)
    {
        auto mappedKey = options.selection[i];
        auto hex = updateHexCoords[i];

        if (options.selectionOrigin)
            {
                if (options.fillRelative)
                {
                    keyGradientDistance = hex.q - presentColumns[0];
                }
                else
                {
                    keyGradientDistance = hex.distanceTo(selectionOrigin);
                }
            }
        else
        {
            keyGradientDistance = hex.distanceTo(boardOrigin);
        }

        float t = (maxGradientDistance == 0.0f) ? 0.0f : keyGradientDistance / maxGradientDistance;
        auto colour = options.gradient.getColourAtPosition(t);
        auto key = &currentLayout.getBoard(mappedKey.boardIndex)->theKeys[mappedKey.keyIndex];
        key->colour = colour;
        
        keyUpdates.add(MappedLumatoneKey(*key, mappedKey.boardIndex, mappedKey.keyIndex));
    }

    sendSelectionUpdate(keyUpdates, false);
}

void AdjustLayoutColour::beginAction(AdjustLayoutColour::Type type)
{
    if (type == AdjustLayoutColour::Type::NONE)
        return endAction();

    if (currentAction != type)
    {
        if (currentAction == AdjustLayoutColour::Type::NONE)
        {
            originalLayout = *controller->getMappingData();
        layoutBeforeAdjust = *controller->getMappingData();
        }

    }
    
    currentAction = type;
}

void AdjustLayoutColour::endAction()
{
    layoutBeforeAdjust = *controller->getMappingData();
    currentLayout = layoutBeforeAdjust;
    currentAction = AdjustLayoutColour::Type::NONE;
}

juce::Array<MappedLumatoneKey> AdjustLayoutColour::updateAdjustedColoursState(const juce::Array<LumatoneKeyCoord>& selection) const
{
    juce::Array<MappedLumatoneKey> updateKeys;

    for (auto coord : selection)
    {
        auto key = *layoutBeforeAdjust.readKey(coord.boardIndex, coord.keyIndex);
        auto keyCopy = key;

        if (multiplySaturationValue != 1.0f)
        {
            multiplySaturation(multiplySaturationValue, key);
        }

        if (multiplyBrightnessValue != 1.0f)
        {
            multiplyBrightness(multiplyBrightnessValue, key);
        }

        if (hueRotateValue != 0.0f)
        {
            rotateHue(hueRotateValue, key);
        }

    if (!key.colourIsEqual(keyCopy))
        updateKeys.add(MappedLumatoneKey(key, coord));
    }

    return updateKeys;
}

void AdjustLayoutColour::commitChanges()
{
    endAction();

    hueRotateValue = 0.0f;
    multiplySaturationValue = 1.0f;
    multiplyBrightnessValue = 1.0f;
}
void AdjustLayoutColour::resetChanges()
{
    controller->sendCompleteMapping(originalLayout);
    endAction();

    hueRotateValue = 0.0f;
    multiplySaturationValue = 1.0f;
    multiplyBrightnessValue = 1.0f;
}

void AdjustLayoutColour::sendSelectionUpdate(const juce::Array<MappedLumatoneKey>& keyUpdates, bool bufferUpdates)
{
    auto updateAction = new LumatoneEditAction::MultiKeyAssignAction(controller, keyUpdates, bufferUpdates);
    controller->performUndoableAction(updateAction);
    }

void AdjustLayoutColour::sendMappingUpdate(const LumatoneLayout& updatedLayout, bool bufferUpdates)
{
    for (int i = 0; i < controller->getNumBoards(); i++)
        controller->performUndoableAction(new LumatoneEditAction::SectionEditAction(controller, i, *updatedLayout.readBoard(i), bufferUpdates), i == 0, "AdjustLayoutColour");
}
