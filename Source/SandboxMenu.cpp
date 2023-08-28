/*
  ==============================================================================

    SandboxMenu.cpp
    Created: 29 Jul 2023 5:25:47pm
    Author:  Vincenzo Sicurella

  ==============================================================================
*/

#include "SandboxMenu.h"

LumatoneSandbox::Menu::Model::Model(juce::ApplicationCommandManager* commandManager)
{
  theManager = commandManager;
  setApplicationCommandManagerToWatch(commandManager);

}

juce::StringArray LumatoneSandbox::Menu::Model::getMenuBarNames()
{
  const char* const names[] = { "File", "Edit", "View", "Game", "Help", nullptr };
  return juce::StringArray(names);
}

void LumatoneSandbox::Menu::Model::createFileMenu(juce::PopupMenu& menu)
{
  menu.addCommandItem(theManager, openSysExMapping);
  menu.addCommandItem(theManager, saveSysExMapping);
  menu.addCommandItem(theManager, saveSysExMappingAs);
  menu.addCommandItem(theManager, resetSysExMapping);

  menu.addSeparator();

  juce::PopupMenu recentFilesMenu;
  // TerpstraSysExApplication::getApp().getRecentFileList().createPopupMenuItems(recentFilesMenu, recentFilesBaseID, true, true);
  menu.addSubMenu("Recent Files", recentFilesMenu);

#if ! JUCE_MAC
  menu.addSeparator();
  menu.addCommandItem(theManager, juce::StandardApplicationCommandIDs::quit);
#endif
}

void LumatoneSandbox::Menu::Model::createEditMenu(juce::PopupMenu& menu)
{
  menu.addCommandItem(theManager, deleteOctaveBoard);
  menu.addCommandItem(theManager, copyOctaveBoard);
  menu.addCommandItem(theManager, pasteOctaveBoard);
  menu.addCommandItem(theManager, pasteOctaveBoardChannels);
  menu.addCommandItem(theManager, pasteOctaveBoardNotes);
  menu.addCommandItem(theManager, pasteOctaveBoardColours);
  menu.addCommandItem(theManager, pasteOctaveBoardTypes);

  menu.addCommandItem(theManager, adjustColour);

  menu.addCommandItem(theManager, undo);
  menu.addCommandItem(theManager, redo);
}

void LumatoneSandbox::Menu::Model::createViewMenu(juce::PopupMenu& menu)
{
  juce::PopupMenu renderMenu;
  renderMenu.addCommandItem(theManager, setRenderModeKeys);
  renderMenu.addCommandItem(theManager, setRenderModeMaxRes);
  menu.addSubMenu("Render Mode", renderMenu, true);
}

void LumatoneSandbox::Menu::Model::createGameMenu(juce::PopupMenu& menu)
{
    menu.addCommandItem(theManager, openRandomColorsGame);
    menu.addCommandItem(theManager, openHexRingsGame);
    menu.addCommandItem(theManager, openHexagonAutomata);
}

void LumatoneSandbox::Menu::Model::createHelpMenu(juce::PopupMenu& menu)
{
  menu.addCommandItem(theManager, aboutSysEx);
}

juce::PopupMenu LumatoneSandbox::Menu::Model::getMenuForIndex(int topLevelMenuIndex, const juce::String& menuName)
{
  juce::PopupMenu menu;

  if (menuName == "File")           createFileMenu(menu);
  else if (menuName == "Edit")		createEditMenu(menu);
  else if (menuName == "View")		createViewMenu(menu);
  else if (menuName == "Game")      createGameMenu(menu);
  else if (menuName == "Help")		createHelpMenu(menu);
  else                                jassertfalse; // names have changed?

  return menu;
}

void LumatoneSandbox::Menu::Model::menuItemSelected(int menuItemID, int topLevelMenuIndex)
{
  if (menuItemID >= recentFilesBaseID && menuItemID < recentFilesBaseID + 100)
  {
    // open a file from the "recent files" menu
    // TerpstraSysExApplication::getApp().openRecentFile(menuItemID - recentFilesBaseID);
  }
}
