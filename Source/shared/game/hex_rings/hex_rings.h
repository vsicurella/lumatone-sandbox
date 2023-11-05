/*
  ==============================================================================

    hex_rings.h
    Created: 6 Aug 2023 11:50:57am
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once
#include "../game_base.h"
#include "../../lumatone_editor_library/hex/lumatone_hex_map.h"
class HexRings : public LumatoneSandboxGameBase
{
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

    HexRings(LumatoneController* controllerIn);

    void reset(bool clearQueue) override;

    void nextTick() override;

protected:
    LumatoneAction* renderFrame() const override;

private:

    void advanceFrameQueue();

public:
    juce::Colour getRandomColourVelocity(juce::uint8 velocity);
    int getRingSizeVelocity(juce::uint8 velocity) const;

public:
    void handleAnyNoteOn(int midiChannel, int midiNote, juce::uint8 velocity) override;

private:

    juce::Random random;

    juce::Array<HexRings::Frame> frameQueue;
    juce::Array<HexRings::Frame> currentFrame;
    int maxQueueFramesPerTick = 5;
    int maxUpdatesPerFrame = 10;

    std::unique_ptr<LumatoneHexMap> hexMap;

};