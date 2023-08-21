#include "adjust_layout_colour.h"

AdjustLayoutColour::AdjustLayoutColour(LumatoneController* controllerIn)
    : controller(controllerIn)
    , hexMap(controllerIn->shareMappingData())
{
    controller->addEditorListener(this);
    endAction();
}

AdjustLayoutColour::~AdjustLayoutColour()
{
    controller->removeEditorListener(this);
}

void AdjustLayoutColour::replaceColour(juce::Colour oldColour, juce::Colour newColour)
{
    sendColourUpdate(oldColour, newColour);
}

void AdjustLayoutColour::rotateHue(float change)
{
    if (currentAction != AdjustLayoutColour::Type::ROTATEHUE)
    {
        beginAction(AdjustLayoutColour::Type::ROTATEHUE);
        currentLayout = *controller->getMappingData();
    }

    for (int boardIndex = 0; boardIndex < controller->getNumBoards(); boardIndex++)
    {
        for (int keyIndex = 0; keyIndex < controller->getOctaveBoardSize(); keyIndex++)
        {
            auto key = &currentLayout.getBoard(boardIndex)->theKeys[keyIndex];
            auto colour = (&layoutBeforeAdjust.getBoard(boardIndex)->theKeys[keyIndex])->colour;
            if (colour.isTransparent() 
                || (colour.getRed() == colour.getGreen() && colour.getRed() == colour.getBlue())
                )
                continue;

            auto rotated = colour.withRotatedHue(change);
            key->colour = rotated;
        }
    }

    totalHueAdjustment += change;
    sendMappingUpdate(currentLayout);
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
    for (auto mappedKey : options.selection)
    {
        auto hex = hexMap.keyCoordsToHex(mappedKey.getKeyCoord());
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

        float t = keyGradientDistance / maxGradientDistance;
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

void AdjustLayoutColour::sendColourUpdate(juce::Colour oldColour, juce::Colour newColour)
{
    auto keyCoords = controller->getMappingData()->getKeysWithColour(oldColour);
    juce::Array<MappedLumatoneKey> keyUpdates;

    for (auto coord : keyCoords)
    {
        auto key = controller->getKey(coord);
        keyUpdates.add(MappedLumatoneKey(key->withColour(newColour), coord));
    }

    sendSelectionUpdate(keyUpdates);
}

void AdjustLayoutColour::sendSelectionUpdate(const juce::Array<MappedLumatoneKey>& keyUpdates)
{
    auto updateAction = new LumatoneEditAction::MultiKeyAssignAction(controller, keyUpdates);
    controller->performUndoableAction(updateAction);
}

void AdjustLayoutColour::sendMappingUpdate(const LumatoneLayout& updatedLayout)
{
    for (int i = 0; i < controller->getNumBoards(); i++)
        controller->performUndoableAction(new LumatoneEditAction::SectionEditAction(controller, i, *updatedLayout.readBoard(i)), i == 0, "AdjustLayoutColour");
}

void AdjustLayoutColour::completeMappingLoaded(LumatoneLayout mappingData)
{
}

void AdjustLayoutColour::boardChanged(LumatoneBoard boardData)
{
}

void AdjustLayoutColour::keyChanged(int boardIndex, int keyIndex, LumatoneKey lumatoneKey)
{
}
