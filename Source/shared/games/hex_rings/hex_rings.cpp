/*
  ==============================================================================

    hex_rings.cpp
    Created: 6 Aug 2023 11:50:57am
    Author:  Vincenzo

  ==============================================================================
*/

#include "hex_rings.h"
#include "hex_rings_launcher.h"

#include "../../lumatone_editor_library/LumatoneController.h"
#include "../../lumatone_editor_library/actions/edit_actions.h"

HexRings::HexRings(juce::ValueTree gameEngineState, LumatoneController* controller)
    : LumatoneSandboxGameBase(gameEngineState, controller, "HexRings")
{
    hexMap.reset(new LumatoneHexMap(controller->shareMappingData()));
    reset(true);
}

void HexRings::reset(bool clearQueue)
{
    LumatoneSandboxGameBase::reset(clearQueue);
    auto layout = getIdentityLayout(true);
    queueLayout(layout);
}

void HexRings::nextTick()
{
    int limit = juce::jmin(frameQueue.size(), maxQueueFramesPerTick);
    for (int i = 0; i < limit; i++)
    {
        advanceFrameQueue();
        addToQueue(renderFrame());
    }
}

LumatoneAction* HexRings::renderFrame() const
{
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
                key.colour = frame.colour;

                keyUpdates.add(key);
            }
        }
    }

    if (keyUpdates.size() > 0)
        return new LumatoneEditAction::MultiKeyAssignAction(controller,  keyUpdates, false, true, false);

    return nullptr;
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

void HexRings::handleAnyNoteOn(int midiChannel, int midiNote, juce::uint8 velocity)
{
    auto colour = getRandomColourVelocity(velocity);
    auto ringSize = getRingSizeVelocity(velocity);

    auto hexOrigin = hexMap->keyCoordsToHex(midiChannel - 1, midiNote);

    auto neighbors = hexOrigin.neighbors(ringSize);
    for (auto point : neighbors)
    {
        auto coord = hexMap->hexToKeyCoords(point);
        if (!controller->getMappingData()->isKeyCoordValid(coord))
            continue;

        frameQueue.add({ coord, true, false, false, velocity, colour });
    }
}

LumatoneSandboxGameComponent* HexRings::createController()
{
    return new HexRingLauncher(this);
}

