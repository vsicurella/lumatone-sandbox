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

void AdjustLayoutColour::adjustWhiteBalance(int newWhitePoint, bool sendUpdate)
{
    auto coords = currentLayout.getAllKeyCoords();
    
    whiteKelvinValue = newWhitePoint;
    auto updatedKeys = updateAdjustedColoursState(coords);
    
    // multiplySaturation(change, coords, false);
    if (sendUpdate)
        sendSelectionUpdate(updatedKeys, true);
}

void AdjustLayoutColour::adjustWhiteBalance(int newWhitePoint, const juce::Array<LumatoneKeyCoord>& selection, bool sendUpdate)
{
    if (currentAction != AdjustLayoutColour::Type::ADJUSTWHITE)
    {
        beginAction(AdjustLayoutColour::Type::ADJUSTWHITE);
        currentLayout= *controller->getMappingData();
    }

    juce::Array<MappedLumatoneKey> updateKeys;
    for (auto coord : selection)
    {
        auto key = *currentLayout.readKey(coord.boardIndex, coord.keyIndex);
        key.colour = (&layoutBeforeAdjust.getBoard(coord.boardIndex)->theKeys[coord.keyIndex])->colour;

        if (adjustWhiteBalance(newWhitePoint, key))
            updateKeys.add(MappedLumatoneKey(key, coord));
    }

    if (sendUpdate)
        sendSelectionUpdate(updateKeys, true);
}

bool AdjustLayoutColour::adjustWhiteBalance(int newWhitePoint, LumatoneKey& key) const
{
    if (key.colour.isTransparent())
        return false;
        
    // adjustWhiteBalanceLab(newWhitePoint, key);
    adjustWhiteBalanceRgb(newWhitePoint, key);

    return true;
}

void AdjustLayoutColour::adjustWhiteBalanceRgb(int newWhitePoint, LumatoneKey& key) const
{
    auto newWhiteColour = kelvinToColour(newWhitePoint);
    key.colour = juce::Colour(LumatoneEditor::roundToUint8(key.colour.getRed() * newWhiteColour.getFloatRed()),
                            LumatoneEditor::roundToUint8(key.colour.getGreen() * newWhiteColour.getFloatGreen()),
                            LumatoneEditor::roundToUint8(key.colour.getBlue() * newWhiteColour.getFloatBlue()));
}

void AdjustLayoutColour::adjustWhiteBalanceLab(int newWhitePoint, LumatoneKey& key) const
{
    auto newWhiteColour = kelvinToColour(newWhitePoint);

    auto colourLab = rgbToLab(key.colour);
    auto oldWhiteLab = rgbToLab(juce::Colours::white);
    auto newWhiteLab = rgbToLab(newWhiteColour);
    LAB adjustedLab = { 
        colourLab.L + (newWhiteLab.L - oldWhiteLab.L) / 3.0f, 
        colourLab.A + (newWhiteLab.A - oldWhiteLab.A) / 3.0f, 
        colourLab.B + (newWhiteLab.B - oldWhiteLab.B) / 3.0f, 
        };

    key.colour = labToRgb(adjustedLab);
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

        if (whiteKelvinValue != 6500)
        {
            adjustWhiteBalance(whiteKelvinValue, key);
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
    whiteKelvinValue = 6500;
}
void AdjustLayoutColour::resetChanges()
{
    controller->sendCompleteMapping(originalLayout);
    endAction();

    hueRotateValue = 0.0f;
    multiplySaturationValue = 1.0f;
    multiplyBrightnessValue = 1.0f;
    whiteKelvinValue = 6500;
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

// juce::Colour AdjustLayoutColour::inverseSRGB(juce::Colour rgb)
// {
//     float lr, lg, lb;

//     for (int ch = 0; ch < 3; ch++)
//     {
//         float v = 0.0f;
//         switch (ch)
//         {
//         case 0:
//             v = rgb.getFloatRed();
//             break;
//         case 1:
//             v = rgb.getFloatBlue();
//             break;
//         case 2:
//             v = rgb.getFloatGreen();
//             break;
//         }

//         if (v < 0.04045f)
//             v = v / 12.92f;
//         else
//             v = powf((v + 0.055f) / 1.055f, 2.4);

//         switch (ch)
//         {
//         case 0:
//             lr = v;
//             break;
//         case 1:
//             lg = v;
//             break;
//         case 2:
//             lb = v;
//             break;
//         }        
//     }

//     juce::Colour srgb = juce::Colour((juce::uint8)juce::roundToInt(lr * 255.0f),
//                                     (juce::uint8)juce::roundToInt(lg * 255.0f),
//                                     (juce::uint8)juce::roundToInt(lb * 255.0f));

//     return srgb.withAlpha(1.0f);
// }

AdjustLayoutColour::LAB AdjustLayoutColour::rgbToLab(juce::Colour rgb)
{
    float lr, lg, lb;
    for (int ch = 0; ch < 3; ch++)
    {
        float v = 0.0f;
        switch (ch)
        {
        case 0:
            v = rgb.getFloatRed();
            break;
        case 1:
            v = rgb.getFloatGreen();
            break;
        case 2:
            v = rgb.getFloatBlue();
            break;
        }

        if (v <= 0.04045f)
            v = v / 12.92f;
        else
            v = powf((v + 0.055f) / 1.055f, 2.4f);

        switch (ch)
        {
        case 0:
            lr = v;
            break;
        case 1:
            lg = v;
            break;
        case 2:
            lb = v;
            break;
        }        
    }

    // float x, y, z;

    // x = lr*0.4124564f + lr*0.3575761f + lr*0.1804375f;
    // y = lg*0.2126729f + lg*0.7151522f + lg*0.0721750f;
    // z = lb*0.0193339f + lb*0.1191920f + lb*0.9503041f;

    float x = lr * 0.4124564f + lg * 0.3575761f + lb * 0.1804375f;
    float y = lr * 0.2126729f + lg * 0.7151522f + lb * 0.0721750f;
    float z = lr * 0.0193339f + lg * 0.1191920f + lb * 0.9503041f;

    // juce::Colour xyz = juce::Colour((juce::uint8)juce::roundToInt(x * 255.0f),
    //                                 (juce::uint8)juce::roundToInt(y * 255.0f),
    //                                 (juce::uint8)juce::roundToInt(z * 255.0f));
    // return xyz.withAlpha(1.0f);

    const float epsilon = 0.008856f;
    const float kappa = 903.3f;

    float fx, fy, fz;

    for (int ch = 0; ch < 3; ch++)
    {
        float v;

        switch (ch)
        {
        case 0:
            v = x;
            break;
        case 1:
            v = y;
            break;
        case 2:
            v = z;
            break;
        }

        if (v > epsilon)
            v = powf(v, 1.0f/3.0f);
        else
            v = (kappa * v + 16.0f) / 116.0f;

        switch (ch)
        {
        case 0:
            fx = v;
            break;
        case 1:
            fy = v;
            break;
        case 2:
            fz = v;
            break;
        }
    }

    float L = 116.0f * fy - 16.0f;
    float A = 500.0f * (fx - fy);
    float B = 200.0f * (fy - fz);

    return { L, A, B };
}

juce::Colour AdjustLayoutColour::labToRgb(LAB lab)
{
    float fy = (lab.L + 16.0f) / 116.0f;
    float fx = lab.A / 500.0f + fy;
    float fz = fy - lab.B / 200.0f;
    
    const float epsilon = 0.008856f;
    const float kappa = 903.3f;

    float xr, yr, zr;

    float fxCbd = powf(fx, 3.0f);
    if (fxCbd > epsilon)
        xr = fxCbd;
    else
        xr = (116.0f * fx - 16.0f) / kappa;

    if (lab.L > (kappa * epsilon))
        yr = powf((lab.L + 16.0f) / 116.0f, 3.0f);
    else
        yr = lab.L / kappa;

    float fzCbd = powf(fz, 3.0f);
    if (fzCbd > epsilon)
        zr = fzCbd;
    else
        zr = (116.0f * fz - 16.0f) / kappa;

    float x = xr;
    float y = yr;
    float z = zr;

    // for (int ch = 0; ch < 3; ch++)
    // {
    //     float v;
    //     switch (ch)
    //     {
    //     case 0:
    //         v = fx;
    //         break;
    //     case 1:
    //         v = fy;
    //         break;
    //     case 2:
    //         v = fz;
    //         break;
    //     }

    //     if (v > (6.0f / 29.0f))
    //         v = powf(v, 3);
    //     else
    //         v = (v - 16.0f / 116.0f) * 3.0f * (6.0f / 29.0f) * (6.0f / 29.0f);

    //     switch (ch)
    //     {
    //     case 0:
    //         x = v;
    //         break;
    //     case 1:
    //         y = v;
    //         break;
    //     case 2:
    //         z = v;
    //         break;
    //     }
    // }

    // float Xr = 0.4124564f, Yr = 0.2126729f, Zr = 0.0193339f;
    // float YrF = Yr * (100.0f / 100.0f);
    
    float r = x *  3.2404542f + y * -1.5371385f + z * -0.4985314f;
    float g = x * -0.9692660f + y *  1.8760108f + z *  0.0415560f;
    float b = x *  0.0556434f + y * -0.2040259f + z *  1.0572252f;

    // float lr = x*3.24045f + x*-1.53714f + x*-0.498532f;
    // float lg = y*-0.969266f + y*1.87601f + y*0.0415561f;
    // float lb = z*0.0556434f + z*-0.204026f + z*1.05723f;

    // float lr = x *  3.2404542f + y * -1.5371385f + z * -0.4985314f;
    // float lg = x * -0.9692660f + y *  1.8760108f + z *  0.0415560f;
    // float lb = x *  0.0556434f + y * -0.2040259f + z *  1.0572252f;

    for (int ch = 0; ch < 3; ch++)
    {
        float v;
        switch (ch)
        {
        case 0:
            v = r;
            break;
        case 1:
            v = g;
            break;
        case 2:
            v = b;
            break;
        }

        if (v <= 0.0031308f)
            v *= 12.92f;
        else
            v = 1.055f * powf(v, 1.0f / 2.4f) - 0.055f;

        switch (ch)
        {
        case 0:
            r = v;
            break;
        case 1:
            g = v;
            break;
        case 2:
            b = v;
            break;
        }
    }

    juce::Colour rgb((juce::uint8)juce::roundToInt(r * 255.0f),
                        (juce::uint8)juce::roundToInt(g * 255.0f),
                        (juce::uint8)juce::roundToInt(b * 255.0f));
    
    return rgb.withAlpha(1.0f);
}
