/*
  ==============================================================================

    EditorEmitter.h
    Created: 21 Jun 2023 6:45:13pm
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ApplicationListeners.h"

//==============================================================================
/*
*/

namespace LumatoneEditor
{
    class EditorEmitter
    {
    protected:

        juce::ListenerList<LumatoneEditor::EditorListener> editorListeners;

    public:
        EditorEmitter() { }

        virtual ~EditorEmitter() { editorListeners.clear(); }

        virtual void addListener(LumatoneEditor::EditorListener* listenerIn) { editorListeners.add(listenerIn); }

        virtual void removeListener(LumatoneEditor::EditorListener* listenerIn) { editorListeners.remove(listenerIn); }

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EditorEmitter)
    };
}
