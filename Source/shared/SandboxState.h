/*
  ==============================================================================

    SandboxState.h
    Created: 25 Jan 2024 11:57pm
    Author:  Vincenzo

  ==============================================================================
*/

#ifndef LUMATONE_SANDBOX_STATE_H
#define LUMATONE_SANDBOX_STATE_H

#include "./lumatone_editor_library/data/application_state.h"

#define CHOOSE_FILE_NOOP [](bool) -> void {}

class LumatoneController;
class LumatoneSandboxGameEngine;
class LumatoneEditorColourPalette;

namespace LumatoneSandboxProperty
{
    static const juce::Identifier StateTree = juce::Identifier("LumatoneSandboxState");
\
    static const juce::Identifier ColourPalettes = juce::Identifier("ColourPalettes");
    
    static const juce::Identifier CurrentFile = juce::Identifier("CurrentFile");
    static const juce::Identifier RecentFiles = juce::Identifier("RecentFiles");

    static const juce::Identifier AutoConnectDevice = juce::Identifier("AutoConnectDevice");
    static const juce::Identifier CheckDeviceActivity = juce::Identifier("CheckDeviceActivity");

    static const juce::Identifier UserDocumentsDirectory = juce::Identifier("UserDocumentsDirectory");
    static const juce::Identifier UserMappingsDirectory = juce::Identifier("UserMappingsDirectory");
    static const juce::Identifier UserPalettesDirectory = juce::Identifier("UserPalettesDirectory");

    static const juce::Identifier MainWindowState = juce::Identifier("MainWindowState");
}

static juce::Array<juce::Identifier> GetLumatoneSandboxProperties();

class LumatoneSandboxState : public LumatoneApplicationState
{
public:
    LumatoneSandboxState(LumatoneFirmwareDriver& driverIn, juce::UndoManager* undoManagerIn);
    LumatoneSandboxState(juce::String name, const LumatoneSandboxState& stateIn);

    ~LumatoneSandboxState() override;

    // const juce::String getApplicationName() const { return ProjectInfo::projectName; }
	// const juce::String getApplicationVersion() const { return ProjectInfo::versionString; }

    juce::String getProperty(juce::Identifier propertyId, juce::String fallbackValue=juce::String()) const;

    juce::File getCurrentFile() const { return currentFile; }

    juce::RecentlyOpenedFilesList& getRecentFiles();

    juce::File getUserDocumentsDirectory() const;
    juce::File getUserMappingsDirectory() const;
    juce::File getUserPalettesDirectory() const;

    juce::File getLastOpenedMappingsDirectory() const;

    virtual const juce::Array<LumatoneEditorColourPalette>& getColourPalettes();

protected:
    juce::ValueTree loadStateProperties(juce::ValueTree stateIn) override;
    void handleStatePropertyChange(juce::ValueTree stateIn, const juce::Identifier& property) override;

    // void loadPropertiesFile(juce::PropertiesFile* properties);

    // LumatonePaletteLibrary*         getPaletteLibrary() { return paletteLibrary.get(); }
    LumatoneSandboxGameEngine*          getGameEngine();
    
private:
    std::shared_ptr<LumatoneSandboxGameEngine>      gameEngine;

	std::shared_ptr<juce::Array<LumatoneEditorColourPalette>>   colourPalettes;

	juce::File                                      currentFile;
	std::shared_ptr<juce::RecentlyOpenedFilesList>	recentFiles;

    std::shared_ptr<juce::PropertiesFile>           propertiesFile;

//================================================================================
public:
    class Controller : protected LumatoneApplicationState::Controller
    {
    LumatoneSandboxState& sandboxState;
    
    public:
        Controller(LumatoneSandboxState& stateIn)
            : LumatoneApplicationState::Controller(stateIn)
            , sandboxState(stateIn) {}

        juce::ValueTree getState() { return sandboxState.state; }

        bool performAction(LumatoneAction* action, bool undoable=true, bool newTransaction=true) override;

        bool resetToCurrentFile();
        bool openRecentFile(int recentFileIndex);

        virtual bool requestCompleteConfigFromDevice() override;

        void setColourPalettes(const juce::Array<LumatoneEditorColourPalette>& palettesIn);
        void loadColourPalettesFromFile();

        bool setCurrentFile(juce::File fileToOpen);
        bool saveMappingToFile(juce::File fileToSave);

        void loadStateProperties(juce::ValueTree stateIn);

        juce::PropertiesFile* getPropertiesFile() const { return sandboxState.propertiesFile.get(); }
        bool savePropertiesFile() const;

        // Sets a property in the juce::PropertyFile and saves it to file
        // void savePropertyBoolValue(const juce::Identifier& id, bool value);
        // void savePropertyIntValue(const  juce::Identifier& id, int value);
        // void savePropertyStringValue(const  juce::Identifier& id, juce::String value);
    };
    
private:
    friend class LumatoneSandboxStateController;
};


#endif // LUMATONE_SANDBOX_STATE_H
