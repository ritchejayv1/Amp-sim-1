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
    // Custom knob drawing
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
    // Knobs
    //==============================================================

    juce::Slider gainKnob;
    juce::Slider bassKnob;
    juce::Slider midKnob;
    juce::Slider hiKnob;
    juce::Slider volumeKnob;

    //==============================================================
    // Labels
    //==============================================================

    juce::Label gainLabel;
    juce::Label bassLabel;
    juce::Label midLabel;
    juce::Label hiLabel;
    juce::Label volumeLabel;

    //==============================================================
    // APVTS Slider Attachments
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
    // Look and Feel
    //==============================================================

    RGKnobLookAndFeel knobLookAndFeel;

    //==============================================================
    // Background
    //==============================================================

    juce::Image backgroundImage;

    //==============================================================
    // Helpers
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

    AmpSimAudioProcessor& audioProcessor;

    //==============================================================

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        AmpSimAudioProcessorEditor)
};
