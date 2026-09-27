#include "PluginEditor.h"
#include "BinaryData.h"

#include <cmath>

//==============================================================
// Reference design
//==============================================================

namespace
{
    constexpr float referenceWidth  = 800.0f;
    constexpr float referenceHeight = 500.0f;

    // Knob centers from the RG100 layout
    constexpr float gainX   = 184.0f;
    constexpr float bassX   = 292.0f;
    constexpr float midX    = 400.0f;
    constexpr float hiX     = 508.0f;
    constexpr float volumeX = 616.0f;

    constexpr float knobY = 330.0f;

    constexpr float knobSize = 72.0f;

    constexpr float labelOffset = 50.0f;
}

//==============================================================
// Constructor
//==============================================================

AmpSimAudioProcessorEditor::AmpSimAudioProcessorEditor(
    AmpSimAudioProcessor& p)
    : AudioProcessorEditor(&p),
      audioProcessor(p)
{
    //==========================================================
    // Load RG100 background
    //==========================================================

    int imageSize = 0;

    if (const void* imageData =
            BinaryData::getNamedResource(
                "rg100jpg",
                imageSize))
    {
        backgroundImage =
            juce::ImageFileFormatManager()
                .getInstance()
                .findImageFormatForFileExtension(".jpg")
                ->decodeImage(
                    imageData,
                    imageSize);
    }

    //==========================================================
    // Setup knobs
    //==========================================================

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
        -24.0,
        12.0,
        0.01);

    //==========================================================
    // Labels
    //==========================================================

    setupLabel(gainLabel, "GAIN");
    setupLabel(bassLabel, "BASS");
    setupLabel(midLabel, "MID");
    setupLabel(hiLabel, "HI");
    setupLabel(volumeLabel, "VOLUME");

    //==========================================================
    // APVTS connections
    //==========================================================

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

    //==========================================================
    // Window
    //==========================================================

    setSize(
        static_cast<int>(referenceWidth),
        static_cast<int>(referenceHeight));

    setResizable(
        true,
        true);

    setResizeLimits(
        600,
        375,
        1600,
        1000);
}

//==============================================================
// Destructor
//==============================================================

AmpSimAudioProcessorEditor::~AmpSimAudioProcessorEditor()
{
    gainAttachment.reset();
    bassAttachment.reset();
    midAttachment.reset();
    hiAttachment.reset();
    volumeAttachment.reset();

    gainKnob.setLookAndFeel(nullptr);
    bassKnob.setLookAndFeel(nullptr);
    midKnob.setLookAndFeel(nullptr);
    hiKnob.setLookAndFeel(nullptr);
    volumeKnob.setLookAndFeel(nullptr);
}

//==============================================================
// Setup knob
//==============================================================

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
        (min + max) * 0.5);

    slider.setLookAndFeel(
        &knobLookAndFeel);

    slider.setDoubleClickReturnValue(
        true,
        (min + max) * 0.5);

    addAndMakeVisible(slider);
}

//==============================================================
// Setup label
//==============================================================

void AmpSimAudioProcessorEditor::setupLabel(
    juce::Label& label,
    const juce::String& text)
{
    label.setText(
        text,
        juce::dontSendNotification);

    label.setJustificationType(
        juce::Justification::centred);

    label.setFont(
        juce::Font(
            14.0f,
            juce::Font::bold));

    label.setColour(
        juce::Label::textColourId,
        juce::Colours::white);

    label.setInterceptsMouseClicks(
        false,
        false);

    addAndMakeVisible(label);
}

//==============================================================
// Knob drawing
//==============================================================

void AmpSimAudioProcessorEditor::RGKnobLookAndFeel::drawRotarySlider(
    juce::Graphics& g,
    int x,
    int y,
    int width,
    int height,
    float sliderPosProportional,
    float rotaryStartAngle,
    float rotaryEndAngle,
    juce::Slider&)
{
    const float size =
        static_cast<float>(
            juce::jmin(width, height));

    const float cx =
        static_cast<float>(x)
        + static_cast<float>(width) * 0.5f;

    const float cy =
        static_cast<float>(y)
        + static_cast<float>(height) * 0.5f;

    const float radius =
        size * 0.42f;

    //==========================================================
    // Outer dark ring
    //==========================================================

    g.setColour(
        juce::Colour(15, 16, 18));

    g.fillEllipse(
        cx - radius,
        cy - radius,
        radius * 2.0f,
        radius * 2.0f);

    //==========================================================
    // Inner knob
    //==========================================================

    const float innerRadius =
        radius * 0.82f;

    g.setColour(
        juce::Colour(35, 37, 40));

    g.fillEllipse(
        cx - innerRadius,
        cy - innerRadius,
        innerRadius * 2.0f,
        innerRadius * 2.0f);

    //==========================================================
    // Active blue arc
    //==========================================================

    const float arcThickness =
        juce::jmax(
            3.0f,
            size * 0.045f);

    const float startAngle =
        rotaryStartAngle;

    const float endAngle =
        rotaryStartAngle
        + sliderPosProportional
            * (rotaryEndAngle - rotaryStartAngle);

    juce::Path arc;

    arc.addCentredArc(
        cx,
        cy,
        radius * 0.92f,
        radius * 0.92f,
        0.0f,
        startAngle,
        endAngle,
        true);

    g.setColour(
        juce::Colour(45, 130, 255));

    g.strokePath(
        arc,
        juce::PathStrokeType(
            arcThickness,
            juce::PathStrokeType::curved,
            juce::PathStrokeType::rounded));

    //==========================================================
    // Indicator
    //==========================================================

    const float angle =
        rotaryStartAngle
        + sliderPosProportional
            * (rotaryEndAngle - rotaryStartAngle);

    const float indicatorLength =
        radius * 0.58f;

    const float indicatorX =
        cx + std::cos(angle) * indicatorLength;

    const float indicatorY =
        cy + std::sin(angle) * indicatorLength;

    g.setColour(
        juce::Colours::white);

    g.drawLine(
        cx,
        cy,
        indicatorX,
        indicatorY,
        juce::jmax(
            2.0f,
            size * 0.035f));
}

//==============================================================
// Paint
//==============================================================

void AmpSimAudioProcessorEditor::paint(
    juce::Graphics& g)
{
    //==========================================================
    // Background image
    //==========================================================

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
            juce::Colour(20, 20, 20));
    }
}

//==============================================================
// Resized
//==============================================================

void AmpSimAudioProcessorEditor::resized()
{
    const float scaleX =
        static_cast<float>(getWidth())
        / referenceWidth;

    const float scaleY =
        static_cast<float>(getHeight())
        / referenceHeight;

    // Keep proportions based on the 800x500 design.
    const float scale =
        juce::jmin(scaleX, scaleY);

    const float offsetX =
        (static_cast<float>(getWidth())
         - referenceWidth * scale) * 0.5f;

    const float offsetY =
        (static_cast<float>(getHeight())
         - referenceHeight * scale) * 0.5f;

    auto placeKnob =
        [scale, offsetX, offsetY](
            juce::Slider& knob,
            float centerX,
            float centerY)
    {
        const float size =
            knobSize * scale;

        const float x =
            offsetX
            + centerX * scale
            - size * 0.5f;

        const float y =
            offsetY
            + centerY * scale
            - size * 0.5f;

        knob.setBounds(
            juce::roundToInt(x),
            juce::roundToInt(y),
            juce::roundToInt(size),
            juce::roundToInt(size));
    };

    placeKnob(
        gainKnob,
        gainX,
        knobY);

    placeKnob(
        bassKnob,
        bassX,
        knobY);

    placeKnob(
        midKnob,
        midX,
        knobY);

    placeKnob(
        hiKnob,
        hiX,
        knobY);

    placeKnob(
        volumeKnob,
        volumeX,
        knobY);

    //==========================================================
    // Labels
    //==========================================================

    auto placeLabel =
        [scale, offsetX, offsetY](
            juce::Label& label,
            float centerX,
            float centerY)
    {
        const float width =
            90.0f * scale;

        const float height =
            22.0f * scale;

        const float x =
            offsetX
            + centerX * scale
            - width * 0.5f;

        const float y =
            offsetY
            + centerY * scale
            + labelOffset * scale;

        label.setBounds(
            juce::roundToInt(x),
            juce::roundToInt(y),
            juce::roundToInt(width),
            juce::roundToInt(height));

        label.setFont(
            juce::Font(
                juce::jmax(
                    10.0f,
                    14.0f * scale),
                juce::Font::bold));
    };

    placeLabel(
        gainLabel,
        gainX,
        knobY);

    placeLabel(
        bassLabel,
        bassX,
        knobY);

    placeLabel(
        midLabel,
        midX,
        knobY);

    placeLabel(
        hiLabel,
        hiX,
        knobY);

    placeLabel(
        volumeLabel,
        volumeX,
        knobY);
}
