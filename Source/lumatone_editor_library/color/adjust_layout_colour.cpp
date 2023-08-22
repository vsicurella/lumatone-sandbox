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
        sendSelectionUpdate(keyUpdates);
}

void AdjustLayoutColour::rotateHue(float change, bool sendUpdate)
{
    auto coords = currentLayout.getAllKeyCoords();
    rotateHue(change, coords, false);
    if (sendUpdate)
        sendMappingUpdate(currentLayout);
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
        auto key = &currentLayout.getBoard(coord.boardIndex)->theKeys[coord.keyIndex];
        auto colour = (&layoutBeforeAdjust.getBoard(coord.boardIndex)->theKeys[coord.keyIndex])->colour;
        if (colour.isTransparent() 
            || (colour.getRed() == colour.getGreen() && colour.getRed() == colour.getBlue())
            )
            continue;

        auto rotated = colour.withRotatedHue(change);
        key->colour = rotated;
        updateKeys.add(MappedLumatoneKey(*key, coord));
    }

    if (sendUpdate)
        sendSelectionUpdate(updateKeys);
}

void AdjustLayoutColour::multiplyBrightness(float change, bool sendUpdate)
{
    auto coords = currentLayout.getAllKeyCoords();
    multiplyBrightness(change, coords, false);
    if (sendUpdate)
        sendMappingUpdate(currentLayout);
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
        auto key = &currentLayout.getBoard(coord.boardIndex)->theKeys[coord.keyIndex];
        auto colour = (&layoutBeforeAdjust.getBoard(coord.boardIndex)->theKeys[coord.keyIndex])->colour;
        if (colour.isTransparent())
            continue;

        auto adjusted = colour.withMultipliedBrightness(change);
        key->colour = adjusted;
        updateKeys.add(MappedLumatoneKey(*key, coord));
    }

    if (sendUpdate)
        sendSelectionUpdate(updateKeys);
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

    sendSelectionUpdate(keyUpdates);
}

void AdjustLayoutColour::beginAction(AdjustLayoutColour::Type type)
{
    if (type == AdjustLayoutColour::Type::NONE)
        return endAction();

    if (currentAction != type)
    {
        if (currentAction == AdjustLayoutColour::Type::NONE)
        {
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

void AdjustLayoutColour::commitChanges()
{
    endAction();
}
void AdjustLayoutColour::resetChanges()
{
    controller->sendCompleteMapping(layoutBeforeAdjust);
    endAction();
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
