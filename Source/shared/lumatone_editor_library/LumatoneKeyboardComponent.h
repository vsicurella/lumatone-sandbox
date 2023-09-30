/*
  ==============================================================================

    LumatoneKeyboardComponent.h
    Created: 24 Jul 2023 9:52:28pm
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "./data/application_state.h"
#include "./data/lumatone_midi_state.h"
#include "./lumatone_output_map.h"

#include "LumatoneKeyComponent.h"

#include "./listeners/editor_listener.h"
#include "./listeners/midi_listener.h"

#include "lumatone_assets.h"
#include "lumatone_render.h"

class LumatoneController;

//==============================================================================
/*
*/
class LumatoneKeyboardComponent : public juce::Component,
                                  public juce::KeyListener,
                                  public LumatoneApplicationState,
                                  public LumatoneMidiState,
                                  public LumatoneEditor::EditorListener,
                                  public LumatoneEditor::MidiListener
{
public:
    LumatoneKeyboardComponent(LumatoneController* controllerIn);
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
    void contextChanged(LumatoneContext* newOrEmptyContext) override; 
    void keyChanged(int boardIndex, int keyIndex, LumatoneKey lumatoneKey) override;
    void keyConfigChanged(int boardIndex, int keyIndex, LumatoneKey keyData) override;
    void keyColourChanged(int octaveNumber, int keyNumber, juce::Colour keyColour) override;
    void selectionChanged(juce::Array<MappedLumatoneKey> selection) override;
private:

    void updateKeyColour(int boardIndex, int keyIndex, const juce::Colour& colour);
    void resetLayoutState();

    void keyUpdateCallback(int boardIndex, int keyIndex, const LumatoneKey& keyData, bool doRepaint=true);
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
    void updateKeyState(int boardIndex, int keyIndex, bool keyIsDown);

protected:
    // juce::Component UI implementations
    void mouseMove(const juce::MouseEvent& e) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;

private:
    bool keyStateChanged(bool isKeyDown) override;
    bool keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent) override;
    void modifierKeysChanged(const juce::ModifierKeys& modifiers) override;

private:
    // Private implementations of other methods

    void keyDownInternal(int boardIndex, int keyIndex);
    void keyUpInternal(int boardIndex, int keyIndex);

    void noteOnInternal(int midiChannel, int midiNote, juce::uint8 velocity);
    void noteOffInternal(int midiChannel, int midiNote);

private:
    // LumatoneMidiState implementations (note callbacks)
    void noteOnInternal(const juce::MidiMessage& msg, int midiChannel, int midiNote, juce::uint8 velocity) override;
    void noteOffInternal(const juce::MidiMessage& msg, int midiChannel, int midiNote, juce::uint8 velocity) override;

private:
    // LumatoneMidiState::Listener implementations

    // void handleNoteOn(LumatoneMidiState* midiState, int midiChannel, int midiNote, juce::uint8 velocity) override;
    // void handleNoteOff(LumatoneMidiState* midiState, int midiChannel, int midiNote) override;
    // void handleAftertouch(LumatoneMidiState* midiState, int midiChannel, int midiNote, juce::uint8 aftertouch) override;
    // void handleController(LumatoneMidiState* midiState, int midiChannel, int midiNote, juce::uint8 value) override;

    void handleDeviceNoteOn(int midiChannel, int midiNote, juce::uint8 velocity) override;
    void handleDeviceNoteOff(int midiChannel, int midiNote) override;
    void handleDeviceAftertouch(int midiChannel, int midiNote, juce::uint8 aftertouch) override { }
    void handleDeviceController(int midiChannel, int midiNote, juce::uint8 value) override { }


private:
    LumatoneKeyDisplay* getKeyFromMouseEvent(const juce::MouseEvent& e);

    void rerender();

private:

    LumatoneController* controller;

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
