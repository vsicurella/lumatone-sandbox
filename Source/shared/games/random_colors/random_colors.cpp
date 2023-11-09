/*
  ==============================================================================

    random_colors.cpp
    Created: 30 Jul 2023 3:44:03pm
    Author:  Vincenzo

  ==============================================================================
*/

#include "random_colors.h"
#include "random_colors_launcher.h"

#include "../../lumatone_editor_library/LumatoneController.h"
#include "../../lumatone_editor_library/actions/edit_actions.h"

RandomColors::RandomColors(juce::ValueTree gameEngineState, LumatoneController* controllerIn, RandomColors::Options options)
    : LumatoneSandboxGameBase(gameEngineState, controllerIn, "Random Colors")
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
        auto layout = getIdentityLayout(true);
        queueLayout(layout);
    }
}

void RandomColors::nextTick()
{
    ticks++;

    if (ticks % nextStepTicks != 0)
        return;

    nextRandomKey();

    ticks = 0;
    addToQueue(renderFrame());
}

LumatoneAction* RandomColors::renderFrame() const
{
    auto action = new LumatoneEditAction::SingleNoteAssignAction(controller, nextKeyState.boardIndex, nextKeyState.keyIndex, nextKeyState.colour);
    return action;
}

void RandomColors::nextRandomKey()
{
    nextKeyState.boardIndex = random.nextInt(controller->getNumBoards());
    nextKeyState.keyIndex = random.nextInt(controller->getOctaveBoardSize());

    auto colour = juce::Colour(
        (juce::uint8)random.nextInt(255),
        (juce::uint8)random.nextInt(255),
        (juce::uint8)random.nextInt(255)
    );

    if (keyColorConstrainer != nullptr)
        colour = keyColorConstrainer->validColour(colour, nextKeyState.boardIndex, nextKeyState.keyIndex);

    nextKeyState.colour = colour;
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

LumatoneSandboxGameComponent* RandomColors::createController()
{
    return new RandomColorsComponent(this);
}
