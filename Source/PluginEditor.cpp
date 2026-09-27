#include "PluginEditor.h"

//==============================================================================
// CONSTRUCTOR
//==============================================================================

AmpSimAudioProcessorEditor::AmpSimAudioProcessorEditor (
    AmpSimAudioProcessor& p)
    : AudioProcessorEditor (&p),
      audioProcessor (p)
{
    //==============================================================
    // WINDOW
    //==============================================================

    setSize (540, 740);

    //==============================================================
    // KNOBS
    //==============================================================

    setupKnob (gainSlider,   "GAIN");
    setupKnob (bassSlider,   "BASS");
    setupKnob (midSlider,    "MID");
    setupKnob (hiSlider,     "HI");
    setupKnob (volumeSlider, "VOLUME");

    //==============================================================
    // KNOB LABELS
    //==============================================================

    setupLabel (gainLabel,   "GAIN");
    setupLabel (bassLabel,   "BASS");
    setupLabel (midLabel,    "MID");
    setupLabel (hiLabel,     "HI");
    setupLabel (volumeLabel, "VOLUME");

    //==============================================================
    // PARAMETER ATTACHMENTS
    //==============================================================

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

    //==============================================================
    // LOAD NAM
    //==============================================================

    setupButton (loadNamButton, "LOAD NAM");

    loadNamButton.onClick =
        [this]
        {
            chooseNAM();
        };

    addAndMakeVisible (loadNamButton);

    //==============================================================
    // LOAD IR
    //==============================================================

    setupButton (loadIrButton, "LOAD IR");

    loadIrButton.onClick =
        [this]
        {
            chooseIR();
        };

    addAndMakeVisible (loadIrButton);

    //==============================================================
    // NAM / IR DISPLAY
    //==============================================================

    setupLabel (namLabel, "NAM: Embedded");
    setupLabel (irLabel, "IR: Embedded");

    namLabel.setJustificationType (
        juce::Justification::centredLeft
    );

    irLabel.setJustificationType (
        juce::Justification::centredLeft
    );

    //==============================================================
    // BYPASS FOOTSWITCH
    //==============================================================

    bypassButton.setButtonText ("BYPASS");

    bypassButton.setColour (
        juce::TextButton::buttonColourId,
        juce::Colour (20, 21, 23)
    );

    bypassButton.setColour (
        juce::TextButton::textColourOffId,
        juce::Colour (230, 232, 235)
    );

    bypassButton.setColour (
        juce::TextButton::textColourOnId,
        juce::Colour (255, 255, 255)
    );

    bypassButton.setClickingTogglesState (false);

    bypassButton.onClick =
        [this]
        {
            toggleBypass();
        };

    addAndMakeVisible (bypassButton);

    //==============================================================
    // TIMER
    //==============================================================

    startTimerHz (15);

    updateFileLabels();
    updateBypassButton();
}

//==============================================================================
// DESTRUCTOR
//==============================================================================

AmpSimAudioProcessorEditor::~AmpSimAudioProcessorEditor()
{
    stopTimer();
}

//==============================================================================
// PAINT
//==============================================================================

void AmpSimAudioProcessorEditor::paint (
    juce::Graphics& g)
{
    //==============================================================
    // MAIN BACKGROUND
    //==============================================================

    g.fillAll (
        juce::Colour (9, 10, 11)
    );

    //==============================================================
    // AMP BODY
    //==============================================================

    auto ampArea =
        getLocalBounds()
            .reduced (18);

    g.setColour (
        juce::Colour (30, 32, 35)
    );

    g.fillRoundedRectangle (
        ampArea.toFloat(),
        18.0f
    );

    //==============================================================
    // TOP PANEL
    //==============================================================

    auto topPanel =
        juce::Rectangle<float> (
            32.0f,
            30.0f,
            476.0f,
            70.0f
        );

    g.setColour (
        juce::Colour (17, 18, 20)
    );

    g.fillRoundedRectangle (
        topPanel,
        12.0f
    );

    //==============================================================
    // RG LOGO
    //==============================================================

    g.setColour (
        juce::Colour (235, 237, 240)
    );

    g.setFont (
        juce::Font (
            30.0f,
            juce::Font::bold
        )
    );

    g.drawText (
        "RG",
        48,
        42,
        70,
        40,
        juce::Justification::centredLeft,
        false
    );

    //==============================================================
    // AMP SIM TITLE
    //==============================================================

    g.setColour (
        juce::Colour (80, 150, 255)
    );

    g.setFont (
        juce::Font (
            21.0f,
            juce::Font::bold
        )
    );

    g.drawText (
        "AMP SIM",
        120,
        45,
        220,
        30,
        juce::Justification::centredLeft,
        false
    );

    //==============================================================
    // VERSION / DESCRIPTION
    //==============================================================

    g.setColour (
        juce::Colour (150, 153, 158)
    );

    g.setFont (
        juce::Font (11.0f)
    );

    g.drawText (
        "NAM + 4x12 IR",
        120,
        70,
        200,
        18,
        juce::Justification::centredLeft,
        false
    );

    //==============================================================
    // BLUE ACCENT LINE
    //==============================================================

    g.setColour (
        juce::Colour (35, 105, 255)
    );

    g.fillRect (
        32,
        100,
        476,
        3
    );

    //==============================================================
    // SECTION TITLES
    //==============================================================

    g.setColour (
        juce::Colour (180, 184, 190)
    );

    g.setFont (
        juce::Font (
            12.0f,
            juce::Font::bold
        )
    );

    g.drawText (
        "AMP CONTROLS",
        40,
        120,
        200,
        20,
        juce::Justification::left,
        false
    );

    //==============================================================
    // NAM / IR SECTION
    //==============================================================

    g.setColour (
        juce::Colour (180, 184, 190)
    );

    g.drawText (
        "CABINET / MODEL",
        40,
        325,
        220,
        20,
        juce::Justification::left,
        false
    );

    //==============================================================
    // NAM DISPLAY BOX
    //==============================================================

    g.setColour (
        juce::Colour (13, 14, 16)
    );

    g.fillRoundedRectangle (
        40.0f,
        355.0f,
        460.0f,
        48.0f,
        8.0f
    );

    g.setColour (
        juce::Colour (60, 64, 70)
    );

    g.drawRoundedRectangle (
        40.0f,
        355.0f,
        460.0f,
        48.0f,
        8.0f,
        1.0f
    );

    //==============================================================
    // IR DISPLAY BOX
    //==============================================================

    g.setColour (
        juce::Colour (13, 14, 16)
    );

    g.fillRoundedRectangle (
        40.0f,
        415.0f,
        460.0f,
        48.0f,
        8.0f
    );

    g.setColour (
        juce::Colour (60, 64, 70)
    );

    g.drawRoundedRectangle (
        40.0f,
        415.0f,
        460.0f,
        48.0f,
        8.0f,
        1.0f
    );

    //==============================================================
    // FOOTSWITCH AREA
    //==============================================================

    g.setColour (
        juce::Colour (13, 14, 16)
    );

    g.fillRoundedRectangle (
        180.0f,
        500.0f,
        180.0f,
        155.0f,
        14.0f
    );

    g.setColour (
        juce::Colour (50, 53, 58)
    );

    g.drawRoundedRectangle (
        180.0f,
        500.0f,
        180.0f,
        155.0f,
        14.0f,
        1.0f
    );

    //==============================================================
    // FOOTSWITCH LABEL
    //==============================================================

    g.setColour (
        juce::Colour (120, 124, 130)
    );

    g.setFont (
        juce::Font (
            10.0f,
            juce::Font::bold
        )
    );

    g.drawText (
        "RG AMP SIM",
        180,
        625,
        180,
        18,
        juce::Justification::centred,
        false
    );
}

//==============================================================================
// RESIZED
//==============================================================================

void AmpSimAudioProcessorEditor::resized()
{
    //==============================================================
    // KNOBS
    //==============================================================

    gainSlider.setBounds (
        35, 145, 85, 85
    );

    bassSlider.setBounds (
        130, 145, 85, 85
    );

    midSlider.setBounds (
        225, 145, 85, 85
    );

    hiSlider.setBounds (
        320, 145, 85, 85
    );

    volumeSlider.setBounds (
        415, 145, 85, 85
    );

    //==============================================================
    // LABELS
    //==============================================================

    gainLabel.setBounds (
        35, 225, 85, 22
    );

    bassLabel.setBounds (
        130, 225, 85, 22
    );

    midLabel.setBounds (
        225, 225, 85, 22
    );

    hiLabel.setBounds (
        320, 225, 85, 22
    );

    volumeLabel.setBounds (
        415, 225, 85, 22
    );

    //==============================================================
    // NAM / IR LABELS
    //==============================================================

    namLabel.setBounds (
        52, 363, 330, 32
    );

    irLabel.setBounds (
        52, 423, 330, 32
    );

    //==============================================================
    // LOAD BUTTONS
    //==============================================================

    loadNamButton.setBounds (
        390, 362, 95, 32
    );

    loadIrButton.setBounds (
        390, 422, 95, 32
    );

    //==============================================================
    // FOOTSWITCH
    //==============================================================

    bypassButton.setBounds (
        210, 515, 120, 90
    );
}

//==============================================================================
// SETUP KNOB
//==============================================================================

void AmpSimAudioProcessorEditor::setupKnob (
    juce::Slider& slider,
    const juce::String& parameterID)
{
    slider.setSliderStyle (
        juce::Slider::RotaryHorizontalVerticalDrag
    );

    slider.setTextBoxStyle (
        juce::Slider::TextBoxBelow,
        false,
        65,
        20
    );

    slider.setRange (
        0.0,
        1.0,
        0.001
    );

    slider.setDoubleClickReturnValue (
        true,
        0.5
    );

    slider.setColour (
        juce::Slider::rotarySliderFillColourId,
        juce::Colour (35, 105, 255)
    );

    slider.setColour (
        juce::Slider::rotarySliderOutlineColourId,
        juce::Colour (70, 73, 78)
    );

    slider.setColour (
        juce::Slider::thumbColourId,
        juce::Colour (235, 237, 240)
    );

    slider.setColour (
        juce::Slider::textBoxTextColourId,
        juce::Colour (220, 223, 228)
    );

    slider.setColour (
        juce::Slider::textBoxBackgroundColourId,
        juce::Colour (13, 14, 16)
    );

    slider.setColour (
        juce::Slider::textBoxOutlineColourId,
        juce::Colours::transparentBlack
    );

    addAndMakeVisible (slider);

    juce::ignoreUnused (parameterID);
}

//==============================================================================
// SETUP LABEL
//==============================================================================

void AmpSimAudioProcessorEditor::setupLabel (
    juce::Label& label,
    const juce::String& text)
{
    label.setText (
        text,
        juce::dontSendNotification
    );

    label.setFont (
        juce::Font (
            11.0f,
            juce::Font::bold
        )
    );

    label.setColour (
        juce::Label::textColourId,
        juce::Colour (205, 208, 213)
    );

    label.setJustificationType (
        juce::Justification::centred
    );

    addAndMakeVisible (label);
}

//==============================================================================
// SETUP BUTTON
//==============================================================================

void AmpSimAudioProcessorEditor::setupButton (
    juce::TextButton& button,
    const juce::String& text)
{
    button.setButtonText (text);

    button.setColour (
        juce::TextButton::buttonColourId,
        juce::Colour (24, 26, 29)
    );

    button.setColour (
        juce::TextButton::buttonOnColourId,
        juce::Colour (35, 105, 255)
    );

    button.setColour (
        juce::TextButton::textColourOffId,
        juce::Colour (225, 227, 230)
    );

    button.setColour (
        juce::TextButton::textColourOnId,
        juce::Colours::white
    );

    addAndMakeVisible (button);
}

//==============================================================================
// LOAD NAM
//==============================================================================

void AmpSimAudioProcessorEditor::chooseNAM()
{
    auto chooser =
        std::make_shared<juce::FileChooser> (
            "Select NAM Model",
            juce::File{},
            "*.nam"
        );

    chooser->launchAsync (
        juce::FileBrowserComponent::openMode |
        juce::FileBrowserComponent::canSelectFiles,
        [this, chooser] (
            const juce::FileChooser& fc)
        {
            auto file = fc.getResult();

            if (file.existsAsFile())
            {
                audioProcessor.loadNAM (file);
                updateFileLabels();
            }
        }
    );
}

//==============================================================================
// LOAD IR
//==============================================================================

void AmpSimAudioProcessorEditor::chooseIR()
{
    auto chooser =
        std::make_shared<juce::FileChooser> (
            "Select IR",
            juce::File{},
            "*.wav;*.WAV"
        );

    chooser->launchAsync (
        juce::FileBrowserComponent::openMode |
        juce::FileBrowserComponent::canSelectFiles,
        [this, chooser] (
            const juce::FileChooser& fc)
        {
            auto file = fc.getResult();

            if (file.existsAsFile())
            {
                audioProcessor.loadIR (file);
                updateFileLabels();
            }
        }
    );
}

//==============================================================================
// BYPASS
//==============================================================================

void AmpSimAudioProcessorEditor::toggleBypass()
{
    const bool newState =
        !audioProcessor.getBypass();

    audioProcessor.setBypass (newState);

    updateBypassButton();
}

//==============================================================================
// FILE LABELS
//==============================================================================

void AmpSimAudioProcessorEditor::updateFileLabels()
{
    auto namName =
        audioProcessor.getNAMName();

    auto irName =
        audioProcessor.getIRName();

    if (namName.isEmpty())
        namName = "Embedded NAM";

    if (irName.isEmpty())
        irName = "Embedded 4x12 IR";

    namLabel.setText (
        "NAM: " + namName,
        juce::dontSendNotification
    );

    irLabel.setText (
        "IR: " + irName,
        juce::dontSendNotification
    );
}

//==============================================================================
// BYPASS BUTTON
//==============================================================================

void AmpSimAudioProcessorEditor::updateBypassButton()
{
    const bool bypassed =
        audioProcessor.getBypass();

    if (bypassed)
    {
        bypassButton.setButtonText ("BYPASSED");

        bypassButton.setColour (
            juce::TextButton::buttonColourId,
            juce::Colour (120, 25, 25)
        );
    }
    else
    {
        bypassButton.setButtonText ("BYPASS");

        bypassButton.setColour (
            juce::TextButton::buttonColourId,
            juce::Colour (20, 21, 23)
        );
    }

    bypassButton.repaint();
}

//==============================================================================
// TIMER
//==============================================================================

void AmpSimAudioProcessorEditor::timerCallback()
{
    updateFileLabels();
    updateBypassButton();
}
