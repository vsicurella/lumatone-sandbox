/*
  ==============================================================================

    LumatoneDataStructures.cpp
    Created: 1 Jul 2014 9:26:39pm
    Author:  hsstraub

  ==============================================================================
*/

#include "lumatone_layout.h"

/*
==============================================================================
LumatoneConfigTable class
==============================================================================
*/

int DefaultVelocityIntervalTable[127] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
    21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 60, 61,
    62, 63, 64, 66, 67, 68, 70, 71, 72, 73, 74, 76, 77, 79, 81, 82, 84, 86, 88, 90,
    92, 94, 96, 98, 101, 104, 107, 111, 115, 119, 124, 129, 134, 140, 146, 152, 159, 170, 171, 175,
    180, 185, 190, 195, 200, 205, 210, 215, 220, 225, 230, 235, 240, 245, 250, 255, 260, 265, 270, 275,
    280, 285, 290, 295, 300, 305, 310};

// Empty velocity curve config: one to one
int EmptyVelocityCurveTable[128] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
    20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
    40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
    60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
    80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99,
    100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119,
    120, 121, 122, 123, 124, 125, 126, 127};

int DefaultOnOffVelocityTable[128] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
    11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
    31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
    51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 63, 64, 65, 66, 68, 69, 70, 72, 73,
    74, 76, 77, 79, 80, 82, 84, 85, 87, 88, 90, 92, 94, 96, 97, 99, 101, 103, 105, 108,
    110, 112, 114, 117, 119, 121, 124, 127};

int DefaultFaderVelocityTable[128] = {
    1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 6, 6, 6, 7, 7, 7, 8, 8,
    9, 9, 9, 10, 10, 10, 11, 11, 12, 12, 12, 13, 13, 14, 14, 14, 15, 15, 16, 16,
    17, 17, 17, 18, 18, 19, 19, 20, 20, 20, 21, 21, 22, 22, 23, 23, 24, 24, 25, 25,
    26, 26, 27, 27, 28, 28, 29, 29, 30, 31, 31, 32, 32, 33, 33, 34, 35, 35, 36, 37,
    37, 38, 39, 39, 40, 41, 41, 42, 43, 44, 45, 45, 46, 47, 48, 49, 50, 51, 52, 53,
    55, 56, 57, 59, 62, 65, 68, 71, 74, 77, 79, 82, 85, 88, 91, 94, 97, 99, 102, 105,
    108, 111, 114, 117, 119, 122, 125, 127};

int DefaultAfterTouchVelocityTable[128] = {
    0, 2, 3, 5, 6, 8, 9, 10, 12, 13, 14, 16, 17, 18, 20, 21, 22, 24, 25, 26,
    27, 28, 30, 31, 32, 33, 34, 36, 37, 38, 39, 40, 41, 43, 44, 45, 46, 47, 48, 49,
    50, 51, 52, 53, 54, 55, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70,
    70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 85, 86, 87, 88,
    89, 90, 91, 92, 92, 93, 94, 95, 96, 97, 98, 99, 99, 100, 101, 102, 103, 104, 104, 105,
    106, 107, 108, 108, 109, 110, 111, 112, 112, 113, 114, 115, 116, 116, 117, 118, 119, 120, 120, 121,
    122, 123, 123, 124, 125, 126, 126, 127};

int DefaulLumatouchVelocityTable[128] = {
    0, 1, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 7, 7, 7, 8, 8,
    8, 9, 9, 10, 10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 15, 16,
    16, 17, 17, 18, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 22, 23, 23, 24, 24, 25,
    25, 26, 26, 27, 27, 28, 28, 29, 29, 30, 30, 31, 32, 32, 33, 33, 34, 34, 35, 36,
    36, 37, 37, 38, 39, 39, 40, 41, 41, 42, 43, 43, 44, 45, 46, 47, 47, 48, 49, 50,
    51, 52, 53, 53, 54, 56, 57, 58, 60, 61, 63, 65, 68, 70, 73, 75, 78, 81, 84, 87,
    90, 94, 98, 102, 107, 113, 121, 127};

LumatoneConfigTable::LumatoneConfigTable()
{
    memmove(velocityValues, EmptyVelocityCurveTable, sizeof(EmptyVelocityCurveTable));
    editStrategy = DrawMode::none;
}

LumatoneConfigTable::LumatoneConfigTable(LumatoneConfigTable::TableType velocityCurveType)
{
    switch (velocityCurveType) {
        case LumatoneConfigTable::TableType::velocityInterval:
            jassert(sizeof(DefaultOnOffVelocityTable) == sizeof(velocityValues));
            memmove(velocityValues, DefaultOnOffVelocityTable, sizeof(DefaultOnOffVelocityTable));
            editStrategy = DrawMode::freeDrawing;
            break;

        case LumatoneConfigTable::TableType::fader:
            jassert(sizeof(DefaultFaderVelocityTable) == sizeof(velocityValues));
            memmove(velocityValues, DefaultFaderVelocityTable, sizeof(DefaultFaderVelocityTable));
            editStrategy = DrawMode::freeDrawing;
            break;

        case LumatoneConfigTable::TableType::afterTouch:
            jassert(sizeof(DefaultAfterTouchVelocityTable) == sizeof(velocityValues));
            memmove(velocityValues, DefaultAfterTouchVelocityTable, sizeof(DefaultAfterTouchVelocityTable));
            editStrategy = DrawMode::freeDrawing;
            break;

        case LumatoneConfigTable::TableType::lumaTouch:
            jassert(sizeof(DefaulLumatouchVelocityTable) == sizeof(velocityValues));
            memmove(velocityValues, DefaulLumatouchVelocityTable, sizeof(DefaulLumatouchVelocityTable));
            editStrategy = DrawMode::freeDrawing;
            break;

        default:
            jassertfalse;
            memmove(velocityValues, EmptyVelocityCurveTable, sizeof(EmptyVelocityCurveTable));
            editStrategy = DrawMode::none;
            break;
    }
}

LumatoneConfigTable::LumatoneConfigTable(const juce::String& velocityCurveConfigString)
{
    if (velocityCurveConfigString.startsWith("LINEAR")) {
        editStrategy = LumatoneConfigTable::DrawMode::linearSegments;

        juce::StringArray velocityCurveValueArray = juce::StringArray::fromTokens(velocityCurveConfigString.substring(6), false);
        if (velocityCurveValueArray.size() > 0) {
            jassert(velocityCurveValueArray.size() >= 128);

            for (int x = 0; x < 128; x++)
                velocityValues[x] = velocityCurveValueArray[x].getIntValue();
        } else {
            // Initialize segment table
            for (int x = 0; x < 128; x++)
                velocityValues[x] = -1;
        }
    } else if (velocityCurveConfigString.startsWith("Quadratic")) {
        editStrategy = LumatoneConfigTable::DrawMode::quadraticCurves;

        juce::StringArray velocityCurveValueArray = juce::StringArray::fromTokens(velocityCurveConfigString.substring(6), false);
        if (velocityCurveValueArray.size() > 0) {
            jassert(velocityCurveValueArray.size() >= 128);

            for (int x = 0; x < 128; x++)
                velocityValues[x] = velocityCurveValueArray[x].getIntValue();
        } else {
            // Initialize segment table
            for (int x = 0; x < 128; x++)
                velocityValues[x] = -1;
        }
    } else {
        editStrategy = LumatoneConfigTable::DrawMode::freeDrawing;

        juce::StringArray velocityCurveValueArray = juce::StringArray::fromTokens(velocityCurveConfigString, false);
        if (velocityCurveValueArray.size() > 0) {
            jassert(velocityCurveValueArray.size() >= 128);

            for (int x = 0; x < 128; x++)
                velocityValues[x] = velocityCurveValueArray[x].getIntValue();
        } else {
            editStrategy = DrawMode::none;
            memmove(velocityValues, EmptyVelocityCurveTable, sizeof(EmptyVelocityCurveTable));
        }
    }
}

juce::String LumatoneConfigTable::toConfigString()
{
    switch (editStrategy) {
        case DrawMode::freeDrawing:
        {
            juce::String velocityCurveString;

            for (int x = 0; x < 128; x++)
                velocityCurveString += juce::String(velocityValues[x]) + " ";

            return velocityCurveString;
        }

        case DrawMode::linearSegments:
        {
            juce::String velocityCurveString = "LINEAR";

            for (int x = 0; x < 128; x++)
                velocityCurveString += juce::String(velocityValues[x]) + " ";

            return velocityCurveString;

        }

        case DrawMode::quadraticCurves:
        {
            juce::String velocityCurveString = "Quadratic";

            for (int x = 0; x < 128; x++)
                velocityCurveString += juce::String(velocityValues[x]) + " ";

            return velocityCurveString;
        }

        default:
            return "";
    }
}

/*
==============================================================================
TerpstraKeyMapping class
==============================================================================
*/

LumatoneLayout::LumatoneLayout(int numBoardsIn, int octaveBoardSizeIn, bool initWithNotes)
    : numBoards(numBoardsIn)
    , octaveBoardSize(octaveBoardSizeIn)
{
    clearAll(initWithNotes);
}

void LumatoneLayout::clearVelocityIntervalTable()
{
    jassert(sizeof(DefaultVelocityIntervalTable) == sizeof(table));
    memmove(table, DefaultVelocityIntervalTable, sizeof(DefaultVelocityIntervalTable));
}

const LumatoneBoard* LumatoneLayout::readBoard(int index) const
{
    return &boards[index];
}

LumatoneBoard* LumatoneLayout::getBoard(int index)
{
    return &boards[index];
}

const LumatoneKey* LumatoneLayout::readKey(int boardIndex, int keyIndex) const
{
    return &boards[boardIndex].theKeys[keyIndex];
}

void LumatoneLayout::clearAll(bool initializeWithNoteKeyType)
{
    auto newKeyType = (initializeWithNoteKeyType) ? LumatoneKeyType::noteOnNoteOff : LumatoneKeyType::disabled;

    for (int i = 0; i < numBoards; i++)
    {
        boards[i] = LumatoneBoard(newKeyType, octaveBoardSize);
    }

    // Default values for options
    afterTouchActive = false;
    lightOnKeyStrokes = false;
    invertExpression = false;
    invertSustain = false;
    expressionControllerSensivity = 0;

    clearVelocityIntervalTable();
    velocityTable = LumatoneConfigTable(LumatoneConfigTable::TableType::velocityInterval);
    faderTable = LumatoneConfigTable(LumatoneConfigTable::TableType::fader);
    afterTouchTable = LumatoneConfigTable(LumatoneConfigTable::TableType::afterTouch);
    lumaTouchTable = LumatoneConfigTable(LumatoneConfigTable::TableType::lumaTouch);
}

bool LumatoneLayout::isEmpty() const
{
    bool isEmpty = true;
    for (int i = 0; i < octaveBoardSize && isEmpty; i++) {
        isEmpty &= boards[i].isEmpty();
    }

    return isEmpty;
}

void LumatoneLayout::fromStringArray(const juce::StringArray& stringArray)
{
    clearAll(true);

    bool hasFiftySixKeys = false;

    int boardIndex = -1;
    for (int i = 0; i < stringArray.size(); i++)
    {
        juce::String currentLine = stringArray[i];
        int pos1, pos2;
        if ((pos1 = currentLine.indexOf("[Board")) >= 0) 
        {
            pos2 = currentLine.indexOf("]");
            if (pos2 >= 0 && pos2 > pos1) 
            {
                boardIndex = currentLine.substring(pos1 + 6, pos2).getIntValue();
            } 
            else
                jassert(false);
        } 
        else if ((pos1 = currentLine.indexOf("Key_")) >= 0) 
        {
            pos2 = currentLine.indexOf("=");
            if (pos2 >= 0 && pos2 > pos1) 
            {
                int keyIndex = currentLine.substring(pos1 + 4, pos2).getIntValue();
                int keyValue = currentLine.substring(pos2 + 1).getIntValue();
                if (boardIndex >= 0 && boardIndex < octaveBoardSize) 
                {
                    if (keyIndex >= 0 && keyIndex < 56)
                        boards[boardIndex].theKeys[keyIndex].noteNumber = keyValue;
                    else
                        jassert(false);
                }
                else
                    jassert(false);
            }
        } 
        else if ((pos1 = currentLine.indexOf("Chan_")) >= 0) 
        {
            pos2 = currentLine.indexOf("=");
            if (pos2 >= 0 && pos2 > pos1)
            {
                int keyIndex = currentLine.substring(pos1 + 5, pos2).getIntValue();
                int keyValue = currentLine.substring(pos2 + 1).getIntValue();
                if (boardIndex >= 0 && boardIndex < octaveBoardSize)
                {
                    if (keyIndex >= 0 && keyIndex < 56)
                    {

                        if (keyValue > 0 && keyValue <= 16)
                        {
                            boards[boardIndex].theKeys[keyIndex].channelNumber = keyValue;
                        } else
                        {
                            boards[boardIndex].theKeys[keyIndex].channelNumber = 1;
                            boards[boardIndex].theKeys[keyIndex].keyType = LumatoneKeyType::disabledDefault;
                        }

                        if (keyIndex == 55)
                            hasFiftySixKeys = true;
                    }
                    else
                        jassert(false);
                }
                else
                    jassert(false);
            }
        }
        else if ((pos1 = currentLine.indexOf("Col_")) >= 0)
        {
            pos2 = currentLine.indexOf("=");
            if (pos2 >= 0 && pos2 > pos1)
            {
                int keyIndex = currentLine.substring(pos1 + 4, pos2).getIntValue();
                int colValue = currentLine.substring(pos2 + 1).getHexValue32();
                if (boardIndex >= 0 && boardIndex < octaveBoardSize)
                {
                    if (keyIndex >= 0 && keyIndex < 56)
                        boards[boardIndex].theKeys[keyIndex].colour = juce::Colour(colValue).withAlpha(1.0f);
                    else
                        jassert(false);
                }
                else
                    jassert(false);
            }
        }
        else if ((pos1 = currentLine.indexOf("KTyp_")) >= 0)
        {
            pos2 = currentLine.indexOf("=");
            if (pos2 >= 0 && pos2 > pos1)
            {
                int keyIndex = currentLine.substring(pos1 + 5, pos2).getIntValue();
                int keyValue = currentLine.substring(pos2 + 1).getIntValue();
                if (boardIndex >= 0 && boardIndex < octaveBoardSize)
                {
                    if (keyIndex >= 0 && keyIndex < 56)
                    {
                        auto currentKeyType = boards[boardIndex].theKeys[keyIndex].keyType;
                        if (keyValue < 0 && keyValue >= 5 || currentKeyType == LumatoneKeyType::disabledDefault)
                            boards[boardIndex].theKeys[keyIndex].keyType = LumatoneKeyType::disabled;
                        else
                            boards[boardIndex].theKeys[keyIndex].keyType = (LumatoneKeyType)keyValue;
                    }
                    else
                        jassert(false);
                }
            }
            else
                jassert(false);
        }
        else if ((pos1 = currentLine.indexOf("CCInvert_")) >= 0)
        {
            int keyIndex = currentLine.substring(pos1 + 9, currentLine.length()).getIntValue();
            if (boardIndex >= 0 && boardIndex < octaveBoardSize)
            {
                if (keyIndex >= 0 && keyIndex < 56)
                        boards[boardIndex].theKeys[keyIndex].ccFaderDefault = false;
                else
                    jassert(false);
            }
        }

        // General options
        else if ((pos1 = currentLine.indexOf("AfterTouchActive=")) >= 0)
            afterTouchActive = currentLine.substring(pos1 + 17).getIntValue() > 0;

        else if ((pos1 = currentLine.indexOf("LightOnKeyStrokes=")) >= 0)
            lightOnKeyStrokes = currentLine.substring(pos1 + 18).getIntValue() > 0;

        else if ((pos1 = currentLine.indexOf("InvertFootController=")) >= 0)
            invertExpression = currentLine.substring(pos1 + 21).getIntValue() > 0;

        else if ((pos1 = currentLine.indexOf("InvertSustain=")) >= 0)
            invertSustain = currentLine.substring(pos1 + 14).getIntValue() > 0;

        else if ((pos1 = currentLine.indexOf("ExprCtrlSensivity=")) >= 0)
            expressionControllerSensivity = currentLine.substring(pos1 + 18).getIntValue();

        // Velocity curve config
        else if ((pos1 = currentLine.indexOf("VelocityIntrvlTbl=")) >= 0)
        {
            juce::String intervalTableString = currentLine.substring(pos1 + 18);
            juce::StringArray intervalTableValueArray = juce::StringArray::fromTokens(intervalTableString, false);
            if (intervalTableValueArray.size() > 0)
            {
                jassert(intervalTableValueArray.size() >= VELOCITYINTERVALTABLESIZE);

                for (int x = 0; x < VELOCITYINTERVALTABLESIZE; x++)
                {
                    table[x] = intervalTableValueArray[x].getIntValue();
                }
            }
            else
            {
                clearVelocityIntervalTable();
            }
        }
        // Note on/off velocity configuration
        else if ((pos1 = currentLine.indexOf("NoteOnOffVelocityCrvTbl=")) >= 0) 
        {
            velocityTable = LumatoneConfigTable(currentLine.substring(pos1 + 24));
        }
        // Fader configuration
        else if ((pos1 = currentLine.indexOf("FaderConfig=")) >= 0) 
        {
            faderTable = LumatoneConfigTable(currentLine.substring(pos1 + 12));
        }
        // Aftertouch configuration
        else if ((pos1 = currentLine.indexOf("afterTouchConfig=")) >= 0) 
        {
            afterTouchTable = LumatoneConfigTable(currentLine.substring(pos1 + 17));
        }
        // Lumatouch configuration
        else if ((pos1 = currentLine.indexOf("LumaTouchConfig=")) >= 0) 
        {
            lumaTouchTable = LumatoneConfigTable(currentLine.substring(pos1 + 17));
        }
    }

    // Conversion between 55-key and 56-key layout
    //if (TerpstraSysExApplication::getApp().getOctaveBoardSize() == 56 && !hasFiftySixKeys) {
    //    // Loaded layout has 55-key layout. Adjust geometry to 56-key layout
    //    for (boardIndex = 0; boardIndex < octaveBoardSize; boardIndex++) {
    //        boards[boardIndex].theKeys[55] = boards[boardIndex].theKeys[54];
    //        boards[boardIndex].theKeys[54] = TerpstraKey();
    //    }
    //} else if (TerpstraSysExApplication::getApp().getOctaveBoardSize() == 55 && hasFiftySixKeys) {
    //    // Loaded layout has 56-key layout. Adjust geometry to 55-key layout
    //    for (boardIndex = 0; boardIndex < octaveBoardSize; boardIndex++) {
    //        boards[boardIndex].theKeys[54] = boards[boardIndex].theKeys[55];
    //        boards[boardIndex].theKeys[55] = TerpstraKey();
    //    }
    //}
}

juce::StringArray LumatoneLayout::toStringArray()
{
    juce::StringArray result;

    for (int boardIndex = 0; boardIndex < octaveBoardSize; boardIndex++) {
        result.add("[Board" + juce::String(boardIndex) + "]");

        //for (int keyIndex = 0; keyIndex < TerpstraSysExApplication::getApp().getOctaveBoardSize(); keyIndex++) {
            for (int keyIndex = 0; keyIndex < 56; keyIndex++) {
            result.add("Key_" + juce::String(keyIndex) + "=" + juce::String(boards[boardIndex].theKeys[keyIndex].noteNumber));
            result.add("Chan_" + juce::String(keyIndex) + "=" + juce::String(boards[boardIndex].theKeys[keyIndex].channelNumber));
            //if (boards[boardIndex].theKeys[keyIndex].colour != juce::Colour())
            result.add("Col_" + juce::String(keyIndex) + "=" + boards[boardIndex].theKeys[keyIndex].colour.toDisplayString(false));
            if (boards[boardIndex].theKeys[keyIndex].keyType != LumatoneKeyType::noteOnNoteOff)
                result.add("KTyp_" + juce::String(keyIndex) + "=" + juce::String(boards[boardIndex].theKeys[keyIndex].keyType));
            if (boards[boardIndex].theKeys[keyIndex].ccFaderDefault != true)
                result.add("CCInvert_" + juce::String(keyIndex));
        }
    }

    // General options
    result.add("AfterTouchActive=" + juce::String(afterTouchActive ? 1 : 0));
    result.add("LightOnKeyStrokes=" + juce::String(lightOnKeyStrokes ? 1 : 0));
    result.add("InvertFootController=" + juce::String(invertExpression ? 1 : 0));
    result.add("InvertSustain=" + juce::String(invertSustain ? 1 : 0));
    result.add("ExprCtrlSensivity=" + juce::String(expressionControllerSensivity));

    // Velocity curve interval table
    juce::String intervalTableString;
    for (auto intervalTableValue : table)
        intervalTableString += juce::String(intervalTableValue) + " ";
    result.add("VelocityIntrvlTbl=" + intervalTableString);

    // Note on/off velocity configuration
    result.add("NoteOnOffVelocityCrvTbl=" + velocityTable.toConfigString());
    // Fader configuration
    result.add("FaderConfig=" + faderTable.toConfigString());
    // Aftertouch configuration
    result.add("afterTouchConfig=" + afterTouchTable.toConfigString());
    // Lumatouch configuration
    result.add("LumaTouchConfig=" + lumaTouchTable.toConfigString());

    return result;
}

LumatoneConfigTable* LumatoneLayout::getConfigTable(LumatoneConfigTable::TableType velocityCurveType)
{
	switch (velocityCurveType)
	{
	case LumatoneConfigTable::TableType::velocityInterval:
		return &velocityTable;

	case LumatoneConfigTable::TableType::fader:
		return &faderTable;
	case LumatoneConfigTable::TableType::afterTouch:
		return &afterTouchTable;
	case LumatoneConfigTable::TableType::lumaTouch:
		return &lumaTouchTable;
	default:
		jassertfalse;
		return nullptr;
	}
}


