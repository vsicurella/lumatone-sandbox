#ifndef LUMATONE_APPLICATION_STATE_H
#define LUMATONE_APPLICATION_STATE_H

#include "lumatone_state.h"
#include "lumatone_context.h"

enum class ConnectionState
{
    DISCONNECTED = 0,
    SEARCHING,
    OFFLINE,
    ONLINE,
    BUSY,
    ERROR
};

namespace LumatoneEditor
{
    class StatusListener;
    class EditorListener;
    class FirmwareListener;
    class MidiListener;
}

namespace LumatoneApplicationProperty
{
    // Device Management
    static const juce::Identifier DetectDeviceIfDisconnected = juce::Identifier("DetectDeviceIfDisconnected");
    static const juce::Identifier CheckConnectionIfInactive = juce::Identifier("CheckConnectionIfInactive");

    static const juce::Identifier DetectDevicesTimeout = juce::Identifier("DetectDevicesTimeout");
    static const juce::Identifier LastInputDeviceId = juce::Identifier("LastInputDeviceId");
    static const juce::Identifier LastOutputDeviceId = juce::Identifier("LastOutputDeviceId");

    // UI States
    static const juce::Identifier ConnectionStateId = juce::Identifier("ConnectionState");

    static const juce::Identifier LayoutContextIsSetId = juce::Identifier("LayoutContextIsSetId");

    // Settings
    // static const juce::Identifier DefaultMappingsDirectory = juce::Identifier("DefaultMappingsDirectory");
    // static const juce::Identifier LastMappingsDirectory = juce::Identifier("LastMappingsDirectory");
}

juce::Array<juce::Identifier> getLumatoneApplicationProperties();

class LumatoneFirmwareDriver;
class LumatoneController;
class LumatoneColourModel;
class LumatoneAction;
class DeviceActivityMonitor;

class LumatoneApplicationState : public LumatoneState
{
public:
    LumatoneApplicationState(juce::String nameIn, LumatoneFirmwareDriver& driverIn, juce::ValueTree stateIn=juce::ValueTree(), juce::UndoManager* undoManager=nullptr);
    LumatoneApplicationState(juce::String nameIn, const LumatoneApplicationState& stateIn);

    virtual ~LumatoneApplicationState() override;

    LumatoneController* getLumatoneController() const;
    LumatoneColourModel* getColourModel() const;

    // Connection Methods
    ConnectionState getConnectionState() const;
    int getMidiInputIndex() const;
    int getMidiOutputIndex() const;

    bool isAutoConnectionEnabled() const;

    virtual bool doSendChangesToDevice() const;

    // Context Methods
    bool isContextSet() const { return contextIsSet; }
    const LumatoneContext* getContext() const;
    std::shared_ptr<LumatoneContext> shareContext();

    LumatoneKeyContext getKeyContext(int boardIndex, int keyIndex) const;

    virtual void setContext(const LumatoneContext& contextIn);
    virtual void clearContext();

public:
    virtual void setCompleteConfig(const LumatoneLayout& layoutIn) override;
    virtual void setLayout(const LumatoneLayout& layoutIn) override;
    virtual void setBoard(const LumatoneBoard& boardIn, int boardId) override;
    
    virtual void setKey(const LumatoneKey& keyIn, int boardId, int keyIndex) override;
    virtual void setKeyConfig(const LumatoneKey& keyIn, int boardId, int keyIndex) override;
    virtual void setKeyColour(juce::Colour colour, int boardId, int keyIndex) override;
    virtual void sendSelectionParam(const juce::Array<MappedLumatoneKey>& selection, bool signalEditorListeners = true, bool bufferKeyUpdates = false) override;
    virtual void sendSelectionColours(const juce::Array<MappedLumatoneKey>& selection, bool signalEditorListeners = true, bool bufferKeyUpdates = false) override;
    
    virtual void setAftertouchEnabled(bool enabled) override;
    virtual void setLightOnKeyStrokes(bool enabled) override;
    virtual void setInvertExpression(bool invert) override;
    virtual void setInvertSustain(bool invert) override;
    virtual void setExpressionSensitivity(juce::uint8 sensitivity) override;
    
    virtual void setConfigTable(LumatoneConfigTable::TableType type, const LumatoneConfigTable& table) override;

private:
    bool performLumatoneAction(LumatoneAction* action, bool undoable = true, bool newTransaction = true);


protected:
    virtual void setInactiveMacroButtonColour(juce::Colour buttonColour) override;
    virtual void setActiveMacroButtonColour(juce::Colour buttonColour) override;

protected:
    virtual juce::ValueTree loadStateProperties(juce::ValueTree stateIn);

    virtual void handleStatePropertyChange(juce::ValueTree stateIn, const juce::Identifier& property) override;

    virtual void loadPropertiesFile(juce::PropertiesFile* properties) override;

private:
    ConnectionState connectionState = ConnectionState::DISCONNECTED;

private:
    std::shared_ptr<juce::ListenerList<LumatoneEditor::StatusListener>> statusListeners;
public:
    void addStatusListener(LumatoneEditor::StatusListener* listenerIn);
    void removeStatusListener(LumatoneEditor::StatusListener* listenerIn);

private:
    std::shared_ptr<juce::ListenerList<LumatoneEditor::EditorListener>> editorListeners;
public:
    void addEditorListener(LumatoneEditor::EditorListener* listenerIn);
    void removeEditorListener(LumatoneEditor::EditorListener* listenerIn);

private:
    std::shared_ptr<juce::ListenerList<LumatoneEditor::MidiListener>> midiListeners;
public:
    void addMidiListener(LumatoneEditor::MidiListener* listenerIn);
    void removeMidiListener(LumatoneEditor::MidiListener* listenerIn);

private:
    std::shared_ptr<juce::ListenerList<LumatoneEditor::FirmwareListener>> firmwareListeners;
public:
    void addFirmwareListener(LumatoneEditor::FirmwareListener* listenerIn);
    void removeFirmwareListener(LumatoneEditor::FirmwareListener* listenerIn);


private:
    LumatoneFirmwareDriver& firmwareDriver;

    std::shared_ptr<LumatoneContext> layoutContext;
	std::shared_ptr<LumatoneController> controller;
    std::shared_ptr<DeviceActivityMonitor> activityMonitor;
    std::shared_ptr<LumatoneColourModel> colourModel;

    bool contextIsSet = false;

//================================================================================
public:
    class Controller
    {
    public:
        Controller(LumatoneApplicationState& stateIn)
            : appState(stateIn) {}

    virtual bool requestSettingsFromDevice();
    virtual bool requestMappingFromDevice();
    virtual bool requestCompleteConfigFromDevice();

    void setInactiveMacroButtonColour(juce::Colour buttonColour);
    void setActiveMacroButtonColour(juce::Colour buttonColour);

    virtual bool performAction(LumatoneAction* action, bool undoable=true, bool newTransaction=true);

    protected:
        juce::ListenerList<LumatoneEditor::EditorListener>* getEditorListeners() const { return appState.editorListeners.get(); }
        juce::ListenerList<LumatoneEditor::StatusListener>* getStatusListeners() const { return appState.statusListeners.get(); }
        juce::ListenerList<LumatoneEditor::FirmwareListener>* getFirmwareListeners() const { return appState.firmwareListeners.get(); }
        juce::ListenerList<LumatoneEditor::MidiListener>* getMidiListeners() const { return appState.midiListeners.get(); }

    private:
        LumatoneApplicationState& appState;
    };

    class DeviceController : protected Controller
    {
    public:
        DeviceController(LumatoneApplicationState& stateIn)
            : Controller(stateIn)
            , deviceAppState(stateIn) {}

        juce::Array<juce::MidiDeviceInfo> getMidiInputList();
        juce::Array<juce::MidiDeviceInfo> getMidiOutputList();

    protected:
        virtual void setMidiInput(int deviceIndex, bool test = true);
        virtual void setMidiOutput(int deviceIndex, bool test = true);

        void setConnectionState(ConnectionState newState, bool sendNotification=true);

        void setAutoConnectionEnabled(bool enabled);

    private:
        LumatoneApplicationState& deviceAppState;
    };

private:
    friend class Controller;
    friend class DeviceController;
};

#endif // LUMATONE_APPLICATION_STATE_H
