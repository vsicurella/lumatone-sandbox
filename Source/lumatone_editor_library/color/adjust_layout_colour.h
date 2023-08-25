#pragma once

#include "../LumatoneController.h"
#include "../actions/edit_actions.h"
#include "../hex/lumatone_hex_map.h"

class AdjustLayoutColour 
{
public:
    enum class Type
    {
        NONE,
        FINDREPLACE,
        ROTATEHUE,
        ADJUSTBRIGHTNESS,
        ADJUSTCONTRAST,
        ADJUSTWHITE,
        SETGRADIENT,
    };

    struct SetGradientOptions
    {
        const juce::Array<LumatoneKeyCoord>& selection;
        juce::ColourGradient gradient;
        bool selectionOrigin = true;
        bool fillRelative = true;
        int numColumns = 1;
        int numRows = 1;

        SetGradientOptions(const juce::Array<LumatoneKeyCoord>& selectionIn, juce::ColourGradient gradientIn, bool selectionOriginIn = true, bool fillRelativeIn=true, int numColumnsIn = 1, int numRowsIn = 1)
            : selection(selectionIn)
            , gradient(gradientIn)
            , selectionOrigin(selectionOriginIn)
            , fillRelative(fillRelativeIn)
            , numColumns(numColumnsIn)
            , numRows(numRowsIn) {}
    };

public:

    AdjustLayoutColour(LumatoneController* controller);
    ~AdjustLayoutColour();

    void replaceColour(juce::Colour oldColour, juce::Colour newColour, bool sendUpdate=true);

    void rotateHue(float change, bool sendUpdate=true);
    void rotateHue(float change, const juce::Array<LumatoneKeyCoord>& selection, bool sendUpdate=true);

    void multiplyBrightness(float change, bool sendUpdate=true);
    void multiplyBrightness(float change, const juce::Array<LumatoneKeyCoord>& selection, bool sendUpdate=true);

    void adjustWhiteBalance(float newKelvin, float compensation=1, int compensationMode=0);

    void setGradient(SetGradientOptions options);
    
    void commitChanges();
    void resetChanges();

private:
    void beginAction(AdjustLayoutColour::Type type);
    void endAction();

private:
    void sendColourUpdate(juce::Colour oldColour, juce::Colour newColour, bool bufferUpdates=true);
    void sendSelectionUpdate(const juce::Array<MappedLumatoneKey>& keyUpdates, bool bufferUpdates=true);
    void sendMappingUpdate(const LumatoneLayout& updatedLayout, bool bufferUpdates=true);

public:

    // Converts from K to RGB, algorithm courtesy of
    // http://www.tannerhelland.com/4435/convert-temperature-rgb-algorithm-code/
    static juce::Colour kelvinToColour(int kelvin)
    {
        int temperature = kelvin;

        // range check
        if (temperature < 1000)
            temperature = 1000;
        else if (temperature > 40000)
            temperature = 40000;

        float tempFloat = temperature * 0.01;

        // red
        juce::uint8 red = 0;
        float tempRed = 0.0f;

        if (tempFloat <= 66)
            red = 255;
        else
        {
            tempRed = 329.698727446f * pow(tempFloat - 60, -0.1332047592f);
            if (tempRed < 0)
                red = 0;
            else if (tempRed > 255)
                red = 255;
            else
                red = tempRed;
        }

        //  green
        juce::uint8 green = 0;
        float tempGreen = 0.0f;

        if (tempFloat <= 66)
        {
            tempGreen = 99.4708025861 * logf(tempFloat) - 161.1195681661f;
            if (tempGreen < 0)
                green = 0;
            else if (tempGreen > 255)
                green = 255;
            else
                green = tempGreen;
        }
        else
        {
            tempGreen = 288.1221695283 * powf(tempFloat - 60, -0.0755148492f);
            if (tempGreen < 0)
                green = 0;
            else if (tempGreen > 255)
                green = 255;
            else
                green = tempGreen;
        }

        //  blue
        juce::uint8 blue = 0;
        float tempBlue = 0;

        if (tempFloat >= 66)
            blue = 255;
        else if (tempFloat <= 19)
            blue = 0;
        else
        {
            tempBlue = 138.5177312231f * logf(tempFloat - 10) - 305.0447927307f;
            if (tempBlue < 0)
                blue = 0;
            else if (tempBlue > 255)
                blue = 255;
            else
                blue = tempBlue;
        }

        return juce::Colour(red, green, blue);
    }

private:

    LumatoneController* controller;
    // LumatonePaletteLibrary* paletteLibrary;

    LumatoneHexMap hexMap;

    LumatoneLayout layoutBeforeAdjust;
    LumatoneLayout currentLayout;

    AdjustLayoutColour::Type currentAction;
};
