#pragma once

#include <JuceHeader.h>

namespace LumatoneAssets
{
    enum class ID
    {
        LoadIcon        = 0x0002000,
        SaveIcon        = 0x0002001,
        ImportIcon      = 0x0002002,

        LumatoneGraphic = 0x0002100,
        KeybedShadows   = 0x0002110,
        KeyShape        = 0x0002120,
        KeyShadow       = 0x0002130,

        TickBox         = 0x0003100,

        SavePalette     = 0x0005000,
        CancelPalette   = 0x0005001,
        TrashCanIcon    = 0x0005002,
        CloneIcon       = 0x0005003
    };


    // Hash codes for use with ImageCache::getFromHashCode()
    enum class HashCodes
    {
        LumatoneGraphic_1x = 0x0002100,
        LumatoneGraphic_2x = 0x0002101,
        LumatoneGraphic_4x = 0x0002102,
        LumatoneGraphic_8x = 0x0002103,

        KeybedShadows_1x = 0x0002110,
        KeybedShadows_2x = 0x0002111,
        KeybedShadows_4x = 0x0002112,
        KeybedShadows_8x = 0x0002112,

        KeyShape_1x = 0x0002120,
        KeyShape_2x = 0x0002121,
        KeyShape_4x = 0x0002122,

        KeyShadow_1x = 0x0002230,
        KeyShadow_2x = 0x0002231,
        KeyShadow_4x = 0x0002232,
    };

    static void LoadAssets(LumatoneAssets::ID assetId)
    {
        switch (assetId)
        {
        case LumatoneAssets::ID::LumatoneGraphic:
            juce::ImageCache::addImageToCache(juce::ImageCache::getFromMemory(BinaryData::KeyboardBase_png,   BinaryData::KeyboardBase_pngSize),   (juce::int64)LumatoneAssets::HashCodes::LumatoneGraphic_1x);
            juce::ImageCache::addImageToCache(juce::ImageCache::getFromMemory(BinaryData::KeyboardBase2x_png, BinaryData::KeyboardBase2x_pngSize), (juce::int64)LumatoneAssets::HashCodes::LumatoneGraphic_2x);
            juce::ImageCache::addImageToCache(juce::ImageCache::getFromMemory(BinaryData::KeyboardBase4x_png, BinaryData::KeyboardBase4x_pngSize), (juce::int64)LumatoneAssets::HashCodes::LumatoneGraphic_4x);
            juce::ImageCache::addImageToCache(juce::ImageCache::getFromMemory(BinaryData::KeyboardBase8x_png, BinaryData::KeyboardBase8x_pngSize), (juce::int64)LumatoneAssets::HashCodes::LumatoneGraphic_8x);
            break;

        // case LumatoneAssets::ID::KeybedShadows:
        //     juce::ImageCache::addImageToCache(juce::ImageCache::getFromMemory(BinaryData::KeybedShadows_png,   BinaryData::KeybedShadows_pngSize),   (juce::int64)LumatoneAssets::HashCodes::KeybedShadows_1x);
        //     juce::ImageCache::addImageToCache(juce::ImageCache::getFromMemory(BinaryData::KeybedShadows2x_png, BinaryData::KeybedShadows2x_pngSize), (juce::int64)LumatoneAssets::HashCodes::KeybedShadows_2x);
        //     juce::ImageCache::addImageToCache(juce::ImageCache::getFromMemory(BinaryData::KeybedShadows4x_png, BinaryData::KeybedShadows4x_pngSize), (juce::int64)LumatoneAssets::HashCodes::KeybedShadows_4x);
        //     juce::ImageCache::addImageToCache(juce::ImageCache::getFromMemory(BinaryData::KeybedShadows8x_png, BinaryData::KeybedShadows8x_pngSize), (juce::int64)LumatoneAssets::HashCodes::KeybedShadows_8x);
        //     break;

        case LumatoneAssets::ID::KeyShape:
            juce::ImageCache::addImageToCache(juce::ImageCache::getFromMemory(BinaryData::KeyShape_png,   BinaryData::KeyShape_pngSize),   (juce::int64)LumatoneAssets::HashCodes::KeyShape_1x);
            juce::ImageCache::addImageToCache(juce::ImageCache::getFromMemory(BinaryData::KeyShape2x_png, BinaryData::KeyShape2x_pngSize), (juce::int64)LumatoneAssets::HashCodes::KeyShape_2x);
            juce::ImageCache::addImageToCache(juce::ImageCache::getFromMemory(BinaryData::KeyShape4x_png, BinaryData::KeyShape4x_pngSize), (juce::int64)LumatoneAssets::HashCodes::KeyShape_4x);
            break;

        case LumatoneAssets::ID::KeyShadow:
            {
            juce::ImageCache::addImageToCache(juce::ImageCache::getFromMemory(BinaryData::KeyShadowInv_png, BinaryData::KeyShadowInv_pngSize), (juce::int64)LumatoneAssets::HashCodes::KeyShadow_1x);
            juce::ImageCache::addImageToCache(juce::ImageCache::getFromMemory(BinaryData::KeyShadowInv2x_png, BinaryData::KeyShadowInv2x_pngSize), (juce::int64)LumatoneAssets::HashCodes::KeyShadow_2x);
            juce::ImageCache::addImageToCache(juce::ImageCache::getFromMemory(BinaryData::KeyShadowInv4x_png, BinaryData::KeyShadowInv4x_pngSize), (juce::int64)LumatoneAssets::HashCodes::KeyShadow_4x);
            }
            break;

        default:
            break;
        }
    }

    static juce::Image getImage(LumatoneAssets::ID assetId, int height = 0, int width = 0)
    {
        switch (assetId)
        {
        case LumatoneAssets::ID::LumatoneGraphic:
            if (height <= 212 || width <= 530)
                return juce::ImageCache::getFromHashCode((juce::int64)LumatoneAssets::HashCodes::LumatoneGraphic_1x);
            else if (height <= 424 || width <= 1060)
                return juce::ImageCache::getFromHashCode((juce::int64)LumatoneAssets::HashCodes::LumatoneGraphic_2x);
            else if (height <= 849 || width <= 2121)
                return juce::ImageCache::getFromHashCode((juce::int64)LumatoneAssets::HashCodes::LumatoneGraphic_4x);
            else
                return juce::ImageCache::getFromHashCode((juce::int64)LumatoneAssets::HashCodes::LumatoneGraphic_8x);

        case LumatoneAssets::ID::KeybedShadows:
            if (height <= 212 || width <= 530)
                return juce::ImageCache::getFromHashCode((juce::int64)LumatoneAssets::HashCodes::KeybedShadows_1x);
            else if (height <= 424 || width <= 1060)
                return juce::ImageCache::getFromHashCode((juce::int64)LumatoneAssets::HashCodes::KeybedShadows_2x);
            else if (height <= 849 || width <= 2121)
                return juce::ImageCache::getFromHashCode((juce::int64)LumatoneAssets::HashCodes::KeybedShadows_4x);
            else
                return juce::ImageCache::getFromHashCode((juce::int64)LumatoneAssets::HashCodes::KeybedShadows_8x);

        case LumatoneAssets::ID::KeyShape:
            if (height <= 62 || width <= 58)
                return juce::ImageCache::getFromHashCode((juce::int64)LumatoneAssets::HashCodes::KeyShape_1x);
            else if (height <= 124 || width <= 116)
                return juce::ImageCache::getFromHashCode((juce::int64)LumatoneAssets::HashCodes::KeyShape_2x);
            else
                return juce::ImageCache::getFromHashCode((juce::int64)LumatoneAssets::HashCodes::KeyShape_4x);

        case LumatoneAssets::ID::KeyShadow:
            if (height <= 62 || width <= 58)
                return juce::ImageCache::getFromHashCode((juce::int64)LumatoneAssets::HashCodes::KeyShadow_1x);
            else if (height <= 124 || width <= 116)
                return juce::ImageCache::getFromHashCode((juce::int64)LumatoneAssets::HashCodes::KeyShadow_2x);
            else
                return juce::ImageCache::getFromHashCode((juce::int64)LumatoneAssets::HashCodes::KeyShadow_4x);

        default:
            return juce::ImageCache::getFromHashCode((juce::int64)assetId);
        }
    }
}
