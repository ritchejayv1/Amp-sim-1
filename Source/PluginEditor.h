#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>

#include "PluginProcessor.h"

class AmpSimAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit AmpSimAudioProcessorEditor(AmpSimAudioProcessor&);
    ~AmpSimAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:

    //==============================================================
    // CUSTOM KNOB LOOK AND FEEL
    //==============================================================

    class RGKnobLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
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

    //==============================================================
    // KNOBS
    //==============================================================

    juce::Slider gainKnob;
    juce::Slider bassKnob;
    juce::Slider midKnob;
    juce::Slider hiKnob;
    juce::Slider volumeKnob;

    //==============================================================
    // MODE SWITCH
    // CLEAN / DRIVE
    //==============================================================

    juce::ToggleButton modeSwitch;

    bool isDriveMode = false;

    //==============================================================
    // AMP SWITCH
    // ON / OFF
    //==============================================================

    juce::ToggleButton ampSwitch;

    bool ampIsOn = true;

    //==============================================================
    // LABELS
    //==============================================================

    juce::Label inputLabel;
    juce::Label gainLabel;
    juce::Label bassLabel;
    juce::Label midLabel;
    juce::Label hiLabel;
    juce::Label volumeLabel;
    juce::Label modeLabel;
    juce::Label ampLabel;

    //==============================================================
    // APVTS SLIDER ATTACHMENTS
    //==============================================================

    std::unique_ptr<
        juce::AudioProcessorValueTreeState::SliderAttachment>
        gainAttachment;

    std::unique_ptr<
        juce::AudioProcessorValueTreeState::SliderAttachment>
        bassAttachment;

    std::unique_ptr<
        juce::AudioProcessorValueTreeState::SliderAttachment>
        midAttachment;

    std::unique_ptr<
        juce::AudioProcessorValueTreeState::SliderAttachment>
        hiAttachment;

    std::unique_ptr<
        juce::AudioProcessorValueTreeState::SliderAttachment>
        volumeAttachment;

    //==============================================================
    // APVTS BUTTON ATTACHMENTS
    //==============================================================

    std::unique_ptr<
        juce::AudioProcessorValueTreeState::ButtonAttachment>
        modeAttachment;

    std::unique_ptr<
        juce::AudioProcessorValueTreeState::ButtonAttachment>
        ampAttachment;

    //==============================================================
    // LOOK AND FEEL
    //==============================================================

    RGKnobLookAndFeel knobLookAndFeel;

    //==============================================================
    // BACKGROUND IMAGE
    //==============================================================

    juce::Image backgroundImage;

    //==============================================================
    // HELPERS
    //==============================================================

    void setupKnob(
        juce::Slider& slider,
        double min,
        double max,
        double interval);

    void setupLabel(
        juce::Label& label,
        const juce::String& text);

    //==============================================================
    // PROCESSOR
    //==============================================================

    AmpSimAudioProcessor& audioProcessor;

    //==============================================================

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        AmpSimAudioProcessorEditor)
};
