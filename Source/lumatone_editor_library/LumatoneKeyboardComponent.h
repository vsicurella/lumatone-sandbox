/*
  ==============================================================================

    LumatoneKeyboardComponent.h
    Created: 24 Jul 2023 9:52:28pm
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "lumatone_assets.h"
#include "./data/lumatone_state.h"
#include "./data/lumatone_midi_state.h"
#include "LumatoneKeyComponent.h"
#include "ApplicationListeners.h"
#include "lumatone_output_map.h"


//==============================================================================
/*
*/
class LumatoneKeyboardComponent : public juce::Component,
                                  public juce::KeyListener,
                                  public LumatoneEditor::EditorListener,
                                  public LumatoneMidiState,
                                  public LumatoneMidiState::Listener
{
public:
    LumatoneKeyboardComponent(LumatoneState stateIn);
    ~LumatoneKeyboardComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void resetOctaveSize();

public:
    // LumatoneEditor::EditorListener Implementation
    void completeMappingLoaded(LumatoneLayout mappingData) override;
    void boardChanged(LumatoneBoard boardData) override;
    void keyChanged(int boardIndex, int keyIndex, LumatoneKey lumatoneKey) override;

private:

    void updateKeyColour(int boardIndex, int keyIndex, const juce::Colour& colour);
    void resetLayoutState();

public:
    // Playing mode methods
    
    void clearHeldNotes();

    void sustainStarted();
    void sustainEnded();

    void lumatoneKeyDown(int boardIndex, int keyIndex);
    void lumatoneKeyUp(int boardIndex, int keyIndex);

private:
    // Playing mode implementations
    void showKeyDown(int boardIndex, int keyIndex, bool keyIsDown);

protected:
    // juce::Component UI implementations
    void mouseMove(const juce::MouseEvent& e) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;

private:
    bool keyStateChanged(bool isKeyDown) override;
    bool keyPressed(const juce::KeyPress& key, Component* originatingComponent) override;
    void modifierKeysChanged(const juce::ModifierKeys& modifiers) override;

private:
    // LumatoneMidiState implementations (note callbacks)
    void noteOnInternal(const juce::MidiMessage& msg, int midiChannel, int midiNote, juce::uint8 velocity) override;
    void noteOffInternal(const juce::MidiMessage& msg, int midiChannel, int midiNote, juce::uint8 velocity) override;

private:
    // LumatoneMidiState::Listener implementations

    void handleNoteOn(LumatoneMidiState* midiState, int midiChannel, int midiNote, juce::uint8 velocity) override;
    void handleNoteOff(LumatoneMidiState* midiState, int midiChannel, int midiNote) override;
    void handleAftertouch(LumatoneMidiState* midiState, int midiChannel, int midiNote, juce::uint8 aftertouch) override;
    void handleController(LumatoneMidiState* midiState, int midiChannel, int midiNote, juce::uint8 value) override;


private:

    void prepareHexTiling();

    LumatoneKeyDisplay* getKeyFromMouseEvent(const juce::MouseEvent& e);

    juce::Image getResizedImage(LumatoneAssets::ID assetId, int targetWidth, int targetHeight, bool useJuceResize=false);

private:

    struct OctaveBoard
    {
        juce::OwnedArray<LumatoneKeyDisplay> keyMiniDisplay;
        int leftPos;
        int rightPos;
    };

    juce::OwnedArray<OctaveBoard> octaveBoards;

    int currentOctaveSize = 0;
    int currentSetSelection;

    // Geometry settings
    LumatoneGeometry    lumatoneGeometry;
    LumatoneTiling      tilingGeometry;
    LumatoneOutputMap   lumatoneMidiMap;

    // std::unique_ptr<juce::Label> lblFirmwareVersion;

    //==============================================================================
    // UI Data

    juce::MidiKeyboardState* realtimeKeyboardState = nullptr;

    LumatoneKeyDisplay* lastMouseKeyDown = nullptr;
    LumatoneKeyDisplay* lastMouseKeyOver = nullptr;

    juce::Array<LumatoneKeyCoord> keysOverPerMouse;
    juce::Array<LumatoneKeyCoord> keysDownPerMouse; // future compatibility with touchpad

    juce::Array<LumatoneKeyDisplay*> keysOn;

    bool shiftHeld = false;
    bool altHeld = false;
    bool ctrlHeld = false;

    //bool rightHeld = false;
    //bool upHeld = false;
    //bool downHeld = false;
    //bool leftHeld = false;

    //==============================================================================
    // Style helpers

    std::unique_ptr<ImageProcessor> imageProcessor;

    int currentWidth = 0;
    int currentHeight = 0;

    juce::Rectangle<int> lumatoneBounds;
    int octaveLineY = 0;

    int keyWidth = 0;
    int keyHeight = 0;

    juce::Array<juce::Point<float>> keyCentres;

    juce::Image lumatoneGraphic;
    juce::Image keyShapeGraphic;
    juce::Image keyShadowGraphic;

    //==============================================================================
    // Position and sizing constants in reference to parent bounds

    const float imageAspect = 2.498233f;
    const float imageY      = 1.0f / 7.0f;
    const float imageHeight = 5.0f / 7.0f;

    //const float saveLoadH           = 0.0537634f;

    const float octaveLineYRatio    = 0.0236559f;

    // In reference to lumatoneBounds
    const float keybedX = 0.06908748f;

    const float keyW = 0.027352f;
    const float keyH = 0.07307f;

    const float oct1Key1X = 0.0839425f;
    const float oct1Key1Y = 0.335887f;

    const float oct1Key56X = 0.27304881f;
    const float oct1Key56Y = 0.8314673f;

    const float oct5Key7X = 0.878802f;
    const float oct5Key7Y = 0.356511491f;

    //===============================================================================

    juce::Point<float>  oct1Key1;
    juce::Point<float> oct1Key56;
    juce::Point<float>  oct5Key7;

    //[/UserVariables]

    //==============================================================================
    std::unique_ptr<juce::TextButton> btnLoadFile;
    std::unique_ptr<juce::TextButton> btnSaveFile;
    std::unique_ptr<juce::TextButton> buttonReceive;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LumatoneKeyboardComponent)
};
