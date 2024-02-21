/*
  ==============================================================================

    hex_rings.cpp
    Created: 6 Aug 2023 11:50:57am
    Author:  Vincenzo

  ==============================================================================
*/

#include "hex_rings.h"
#include "hex_rings_launcher.h"

#include "../games_index.h"

#include "../../lumatone_editor_library/device/lumatone_controller.h"
#include "../../lumatone_editor_library/actions/edit_actions.h"

HexRings::HexRings(LumatoneGameEngineState& gameEngineState)
    : LumatoneSandboxGameBase("HexRings", gameEngineState)
    , LumatoneGameBaseState(LumatoneSandbox::GameName::HexRings, HexRings::ID::GameId, gameEngineState)
{
    hexMap.reset(new LumatoneHexMap(gameEngineState.shareMappingData()));
    reset(true);
}

bool HexRings::reset(bool clearQueue)
{
    LumatoneSandboxGameBase::reset(clearQueue);
    auto layout = getIdentityLayout(true);
    queueLayout(layout);

    return false;
}

bool HexRings::nextTick()
{
    int limit = juce::jmin(frameQueue.size(), maxQueueFramesPerTick);
    for (int i = 0; i < limit; i++)
    {
        advanceFrameQueue();
        auto frame = renderFrame();
        addToQueue(frame);
    }

    return true;
}

LumatoneEditor::LayoutAction HexRings::renderFrame() const
{
    LumatoneEditor::LayoutAction action;
    juce::Array<MappedLumatoneKey> keyUpdates;

    int limit = juce::jmin(currentFrame.size(), maxUpdatesPerFrame);
    for (int i = 0; i < limit; i++)
    {
        const HexRings::Frame& frame = currentFrame.getReference(i);
        if (frame.isNoteOn)
        {
            if (frame.value > 0)
            {
                auto key = static_cast<MappedLumatoneKey>(getKeyAt(frame.origin.boardIndex, frame.origin.keyIndex));
                key.boardIndex = frame.origin.boardIndex;
                key.keyIndex = frame.origin.keyIndex;
                key.setColour(frame.colour);

                keyUpdates.add(key);
            }
        }
    }

    if (keyUpdates.size() > 0)
    {
        action.setData(keyUpdates);
    }

    return action;
}

void HexRings::advanceFrameQueue()
{
    currentFrame.clear();

    int limit = juce::jmin(frameQueue.size(), maxUpdatesPerFrame);
    for (int i = 0; i < limit; i++)
    {
        currentFrame.add(frameQueue.removeAndReturn(0));
    }
}

juce::Colour HexRings::getRandomColourVelocity(juce::uint8 velocity)
{
    auto c = juce::Colour(
        (juce::uint8)random.nextInt(255),
        (juce::uint8)random.nextInt(255),
        (juce::uint8)random.nextInt(255));

    return c.withAlpha((float)velocity / 127.0f);
}

int HexRings::getRingSizeVelocity(juce::uint8 velocity) const
{
    return juce::roundToInt((float)velocity / 127.0f * 4);
}

void HexRings::handleNoteOn(int midiChannel, int midiNote, juce::uint8 velocity)
{
    auto colour = getRandomColourVelocity(velocity);
    auto ringSize = getRingSizeVelocity(velocity);

    auto hexOrigin = hexMap->keyCoordsToHex(midiChannel - 1, midiNote);

    auto neighbors = hexOrigin.neighbors(ringSize);
    for (auto point : neighbors)
    {
        auto coord = hexMap->hexToKeyCoords(point);
        if (!getMappingData()->isKeyCoordValid(coord))
            continue;

        frameQueue.add({ coord, true, false, false, velocity, colour });
    }
}

LumatoneSandboxGameComponent* HexRings::createController()
{
    return new HexRingLauncher(this);
}

