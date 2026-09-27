#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
class AmpSimAudioProcessorEditor : public juce::AudioProcessorEditor,
                                   private juce::Timer
{
public:
    explicit AmpSimAudioProcessorEditor (AmpSimAudioProcessor&);
    ~AmpSimAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    //==============================================================================
    AmpSimAudioProcessor& audioProcessor;

    //==============================================================================
    // KNOBS
    juce::Slider gainSlider;
    juce::Slider bassSlider;
    juce::Slider midSlider;
    juce::Slider hiSlider;
    juce::Slider volumeSlider;

    //==============================================================================
    // BUTTONS
    juce::TextButton loadNamButton;
    juce::TextButton loadIrButton;
    juce::TextButton bypassButton;

    //==============================================================================
    // LABELS
    juce::Label gainLabel;
    juce::Label bassLabel;
    juce::Label midLabel;
    juce::Label hiLabel;
    juce::Label volumeLabel;

    juce::Label namLabel;
    juce::Label irLabel;

    //==============================================================================
    // PARAMETER ATTACHMENTS
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
        gainAttachment;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
        bassAttachment;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
        midAttachment;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
        hiAttachment;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
        volumeAttachment;

    //==============================================================================
    // FILE LOADING
    void chooseNAM();
    void chooseIR();

    //==============================================================================
    // BYPASS
    void toggleBypass();

    //==============================================================================
    // UI HELPERS
    void setupKnob (juce::Slider& slider,
                    const juce::String& parameterID);

    void setupLabel (juce::Label& label,
                     const juce::String& text);

    void setupButton (juce::TextButton& button,
                      const juce::String& text);

    void updateFileLabels();
    void updateBypassButton();

    //==============================================================================
    void timerCallback() override;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (
        AmpSimAudioProcessorEditor
    )
};
