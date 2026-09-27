#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
class AmpSimAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit AmpSimAudioProcessorEditor (AmpSimAudioProcessor&);
    ~AmpSimAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
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
    // Controls
    //==============================================================

    juce::Slider gainSlider;
    juce::Slider bassSlider;
    juce::Slider midSlider;
    juce::Slider hiSlider;
    juce::Slider volumeSlider;

    juce::Label gainLabel;
    juce::Label bassLabel;
    juce::Label midLabel;
    juce::Label hiLabel;
    juce::Label volumeLabel;

    juce::TextButton loadNAMButton;
    juce::TextButton loadIRButton;
    juce::TextButton bypassButton;

    juce::Label namNameLabel;
    juce::Label irNameLabel;

    //==============================================================
    // Helpers
    //==============================================================

    void setupSlider(
        juce::Slider& slider,
        juce::Label& label,
        const juce::String& labelText);

    void setupButton(
        juce::TextButton& button,
        const juce::String& text);

    void loadNAM();
    void loadIR();
    void toggleBypass();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AmpSimAudioProcessorEditor)
};
