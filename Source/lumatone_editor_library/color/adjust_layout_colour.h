#pragma once

#include "../LumatoneController.h"
#include "../palettes/ColourPaletteWindow.h"
#include "../palettes/palette_library.h"

#include "../actions/edit_actions.h"

#include "../hex/lumatone_hex_map.h"

class AdjustLayoutColour : public LumatoneEditor::EditorListener,
                          public ColourSelectionListener
{

public:
    enum class Type
    {
        NONE,
        FINDREPLACE,
        ROTATEHUE,
        ADJUSTBRIGHTNESS,
        SETGRADIENT
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
    
    void setGradient(SetGradientOptions options);
    
    void commitChanges();
    void resetChanges();

private:

    void beginAction(AdjustLayoutColour::Type type);
    void endAction();

private:
    void completeMappingLoaded(LumatoneLayout mappingData) override;
    void boardChanged(LumatoneBoard boardData) override;
    void keyChanged(int boardIndex, int keyIndex, LumatoneKey lumatoneKey) override;

private:
    void colourChangedCallback(ColourSelectionBroadcaster* source, juce::Colour newColour) override {}

private:

    void sendColourUpdate(juce::Colour oldColour, juce::Colour newColour);
    void sendSelectionUpdate(const juce::Array<MappedLumatoneKey>& keyUpdates);
    void sendMappingUpdate(const LumatoneLayout& updatedLayout);

private:

    LumatoneController* controller;
    // LumatonePaletteLibrary* paletteLibrary;

    LumatoneHexMap hexMap;

    LumatoneLayout layoutBeforeAdjust;
    LumatoneLayout currentLayout;

    AdjustLayoutColour::Type currentAction;
};
