#include "PluginEditor.h"
#include "BinaryData.h"

#include <cmath>

//==============================================================================
namespace
{
    constexpr float twoPi = juce::MathConstants<float>::twoPi;

    constexpr float rotaryStart =
        juce::MathConstants<float>::pi * 1.25f;

    constexpr float rotaryEnd =
        juce::MathConstants<float>::pi * 2.75f;

    // Reference knob size
    constexpr float knobSize = 72.0f;
}

//==============================================================================
// RG KNOB LOOK AND FEEL
//==============================================================================

RGKnobLookAndFeel::RGKnobLookAndFeel()
{
}

//==============================================================================
void RGKnobLookAndFeel::drawRotarySlider(
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
    const float diameter =
        static_cast<float>(
            juce::jmin(width, height));

    const float radius =
        diameter * 0.5f;

    const float centreX =
        static_cast<float>(x) +
        static_cast<float>(width) * 0.5f;

    const float centreY =
        static_cast<float>(y) +
        static_cast<float>(height) * 0.5f;

    const float knobRadius =
        radius - 4.0f;

    //==============================================================
    // Outer shadow
    //==============================================================

    g.setColour(
        juce::Colours::black.withAlpha(0.80f));

    g.fillEllipse(
        centreX - knobRadius - 2.0f,
        centreY - knobRadius + 3.0f,
        (knobRadius + 2.0f) * 2.0f,
        (knobRadius + 2.0f) * 2.0f);

    //==============================================================
    // Outer ring
    //==============================================================

    g.setColour(
        juce::Colour(20, 20, 20));

    g.fillEllipse(
        centreX - knobRadius,
        centreY - knobRadius,
        knobRadius * 2.0f,
        knobRadius * 2.0f);

    //==============================================================
    // Main knob
    //==============================================================

    const float innerRadius =
        knobRadius - 5.0f;

    g.setColour(
        juce::Colour(55, 55, 55));

    g.fillEllipse(
        centreX - innerRadius,
        centreY - innerRadius,
        innerRadius * 2.0f,
        innerRadius * 2.0f);

    //==============================================================
    // Highlight
    //==============================================================

    g.setColour(
        juce::Colour(90, 90, 90));

    g.drawEllipse(
        centreX - innerRadius,
        centreY - innerRadius,
        innerRadius * 2.0f,
        innerRadius * 2.0f,
        1.5f);

    //==============================================================
    // Position indicator
    //==============================================================

    const float angle =
        rotaryStartAngle +
        sliderPosProportional *
        (rotaryEndAngle - rotaryStartAngle);

    const float indicatorRadius =
        innerRadius - 8.0f;

    const float indicatorX =
        centreX +
        std::cos(angle) * indicatorRadius;

    const float indicatorY =
        centreY +
        std::sin(angle) * indicatorRadius;

    g.setColour(
        juce::Colours::white);

    g.drawLine(
        centreX,
        centreY,
        indicatorX,
        indicatorY,
        3.0f);

    //==============================================================
    // Center
    //==============================================================

    g.setColour(
        juce::Colour(25, 25, 25));

    g.fillEllipse(
        centreX - 6.0f,
        centreY - 6.0f,
        12.0f,
        12.0f);

    //==============================================================
    // Active indicator arc
    //==============================================================

    juce::Path arc;

    const float arcRadius =
        knobRadius + 1.0f;

    arc.addCentredArc(
        centreX,
        centreY,
        arcRadius,
        arcRadius,
        0.0f,
        rotaryStartAngle,
        angle,
        true);

    g.setColour(
        juce::Colour(80, 160, 255));

    g.strokePath(
        arc,
        juce::PathStrokeType(
            3.0f,
            juce::PathStrokeType::curved,
            juce::PathStrokeType::rounded));
}

//==============================================================================
// EDITOR
//==============================================================================

AmpSimAudioProcessorEditor::AmpSimAudioProcessorEditor(
    AmpSimAudioProcessor& p)
    : AudioProcessorEditor(&p),
      audioProcessor(p)
{
    //==============================================================
    // Load RG100 background
    //==============================================================

    backgroundImage =
        juce::ImageCache::getFromMemory(
            BinaryData::rg100jpg,
            BinaryData::rg100jpgSize);

    //==============================================================
    // Plugin window
    //==============================================================

    setSize(800, 500);

    setResizable(
        true,
        true);

    //==============================================================
    // GAIN
    //==============================================================

    setupKnob(
        gainKnob,
        gainLabel,
        "GAIN");

    //==============================================================
    // BASS
    //==============================================================

    setupKnob(
        bassKnob,
        bassLabel,
        "BASS");

    //==============================================================
    // MID
    //==============================================================

    setupKnob(
        midKnob,
        midLabel,
        "MID");

    //==============================================================
    // HI
    //==============================================================

    setupKnob(
        hiKnob,
        hiLabel,
        "HI");

    //==============================================================
    // VOLUME
    //==============================================================

    setupKnob(
        volumeKnob,
        volumeLabel,
        "VOLUME");

    //==============================================================
    // Find processor parameters
    //==============================================================

    gainParameter =
        findParameter("GAIN");

    bassParameter =
        findParameter("BASS");

    midParameter =
        findParameter("MID");

    hiParameter =
        findParameter("HI");

    volumeParameter =
        findParameter("VOLUME");

    //==============================================================
    // Connect knobs
    //==============================================================

    connectKnobToParameter(
        gainKnob,
        gainParameter,
        "GAIN");

    connectKnobToParameter(
        bassKnob,
        bassParameter,
        "BASS");

    connectKnobToParameter(
        midKnob,
        midParameter,
        "MID");

    connectKnobToParameter(
        hiKnob,
        hiParameter,
        "HI");

    connectKnobToParameter(
        volumeKnob,
        volumeParameter,
        "VOLUME");
}

//==============================================================================
AmpSimAudioProcessorEditor::~AmpSimAudioProcessorEditor()
{
}

//==============================================================================
// PAINT
//==============================================================================

void AmpSimAudioProcessorEditor::paint(
    juce::Graphics& g)
{
    //==============================================================
    // Background image
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
        g.fillAll(
            juce::Colour(30, 30, 30));
    }

    //==============================================================
    // Very subtle overlay
    //==============================================================

    g.setColour(
        juce::Colours::black.withAlpha(0.06f));

    g.fillRect(
        getLocalBounds());

    //==============================================================
    // Optional amplifier overlay
    //==============================================================

    drawAmplifierOverlay(g);
}

//==============================================================================
// AMPLIFIER OVERLAY
//==============================================================================

void AmpSimAudioProcessorEditor::drawAmplifierOverlay(
    juce::Graphics& g)
{
    const float scaleX =
        static_cast<float>(getWidth()) /
        referenceWidth;

    const float scaleY =
        static_cast<float>(getHeight()) /
        referenceHeight;

    //==============================================================
    // Keep text extremely subtle because the JPG is the main UI.
    //==============================================================

    juce::ignoreUnused(scaleX);
    juce::ignoreUnused(scaleY);
}

//==============================================================================
// RESIZED
//==============================================================================

void AmpSimAudioProcessorEditor::resized()
{
    //==============================================================
    // Scale reference 800x500 coordinates
    //==============================================================

    const float scaleX =
        static_cast<float>(getWidth()) /
        referenceWidth;

    const float scaleY =
        static_cast<float>(getHeight()) /
        referenceHeight;

    const int actualKnobSize =
        static_cast<int>(
            knobSize *
            juce::jmin(scaleX, scaleY));

    //==============================================================
    // Helper
    //==============================================================

    auto positionKnob =
        [&](juce::Slider& knob,
            juce::Label& label,
            float centreX,
            float centreY)
    {
        const int x =
            static_cast<int>(
                centreX * scaleX -
                actualKnobSize * 0.5f);

        const int y =
            static_cast<int>(
                centreY * scaleY -
                actualKnobSize * 0.5f);

        knob.setBounds(
            x,
            y,
            actualKnobSize,
            actualKnobSize);

        const int labelWidth =
            static_cast<int>(
                90.0f * scaleX);

        const int labelHeight =
            static_cast<int>(
                22.0f * scaleY);

        label.setBounds(
            static_cast<int>(
                centreX * scaleX -
                labelWidth * 0.5f),
            y + actualKnobSize + 2,
            labelWidth,
            labelHeight);
    };

    //==============================================================
    // EXACT REFERENCE COORDINATES
    //==============================================================

    positionKnob(
        gainKnob,
        gainLabel,
        gainX,
        knobY);

    positionKnob(
        bassKnob,
        bassLabel,
        bassX,
        knobY);

    positionKnob(
        midKnob,
        midLabel,
        midX,
        knobY);

    positionKnob(
        hiKnob,
        hiLabel,
        hiX,
        knobY);

    positionKnob(
        volumeKnob,
        volumeLabel,
        volumeX,
        knobY);
}

//==============================================================================
// SETUP KNOB
//==============================================================================

void AmpSimAudioProcessorEditor::setupKnob(
    juce::Slider& knob,
    juce::Label& label,
    const juce::String& text)
{
    addAndMakeVisible(knob);
    addAndMakeVisible(label);

    //==============================================================
    // Rotary
    //==============================================================

    knob.setSliderStyle(
        juce::Slider::RotaryHorizontalVerticalDrag);

    knob.setRotaryParameters(
        rotaryStart,
        rotaryEnd,
        true);

    knob.setTextBoxStyle(
        juce::Slider::NoTextBox,
        false,
        0,
        0);

    knob.setRange(
        0.0,
        1.0,
        0.001);

    knob.setValue(
        0.5,
        juce::dontSendNotification);

    knob.setLookAndFeel(
        &knobLookAndFeel);

    //==============================================================
    // Label
    //==============================================================

    label.setText(
        text,
        juce::dontSendNotification);

    label.setFont(
        juce::FontOptions(12.0f)
            .withStyle("bold"));

    label.setColour(
        juce::Label::textColourId,
        juce::Colours::white);

    label.setJustificationType(
        juce::Justification::centred);
}

//==============================================================================
// FIND PARAMETER
//==============================================================================

juce::AudioProcessorParameter*
AmpSimAudioProcessorEditor::findParameter(
    const juce::String& parameterName)
{
    for (auto* parameter :
         audioProcessor.getParameters())
    {
        if (parameter == nullptr)
            continue;

        const auto name =
            parameter->getName(100);

        if (name.equalsIgnoreCase(parameterName))
            return parameter;

        // Also accept names such as:
        // "Gain", "Amp Gain", "GAIN"
        if (name.containsIgnoreCase(parameterName))
            return parameter;
    }

    return nullptr;
}

//==============================================================================
// CONNECT KNOB
//==============================================================================

void AmpSimAudioProcessorEditor::connectKnobToParameter(
    juce::Slider& knob,
    juce::AudioProcessorParameter*& parameter,
    const juce::String& parameterName)
{
    if (parameter == nullptr)
    {
        // Parameter does not exist yet.
        // The knob remains usable as a UI control.
        knob.setValue(
            0.5,
            juce::dontSendNotification);

        knob.onValueChange = [&knob]
        {
            juce::ignoreUnused(knob);
        };

        return;
    }

    //==============================================================
    // Set knob from processor parameter
    //==============================================================

    updateKnobFromParameter(
        knob,
        parameter);

    //==============================================================
    // Send knob changes to processor
    //==============================================================

    knob.onValueChange =
        [this, &knob, parameter]
    {
        knobChanged(
            knob,
            parameter);
    };

    juce::ignoreUnused(parameterName);
}

//==============================================================================
// UPDATE KNOB FROM PARAMETER
//==============================================================================

void AmpSimAudioProcessorEditor::updateKnobFromParameter(
    juce::Slider& knob,
    juce::AudioProcessorParameter* parameter)
{
    if (parameter == nullptr)
        return;

    const float normalized =
        parameter->getValue();

    knob.setValue(
        juce::jlimit(
            0.0,
            1.0,
            static_cast<double>(normalized)),
        juce::dontSendNotification);
}

//==============================================================================
// KNOB CHANGED
//==============================================================================

void AmpSimAudioProcessorEditor::knobChanged(
    juce::Slider& knob,
    juce::AudioProcessorParameter* parameter)
{
    if (parameter == nullptr)
        return;

    const float normalized =
        static_cast<float>(
            juce::jlimit(
                0.0,
                1.0,
                knob.getValue()));

    parameter->setValueNotifyingHost(
        normalized);
}
