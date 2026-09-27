#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
class RGKnobLookAndFeel : public juce::LookAndFeel_V4
{
public:
    RGKnobLookAndFeel();

    void drawRotarySlider(
        juce::Graphics& g,
        int x,
        int y,
        int width,
        int height,
        float sliderPosProportional,
        float rotaryStartAngle,
        float rotaryEndAngle,
        juce::Slider& slider) override;
};

//==============================================================================
class AmpSimAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit AmpSimAudioProcessorEditor(AmpSimAudioProcessor&);
    ~AmpSimAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    //==============================================================
    // Processor
    //==============================================================

    AmpSimAudioProcessor& audioProcessor;

    //==============================================================
    // Background
    //==============================================================

    juce::Image backgroundImage;

    //==============================================================
    // Look and feel
    //==============================================================

    RGKnobLookAndFeel knobLookAndFeel;

    //==============================================================
    // Knobs
    //==============================================================

    juce::Slider gainKnob;
    juce::Slider bassKnob;
    juce::Slider midKnob;
    juce::Slider hiKnob;
    juce::Slider volumeKnob;

    juce::Label gainLabel;
    juce::Label bassLabel;
    juce::Label midLabel;
    juce::Label hiLabel;
    juce::Label volumeLabel;

    //==============================================================
    // Parameter pointers
    //==============================================================

    juce::AudioProcessorParameter* gainParameter   = nullptr;
    juce::AudioProcessorParameter* bassParameter   = nullptr;
    juce::AudioProcessorParameter* midParameter    = nullptr;
    juce::AudioProcessorParameter* hiParameter     = nullptr;
    juce::AudioProcessorParameter* volumeParameter = nullptr;

    //==============================================================
    // Reference layout
    //==============================================================

    static constexpr float referenceWidth  = 800.0f;
    static constexpr float referenceHeight = 500.0f;

    static constexpr float gainX   = 184.0f;
    static constexpr float bassX   = 292.0f;
    static constexpr float midX    = 400.0f;
    static constexpr float hiX     = 508.0f;
    static constexpr float volumeX = 616.0f;

    static constexpr float knobY = 330.0f;

    //==============================================================
    // Setup
    //==============================================================

    void setupKnob(
        juce::Slider& knob,
        juce::Label& label,
        const juce::String& text);

    juce::AudioProcessorParameter* findParameter(
        const juce::String& parameterName);

    void connectKnobToParameter(
        juce::Slider& knob,
        juce::AudioProcessorParameter*& parameter,
        const juce::String& parameterName);

    void updateKnobFromParameter(
        juce::Slider& knob,
        juce::AudioProcessorParameter* parameter);

    //==============================================================
    // Parameter callbacks
    //==============================================================

    void knobChanged(
        juce::Slider& knob,
        juce::AudioProcessorParameter* parameter);

    //==============================================================
    // Painting
    //==============================================================

    void drawAmplifierOverlay(juce::Graphics& g);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        AmpSimAudioProcessorEditor)
};
