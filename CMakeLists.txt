cmake_minimum_required(VERSION 3.27)

project(LUMATONE_SANDBOX VERSION 0.0.1)

# find_package(juce)
add_subdirectory($ENV{juce_DIR} subprojects/juce)

juce_add_gui_app(LumatoneSandbox PRODUCT_NAME "Lumatone Sandbox App")
juce_generate_juce_header(LumatoneSandbox)

file(GLOB_RECURSE SourceCode 
    CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h"
    )
target_sources(LumatoneSandbox PRIVATE ${SourceCode})

target_compile_definitions(LumatoneSandbox
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:LumatoneSandbox,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:LumatoneSandbox,JUCE_VERSION>"
        DONT_SET_USING_JUCE_NAMESPACE=1
    )

file(GLOB_RECURSE BinaryData
        CONFIGURE_DEPENDS
            "${CMAKE_CURRENT_SOURCE_DIR}/Source/lumatone_editor_library/assets/*"
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
        juce::juce_audio_devices
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
    )
