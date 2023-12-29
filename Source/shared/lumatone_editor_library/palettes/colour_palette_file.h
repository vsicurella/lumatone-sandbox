/*
  ==============================================================================

    colour_palette_file.h
    Created: 17 Jan 2021 9:55:13pm
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

namespace LumatoneEditorPaletteIds
{
    static juce::Identifier libraryId         = juce::Identifier("PaletteLibrary"); // Currently unused
    static juce::Identifier paletteId         = juce::Identifier("ColourPalette");
    static juce::Identifier swatchId          = juce::Identifier("Swatch");
    static juce::Identifier colourId          = juce::Identifier("Colour");

    static juce::Identifier nameId            = juce::Identifier("Name");
    static juce::Identifier dateCreatedId     = juce::Identifier("DateCreated");
    static juce::Identifier authorId          = juce::Identifier("Author");
    static juce::Identifier notesId           = juce::Identifier("Notes");
}

#define PALETTEFILEEXTENSION ".ltp"
#define DEFAULTPALETTESIZE 10

//==============================================================================
/*
* Container for colour palette data
*/
class LumatoneEditorColourPalette
{
public:
    LumatoneEditorColourPalette(juce::String paletteName="")
    {
        colourPalette.resize(DEFAULTPALETTESIZE);
        colourPalette.fill(juce::Colour(0xff1b1b1b));
        name = paletteName;
        dateCreated = juce::Time::getCurrentTime().toISO8601(false);
    }

    LumatoneEditorColourPalette(juce::Array<juce::Colour> colourPaletteIn, juce::String paletteName="")
        : LumatoneEditorColourPalette(paletteName)
    {
        setColours(colourPaletteIn);
    }

    LumatoneEditorColourPalette(juce::Array<juce::Colour> colourPaletteIn, juce::String paletteName, juce::String paletteAuthor, juce::String paletteNotes="")
        : LumatoneEditorColourPalette(colourPaletteIn, paletteName)
    {
        author = paletteAuthor;
        notes = paletteNotes;
    }
    
    LumatoneEditorColourPalette clone() const { return LumatoneEditorColourPalette(colourPalette, name, author, notes); }

    int size() const { return colourPalette.size(); }

    juce::Array<juce::Colour>* getColours() { return &colourPalette; }

    juce::String getName() const { return name; }

    juce::String getAuthor() const { return author; }

    juce::String getNotes() const { return notes; }

    bool hasBeenModified() const { return modifiedSinceLastSave; }

    /// <summary>
    /// The timestamp from when this was first created
    /// </summary>
    /// <returns>String created from Time::toISO8601()</returns>
    juce::String getDateCreated() const { return dateCreated; }

    juce::String getPathToFile() const { return pathToFile; }

    void setColours(const juce::Array<juce::Colour>& newColourPalette)
    {
        setModifiedIfChanged(colourPalette, newColourPalette);

        colourPalette.clear();
        colourPalette.addArray(newColourPalette);
    }

    void setName(juce::String nameIn)     { setModifiedIfChanged(name, nameIn); name = nameIn; }

    void setAuthor(juce::String authorIn) { setModifiedIfChanged(author, authorIn); author = authorIn; }

    void setNotes(juce::String notesIn)   { setModifiedIfChanged(notes, notesIn); notes = notesIn; }

    /// <summary>
    /// Save palette to a .ltp file (and update internal file path)
    /// Warning: deletes old version regardless if file path is not different from the previous version
    /// </summary>
    /// <param name="fileToSaveTo"></param>
    /// <param name="deleteOldVersion"></param>
    bool saveToFile(juce::File fileToSaveTo, bool deleteOldVersion = true)
    {
        // Make sure it has proper extension
        fileToSaveTo = fileToSaveTo.withFileExtension(PALETTEFILEEXTENSION);

        if (fileToSaveTo.getFullPathName() == pathToFile)
            deleteOldVersion = true;

        if (deleteOldVersion)
        {
            juce::File originalFile = pathToFile;

            if (originalFile.existsAsFile())
                originalFile.deleteFile();
        }

        if (!fileToSaveTo.existsAsFile())
            fileToSaveTo.create();

        if (fileToSaveTo.replaceWithText(toValueTree().toXmlString()))
        {
            modifiedSinceLastSave = false;
            pathToFile = fileToSaveTo.getFullPathName();

            return true;
        }

        return false;
    }
    bool saveToFile()
    {
        return saveToFile(juce::File(pathToFile));
    }

    juce::ValueTree toValueTree() const
    {
        juce::ValueTree node(LumatoneEditorPaletteIds::paletteId);
        node.setProperty(LumatoneEditorPaletteIds::nameId, name, nullptr);
        node.setProperty(LumatoneEditorPaletteIds::authorId, author, nullptr);
        node.setProperty(LumatoneEditorPaletteIds::dateCreatedId, dateCreated, nullptr);
        node.setProperty(LumatoneEditorPaletteIds::notesId, notes, nullptr);

        for (auto c : colourPalette)
        {
            juce::ValueTree swatch(LumatoneEditorPaletteIds::swatchId);
            swatch.setProperty(LumatoneEditorPaletteIds::colourId, c.toString(), nullptr);
            node.appendChild(swatch, nullptr);
        }

        return node;
    }

    juce::String toString() const { return toValueTree().toXmlString(); }

    //==============================================================================
    // Palette static methods

    static LumatoneEditorColourPalette loadFromValueTree(juce::ValueTree paletteNode)
    {
        LumatoneEditorColourPalette loadedPalette;

        if (paletteNode.hasType(LumatoneEditorPaletteIds::paletteId))
        {
            juce::Array<juce::Colour> palette;

            for (auto swatch : paletteNode)
            {
                if (swatch.hasType(LumatoneEditorPaletteIds::swatchId))
                    palette.add(juce::Colour::fromString(swatch[LumatoneEditorPaletteIds::colourId].toString()));
            }

            loadedPalette = LumatoneEditorColourPalette(palette,
                paletteNode[LumatoneEditorPaletteIds::nameId],
                paletteNode[LumatoneEditorPaletteIds::authorId],
                paletteNode[LumatoneEditorPaletteIds::notesId]
            );

            if (paletteNode[LumatoneEditorPaletteIds::dateCreatedId].toString() != juce::String())
                loadedPalette.dateCreated = paletteNode[LumatoneEditorPaletteIds::dateCreatedId];

            loadedPalette.modifiedSinceLastSave = false;
        }

        return loadedPalette;
    }

    static LumatoneEditorColourPalette loadFromFile(const juce::File& pathToPalette)
    {
        LumatoneEditorColourPalette loadedPalette;

        if (pathToPalette.existsAsFile())
        {
            loadedPalette = loadFromValueTree(juce::ValueTree::fromXml(pathToPalette.loadFileAsString()));

            // Set empty palette name to file name if it does not appear to be a placeholder
            if (loadedPalette.name == juce::String() && !pathToPalette.getFileNameWithoutExtension().contains("UnnamedPalette"))
            {
                loadedPalette.name = pathToPalette.getFileNameWithoutExtension();
            }
        }

        loadedPalette.pathToFile = pathToPalette.getFullPathName();

        return loadedPalette;
    }

private:

    template <class T>
    void setModifiedIfChanged(const T& originalValue, const T& newValue) { if (originalValue != newValue) modifiedSinceLastSave = true; }

private:
    
    juce::Array<juce::Colour> colourPalette;
    
    juce::String name = "";
    juce::String author = "";
    juce::String notes = "";

    juce::String dateCreated = "";

    // Internal use only
    juce::String pathToFile = "";
    bool modifiedSinceLastSave = true;
};

class LumatoneEditorPaletteSorter : juce::DefaultElementComparator<const LumatoneEditorColourPalette&>
{
public:
    static int compareElements(const LumatoneEditorColourPalette& first, const LumatoneEditorColourPalette& second)
    {
        // Reverse chronological sort
        juce::Time t0 = juce::Time::fromISO8601(first.getDateCreated());
        juce::Time t1 = juce::Time::fromISO8601(second.getDateCreated());
        return (t1 < t0) ? -1 : ((t0 < t1) ? 1 : 0);
    }
};
