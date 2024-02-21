/*
  ==============================================================================

    game_base.cpp
    Created: 30 Jul 2023 3:45:04pm
    Author:  Vincenzo

  ==============================================================================
*/

#include "game_base.h"

#include "../lumatone_editor_library/actions/edit_actions.h"
#include "../lumatone_editor_library/device/lumatone_controller.h"

LumatoneSandboxGameBase::LumatoneSandboxGameBase(juce::String nameIn, const LumatoneApplicationState& stateIn)
    // : LumatoneApplicationState(nameIn, stateIn)
    : LumatoneApplicationMidi::Controller(static_cast<LumatoneApplicationMidi&>(*stateIn.getLumatoneController()))
    , LumatoneSandboxLogger(nameIn)
    , name(nameIn)
    , appState(stateIn)
{
    reset(true);
}

bool LumatoneSandboxGameBase::reset(bool clearActionQueue)
{
    if (clearActionQueue)
    {
        clearQueue();
    }

    quitGame = false;
    return true;
}

void LumatoneSandboxGameBase::clearQueue()
{
    // for (int i = 0; i < queueSize; i++)
    // {
    //     int ptr = (queuePtr + i) % MAX_QUEUE_SIZE;
    //     if (queuedActions[ptr] != nullptr)
    //         queuedActions[ptr] = nullptr;
    // }

    queueSize = 0;
    queuePtr = 0;
}

void LumatoneSandboxGameBase::readQueue(LumatoneEditor::LayoutAction* buffer, int& numActions)
{
    if (queueSize < 0)
    {
        queueSize = 0;
    }

    numActions = queueSize;

    for (int i = 0; i < queueSize; i++)
    {
        int ptr = (queuePtr + i) % MAX_QUEUE_SIZE;
        buffer[i] = queuedActions[ptr];
        // queuedActions[ptr] = nullptr;
    }

    queuePtr = 0;
    queueSize = 0;
}

void LumatoneSandboxGameBase::end()
{
    reset(true);
    // queueLayout(layoutBeforeStart);
}

void LumatoneSandboxGameBase::addToQueue(const LumatoneEditor::LayoutAction& action)
{
    // jassert(action != nullptr);
    // if (action == nullptr)
    //     return;

    if (queueSize < 0)
        queueSize = 1;
    else if (queueSize < MAX_QUEUE_SIZE)
        queueSize += 1;
    // else
        // queuedActions[getQueuePtr()] = nullptr;

    queuedActions[getQueuePtr()] = action;
}

void LumatoneSandboxGameBase::queueLayout(const LumatoneLayout& layout)
{
    for (int i = 0; i < appState.getNumBoards(); i++)
    {
        auto action = LumatoneEditor::LayoutAction();
        action.setData(layout.getBoard(i), i);
        addToQueue(action);
    }
    // addToQueue(new LumatoneGame::LayoutUpdateAction(layout));
}

void LumatoneSandboxGameBase::completeMappingLoaded(const LumatoneLayout& layout)
{
    // this is fine because we update via section update actions
    // so this should only be triggered by loading a new layout

    logInfo("completeMappingLoaded", "Backing up current layout.");
    // updateSavedLayout();
}
