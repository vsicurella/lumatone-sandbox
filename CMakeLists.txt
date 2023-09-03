cmake_minimum_required(VERSION 3.27)

project(LUMATONE_SANDBOX VERSION 0.0.1)

# find_package(juce)
add_subdirectory($ENV{juce_DIR} subprojects/juce)

# juce_add_gui_app(LumatoneSandbox PRODUCT_NAME "Lumatone Sandbox App")
juce_add_plugin(LumatoneSandbox
    # VERSION                               # Set this if the plugin version is different to the project version
    # ICON_BIG ...                              # ICON_* arguments specify a path to an image file to use as an icon for the Standalone
    # ICON_SMALL ...
    COMPANY_NAME Lumatone                          # Specify the name of the plugin's author
    # IS_SYNTH TRUE/FALSE                       # Is this a synth or an effect?
    NEEDS_MIDI_INPUT TRUE               # Does the plugin need midi input?
    NEEDS_MIDI_OUTPUT TRUE              # Does the plugin need midi output?
    # IS_MIDI_EFFECT TRUE/FALSE                 # Is this plugin a MIDI effect?
    # EDITOR_WANTS_KEYBOARD_FOCUS TRUE/FALSE    # Does the editor need keyboard focus?
    # COPY_PLUGIN_AFTER_BUILD TRUE/FALSE        # Should the plugin be installed to a default location after building?
    PLUGIN_MANUFACTURER_CODE Luma               # A four-character manufacturer id with at least one upper-case character
    PLUGIN_CODE Lmsb                            # A unique four-character plugin id with exactly one upper-case character
                                                # GarageBand 10.3 requires the first letter to be upper-case, and the remaining letters to be lower-case
    FORMATS AU VST3 Standalone                 # The formats to build. Other valid formats are: AAX Unity VST AU AUv3
    PRODUCT_NAME "Lumatone Sandbox"        # The name of the final executable, which can differ from the target name 
)

juce_generate_juce_header(LumatoneSandbox)

file(GLOB_RECURSE SharedSourceCode 
    CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/Source/shared/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Source/shared/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/Source/plugin/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Source/plugin/*.h"
    )
target_sources(LumatoneSandbox PRIVATE ${SharedSourceCode})

# file(GLOB_RECURSE StandaloneSourceCode 
#     CONFIGURE_DEPENDS
#         "${CMAKE_CURRENT_SOURCE_DIR}/Source/standalone/*.cpp"
#         "${CMAKE_CURRENT_SOURCE_DIR}/Source/standalone/*.h"
#     )
# target_sources(LumatoneSandbox_Standalone
#      PRIVATE 
#         ${SharedSourceCode}
#         ${StandaloneSourceCode}
#     )

# file(GLOB_RECURSE PluginSourceCode 
#     CONFIGURE_DEPENDS
#         "${CMAKE_CURRENT_SOURCE_DIR}/Source/plugin/*.cpp"
#         "${CMAKE_CURRENT_SOURCE_DIR}/Source/plugin/*.h"
#     )
# target_sources(LumatoneSandboxPlugin
#      PRIVATE 
#         ${SharedSourceCode}
#         ${PluginSourceCode}
#     )

target_compile_definitions(LumatoneSandbox
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:LumatoneSandbox,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:LumatoneSandbox,JUCE_VERSION>"
        DONT_SET_USING_JUCE_NAMESPACE=1
        JUCE_VST3_CAN_REPLACE_VST2=0
    )

file(GLOB_RECURSE BinaryData
        CONFIGURE_DEPENDS
            "${CMAKE_CURRENT_SOURCE_DIR}/Source/shared/lumatone_editor_library/assets/*"
    )
juce_add_binary_data(LumatoneSandboxAssets 
    lumatone_binary_assets.h
    LumatoneEditorAssets
    SOURCES 
    ${BinaryData})

target_link_libraries(LumatoneSandbox
        PRIVATE
            LumatoneSandboxAssets
            juce::juce_gui_extra
            juce::juce_audio_utils
            juce::juce_opengl
            juce::juce_audio_devices
        PUBLIC
            juce::juce_recommended_config_flags
            juce::juce_recommended_lto_flags
            juce::juce_recommended_warning_flags
    )
