#ifndef LUMATONE_EDITOR_EDITOR_LISTENER_H
#define LUMATONE_EDITOR_EDITOR_LISTENER_H

#include <JuceHeader.h>
#include "../data/lumatone_context.h"

namespace LumatoneEditor
{

class EditorListener
{
public:

    virtual ~EditorListener() {}
    
    // App Actions
    virtual void completeMappingLoaded(const LumatoneLayout& mappingData) {}
    virtual void boardChanged(const LumatoneBoard& boardData) {}
    virtual void keyChanged(int boardIndex, int keyIndex, const LumatoneKey& lumatoneKey) {}

    virtual void selectionChanged(juce::Array<MappedLumatoneKey> selection) {}

    virtual void newFileLoaded(juce::File file) {}

    //virtual void tableChanged(LumatoneConfigTable::TableType type, const juce::uint8* table, int tableSize) {}

    virtual void contextChanged(LumatoneContext* context) {}

    virtual void configTableChanged(LumatoneConfigTable::TableType type) {}


    // Firmware Actions
    virtual void keyConfigChanged(int boardIndex, int keyIndex, const LumatoneKey& keyData) {}
    virtual void keyColourChanged(int boardIndex, int keyIndex, juce::Colour keyColour) {}

    virtual void expressionPedalSensitivityChanged(unsigned char value) {}
    virtual void invertFootControllerChanged(bool inverted) {}
    
    virtual void macroButtonActiveColourChanged(juce::Colour colour) {}
    virtual void macroButtonInactiveColourChanged(juce::Colour colour) {}
    
    virtual void lightOnKeyStrokesChanged(bool lightOn) {}

    virtual void velocityConfigChanged() {}

    virtual void aftertouchToggled(bool enabled) {}
    virtual void calibrateAftertouchToggled(bool active) {}

    virtual void aftertouchConfigChanged() {}

    virtual void calibrateKeysRequested() {}
    virtual void calibratePitchModWheelToggled(bool active) {}

    virtual void lumatouchConfigChanged() {}

    virtual void firmwareVersionChanged() {}
    virtual void pingSent(juce::uint8 pingId) {}

    virtual void peripheralChannelsChanged(int pitchWheelChannel, int modWheelChannel, int expressionChannel, int sustainChannel) {}
    virtual void invertSustainToggled(bool inverted) {}

};

class EditorEmitter
{
protected:

    juce::ListenerList<LumatoneEditor::EditorListener> editorListeners;

public:
    EditorEmitter() { }

    virtual ~EditorEmitter() { editorListeners.clear(); }

    virtual void addEditorListener(LumatoneEditor::EditorListener* listenerIn) { editorListeners.add(listenerIn); }

    virtual void removeEditorListener(LumatoneEditor::EditorListener* listenerIn) { editorListeners.remove(listenerIn); }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EditorEmitter)
};

}

#endif
