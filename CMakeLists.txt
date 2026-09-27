cmake_minimum_required(VERSION 3.22)

project(AmpSim VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

#==============================================================
# JUCE
#==============================================================

FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 7.0.12
)

FetchContent_MakeAvailable(JUCE)

#==============================================================
# NeuralAudio
#==============================================================

FetchContent_Declare(
    NeuralAudio
    GIT_REPOSITORY https://github.com/mikeoliphant/NeuralAudio.git
    GIT_TAG main
)

FetchContent_MakeAvailable(NeuralAudio)

#==============================================================
# Find NeuralAudio headers
#==============================================================

file(GLOB_RECURSE NEURAL_AUDIO_HEADERS
    "${neuralaudio_SOURCE_DIR}/*.h"
)

message(STATUS "==========================================")
message(STATUS "NeuralAudio source: ${neuralaudio_SOURCE_DIR}")
message(STATUS "NeuralAudio headers:")
foreach(HEADER ${NEURAL_AUDIO_HEADERS})
    message(STATUS "  ${HEADER}")
endforeach()
message(STATUS "==========================================")

#==============================================================
# Binary Assets
#==============================================================

juce_add_binary_data(
    AmpSimAssets
    SOURCES
        "Assets/RG MBDR+precision.nam"
        "Assets/RG 412 MB mic 1.wav"
        "Assets/rg100.jpg"
)

#==============================================================
# Plugin
#==============================================================

juce_add_plugin(
    AmpSim
    COMPANY_NAME "RG Audio"
    BUNDLE_ID com.rgaudio.ampsim
    PLUGIN_MANUFACTURER_CODE RGAu
    PLUGIN_CODE Amp1
    FORMATS VST3
    PRODUCT_NAME "RG Amp SIM"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
)

#==============================================================
# Source
#==============================================================

target_sources(
    AmpSim
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
)

#==============================================================
# NeuralAudio include directories
#==============================================================

target_include_directories(
    AmpSim
    PRIVATE
        ${neuralaudio_SOURCE_DIR}
        ${neuralaudio_SOURCE_DIR}/NeuralAudio
        ${neuralaudio_SOURCE_DIR}/include
        ${neuralaudio_SOURCE_DIR}/src
)

#==============================================================
# Libraries
#==============================================================

target_link_libraries(
    AmpSim
    PRIVATE
        AmpSimAssets
        NeuralAudio
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_gui_extra

    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

#==============================================================
# Definitions
#==============================================================

target_compile_definitions(
    AmpSim
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

#==============================================================
# Folder
#==============================================================

set_target_properties(
    AmpSim
    PROPERTIES
        FOLDER "RG Audio"
)
