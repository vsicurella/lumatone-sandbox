/*
  ==============================================================================

    SandboxMenu.h
    Created: 29 Jul 2023 5:25:47pm
    Author:  Vincenzo Sicurella

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

namespace LumatoneSandbox {
	namespace Menu {
		enum commandIDs {
			openSysExMapping = 0x200010,
			saveSysExMapping = 0x200011,
			saveSysExMappingAs = 0x200012,
			resetSysExMapping = 0x200013,

			deleteOctaveBoard = 0x200100,
			copyOctaveBoard = 0x200101,
			pasteOctaveBoard = 0x200102,
            pasteOctaveBoardNotes = 0x200103,
            pasteOctaveBoardChannels = 0x200104,
            pasteOctaveBoardColours = 0x200105,
            pasteOctaveBoardTypes = 0x200106,
            
			undo = 0x200200,
			redo = 0x200201,

			recentFilesBaseID = 0x200300,

			aboutSysEx = 0x200400,

			openRandomColorsGame	= 0x300100,
			openHexRingsGame		= 0x300200,
		};

		class Model : public juce::MenuBarModel
		{
		public:
			Model(juce::ApplicationCommandManager* commandManager);

			virtual juce::StringArray getMenuBarNames();
			juce::PopupMenu getMenuForIndex(int topLevelMenuIndex, const juce::String& menuName);
			void menuItemSelected(int menuItemID, int topLevelMenuIndex);

			virtual void createFileMenu(juce::PopupMenu& menu);
			virtual void createEditMenu(juce::PopupMenu& menu);
			virtual void createGameMenu(juce::PopupMenu& menu);
			virtual void createHelpMenu(juce::PopupMenu& menu);

		private:
			juce::ApplicationCommandManager* theManager;
		};
	}
}
