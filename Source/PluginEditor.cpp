#include "PluginEditor.h"

#include "BinaryData.h"

//==============================================================================
AmpSimAudioProcessorEditor::AmpSimAudioProcessorEditor(
    AmpSimAudioProcessor& p)
    : AudioProcessorEditor(&p),
      audioProcessor(p)
{
    //==============================================================
    // Load embedded RG100 JPG
    //==============================================================

    backgroundImage = juce::ImageCache::getFromMemory(
        BinaryData::rg100jpg,
        BinaryData::rg100jpgSize);

    //==============================================================
    // Window
    //==============================================================

    setSize(1000, 700);

    //==============================================================
    // Sliders
    //==============================================================

    setupSlider(gainSlider, gainLabel, "GAIN");
    setupSlider(bassSlider, bassLabel, "BASS");
    setupSlider(midSlider, midLabel, "MID");
    setupSlider(hiSlider, hiLabel, "HI");
    setupSlider(volumeSlider, volumeLabel, "VOLUME");

    //==============================================================
    // Buttons
    //==============================================================

    setupButton(loadNAMButton, "LOAD NAM");
    setupButton(loadIRButton, "LOAD IR");
    setupButton(bypassButton, "BYPASS");

    loadNAMButton.onClick = [this]
    {
        loadNAM();
    };

    loadIRButton.onClick = [this]
    {
        loadIR();
    };

    bypassButton.onClick = [this]
    {
        toggleBypass();
    };

    //==============================================================
    // File labels
    //==============================================================

    namNameLabel.setText(
        "NAM: Embedded",
        juce::dontSendNotification);

    namNameLabel.setColour(
        juce::Label::textColourId,
        juce::Colours::white);

    namNameLabel.setJustificationType(
        juce::Justification::centredLeft);

    addAndMakeVisible(namNameLabel);

    irNameLabel.setText(
        "IR: Embedded",
        juce::dontSendNotification);

    irNameLabel.setColour(
        juce::Label::textColourId,
        juce::Colours::white);

    irNameLabel.setJustificationType(
        juce::Justification::centredLeft);

    addAndMakeVisible(irNameLabel);
}

//==============================================================================
AmpSimAudioProcessorEditor::~AmpSimAudioProcessorEditor()
{
}

//==============================================================================
void AmpSimAudioProcessorEditor::paint(juce::Graphics& g)
{
    //==============================================================
    // Background
    //==============================================================

    if (backgroundImage.isValid())
    {
        g.drawImageWithin(
            backgroundImage,
            0,
            0,
            getWidth(),
            getHeight(),
            juce::RectanglePlacement::stretchToFit,
            false);
    }
    else
    {
        g.fillAll(juce::Colours::black);
    }

    //==============================================================
    // Subtle dark overlay
    //==============================================================

    g.setColour(
        juce::Colours::black.withAlpha(0.18f));

    g.fillRect(getLocalBounds());

    //==============================================================
    // RG Amp SIM title
    //==============================================================

    g.setColour(juce::Colours::white);

    g.setFont(
        juce::FontOptions(28.0f)
            .withStyle("bold"));

    g.drawText(
        "RG AMP SIM",
        25,
        15,
        300,
        40,
        juce::Justification::left,
        false);

    //==============================================================
    // Small RG Audio text
    //==============================================================

    g.setFont(
        juce::FontOptions(13.0f));

    g.setColour(
        juce::Colours::white.withAlpha(0.75f));

    g.drawText(
        "RG AUDIO",
        27,
        48,
        200,
        20,
        juce::Justification::left,
        false);
}

//==============================================================================
void AmpSimAudioProcessorEditor::resized()
{
    //==============================================================
    // The image is the main UI.
    //
    // These controls are positioned as overlays.
    // Adjust these coordinates later to exactly match the controls
    // in rg100.jpg.
    //==============================================================

    // GAIN
    gainSlider.setBounds(
        170,
        300,
        100,
        100);

    gainLabel.setBounds(
        170,
        405,
        100,
        22);

    // BASS
    bassSlider.setBounds(
        300,
        300,
        100,
        100);

    bassLabel.setBounds(
        300,
        405,
        100,
        22);

    // MID
    midSlider.setBounds(
        430,
        300,
        100,
        100);

    midLabel.setBounds(
        430,
        405,
        100,
        22);

    // HI
    hiSlider.setBounds(
        560,
        300,
        100,
        100);

    hiLabel.setBounds(
        560,
        405,
        100,
        22);

    // VOLUME
    volumeSlider.setBounds(
        690,
        300,
        100,
        100);

    volumeLabel.setBounds(
        690,
        405,
        100,
        22);

    //==============================================================
    // NAM / IR buttons
    //==============================================================

    loadNAMButton.setBounds(
        30,
        getHeight() - 100,
        140,
        40);

    loadIRButton.setBounds(
        180,
        getHeight() - 100,
        140,
        40);

    bypassButton.setBounds(
        getWidth() - 170,
        getHeight() - 100,
        140,
        40);

    //==============================================================
    // File names
    //==============================================================

    namNameLabel.setBounds(
        30,
        getHeight() - 145,
        300,
        25);

    irNameLabel.setBounds(
        30,
        getHeight() - 120,
        300,
        25);
}

//==============================================================================
void AmpSimAudioProcessorEditor::setupSlider(
    juce::Slider& slider,
    juce::Label& label,
    const juce::String& labelText)
{
    addAndMakeVisible(slider);
    addAndMakeVisible(label);

    slider.setSliderStyle(
        juce::Slider::RotaryHorizontalVerticalDrag);

    slider.setTextBoxStyle(
        juce::Slider::TextBoxBelow,
        false,
        70,
        20);

    slider.setRange(
        0.0,
        1.0,
        0.001);

    slider.setValue(
        0.5,
        juce::dontSendNotification);

    slider.setColour(
        juce::Slider::rotarySliderFillColourId,
        juce::Colours::white);

    slider.setColour(
        juce::Slider::thumbColourId,
        juce::Colours::white);

    slider.setColour(
        juce::Slider::textBoxTextColourId,
        juce::Colours::white);

    slider.setColour(
        juce::Slider::textBoxBackgroundColourId,
        juce::Colours::black.withAlpha(0.65f));

    slider.setColour(
        juce::Slider::textBoxOutlineColourId,
        juce::Colours::transparentBlack);

    label.setText(
        labelText,
        juce::dontSendNotification);

    label.setColour(
        juce::Label::textColourId,
        juce::Colours::white);

    label.setFont(
        juce::FontOptions(13.0f)
            .withStyle("bold"));

    label.setJustificationType(
        juce::Justification::centred);
}

//==============================================================================
void AmpSimAudioProcessorEditor::setupButton(
    juce::TextButton& button,
    const juce::String& text)
{
    addAndMakeVisible(button);

    button.setButtonText(text);

    button.setColour(
        juce::TextButton::buttonColourId,
        juce::Colours::black.withAlpha(0.75f));

    button.setColour(
        juce::TextButton::buttonOnColourId,
        juce::Colours::white.withAlpha(0.20f));

    button.setColour(
        juce::TextButton::textColourOffId,
        juce::Colours::white);

    button.setColour(
        juce::TextButton::textColourOnId,
        juce::Colours::white);
}

//==============================================================================
void AmpSimAudioProcessorEditor::loadNAM()
{
    juce::FileChooser chooser(
        "Load NAM Model",
        juce::File{},
        "*.nam");

    chooser.launchAsync(
        juce::FileBrowserComponent::openMode
        | juce::FileBrowserComponent::canSelectFiles,
        [this](const juce::FileChooser& fc)
        {
            const auto file = fc.getResult();

            if (file.existsAsFile())
            {
                audioProcessor.loadNAM(file);

                namNameLabel.setText(
                    "NAM: " + file.getFileName(),
                    juce::dontSendNotification);
            }
        });
}

//==============================================================================
void AmpSimAudioProcessorEditor::loadIR()
{
    juce::FileChooser chooser(
        "Load IR",
        juce::File{},
        "*.wav;*.aiff;*.aif");

    chooser.launchAsync(
        juce::FileBrowserComponent::openMode
        | juce::FileBrowserComponent::canSelectFiles,
        [this](const juce::FileChooser& fc)
        {
            const auto file = fc.getResult();

            if (file.existsAsFile())
            {
                audioProcessor.loadIR(file);

                irNameLabel.setText(
                    "IR: " + file.getFileName(),
                    juce::dontSendNotification);
            }
        });
}

//==============================================================================
void AmpSimAudioProcessorEditor::toggleBypass()
{
    const bool newState =
        !audioProcessor.getBypass();

    audioProcessor.setBypass(newState);

    bypassButton.setButtonText(
        newState ? "BYPASSED" : "BYPASS");
}
