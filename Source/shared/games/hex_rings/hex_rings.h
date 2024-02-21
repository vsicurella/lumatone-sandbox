/*
  ==============================================================================

    hex_rings.h
    Created: 6 Aug 2023 11:50:57am
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once
#include "../game_base.h"
#include "../game_base_state.h"
#include "../../lumatone_editor_library/mapping/lumatone_hex_map.h"

class HexRings : public LumatoneSandboxGameBase
                , protected LumatoneGameBaseState
{
public:
    struct ID
    {
        inline static const juce::Identifier GameId = juce::Identifier("HexRingsGame");
    };

public:
    struct Frame
    {
        LumatoneKeyCoord origin;
        
        bool isNoteOn = false;
        bool isAfterTouch = false;
        bool isController = false;

        juce::uint8 value;

        juce::Colour colour;
    };

public:

    HexRings(LumatoneGameEngineState& gameEngineState);

    bool reset(bool clearQueue) override;

    bool nextTick() override;

protected:
    LumatoneEditor::LayoutAction renderFrame() const override;

private:
    void advanceFrameQueue();

    juce::ValueTree loadStateProperties(juce::ValueTree stateIn) override { return stateIn; }
    void handleStatePropertyChange(juce::ValueTree stateIn, const juce::Identifier& property) override { }

public:
    juce::Colour getRandomColourVelocity(juce::uint8 velocity);
    int getRingSizeVelocity(juce::uint8 velocity) const;

public:
    void handleNoteOn(int midiChannel, int midiNote, juce::uint8 velocity) override;

public:
    virtual LumatoneSandboxGameComponent* createController() override;

private:
    juce::Random random;

    juce::Array<HexRings::Frame> frameQueue;
    juce::Array<HexRings::Frame> currentFrame;
    int maxQueueFramesPerTick = 5;

    std::unique_ptr<LumatoneHexMap> hexMap;

};