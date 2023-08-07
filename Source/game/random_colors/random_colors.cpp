/*
  ==============================================================================

    random_colors.cpp
    Created: 30 Jul 2023 3:44:03pm
    Author:  Vincenzo

  ==============================================================================
*/

#include "random_colors.h"

RandomColors::RandomColors(LumatoneController* controllerIn, RandomColors::Options options)
    : LumatoneSandboxGameBase(controllerIn, "Random Colors")
{
    setOptions(options);
}

void RandomColors::reset(bool clearQueue)
{
    LumatoneSandboxGameBase::reset(clearQueue);

    random.setSeedRandomly();

    ticks = 0;

    if (clearQueue)
    {
        if (layoutBeforeStart.isEmpty())
            layoutBeforeStart = *controller->getMappingData();

        queueIdentityLayout(true);
    }
    else
    {
        addToQueue(renderFrame());
    }
}

void RandomColors::nextTick()
{
    ticks++;

    if (ticks % nextStepTicks != 0)
        return;

    reset(false);
}

juce::UndoableAction* RandomColors::renderFrame()
{
    int boardIndex = random.nextInt(5);
    int keyIndex = random.nextInt(56);

    auto colour = juce::Colour(
        random.nextInt(255),
        random.nextInt(255),
        random.nextInt(255)
    );

    if (keyColorConstrainer != nullptr)
        colour = keyColorConstrainer->validColour(colour, boardIndex, keyIndex);

    auto action = new LumatoneEditAction::SingleNoteAssignAction(controller, boardIndex, keyIndex, colour);
    return dynamic_cast<juce::UndoableAction*>(action);
}

void RandomColors::setOptions(RandomColors::Options newOptions)
{
    if (ticks > newOptions.nextStepTicks)
        ticks = newOptions.nextStepTicks - 1;
    nextStepTicks = newOptions.nextStepTicks;

    keyColorConstrainer = newOptions.keyColourConstrainer;
}

RandomColors::Options RandomColors::getOptions() const
{
    auto options = RandomColors::Options();
    options.nextStepTicks = nextStepTicks;
    options.keyColourConstrainer = keyColorConstrainer;
    return options;
}
