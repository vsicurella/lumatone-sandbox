/*
==============================================================================

    LumatoneSandboxState.cpp
    Created: 25 Jan 2024 11:57pm
    Author:  Vincenzo

==============================================================================
*/

#include "SandboxState.h"

#include "./lumatone_editor_library/device/lumatone_controller.h"
#include "./lumatone_editor_library/listeners/editor_listener.h"

#include "./lumatone_editor_library/palettes/colour_palette_file.h"

#include "../shared/game_engine/game_engine.h"

static juce::File getDefaultUserDocumentsDirectory()
{
    return juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("Lumatone Editor");
}
static juce::File getDefaultUserMappingsDirectory()
{
    return getDefaultUserDocumentsDirectory().getChildFile("Mappings");
}
static juce::File getDefaultUserPalettesDirectory()
{
    return getDefaultUserDocumentsDirectory().getChildFile("Palettes");
}

// juce::Array<juce::Identifier> GetLumatoneSandboxProperty()
// {
//     juce::Array<juce::Identifier> properties;
//     properties.add(LumatoneSandboxProperty::HasChangesToSave);
//     properties.add(LumatoneSandboxProperty::HasChangesToSend);
//     properties.add(LumatoneSandboxProperty::InCalibrationMode);
//     properties.add(LumatoneSandboxProperty::FirmwareUpdatePerformed);
//     properties.add(LumatoneSandboxProperty::ColourPalettes);
//     properties.add(LumatoneSandboxProperty::CurrentFile);
//     properties.add(LumatoneSandboxProperty::RecentFiles);
//     properties.add(LumatoneSandboxProperty::UserDocumentsDirectory);
//     properties.add(LumatoneSandboxProperty::UserMappingsDirectory);
//     properties.add(LumatoneSandboxProperty::UserPalettesDirectory);
//     properties.add(LumatoneSandboxProperty::DeveloperModeOn);
//     properties.add(LumatoneSandboxProperty::EditorMode);
//     return properties;
// }

LumatoneSandboxState::LumatoneSandboxState(LumatoneFirmwareDriver& driverIn, juce::UndoManager *undoManagerIn)
    : LumatoneApplicationState("LumatoneSandbox", driverIn, juce::ValueTree(), undoManagerIn)
{
    // loadPropertiesFile(nullptr);
    colourPalettes = std::make_shared<juce::Array<LumatoneEditorColourPalette>>();
    gameEngine = std::make_shared<LumatoneSandboxGameEngine>(*this);
    recentFiles = std::make_shared<juce::RecentlyOpenedFilesList>();
}

LumatoneSandboxState::LumatoneSandboxState(juce::String name, const LumatoneSandboxState &stateIn)
    : LumatoneApplicationState(name, stateIn)
    , gameEngine(stateIn.gameEngine)
    , colourPalettes(stateIn.colourPalettes)
    , recentFiles(stateIn.recentFiles)
    , propertiesFile(stateIn.propertiesFile)
{
}

LumatoneSandboxState::~LumatoneSandboxState()
{
    gameEngine = nullptr;
    recentFiles = nullptr;
    propertiesFile = nullptr;
}

juce::String LumatoneSandboxState::getProperty(juce::Identifier propertyId, juce::String fallbackValue) const
{
    return propertiesFile->getValue(propertyId, fallbackValue);
}

juce::RecentlyOpenedFilesList& LumatoneSandboxState::getRecentFiles()
{
    return *recentFiles;
}

const juce::Array<LumatoneEditorColourPalette>& LumatoneSandboxState::getColourPalettes()
{
    return *colourPalettes;
}

juce::File LumatoneSandboxState::getUserDocumentsDirectory() const
{
    juce::String possibleDirectory = propertiesFile->getValue(LumatoneSandboxProperty::UserDocumentsDirectory);
    juce::File directory;

    if (juce::File::isAbsolutePath(possibleDirectory))
    {
        directory = juce::File(possibleDirectory);
    }
    if (!directory.exists() || directory.existsAsFile())
    {
        directory = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("Lumatone Editor");

        // User pref?
        directory.createDirectory();
    }
    return directory;
}

juce::File LumatoneSandboxState::getUserMappingsDirectory() const
{
    juce::File parentFolder = getUserDocumentsDirectory();

    juce::String possibleDirectory = propertiesFile->getValue(LumatoneSandboxProperty::UserMappingsDirectory);
    juce::File directory;

    if (juce::File::isAbsolutePath(possibleDirectory))
    {
        directory = juce::File(possibleDirectory);
    }
    if (!directory.exists() || parentFolder.isDirectory())
    {
        directory = parentFolder.getChildFile("Mappings");

        // User pref?
        directory.createDirectory();
    }
    return directory;
}

juce::File LumatoneSandboxState::getUserPalettesDirectory() const
{
    juce::File parentFolder = getUserDocumentsDirectory();

    juce::String possibleDirectory = propertiesFile->getValue(LumatoneSandboxProperty::UserPalettesDirectory);
    juce::File directory;

    if (juce::File::isAbsolutePath(possibleDirectory))
    {
        directory = juce::File(possibleDirectory);
    }
    if (!directory.exists() || parentFolder.isDirectory())
    {
        directory = parentFolder.getChildFile("Palettes");

        // User pref?
        directory.createDirectory();
    }
    return directory;
}

juce::File LumatoneSandboxState::getLastOpenedMappingsDirectory() const
{
    if (recentFiles->getNumFiles() > 0)
    {
        auto file = recentFiles->getFile(0);
        if (file.existsAsFile())
            return file.getParentDirectory();
    }

    return getDefaultUserMappingsDirectory();
}

// void LumatoneSandboxState::setHasChangesToSave(bool hasChangesToSaveIn)
// {
//     hasChangesToSave = hasChangesToSaveIn;
//     setStateProperty(LumatoneSandboxProperty::HasChangesToSave, hasChangesToSave);
// }

// bool LumatoneSandboxState::doSendChangesToDevice() const
// {
//     return LumatoneApplicationState::doSendChangesToDevice() && editorMode == EditorMode::ONLINE;
// }

juce::ValueTree LumatoneSandboxState::loadStateProperties(juce::ValueTree stateIn)
{
    juce::ValueTree newState = (stateIn.hasType(LumatoneSandboxProperty::StateTree))
                             ? stateIn
                             : juce::ValueTree(LumatoneSandboxProperty::StateTree);

    // TODO load editor properties

    // DBG("LumatoneApplicationState::loadStateProperties:\n" + newState.toXmlString());
    for (auto property : getLumatoneApplicationProperties())
    {
        if (newState.hasProperty(property))
            handleStatePropertyChange(newState, property);
    }

    LumatoneState::loadStateProperties(newState);

    return newState;
}

void LumatoneSandboxState::handleStatePropertyChange(juce::ValueTree stateIn, const juce::Identifier &property)
{
    LumatoneApplicationState::handleStatePropertyChange(stateIn, property);

    if (property == LumatoneSandboxProperty::CurrentFile)
    {
        currentFile = juce::File(stateIn[property]);
    }
}

LumatoneSandboxGameEngine *LumatoneSandboxState::getGameEngine()
{
    return gameEngine.get();
}

// void LumatoneSandboxState::loadPropertiesFile(juce::PropertiesFile *propertiesIn)
// {
//     juce::PropertiesFile::Options options;
//     options.applicationName = "LumatoneSetup";
//     options.filenameSuffix = "settings";
//     options.osxLibrarySubFolder = "Application Support";
// #if JUCE_LINUX
//     options.folderName = "~/.config/LumatoneSetup";
// #else
//     options.folderName = "LumatoneSetup";
// #endif

//     propertiesFile = std::make_shared<juce::PropertiesFile>(options);
//     DBG(propertiesFile->createXml("LumatoneEditorSettings")->toString());
//     jassert(propertiesFile != nullptr);

//     recentFiles = std::make_shared<juce::RecentlyOpenedFilesList>();
//     recentFiles->restoreFromString(propertiesFile->getValue(LumatoneSandboxProperty::RecentFiles));
//     recentFiles->removeNonExistentFiles();

//     LumatoneApplicationState::loadPropertiesFile(propertiesFile.get());

//     setStateProperty(LumatoneSandboxProperty::UserDocumentsDirectory, propertiesFile->getValue(LumatoneSandboxProperty::UserDocumentsDirectory.toString(), getDefaultUserDocumentsDirectory().getFullPathName()));
//     setStateProperty(LumatoneSandboxProperty::UserMappingsDirectory, propertiesFile->getValue(LumatoneSandboxProperty::UserMappingsDirectory.toString(), getDefaultUserMappingsDirectory().getFullPathName()));
//     setStateProperty(LumatoneSandboxProperty::UserPalettesDirectory, propertiesFile->getValue(LumatoneSandboxProperty::UserPalettesDirectory.toString(), getDefaultUserPalettesDirectory().getFullPathName()));
//     setStateProperty(LumatoneSandboxProperty::RecentFiles, propertiesFile->getValue(LumatoneSandboxProperty::RecentFiles.toString(), juce::String()));
//     setStateProperty(LumatoneSandboxProperty::MainWindowState, propertiesFile->getValue(LumatoneSandboxProperty::MainWindowState.toString(), juce::String()));
    
//     setStateProperty(LumatoneSandboxProperty::AutoConnectDevice, propertiesFile->getBoolValue(LumatoneSandboxProperty::AutoConnectDevice.toString(), true));

//     setStateProperty(LumatoneSandboxProperty::SingleNoteKeyTypeSetActive, propertiesFile->getBoolValue(LumatoneSandboxProperty::SingleNoteKeyTypeSetActive.toString(), true));
//     setStateProperty(LumatoneSandboxProperty::SingleNoteNoteSetActive, propertiesFile->getBoolValue(LumatoneSandboxProperty::SingleNoteNoteSetActive.toString(), true));
//     setStateProperty(LumatoneSandboxProperty::SingleNoteChannelSetActive, propertiesFile->getBoolValue(LumatoneSandboxProperty::SingleNoteChannelSetActive.toString(), true));
//     setStateProperty(LumatoneSandboxProperty::SingleNoteColourSetActive, propertiesFile->getBoolValue(LumatoneSandboxProperty::SingleNoteColourSetActive.toString(), true));
//     setStateProperty(LumatoneSandboxProperty::SingleNoteCCFaderIsDefault, propertiesFile->getBoolValue(LumatoneSandboxProperty::SingleNoteCCFaderIsDefault.toString(), false));
//     setStateProperty(LumatoneSandboxProperty::SingleNoteAutoIncNoteActive, propertiesFile->getBoolValue(LumatoneSandboxProperty::SingleNoteAutoIncNoteActive.toString(), true));
//     setStateProperty(LumatoneSandboxProperty::SingleNoteAutoIncChannelActive, propertiesFile->getBoolValue(LumatoneSandboxProperty::SingleNoteAutoIncChannelActive.toString(), true));
//     setStateProperty(LumatoneSandboxProperty::SingleNoteAutoIncChannelAfterNumNotes, propertiesFile->getIntValue(LumatoneSandboxProperty::SingleNoteAutoIncChannelAfterNumNotes.toString(), 127));

//     setStateProperty(LumatoneSandboxProperty::IsomorphicMassAssign, propertiesFile->getBoolValue(LumatoneSandboxProperty::IsomorphicMassAssign.toString(), false));

//     setStateProperty(LumatoneSandboxProperty::LastSettingsPanel, propertiesFile->getIntValue(LumatoneSandboxProperty::LastSettingsPanel.toString(), 1));
//     setStateProperty(LumatoneSandboxProperty::LastColourWindowTab, propertiesFile->getIntValue(LumatoneSandboxProperty::LastColourWindowTab.toString(), 1));
//     setStateProperty(LumatoneSandboxProperty::LastFirmwareBinPath, propertiesFile->getValue(LumatoneSandboxProperty::LastFirmwareBinPath.toString()
//         , juce::File::getSpecialLocation(juce::File::SpecialLocationType::userHomeDirectory).getFullPathName()
//         ));

//     setStateProperty(LumatoneSandboxProperty::DeveloperModeOn, propertiesFile->getBoolValue(LumatoneSandboxProperty::DeveloperModeOn, false));
// }

void LumatoneSandboxState::Controller::setColourPalettes(const juce::Array<LumatoneEditorColourPalette> &palettesIn)
{
    *sandboxState.colourPalettes = palettesIn;
    // TODO
    // sandboxState.setPropertyExcludingListener(this, LumatoneSandboxProperty::ColourPalettes, "", nullptr);
}

void LumatoneSandboxState::Controller::loadColourPalettesFromFile()
{
    auto directory = sandboxState.getUserPalettesDirectory();
    auto foundPaletteFiles = directory.findChildFiles(juce::File::TypesOfFileToFind::findFiles, true, '*' + juce::String(PALETTEFILEEXTENSION));

    juce::Array<LumatoneEditorColourPalette> newPalettes;

    auto paletteSorter = LumatoneEditorPaletteSorter();
    for (auto file : foundPaletteFiles)
    {
        LumatoneEditorColourPalette palette = LumatoneEditorColourPalette::loadFromFile(file);
        newPalettes.addSorted(paletteSorter, palette);
    }

    setColourPalettes(newPalettes);
}

// void LumatoneSandboxState::Controller::addPalette(const LumatoneEditorColourPalette &newPalette)
// {
//     sandboxState.colourPalettes->add(newPalette);
//     // TODO
//     // sandboxState.setPropertyExcludingListener(this, LumatoneSandboxProperty::ColourPalettes, "", nullptr);
// }

// bool LumatoneSandboxState::Controller::deletePaletteFile(juce::File pathToPalette)
// {
// 	bool success = false;

// 	if (pathToPalette.existsAsFile())
// 	{
// 		success = pathToPalette.deleteFile();
// 	}

// 	return success;
// }

// bool LumatoneSandboxState::Controller::performAction(LumatoneAction *action, bool undoable, bool newTransaction)
// {
//     if (LumatoneApplicationState::Controller::performAction(action, undoable, newTransaction))
//     {
//         // setHasChangesToSave(true);
//         return true;
//     }

//     return false;
// }

// Open a SysEx mapping from the file specified in currentFile
bool LumatoneSandboxState::Controller::resetToCurrentFile()
{
    getEditorListeners()->call(&LumatoneEditor::EditorListener::newFileLoaded, sandboxState.getCurrentFile());

    if (sandboxState.getCurrentFile().getFullPathName().isEmpty())
    {
        // Replace with blank file
		LumatoneLayout defaultLayout;
        sandboxState.setCompleteConfig(defaultLayout);
        return true;
    }

    if (sandboxState.getCurrentFile().existsAsFile())
	{
		// XXX StringArray format: platform-independent?
		juce::StringArray stringArray;
		sandboxState.getCurrentFile().readLines(stringArray);
		LumatoneLayout keyMapping(stringArray);

		// Send configuration to controller, if connected
        sandboxState.setCompleteConfig(keyMapping);

		// Add file to recent files list
		sandboxState.recentFiles->addFile(sandboxState.currentFile);

		return true;
	}

	// Show error message
	juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::AlertIconType::WarningIcon, "Open File Error", "The file " + sandboxState.getCurrentFile().getFullPathName() + " could not be opened.");

	// XXX Update Window title in any case? Make file name empty/make data empty in case of error?
	return false;
}

bool LumatoneSandboxState::Controller::setCurrentFile(juce::File fileToOpen)
{
    sandboxState.currentFile = fileToOpen;
    sandboxState.setStateProperty(LumatoneSandboxProperty::CurrentFile, sandboxState.currentFile.getFullPathName());
    return resetToCurrentFile();
}

// open a file from the "recent files" menu
bool LumatoneSandboxState::Controller::openRecentFile(int recentFileIndex)
{
    jassert(recentFileIndex >= 0 && recentFileIndex < sandboxState.recentFiles->getNumFiles());
    return setCurrentFile(sandboxState.recentFiles->getFile(recentFileIndex));
}

bool LumatoneSandboxState::Controller::requestCompleteConfigFromDevice()
{
    // setHasChangesToSave(false);
    sandboxState.undoManager->clearUndoHistory();
    return LumatoneApplicationState::Controller::requestCompleteConfigFromDevice();
}

bool LumatoneSandboxState::Controller::saveMappingToFile(juce::File fileToSave)
{
    juce::StringArray stringArray = sandboxState.getMappingData()->toStringArray();
    juce::String fileText = stringArray.joinIntoString("\n");

    bool success = false;

    if (fileToSave.existsAsFile())
        success = fileToSave.replaceWithText(fileText, false, false);

    else if (fileToSave.create().ok())
    {
        success = fileToSave.appendText(fileText, false, false);
    }

    if (success && sandboxState.getCurrentFile() != fileToSave)
    {
        // TODO skip certain updates?
        setCurrentFile(fileToSave);
    }

    return success;
}

void LumatoneSandboxState::Controller::loadStateProperties(juce::ValueTree stateIn)
{
    sandboxState.loadStateProperties(stateIn);
}

bool LumatoneSandboxState::Controller::savePropertiesFile() const
{
    // TODO Save documents directories (Future: provide option to change them and save after changed by user)
    //propertiesFile->setValue(LumatoneSandboxProperty::UserDocumentsDirectory, getUserDocumentsDirectory().getFullPathName());
    //propertiesFile->setValue(LumatoneSandboxProperty::UserMappingsDirectory, getUserMappingsDirectory().getFullPathName());
    //propertiesFile->setValue(LumatoneSandboxProperty::UserPalettesDirectory, getUserPalettesDirectory().getFullPathName());

    // Save recent files list
    sandboxState.recentFiles->removeNonExistentFiles();
    jassert(sandboxState.propertiesFile != nullptr);
    sandboxState.propertiesFile->setValue(LumatoneSandboxProperty::RecentFiles, sandboxState.recentFiles->toString());

    return sandboxState.propertiesFile->saveIfNeeded();
}

// void LumatoneSandboxState::Controller::savePropertyBoolValue(const juce::Identifier &id, bool value)
// {
//     sandboxState.propertiesFile->setValue(id.toString(), juce::var(value));
//     savePropertiesFile();
//     sandboxState.setStateProperty(id, juce::var(value));
// }

// void LumatoneSandboxState::Controller::savePropertyIntValue(const juce::Identifier &id, int value)
// {
//     sandboxState.propertiesFile->setValue(id.toString(), juce::var(value));
//     savePropertiesFile();
//     sandboxState.setStateProperty(id, juce::var(value));
// }

// void LumatoneSandboxState::Controller::savePropertyStringValue(const juce::Identifier &id, juce::String value)
// {
//     sandboxState.propertiesFile->setValue(id.toString(), juce::var(value));
//     savePropertiesFile();
//     sandboxState.setStateProperty(id, juce::var(value));
// }
