#include "PluginEditor.h"
#include "BinaryData.h"

AmpSimAudioProcessorEditor::AmpSimAudioProcessorEditor (
    AmpSimAudioProcessor& p)
    : AudioProcessorEditor (&p),
      audioProcessor (p)
{
    backgroundImage = juce::ImageCache::getFromMemory (
        BinaryData::rg100jpg,
        BinaryData::rg100jpgSize
    );

    setSize (540, 740);

    addAndMakeVisible (gainSlider);
    addAndMakeVisible (bassSlider);
    addAndMakeVisible (midSlider);
    addAndMakeVisible (hiSlider);
    addAndMakeVisible (volumeSlider);

    gainSlider.setSliderStyle (
        juce::Slider::RotaryHorizontalVerticalDrag);

    bassSlider.setSliderStyle (
        juce::Slider::RotaryHorizontalVerticalDrag);

    midSlider.setSliderStyle (
        juce::Slider::RotaryHorizontalVerticalDrag);

    hiSlider.setSliderStyle (
        juce::Slider::RotaryHorizontalVerticalDrag);

    volumeSlider.setSliderStyle (
        juce::Slider::RotaryHorizontalVerticalDrag);

    gainSlider.setTextBoxStyle (
        juce::Slider::NoTextBox, false, 0, 0);

    bassSlider.setTextBoxStyle (
        juce::Slider::NoTextBox, false, 0, 0);

    midSlider.setTextBoxStyle (
        juce::Slider::NoTextBox, false, 0, 0);

    hiSlider.setTextBoxStyle (
        juce::Slider::NoTextBox, false, 0, 0);

    volumeSlider.setTextBoxStyle (
        juce::Slider::NoTextBox, false, 0, 0);

    gainAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::SliderAttachment>
        (
            audioProcessor.parameters,
            "GAIN",
            gainSlider
        );

    bassAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::SliderAttachment>
        (
            audioProcessor.parameters,
            "BASS",
            bassSlider
        );

    midAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::SliderAttachment>
        (
            audioProcessor.parameters,
            "MID",
            midSlider
        );

    hiAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::SliderAttachment>
        (
            audioProcessor.parameters,
            "HI",
            hiSlider
        );

    volumeAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::SliderAttachment>
        (
            audioProcessor.parameters,
            "VOLUME",
            volumeSlider
        );
}

AmpSimAudioProcessorEditor::~AmpSimAudioProcessorEditor()
{
}

void AmpSimAudioProcessorEditor::paint (juce::Graphics& g)
{
    if (backgroundImage.isValid())
    {
        g.drawImage (
            backgroundImage,
            getLocalBounds().toFloat(),
            juce::RectanglePlacement::stretchToFit
        );
    }
    else
    {
        g.fillAll (juce::Colours::black);
    }
}

void AmpSimAudioProcessorEditor::resized()
{
    // Temporary positions.
    // I-a-adjust natin ito eksakto ayon sa rg100.jpg.

    gainSlider.setBounds   (55, 180, 80, 80);
    bassSlider.setBounds   (150, 180, 80, 80);
    midSlider.setBounds    (245, 180, 80, 80);
    hiSlider.setBounds     (340, 180, 80, 80);
    volumeSlider.setBounds (435, 180, 80, 80);
}
