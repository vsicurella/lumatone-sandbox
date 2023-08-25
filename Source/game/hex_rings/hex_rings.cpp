/*
  ==============================================================================

    hex_rings.cpp
    Created: 6 Aug 2023 11:50:57am
    Author:  Vincenzo

  ==============================================================================
*/

#include "hex_rings.h"

HexRings::HexRings(LumatoneController* controller)
    : LumatoneSandboxGameBase(controller, "Hex Rings")
{
    hexMap.reset(new LumatoneHexMap(controller->shareMappingData()));
    reset(true);
}

void HexRings::reset(bool clearQueue)
{
    LumatoneSandboxGameBase::reset(clearQueue);
    auto layout = queueIdentityLayout(true);
}

void HexRings::nextTick()
{
    while (frameQueue.size() > 0)
    {
        addToQueue(renderFrame());
    }
}

juce::UndoableAction* HexRings::renderFrame()
{
    if (frameQueue.size() > 0)
    {
        auto frame = frameQueue.removeAndReturn(0);
        if (frame.isNoteOn)
        {
            if (frame.value > 0)
            {
                return new LumatoneEditAction::SingleNoteAssignAction(controller, frame.origin.boardIndex, frame.origin.keyIndex, frame.colour);
            }
        }
    }

    return nullptr;
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
    return juce::roundToInt(velocity / 127) * 4;
}

void HexRings::handleNoteOn(LumatoneMidiState* midiState, int midiChannel, int midiNote, juce::uint8 velocity)
{
    auto colour = getRandomColourVelocity(velocity);
    auto ringSize = getRingSizeVelocity(velocity);

    auto hexOrigin = hexMap->keyCoordsToHex(midiChannel - 1, midiNote);

    auto neighbors = hexOrigin.neighbors(ringSize);
    for (auto point : neighbors)
    {
        auto coord = hexMap->hexToKeyCoords(point);
        if (!controller->isKeyCoordValid(coord))
            continue;

        frameQueue.add({ coord, true, false, false, velocity, colour });
    }
}
void HexRings::handleNoteOff(LumatoneMidiState* midiState, int midiChannel, int midiNote)
{

}
void HexRings::handleAftertouch(LumatoneMidiState* midiState, int midiChannel, int midiNote, juce::uint8 aftertouch)
{

}
void HexRings::handleController(LumatoneMidiState* midiState, int midiChannel, int midiNote, juce::uint8 controller)
{

}