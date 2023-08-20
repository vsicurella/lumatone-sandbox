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
        KeybedShadows   = 0x0002101,
        KeyShape        = 0x0002102,
        KeyShadow       = 0x0002103,

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
        KeyShape_8x = 0x0002123,

        KeyShadow_1x = 0x0002130,
        KeyShadow_2x = 0x0002131,
        KeyShadow_4x = 0x0002132,
        KeyShadow_8x = 0x0002133,
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
            juce::ImageCache::addImageToCache(juce::ImageCache::getFromMemory(BinaryData::KeyShape8x_png, BinaryData::KeyShape8x_pngSize), (juce::int64)LumatoneAssets::HashCodes::KeyShape_8x);
            break;

        case LumatoneAssets::ID::KeyShadow:
            {
            juce::ImageCache::addImageToCache(juce::ImageCache::getFromMemory(BinaryData::KeyShadow_png, BinaryData::KeyShadow_pngSize), (juce::int64)LumatoneAssets::HashCodes::KeyShadow_1x);
            juce::ImageCache::addImageToCache(juce::ImageCache::getFromMemory(BinaryData::KeyShadow2x_png, BinaryData::KeyShadow2x_pngSize), (juce::int64)LumatoneAssets::HashCodes::KeyShadow_2x);
            juce::ImageCache::addImageToCache(juce::ImageCache::getFromMemory(BinaryData::KeyShadow4x_png, BinaryData::KeyShadow4x_pngSize), (juce::int64)LumatoneAssets::HashCodes::KeyShadow_4x);
            juce::ImageCache::addImageToCache(juce::ImageCache::getFromMemory(BinaryData::KeyShadow8x_png, BinaryData::KeyShadow8x_pngSize), (juce::int64)LumatoneAssets::HashCodes::KeyShadow_8x);
            }
            break;

        default:
            break;
        }
    }

    enum class LumatoneGraphicRenderSize
    {
        _1x =   0x00,
        _2x =   0x01,
        _4x =   0x02,
        _8x =   0x03
    };

    static int LumatoneKeyboardRenderWidth(LumatoneAssets::LumatoneGraphicRenderSize size)
    {
        switch (size)
        {
            default:
            case LumatoneGraphicRenderSize::_1x:
                return 530;
            case LumatoneGraphicRenderSize::_2x:
                return 1060;
            case LumatoneGraphicRenderSize::_4x:
                return 2121;
            case LumatoneGraphicRenderSize::_8x:
                return 4241;
        }
    }

    static int LumatoneKeyboardRenderHeight(LumatoneAssets::LumatoneGraphicRenderSize size)
    {
        switch (size)
        {
            default:
            case LumatoneGraphicRenderSize::_1x:
                return 212;
            case LumatoneGraphicRenderSize::_2x:
                return 424;
            case LumatoneGraphicRenderSize::_4x:
                return 849;
            case LumatoneGraphicRenderSize::_8x:
                return 1697;
        }
    }

    static LumatoneAssets::LumatoneGraphicRenderSize GetLumatoneRenderSize(int targetWidth, int targetHeight)
    {
        if (targetHeight <= 212 && targetWidth <= 530)
            return LumatoneAssets::LumatoneGraphicRenderSize::_1x;
        else if (targetHeight <= 424 && targetWidth <= 1060)
            return LumatoneAssets::LumatoneGraphicRenderSize::_2x;
        else if (targetHeight <= 849 && targetWidth <= 2121)
            return LumatoneAssets::LumatoneGraphicRenderSize::_4x;
        else
            return LumatoneAssets::LumatoneGraphicRenderSize::_8x;
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
            if (height <= 15 || width <= 14)
                return juce::ImageCache::getFromHashCode((juce::int64)LumatoneAssets::HashCodes::KeyShape_1x);
            if (height <= 31 || width <= 29)
                return juce::ImageCache::getFromHashCode((juce::int64)LumatoneAssets::HashCodes::KeyShape_2x);

            if (height <= 62 || width <= 58)
                return juce::ImageCache::getFromHashCode((juce::int64)LumatoneAssets::HashCodes::KeyShape_4x);
            // else if (height <= 124 || width <= 116)
            else
                return juce::ImageCache::getFromHashCode((juce::int64)LumatoneAssets::HashCodes::KeyShape_8x);

        case LumatoneAssets::ID::KeyShadow:
            if (height <= 15 || width <= 14)
                return juce::ImageCache::getFromHashCode((juce::int64)LumatoneAssets::HashCodes::KeyShadow_1x);
            if (height <= 31 || width <= 29)
                return juce::ImageCache::getFromHashCode((juce::int64)LumatoneAssets::HashCodes::KeyShadow_2x);
            if (height <= 62 || width <= 58)
                return juce::ImageCache::getFromHashCode((juce::int64)LumatoneAssets::HashCodes::KeyShadow_4x);
            // else if (height <= 124 || width <= 116)
            else
                return juce::ImageCache::getFromHashCode((juce::int64)LumatoneAssets::HashCodes::KeyShadow_8x);

        default:
            return juce::ImageCache::getFromHashCode((juce::int64)assetId);
        }
    }
}
