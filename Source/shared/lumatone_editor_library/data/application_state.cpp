#include "application_state.h"
#include "../color/colour_model.h"

LumatoneApplicationState::LumatoneApplicationState(juce::ValueTree stateIn, juce::UndoManager *undoManagerIn)
    : LumatoneState(stateIn, undoManagerIn)
{ 
    colourModel = std::make_shared<LumatoneColourModel>();
    loadStateProperties(stateIn);
}

LumatoneApplicationState::LumatoneApplicationState(const LumatoneState &stateIn, juce::UndoManager *undoManagerIn)
    : LumatoneState(stateIn, undoManagerIn)
{
    colourModel = std::make_shared<LumatoneColourModel>();
}

LumatoneApplicationState::LumatoneApplicationState(const LumatoneApplicationState &stateIn, juce::UndoManager *undoManagerIn)
    : LumatoneState((const LumatoneState&)stateIn, undoManagerIn)
{
    colourModel = stateIn.colourModel;
}

LumatoneApplicationState::~LumatoneApplicationState()
{
    colourModel = nullptr;
}

juce::Array<juce::Identifier> LumatoneApplicationState::getLumatoneApplicationProperties()
{
    juce::Array<juce::Identifier> properties;
    properties.add(LumatoneApplicationProperty::ConnectionStateId);
    properties.add(LumatoneApplicationProperty::DetectDeviceIfDisconnected);
    properties.add(LumatoneApplicationProperty::CheckConnectionIfInactive);
    return properties;
}

ConnectionState LumatoneApplicationState::getConnectionState() const
{
    return connectionState;
}

LumatoneColourModel* LumatoneApplicationState::getColourModel() const
{
    return colourModel.get();
}


juce::File LumatoneApplicationState::getDefaultMappingsDirectory()
{
    juce::File directory;
    juce::String path = getStringProperty(LumatoneApplicationProperty::DefaultMappingsDirectory);

    if (path.isNotEmpty())
    {
        directory = juce::File(path);
        if (directory.exists() && directory.isDirectory())
            return directory;
    }

    directory = juce::File::getSpecialLocation(juce::File::SpecialLocationType::userDocumentsDirectory);
    auto mappingDirectory = directory.getChildFile("Lumatone Editor").getChildFile("Mappings");
    if (mappingDirectory.exists() && mappingDirectory.isDirectory())
        return mappingDirectory;

    return directory;
}

juce::File LumatoneApplicationState::getLastMappingsDirectory()
{
    juce::File directory;
    juce::String path = getStringProperty(LumatoneApplicationProperty::LastMappingsDirectory);
    if (path.isNotEmpty())
    {
        directory = juce::File(path);
        if (directory.exists() && directory.isDirectory())
            return directory;
    }

    return getDefaultMappingsDirectory();
}

juce::ValueTree LumatoneApplicationState::loadStateProperties(juce::ValueTree stateIn)
{
    juce::ValueTree newState = (stateIn.hasType(LumatoneStateProperty::StateTree))
                             ? stateIn
                             : juce::ValueTree(LumatoneStateProperty::StateTree);

    DBG("LumatoneApplicationState::loadStateProperties:\n" + newState.toXmlString());
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
    else if (property == LumatoneApplicationProperty::DetectDeviceIfDisconnected)
    {
        detectDeviceIfDisconnected = (bool)stateIn.getProperty(property, true);
    }
    else if (property == LumatoneApplicationProperty::CheckConnectionIfInactive)
    {
        monitorConnectionStatus = (bool)stateIn.getProperty(property, true);
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

LumatoneKeyContext LumatoneApplicationState::getKeyContext(int boardIndex, int keyIndex)
{
    if (layoutContext.get() == nullptr)
    {
        auto coord = LumatoneKeyCoord(boardIndex, keyIndex);
        MappedLumatoneKey key = MappedLumatoneKey(*getKey(boardIndex, keyIndex), boardIndex, keyIndex);
        return LumatoneKeyContext(key);
    }

    return layoutContext->getKeyContext(boardIndex, keyIndex);
}

void LumatoneApplicationState::setContext(std::shared_ptr<LumatoneContext> contextIn)
{
    if (layoutContext.get() != nullptr)
        clearContext();

    layoutContext = contextIn;
    contextIsSet = true;

    state.setPropertyExcludingListener(this, LumatoneApplicationProperty::LayoutContextIsSetId, contextIsSet, undoManager);
}

void LumatoneApplicationState::clearContext()
{
    layoutContext = nullptr;
    contextIsSet = false;

    state.setPropertyExcludingListener(this, LumatoneApplicationProperty::LayoutContextIsSetId, contextIsSet, undoManager);
}
