/*
==============================================================================

    lumatone_action.h
    Created: 30 Jul 2023 4:13:04pm
    Author:  Vincenzo

==============================================================================
*/

#ifndef LUMATONE_EDITOR_ACTION_H
#define LUMATONE_EDITOR_ACTION_H

#include <JuceHeader.h>

class LumatoneApplicationState;

namespace LumatoneEditor
{
class Action
{
public:
    Action() {}
    virtual bool perform(LumatoneApplicationState*)=0;
};

class UndoableAction : virtual public LumatoneEditor::Action
                     , public juce::UndoableAction
{
public:
    UndoableAction(LumatoneApplicationState* stateIn, juce::String nameIn=juce::String());

    virtual bool perform() = 0;
    virtual bool undo() = 0;

    int getSizeInUnits() override = 0;

    juce::String getName() const { return name; }

private:
    using LumatoneEditor::Action::perform;

protected:
    LumatoneApplicationState* state;

private:
    juce::String name;
};

}

#endif // LUMATONE_EDITOR_ACTION_H
