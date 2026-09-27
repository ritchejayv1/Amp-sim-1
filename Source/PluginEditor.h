#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class AmpSimAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit AmpSimAudioProcessorEditor (AmpSimAudioProcessor&);
    ~AmpSimAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    AmpSimAudioProcessor& audioProcessor;

    juce::Image backgroundImage;

    juce::Slider gainSlider;
    juce::Slider bassSlider;
    juce::Slider midSlider;
    juce::Slider hiSlider;
    juce::Slider volumeSlider;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bassAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> midAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> hiAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> volumeAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AmpSimAudioProcessorEditor)
};
