/*
  ==============================================================================

    random_colors.cpp
    Created: 30 Jul 2023 3:44:03pm
    Author:  Vincenzo

  ==============================================================================
*/

#include "random_colors.h"

RandomColors::RandomColors(LumatoneController* controllerIn, RandomColors::Options options)
    : LumatoneSandboxGameBase(controllerIn)
{
    setOptions(options);
}

void RandomColors::reset(bool clearQueue)
{
    if (clearQueue)
    {
        queuedActions.clearQuick();

        if (lastLayout.isEmpty())
            lastLayout = *controller->getMappingData();
    }

    ticks = 0;

    queuedActions.add(renderFrame());
}

void RandomColors::nextTick()
{
    ticks++;

    if (ticks % nextStepTicks != 0)
        return;

    reset(false);
}

LumatoneSandboxGameBase::OwnedActionPtr RandomColors::renderFrame()
{
    juce::Random random;

    int boardIndex = random.nextInt(4);
    int keyIndex = random.nextInt(56);

    const LumatoneKey* currentKey = controller->getKey(boardIndex, keyIndex);

    auto colour = juce::Colour(
        random.nextInt(255),
        random.nextInt(255),
        random.nextInt(255)
    );

    if (keyColorConstrainer != nullptr)
        colour = keyColorConstrainer->validColour(colour, boardIndex, keyIndex);

    auto action = new LumatoneEditAction::SingleNoteAssignAction(controller, boardIndex, keyIndex, colour);
    return OwnedActionPtr(action);
}

void RandomColors::setOptions(RandomColors::Options newOptions)
{
    nextStepTicks = newOptions.nextStepTicks;
    keyColorConstrainer = newOptions.keyColourConstrainer;
}

