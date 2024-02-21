#include "application_state.h"

#include "../device/lumatone_controller.h"
#include "../device/activity_monitor.h"
#include "../color/colour_model.h"
#include "../data/lumatone_context.h"
#include "../actions/lumatone_action.h"

#include "../listeners/status_listener.h"
#include "../listeners/editor_listener.h"
#include "../listeners/firmware_listener.h"
#include "../listeners/midi_listener.h"

#include "../lumatone_midi_driver/lumatone_midi_driver.h"

using namespace LumatoneEditor;

juce::Array<juce::Identifier> getLumatoneApplicationProperties()
{
    juce::Array<juce::Identifier> properties;
    properties.add(LumatoneApplicationProperty::ConnectionStateId);
    properties.add(LumatoneApplicationProperty::LayoutContextIsSetId);
    return properties;
}

LumatoneApplicationState::LumatoneApplicationState(juce::String nameIn, LumatoneFirmwareDriver& driverIn, juce::ValueTree stateIn, juce::UndoManager *undoManagerIn)
    : LumatoneState(nameIn, stateIn, undoManagerIn)
    , firmwareDriver(driverIn)
{
    editorListeners = std::make_shared<juce::ListenerList<EditorListener>>();
    statusListeners = std::make_shared<juce::ListenerList<StatusListener>>();
    firmwareListeners = std::make_shared<juce::ListenerList<FirmwareListener>>();
    midiListeners = std::make_shared<juce::ListenerList<MidiListener>>();

    layoutContext = std::make_shared<LumatoneContext>(*mappingData);
	controller = std::make_shared<LumatoneController>(*this, driverIn);
    activityMonitor = std::make_shared<DeviceActivityMonitor>(*this, &driverIn);
    colourModel = std::make_shared<LumatoneColourModel>();

    loadStateProperties(stateIn);
}

// LumatoneApplicationState::LumatoneApplicationState(juce::String nameIn, const LumatoneState &stateIn, juce::UndoManager *undoManagerIn)
//     : LumatoneState(nameIn, stateIn, undoManagerIn)
// {
//     colourModel = std::make_shared<LumatoneColourModel>();
//     layoutContext = std::make_shared<LumatoneContext>(*mappingData);
//     loadStateProperties(state);
// }

LumatoneApplicationState::LumatoneApplicationState(juce::String nameIn, const LumatoneApplicationState &stateIn)
    : LumatoneState(nameIn, (const LumatoneState&)stateIn)
    , firmwareDriver(stateIn.firmwareDriver)
    , editorListeners(stateIn.editorListeners)
    , statusListeners(stateIn.statusListeners)
    , firmwareListeners(stateIn.firmwareListeners)
    , midiListeners(stateIn.midiListeners)
    , layoutContext(stateIn.layoutContext)
    , controller(stateIn.controller)
    , activityMonitor(stateIn.activityMonitor)
    , colourModel(stateIn.colourModel)
{
    loadStateProperties(state);
}

LumatoneApplicationState::LumatoneApplicationState(juce::String nameIn, LumatoneApplicationState &stateIn, juce::Identifier childId)
    : LumatoneState(nameIn, juce::ValueTree(childId), nullptr)   
    , firmwareDriver(stateIn.firmwareDriver)
    , editorListeners(stateIn.editorListeners)
    , statusListeners(stateIn.statusListeners)
    , firmwareListeners(stateIn.firmwareListeners)
    , midiListeners(stateIn.midiListeners)
    , layoutContext(stateIn.layoutContext)
    , controller(stateIn.controller)
    , activityMonitor(stateIn.activityMonitor)
    , colourModel(stateIn.colourModel)
{
    stateIn.addChildState(state);
    state.getParent().addListener(this);
    // loadStateProperties(state);
}

LumatoneApplicationState::~LumatoneApplicationState()
{
    layoutContext = nullptr;
    activityMonitor = nullptr;
    controller = nullptr;
    colourModel = nullptr;
}

ConnectionState LumatoneApplicationState::getConnectionState() const
{
    return connectionState;
}

int LumatoneApplicationState::getMidiInputIndex() const
{
    return firmwareDriver.getMidiInputIndex();
}

int LumatoneApplicationState::getMidiOutputIndex() const
{
    return firmwareDriver.getMidiOutputIndex();
}

bool LumatoneApplicationState::isAutoConnectionEnabled() const
{
    return activityMonitor->willDetectDeviceIfDisconnected();
}

bool LumatoneApplicationState::doSendChangesToDevice() const
{
    return connectionState == ConnectionState::ONLINE;
}

LumatoneController *LumatoneApplicationState::getLumatoneController() const
{
    return controller.get();
}

LumatoneColourModel *LumatoneApplicationState::getColourModel() const
{
    return colourModel.get();
}

const LumatoneContext *LumatoneApplicationState::getContext() const
{
    return contextIsSet ? layoutContext.get() : nullptr;
}

std::shared_ptr<LumatoneContext> LumatoneApplicationState::shareContext()
{
    return layoutContext;
}

bool LumatoneApplicationState::performAction(Action *action)
{
    if (action == nullptr)
        return false;

    return action->perform(this);
}

bool LumatoneApplicationState::performUndoableAction(UndoableAction *action, bool newTransaction)
{
    if (undoManager == nullptr)
        return false;

    if (newTransaction)
        undoManager->beginNewTransaction();

    return undoManager->perform(action, action->getName());
}

void LumatoneApplicationState::setInactiveMacroButtonColour(juce::Colour buttonColour)
{
    LumatoneState::setInactiveMacroButtonColour(buttonColour);

    if (doSendChangesToDevice())
    {
        controller->sendMacroButtonInactiveColour(buttonColour.toString());
    }

    editorListeners->call(&EditorListener::macroButtonInactiveColourChanged, buttonColour);
}

void LumatoneApplicationState::setActiveMacroButtonColour(juce::Colour buttonColour)
{
    LumatoneState::setInactiveMacroButtonColour(buttonColour);

    if (doSendChangesToDevice())
    {
        controller->sendMacroButtonInactiveColour(buttonColour.toString());
    }

    editorListeners->call(&EditorListener::macroButtonActiveColourChanged, buttonColour);
}

juce::ValueTree LumatoneApplicationState::loadStateProperties(juce::ValueTree stateIn)
{
    juce::ValueTree newState = (stateIn.hasType(LumatoneStateProperty::StateTree))
                             ? stateIn
                             : juce::ValueTree(LumatoneStateProperty::StateTree);

    // DBG("LumatoneApplicationState::loadStateProperties:\n" + newState.toXmlString());
    for (auto property : getLumatoneApplicationProperties())
    {
        if (newState.hasProperty(property))
            handleStatePropertyChange(newState, property);
    }

    LumatoneState::loadStateProperties(newState);

    return newState;
}

void LumatoneApplicationState::handleStatePropertyChange(juce::ValueTree stateIn, const juce::Identifier &property)
{
    if (property == LumatoneApplicationProperty::ConnectionStateId)
    {
        connectionState = ConnectionState((int)stateIn.getProperty(property, (int)ConnectionState::DISCONNECTED));
    }
    else if (property == LumatoneApplicationProperty::LayoutContextIsSetId)
    {
        contextIsSet = (bool)stateIn.getProperty(property, false);
    }
    else
    {
        LumatoneState::handleStatePropertyChange(stateIn, property);
    }
}

void LumatoneApplicationState::loadPropertiesFile(juce::PropertiesFile *properties)
{
    setStateProperty(LumatoneApplicationProperty::DetectDeviceIfDisconnected, properties->getBoolValue(LumatoneApplicationProperty::DetectDeviceIfDisconnected.toString(), true));
    setStateProperty(LumatoneApplicationProperty::CheckConnectionIfInactive, properties->getBoolValue(LumatoneApplicationProperty::CheckConnectionIfInactive.toString(), true));

    setStateProperty(LumatoneApplicationProperty::LastInputDeviceId, properties->getValue(LumatoneApplicationProperty::LastInputDeviceId.toString(), juce::String()));
    setStateProperty(LumatoneApplicationProperty::LastOutputDeviceId, properties->getValue(LumatoneApplicationProperty::LastOutputDeviceId.toString(), juce::String()));
}

LumatoneKeyContext LumatoneApplicationState::getKeyContext(int boardIndex, int keyIndex) const
{
    if (contextIsSet)
    {
        return layoutContext->getKeyContext(boardIndex, keyIndex);
    }

    MappedLumatoneKey key = MappedLumatoneKey(getKey(boardIndex, keyIndex), boardIndex, keyIndex);
    return LumatoneKeyContext(key);
}

void LumatoneApplicationState::setContext(const LumatoneContext& contextIn)
{
    if (layoutContext.get() != nullptr)
        clearContext();

    *layoutContext = contextIn;
    contextIsSet = true;

    state.setPropertyExcludingListener(this, LumatoneApplicationProperty::LayoutContextIsSetId, contextIsSet, undoManager);
}

void LumatoneApplicationState::clearContext()
{
    *layoutContext = LumatoneContext(*mappingData);
    contextIsSet = false;

    state.setPropertyExcludingListener(this, LumatoneApplicationProperty::LayoutContextIsSetId, contextIsSet, undoManager);
}

void LumatoneApplicationState::setCompleteConfig(const LumatoneLayout &layoutIn)
{
    LumatoneState::setCompleteConfig(layoutIn);

    if (doSendChangesToDevice())
    {
        controller->sendCurrentCompleteConfig();
    }

    editorListeners->call(&EditorListener::completeMappingLoaded, *mappingData);
    editorListeners->call(&EditorListener::expressionPedalSensitivityChanged, getExpressionSensitivity());
    editorListeners->call(&EditorListener::invertFootControllerChanged, getInvertExpression());
    editorListeners->call(&EditorListener::lightOnKeyStrokesChanged, getLightOnKeyStrokes());
    editorListeners->call(&EditorListener::aftertouchToggled, getAftertouchOn());

    editorListeners->call(&EditorListener::configTableChanged, LumatoneConfigTable::TableType::velocityInterval);
    editorListeners->call(&EditorListener::configTableChanged, LumatoneConfigTable::TableType::fader);
    editorListeners->call(&EditorListener::configTableChanged, LumatoneConfigTable::TableType::afterTouch);
    editorListeners->call(&EditorListener::configTableChanged, LumatoneConfigTable::TableType::lumaTouch);
}

void LumatoneApplicationState::setLayout(const LumatoneLayout &layoutIn)
{
    LumatoneState::setLayout(layoutIn);

    if (doSendChangesToDevice())
    {
        controller->sendCompleteMapping(layoutIn);
    }

    editorListeners->call(&EditorListener::completeMappingLoaded, *mappingData);
}

void LumatoneApplicationState::setBoard(const LumatoneBoard &boardIn, int boardId)
{
    LumatoneState::setBoard(boardIn, boardId);

    if (doSendChangesToDevice())
    {
        controller->sendAllParamsOfBoard(boardId, &boardIn);
    }

    editorListeners->call(&EditorListener::boardChanged, getBoard(boardId-1));
}

void LumatoneApplicationState::setKey(const LumatoneKey &keyIn, int boardId, int keyIndex)
{
    LumatoneState::setKey(keyIn, boardId, keyIndex);

    if (doSendChangesToDevice())
    {
        controller->sendKeyParam(boardId, keyIndex, keyIn);
    }

    editorListeners->call(&EditorListener::keyChanged, boardId - 1, keyIndex, getKey(boardId - 1, keyIndex));
}

void LumatoneApplicationState::setKeyConfig(const LumatoneKey& keyIn, int boardId, int keyIndex)
{
    LumatoneState::setKeyConfig(keyIn, boardId, keyIndex);

    if (doSendChangesToDevice())
    {
        controller->sendKeyConfig(boardId, keyIndex, keyIn);
    }

    editorListeners->call(&EditorListener::keyChanged, boardId - 1, keyIndex, getKey(boardId - 1, keyIndex));
}


void LumatoneApplicationState::setKeyColour(juce::Colour colour, int boardId, int keyIndex)
{
    LumatoneState::setKeyColour(colour, boardId, keyIndex);

    if (doSendChangesToDevice())
    {
        controller->sendKeyColourConfig(boardId, keyIndex, colour);
    }

    editorListeners->call(&EditorListener::keyChanged, boardId - 1, keyIndex, getKey(boardId - 1, keyIndex));
}

void LumatoneApplicationState::sendSelectionParam(const juce::Array<MappedLumatoneKey>& selection, bool signalEditorListeners, bool bufferKeyUpdates)
{
    LumatoneState::sendSelectionParam(selection);

    for (auto mappedKey : selection)
    {
        controller->sendKeyParam(mappedKey.boardIndex + 1, mappedKey.keyIndex, static_cast<const LumatoneKey&>(mappedKey));
    }

    //if (signalEditorListeners)
    editorListeners->call(&EditorListener::selectionChanged, selection);
}

void LumatoneApplicationState::sendSelectionColours(const juce::Array<MappedLumatoneKey>& selection, bool signalEditorListeners, bool bufferKeyUpdates)
{
    LumatoneState::sendSelectionColours(selection);

    for (auto mappedKey : selection)
    {
        controller->sendKeyColourConfig(mappedKey.boardIndex + 1, mappedKey.keyIndex, static_cast<const LumatoneKey&>(mappedKey));
    }

    //if (signalEditorListeners)
    editorListeners->call(&EditorListener::selectionChanged, selection);
}

void LumatoneApplicationState::setAftertouchEnabled(bool enabled)
{
    LumatoneState::setAftertouchEnabled(enabled);

    if (doSendChangesToDevice())
    {
        controller->setAftertouchEnabled(enabled);
    }

    editorListeners->call(&EditorListener::aftertouchToggled, enabled);
}

void LumatoneApplicationState::setLightOnKeyStrokes(bool enabled)
{
    LumatoneState::setLightOnKeyStrokes(enabled);

    if (doSendChangesToDevice())
    {
        controller->sendLightOnKeyStrokes(enabled);
    }

    editorListeners->call(&EditorListener::lightOnKeyStrokesChanged, enabled);
}

void LumatoneApplicationState::setInvertExpression(bool invert)
{
    LumatoneState::setInvertExpression(invert);

    if (doSendChangesToDevice())
    {
        controller->sendInvertFootController(invert);
    }

    editorListeners->call(&EditorListener::invertFootControllerChanged, invert);
}

void LumatoneApplicationState::setInvertSustain(bool invert)
{
    LumatoneState::setInvertSustain(invert);
    
    if (doSendChangesToDevice())
    {
        controller->invertSustainPedal(invert);
    }

    editorListeners->call(&EditorListener::invertSustainToggled, invert);
}

void LumatoneApplicationState::setExpressionSensitivity(juce::uint8 sensitivity)
{
    LumatoneState::setExpressionSensitivity(sensitivity);

    if (doSendChangesToDevice())
    {
        controller->sendExpressionPedalSensivity(sensitivity);
    }

    editorListeners->call(&EditorListener::expressionPedalSensitivityChanged, sensitivity);
}

void LumatoneApplicationState::setConfigTable(LumatoneConfigTable::TableType type, const LumatoneConfigTable& table)
{
    LumatoneState::setConfigTable(type, table);

    if (doSendChangesToDevice())
    {
        controller->sendTableConfig(type, table.velocityValues);
    }

    editorListeners->call(&EditorListener::configTableChanged, type);
}
//
//void LumatoneApplicationState::setVelocityIntervalTable(const LumatoneConfigTable& tableIn)
//{
//    LumatoneState::setVelocityIntervalTable(tableIn);
//
//    if (doSendChangesToDevice())
//    {
//        controller->setVelocityIntervalTable(tableIn);
//    }
//}
//
//void LumatoneApplicationState::setNoteVelocityTable(const LumatoneConfigTable& tableIn)
//{
//    LumatoneState::setNoteVelocityTable(tableIn);
//
//    if (doSendChangesToDevice())
//    {
//        controller->setNoteVelocityTable(tableIn);
//    }
//}
//
//void LumatoneApplicationState::setAftertouchTable(const LumatoneConfigTable& tableIn)
//{
//    LumatoneState::setAftertouchTable(tableIn);
//
//    if (doSendChangesToDevice())
//    {
//        controller->setAftertouchTable(tableIn);
//    }
//}
//
//void LumatoneApplicationState::setLumatouchTable(const LumatoneConfigTable& tableIn)
//{
//    LumatoneState::setLumatouchTable(tableIn);
//
//    if (doSendChangesToDevice())
//    {
//        controller->setLumatouchTable(tableIn);
//    }
//}

void LumatoneApplicationState::addStatusListener(StatusListener* listenerIn)
{
    statusListeners->add(listenerIn);
}

void LumatoneApplicationState::removeStatusListener(StatusListener* listenerIn)
{
    statusListeners->remove(listenerIn);
}

void LumatoneApplicationState::addEditorListener(EditorListener* listenerIn)
{
    editorListeners->add(listenerIn);
}

void LumatoneApplicationState::removeEditorListener(EditorListener* listenerIn)
{
    editorListeners->remove(listenerIn);
}

void LumatoneApplicationState::addFirmwareListener(FirmwareListener* listenerIn)
{
    firmwareListeners->add(listenerIn);
}

void LumatoneApplicationState::removeFirmwareListener(FirmwareListener* listenerIn)
{
    firmwareListeners->remove(listenerIn);
}

void LumatoneApplicationState::addMidiListener(MidiListener* listenerIn)
{
    midiListeners->add(listenerIn);
}

void LumatoneApplicationState::removeMidiListener(MidiListener* listenerIn)
{
    midiListeners->remove(listenerIn);
}

bool LumatoneApplicationState::Controller::requestCompleteConfigFromDevice()
{
    if (appState.connectionState != ConnectionState::ONLINE)
        return false;

    requestSettingsFromDevice();
    requestMappingFromDevice();

    return true;
}

bool LumatoneApplicationState::Controller::requestSettingsFromDevice()
{
    if (appState.connectionState != ConnectionState::ONLINE)
        return false;

    // Macro button colours
    appState.controller->requestMacroButtonColours();
    
	// General options
	appState.controller->requestPresetFlags();
	appState.controller->requestExpressionPedalSensitivity();

	// Velocity curve config
	appState.controller->sendVelocityIntervalConfigRequest();
	appState.controller->sendVelocityConfigRequest();
	appState.controller->sendFaderConfigRequest();
	appState.controller->sendAftertouchConfigRequest();

    return true;
}

bool LumatoneApplicationState::Controller::requestMappingFromDevice()
{
    if (appState.connectionState != ConnectionState::ONLINE)
        return false;

    // Request MIDI channel, MIDI note, colour and key type config for all keys
    appState.controller->sendGetCompleteMappingRequest();
    return true;
}

void LumatoneApplicationState::Controller::setInactiveMacroButtonColour(juce::Colour buttonColour)
{
    appState.setInactiveMacroButtonColour(buttonColour);
}

void LumatoneApplicationState::Controller::setActiveMacroButtonColour(juce::Colour buttonColour)
{
    appState.setActiveMacroButtonColour(buttonColour);
}

// bool LumatoneApplicationState::Controller::performAction(Action *action)
// {
//     return appState.performAction(action);
// }

// bool LumatoneApplicationState::Controller::performUndoableAction(UndoableAction *action, bool newTransaction)
// {
//     return appState.performUndoableAction(action, newTransaction);
// }

void LumatoneApplicationState::DeviceController::setConnectionState(ConnectionState newState, bool sendNotification)
{
    deviceAppState.connectionState = newState;
    deviceAppState.setStateProperty(LumatoneApplicationProperty::ConnectionStateId, juce::var((int)deviceAppState.connectionState));
    if (sendNotification)
        getStatusListeners()->call(&StatusListener::connectionStateChanged, deviceAppState.connectionState);
}

void LumatoneApplicationState::DeviceController::setAutoConnectionEnabled(bool enabled)
{
    deviceAppState.activityMonitor->setDetectDeviceIfDisconnected(enabled);
    deviceAppState.activityMonitor->setCheckForInactivity(enabled);
}

juce::Array<juce::MidiDeviceInfo> LumatoneApplicationState::DeviceController::getMidiInputList()
{
    return deviceAppState.firmwareDriver.getMidiInputList();
}

juce::Array<juce::MidiDeviceInfo> LumatoneApplicationState::DeviceController::getMidiOutputList()
{
    return deviceAppState.firmwareDriver.getMidiOutputList();
}

void LumatoneApplicationState::DeviceController::setMidiInput(int deviceIndex, bool test)
{
    auto deviceInfo = getMidiInputList()[deviceIndex];
    deviceAppState.setStateProperty(LumatoneApplicationProperty::LastInputDeviceId, deviceInfo.identifier);
    deviceAppState.controller->setDriverMidiInput(deviceIndex, test);
}

void LumatoneApplicationState::DeviceController::setMidiOutput(int deviceIndex, bool test)
{
    auto deviceInfo = getMidiOutputList()[deviceIndex];
    deviceAppState.setStateProperty(LumatoneApplicationProperty::LastOutputDeviceId, deviceInfo.identifier);
    deviceAppState.controller->setDriverMidiOutput(deviceIndex, test);
}
