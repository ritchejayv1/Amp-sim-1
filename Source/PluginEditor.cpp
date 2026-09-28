#include "PluginEditor.h"

#include <cmath>

#include "BinaryData.h"

namespace
{
    //==============================================================
    // WINDOW / PEDAL SIZE
    //==============================================================

    constexpr int editorWidth  = 800;
    constexpr int editorHeight = 500;

    //==============================================================
    // MAIN CONTROL POSITIONS
    //==============================================================

    constexpr float inputX  = 130.0f;
    constexpr float gainX   = 200.0f;
    constexpr float bassX   = 270.0f;
    constexpr float midX    = 340.0f;
    constexpr float hiX     = 410.0f;
    constexpr float volumeX = 480.0f;
    constexpr float modeX   = 550.0f;
    constexpr float ampX    = 620.0f;

    constexpr float controlY = 330.0f;

    //==============================================================
    // SMALL KNOB
    //==============================================================

    constexpr float knobSize = 36.0f;
}

//==================================================================
// CONSTRUCTOR
//==================================================================

AmpSimAudioProcessorEditor::
AmpSimAudioProcessorEditor(
    AmpSimAudioProcessor& p)
    : AudioProcessorEditor(&p),
      audioProcessor(p)
{
    //==============================================================
    // WINDOW
    //==============================================================

    setSize(
        editorWidth,
        editorHeight);

    //==============================================================
    // BACKGROUND IMAGE
    //==============================================================

    backgroundImage =
        juce::ImageCache::getFromMemory(
            BinaryData::rg100_jpg,
            BinaryData::rg100_jpgSize);

    //==============================================================
    // KNOBS
    //==============================================================

    setupKnob(
        gainKnob,
        0.0,
        10.0,
        0.01);

    setupKnob(
        bassKnob,
        -12.0,
        12.0,
        0.01);

    setupKnob(
        midKnob,
        -12.0,
        12.0,
        0.01);

    setupKnob(
        hiKnob,
        -12.0,
        12.0,
        0.01);

    setupKnob(
        volumeKnob,
        0.0,
        10.0,
        0.01);

    //==============================================================
    // LABELS
    //==============================================================

    setupLabel(
        inputLabel,
        "INPUT");

    setupLabel(
        gainLabel,
        "GAIN");

    setupLabel(
        bassLabel,
        "BASS");

    setupLabel(
        midLabel,
        "MID");

    setupLabel(
        hiLabel,
        "HI");

    setupLabel(
        volumeLabel,
        "VOLUME");

    setupLabel(
        modeLabel,
        "MODE");

    setupLabel(
        ampLabel,
        "AMP");

    //==============================================================
    // MODE SWITCH
    //==============================================================

    modeSwitch.setButtonText(
        "CLEAN");

    modeSwitch.setClickingTogglesState(
        true);

    modeSwitch.setToggleState(
        false,
        juce::dontSendNotification);

    modeSwitch.setColour(
        juce::ToggleButton::textColourId,
        juce::Colours::white);

    modeSwitch.setColour(
        juce::ToggleButton::tickColourId,
        juce::Colours::white);

    modeSwitch.setColour(
        juce::ToggleButton::tickDisabledColourId,
        juce::Colours::darkgrey);

    addAndMakeVisible(
        modeSwitch);

    //==============================================================
    // MODE PARAMETER ATTACHMENT
    //==============================================================

    modeAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::ButtonAttachment>(
                audioProcessor.parameters,
                "MODE",
                modeSwitch);

    //==============================================================
    // MODE CLICK
    //==============================================================

    modeSwitch.onClick =
        [this]()
        {
            isDriveMode =
                modeSwitch.getToggleState();

            modeSwitch.setButtonText(
                isDriveMode
                    ? "DRIVE"
                    : "CLEAN");

            repaint();
        };

    //==============================================================
    // AMP SWITCH
    //==============================================================

    ampSwitch.setButtonText(
        "ON");

    ampSwitch.setClickingTogglesState(
        true);

    ampSwitch.setToggleState(
        true,
        juce::dontSendNotification);

    ampSwitch.setColour(
        juce::ToggleButton::textColourId,
        juce::Colours::white);

    ampSwitch.setColour(
        juce::ToggleButton::tickColourId,
        juce::Colours::white);

    ampSwitch.setColour(
        juce::ToggleButton::tickDisabledColourId,
        juce::Colours::darkgrey);

    addAndMakeVisible(
        ampSwitch);

    //==============================================================
    // AMP PARAMETER ATTACHMENT
    //==============================================================

    ampAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::ButtonAttachment>(
                audioProcessor.parameters,
                "AMP",
                ampSwitch);

    //==============================================================
    // AMP CLICK
    //==============================================================

    ampSwitch.onClick =
        [this]()
        {
            ampIsOn =
                ampSwitch.getToggleState();

            ampSwitch.setButtonText(
                ampIsOn
                    ? "ON"
                    : "OFF");

            repaint();
        };

    //==============================================================
    // SLIDER ATTACHMENTS
    //==============================================================

    gainAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::SliderAttachment>(
                audioProcessor.parameters,
                "GAIN",
                gainKnob);

    bassAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::SliderAttachment>(
                audioProcessor.parameters,
                "BASS",
                bassKnob);

    midAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::SliderAttachment>(
                audioProcessor.parameters,
                "MID",
                midKnob);

    hiAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::SliderAttachment>(
                audioProcessor.parameters,
                "HI",
                hiKnob);

    volumeAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::SliderAttachment>(
                audioProcessor.parameters,
                "VOLUME",
                volumeKnob);

    //==============================================================
    // INITIAL UI STATE
    //==============================================================

    isDriveMode =
        modeSwitch.getToggleState();

    ampIsOn =
        ampSwitch.getToggleState();

    modeSwitch.setButtonText(
        isDriveMode
            ? "DRIVE"
            : "CLEAN");

    ampSwitch.setButtonText(
        ampIsOn
            ? "ON"
            : "OFF");

    repaint();
}

//==================================================================
// DESTRUCTOR
//==================================================================

AmpSimAudioProcessorEditor::
~AmpSimAudioProcessorEditor()
{
}

//==================================================================
// KNOB SETUP
//==================================================================

void AmpSimAudioProcessorEditor::setupKnob(
    juce::Slider& slider,
    double min,
    double max,
    double interval)
{
    slider.setSliderStyle(
        juce::Slider::RotaryHorizontalVerticalDrag);

    slider.setTextBoxStyle(
        juce::Slider::NoTextBox,
        false,
        0,
        0);

    slider.setRange(
        min,
        max,
        interval);

    slider.setValue(
        (min + max) * 0.5,
        juce::dontSendNotification);

    slider.setLookAndFeel(
        &knobLookAndFeel);

    slider.setPopupDisplayEnabled(
        true,
        true,
        this);

    addAndMakeVisible(
        slider);
}

//==================================================================
// LABEL SETUP
//==================================================================

void AmpSimAudioProcessorEditor::setupLabel(
    juce::Label& label,
    const juce::String& text)
{
    label.setText(
        text,
        juce::dontSendNotification);

    label.setFont(
        juce::Font(
            8.0f,
            juce::Font::bold));

    label.setColour(
        juce::Label::textColourId,
        juce::Colours::white);

    label.setJustificationType(
        juce::Justification::centred);

    label.setInterceptsMouseClicks(
        false,
        false);

    addAndMakeVisible(
        label);
}

//==================================================================
// KNOB LOOK AND FEEL
//==================================================================

void AmpSimAudioProcessorEditor::
RGKnobLookAndFeel::drawRotarySlider(
    juce::Graphics& g,
    int x,
    int y,
    int width,
    int height,
    float sliderPosProportional,
    float rotaryStartAngle,
    float rotaryEndAngle,
    juce::Slider& slider)
{
    juce::ignoreUnused(
        slider);

    const float size =
        static_cast<float>(
            juce::jmin(
                width,
                height));

    const float cx =
        static_cast<float>(x)
        + static_cast<float>(width) * 0.5f;

    const float cy =
        static_cast<float>(y)
        + static_cast<float>(height) * 0.5f;

    const float radius =
        size * 0.5f - 2.0f;

    //==============================================================
    // OUTER RING
    //==============================================================

    g.setColour(
        juce::Colour(
            20,
            20,
            20));

    g.fillEllipse(
        cx - radius,
        cy - radius,
        radius * 2.0f,
        radius * 2.0f);

    //==============================================================
    // WHITE KNOB
    //==============================================================

    const float knobRadius =
        radius - 3.0f;

    g.setColour(
        juce::Colours::white);

    g.fillEllipse(
        cx - knobRadius,
        cy - knobRadius,
        knobRadius * 2.0f,
        knobRadius * 2.0f);

    //==============================================================
    // POINTER
    //==============================================================

    const float angle =
        rotaryStartAngle
        + sliderPosProportional
          * (rotaryEndAngle - rotaryStartAngle);

    const float pointerLength =
        knobRadius * 0.65f;

    const float pointerX =
        cx + std::cos(angle)
            * pointerLength;

    const float pointerY =
        cy + std::sin(angle)
            * pointerLength;

    g.setColour(
        juce::Colours::black);

    g.drawLine(
        cx,
        cy,
        pointerX,
        pointerY,
        2.0f);
}

//==================================================================
// PAINT
//==================================================================

void AmpSimAudioProcessorEditor::paint(
    juce::Graphics& g)
{
    //==============================================================
    // BACKGROUND
    //==============================================================

    if (backgroundImage.isValid())
    {
        g.drawImage(
            backgroundImage,
            getLocalBounds().toFloat(),
            juce::RectanglePlacement::stretchToFit);
    }
    else
    {
        g.fillAll(
            juce::Colour(
                18,
                18,
                18));
    }

    //==============================================================
    // INPUT JACK
    //==============================================================

    constexpr float jackRadius =
        11.0f;

    g.setColour(
        juce::Colour(
            25,
            25,
            25));

    g.fillEllipse(
        inputX - jackRadius,
        controlY - jackRadius,
        jackRadius * 2.0f,
        jackRadius * 2.0f);

    g.setColour(
        juce::Colour(
            5,
            5,
            5));

    g.drawEllipse(
        inputX - jackRadius,
        controlY - jackRadius,
        jackRadius * 2.0f,
        jackRadius * 2.0f,
        2.0f);

    //==============================================================
    // AMP LED
    //==============================================================

    constexpr float ledX =
        ampX;

    constexpr float ledY =
        controlY - 17.0f;

    constexpr float ledRadius =
        5.0f;

    if (ampIsOn)
    {
        g.setColour(
            juce::Colour(
                70,
                220,
                90));
    }
    else
    {
        g.setColour(
            juce::Colour(
                45,
                45,
                45));
    }

    g.fillEllipse(
        ledX - ledRadius,
        ledY - ledRadius,
        ledRadius * 2.0f,
        ledRadius * 2.0f);

    g.setColour(
        juce::Colours::black);

    g.drawEllipse(
        ledX - ledRadius,
        ledY - ledRadius,
        ledRadius * 2.0f,
        ledRadius * 2.0f,
        1.0f);

    //==============================================================
    // MODE INDICATOR
    //==============================================================

    const juce::String modeText =
        isDriveMode
            ? "DRIVE"
            : "CLEAN";

    juce::ignoreUnused(
        modeText);
}

//==================================================================
// RESIZED
//==================================================================

void AmpSimAudioProcessorEditor::resized()
{
    //==============================================================
    // KNOBS
    //==============================================================

    gainKnob.setBounds(
        static_cast<int>(
            gainX - knobSize * 0.5f),
        static_cast<int>(
            controlY - knobSize * 0.5f),
        static_cast<int>(knobSize),
        static_cast<int>(knobSize));

    bassKnob.setBounds(
        static_cast<int>(
            bassX - knobSize * 0.5f),
        static_cast<int>(
            controlY - knobSize * 0.5f),
        static_cast<int>(knobSize),
        static_cast<int>(knobSize));

    midKnob.setBounds(
        static_cast<int>(
            midX - knobSize * 0.5f),
        static_cast<int>(
            controlY - knobSize * 0.5f),
        static_cast<int>(knobSize),
        static_cast<int>(knobSize));

    hiKnob.setBounds(
        static_cast<int>(
            hiX - knobSize * 0.5f),
        static_cast<int>(
            controlY - knobSize * 0.5f),
        static_cast<int>(knobSize),
        static_cast<int>(knobSize));

    volumeKnob.setBounds(
        static_cast<int>(
            volumeX - knobSize * 0.5f),
        static_cast<int>(
            controlY - knobSize * 0.5f),
        static_cast<int>(knobSize),
        static_cast<int>(knobSize));

    //==============================================================
    // LABELS
    //==============================================================

    constexpr int labelWidth =
        60;

    constexpr int labelHeight =
        18;

    constexpr int labelY =
        static_cast<int>(
            controlY + 30.0f);

    inputLabel.setBounds(
        static_cast<int>(
            inputX - labelWidth * 0.5f),
        labelY,
        labelWidth,
        labelHeight);

    gainLabel.setBounds(
        static_cast<int>(
            gainX - labelWidth * 0.5f),
        labelY,
        labelWidth,
        labelHeight);

    bassLabel.setBounds(
        static_cast<int>(
            bassX - labelWidth * 0.5f),
        labelY,
        labelWidth,
        labelHeight);

    midLabel.setBounds(
        static_cast<int>(
            midX - labelWidth * 0.5f),
        labelY,
        labelWidth,
        labelHeight);

    hiLabel.setBounds(
        static_cast<int>(
            hiX - labelWidth * 0.5f),
        labelY,
        labelWidth,
        labelHeight);

    volumeLabel.setBounds(
        static_cast<int>(
            volumeX - labelWidth * 0.5f),
        labelY,
        labelWidth,
        labelHeight);

    modeLabel.setBounds(
        static_cast<int>(
            modeX - labelWidth * 0.5f),
        labelY,
        labelWidth,
        labelHeight);

    ampLabel.setBounds(
        static_cast<int>(
            ampX - labelWidth * 0.5f),
        labelY,
        labelWidth,
        labelHeight);

    //==============================================================
    // MODE SWITCH
    //==============================================================

    modeSwitch.setBounds(
        static_cast<int>(
            modeX - 26.0f),
        static_cast<int>(
            controlY - 12.0f),
        52,
        24);

    //==============================================================
    // AMP SWITCH
    //==============================================================

    ampSwitch.setBounds(
        static_cast<int>(
            ampX - 24.0f),
        static_cast<int>(
            controlY - 12.0f),
        48,
        24);
}
