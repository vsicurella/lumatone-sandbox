/*
  ==============================================================================

    LumatoneKeyboardComponent.h
    Created: 24 Jul 2023 9:52:28pm
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "./data/lumatone_state.h"
#include "./data/lumatone_midi_state.h"

#include "LumatoneKeyComponent.h"
#include "ApplicationListeners.h"

#include "lumatone_assets.h"
#include "lumatone_output_map.h"
#include "lumatone_render.h"


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

    void resetOctaveSize(bool resetState = true);

    LumatoneComponentRenderMode getRenderMode() const { return renderMode; }
    void setRenderMode(LumatoneComponentRenderMode modeIn);

public:
    // LumatoneEditor::EditorListener Implementation
    void completeMappingLoaded(LumatoneLayout mappingData) override;
    void boardChanged(LumatoneBoard boardData) override;
    void keyChanged(int boardIndex, int keyIndex, LumatoneKey lumatoneKey) override;
    void keyConfigChanged(int boardIndex, int keyIndex, LumatoneKey keyData) override;
    void keyColourChanged(int octaveNumber, int keyNumber, juce::Colour keyColour) override;

private:

    void updateKeyColour(int boardIndex, int keyIndex, const juce::Colour& colour);
    void resetLayoutState();

    void keyUpdateCallback(int boardIndex, int keyIndex, const LumatoneKey& keyData);
    void mappingUpdateCallback();

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
    LumatoneKeyDisplay* getKeyFromMouseEvent(const juce::MouseEvent& e);


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

    LumatoneComponentRenderMode renderMode;

    LumatoneRender      lumatoneRender;

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

    juce::Image currentRender;

    //==============================================================================
    // Position and sizing constants in reference to parent bounds

    const float imageAspect = 2.498233f;
    const float imageY      = 1.0f / 7.0f;
    const float imageHeight = 5.0f / 7.0f;

    //const float saveLoadH           = 0.0537634f;

    const float octaveLineYRatio    = 0.0236559f;

    const float keyW = 0.027352f;
    const float keyH = 0.07307f;

    //[/UserVariables]

    //==============================================================================
    std::unique_ptr<juce::TextButton> btnLoadFile;
    std::unique_ptr<juce::TextButton> btnSaveFile;
    std::unique_ptr<juce::TextButton> buttonReceive;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LumatoneKeyboardComponent)
};
