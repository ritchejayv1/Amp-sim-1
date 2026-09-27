#include "PluginEditor.h"

#include "BinaryData.h"

//==============================================================================
// Constructor
//==============================================================================

AmpSimAudioProcessorEditor::AmpSimAudioProcessorEditor(
    AmpSimAudioProcessor& p)
    : AudioProcessorEditor(&p),
      audioProcessor(p)
{
    //==============================================================
    // RG100 BACKGROUND IMAGE
    //==============================================================

    int imageSize = 0;

    const void* imageData =
        BinaryData::getNamedResource(
            "rg100_jpg",
            imageSize);

    if (imageData != nullptr &&
        imageSize > 0)
    {
        backgroundImage =
            juce::ImageCache::getFromMemory(
                imageData,
                imageSize);
    }

    //==============================================================
    // GAIN
    //==============================================================

    setupKnob(
        gainKnob,
        0.0,
        10.0,
        0.01);

    //==============================================================
    // BASS
    //==============================================================

    setupKnob(
        bassKnob,
        -12.0,
        12.0,
        0.01);

    //==============================================================
    // MID
    //==============================================================

    setupKnob(
        midKnob,
        -12.0,
        12.0,
        0.01);

    //==============================================================
    // HI
    //==============================================================

    setupKnob(
        hiKnob,
        -12.0,
        12.0,
        0.01);

    //==============================================================
    // VOLUME
    //==============================================================

    setupKnob(
        volumeKnob,
        0.0,
        24.0,
        0.01);

    //==============================================================
    // LABELS
    //==============================================================

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

    //==============================================================
    // PARAMETER ATTACHMENTS
    //==============================================================

    gainAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::
                SliderAttachment>(
                    audioProcessor.parameters,
                    "GAIN",
                    gainKnob);

    bassAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::
                SliderAttachment>(
                    audioProcessor.parameters,
                    "BASS",
                    bassKnob);

    midAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::
                SliderAttachment>(
                    audioProcessor.parameters,
                    "MID",
                    midKnob);

    hiAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::
                SliderAttachment>(
                    audioProcessor.parameters,
                    "HI",
                    hiKnob);

    volumeAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::
                SliderAttachment>(
                    audioProcessor.parameters,
                    "VOLUME",
                    volumeKnob);

    //==============================================================
    // KNOB LOOK AND FEEL
    //==============================================================

    gainKnob.setLookAndFeel(
        &knobLookAndFeel);

    bassKnob.setLookAndFeel(
        &knobLookAndFeel);

    midKnob.setLookAndFeel(
        &knobLookAndFeel);

    hiKnob.setLookAndFeel(
        &knobLookAndFeel);

    volumeKnob.setLookAndFeel(
        &knobLookAndFeel);

    //==============================================================
    // EDITOR SIZE
    //==============================================================

    setSize(
        800,
        500);
}

//==============================================================================
// Destructor
//==============================================================================

AmpSimAudioProcessorEditor::~AmpSimAudioProcessorEditor()
{
    gainKnob.setLookAndFeel(nullptr);
    bassKnob.setLookAndFeel(nullptr);
    midKnob.setLookAndFeel(nullptr);
    hiKnob.setLookAndFeel(nullptr);
    volumeKnob.setLookAndFeel(nullptr);
}

//==============================================================================
// Setup Knob
//==============================================================================

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

    slider.setDoubleClickReturnValue(
        true,
        min);

    slider.setPopupDisplayEnabled(
        true,
        true,
        this);

    addAndMakeVisible(
        slider);
}

//==============================================================================
// Setup Label
//==============================================================================

void AmpSimAudioProcessorEditor::setupLabel(
    juce::Label& label,
    const juce::String& text)
{
    label.setText(
        text,
        juce::dontSendNotification);

    label.setJustificationType(
        juce::Justification::centred);

    label.setColour(
        juce::Label::textColourId,
        juce::Colours::white);

    label.setFont(
        juce::Font(
            13.0f,
            juce::Font::bold));

    addAndMakeVisible(
        label);
}

//==============================================================================
// Paint
//==============================================================================

void AmpSimAudioProcessorEditor::paint(
    juce::Graphics& g)
{
    //==============================================================
    // RG100 IMAGE
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
        //==========================================================
        // Fallback
        //==========================================================

        g.fillAll(
            juce::Colour(
                20,
                20,
                20));
    }
}

//==============================================================================
// Resized
//==============================================================================

void AmpSimAudioProcessorEditor::resized()
{
    //==============================================================
    // ORIGINAL REFERENCE
    //
    // 800 x 500
    //
    // GAIN    184,330
    // BASS    292,330
    // MID     400,330
    // HI      508,330
    // VOLUME  616,330
    //
    //==============================================================

    const float scaleX =
        static_cast<float>(
            getWidth())
        / 800.0f;

    const float scaleY =
        static_cast<float>(
            getHeight())
        / 500.0f;

    const float scale =
        juce::jmin(
            scaleX,
            scaleY);

    //==============================================================
    // KNOB SIZE
    //==============================================================

    const int knobSize =
        static_cast<int>(
            76.0f * scale);

    //==============================================================
    // KNOB POSITION
    //==============================================================

    auto setKnobPosition =
        [scale, knobSize](
            juce::Slider& knob,
            float centerX,
            float centerY)
        {
            const int x =
                static_cast<int>(
                    centerX * scale
                    - knobSize * 0.5f);

            const int y =
                static_cast<int>(
                    centerY * scale
                    - knobSize * 0.5f);

            knob.setBounds(
                x,
                y,
                knobSize,
                knobSize);
        };

    //==============================================================
    // GAIN
    //==============================================================

    setKnobPosition(
        gainKnob,
        184.0f,
        330.0f);

    //==============================================================
    // BASS
    //==============================================================

    setKnobPosition(
        bassKnob,
        292.0f,
        330.0f);

    //==============================================================
    // MID
    //==============================================================

    setKnobPosition(
        midKnob,
        400.0f,
        330.0f);

    //==============================================================
    // HI
    //==============================================================

    setKnobPosition(
        hiKnob,
        508.0f,
        330.0f);

    //==============================================================
    // VOLUME
    //==============================================================

    setKnobPosition(
        volumeKnob,
        616.0f,
        330.0f);

    //==============================================================
    // LABEL SIZE
    //==============================================================

    const int labelWidth =
        static_cast<int>(
            86.0f * scale);

    const int labelHeight =
        static_cast<int>(
            20.0f * scale);

    const int labelY =
        static_cast<int>(
            374.0f * scale);

    //==============================================================
    // LABEL POSITION
    //==============================================================

    auto setLabelPosition =
        [scale,
         labelWidth,
         labelHeight,
         labelY](
            juce::Label& label,
            float centerX)
        {
            const int x =
                static_cast<int>(
                    centerX * scale
                    - labelWidth * 0.5f);

            label.setBounds(
                x,
                labelY,
                labelWidth,
                labelHeight);
        };

    //==============================================================
    // LABELS
    //==============================================================

    setLabelPosition(
        gainLabel,
        184.0f);

    setLabelPosition(
        bassLabel,
        292.0f);

    setLabelPosition(
        midLabel,
        400.0f);

    setLabelPosition(
        hiLabel,
        508.0f);

    setLabelPosition(
        volumeLabel,
        616.0f);
}

//==============================================================================
// Custom Knob Look And Feel
//==============================================================================

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
    //==============================================================
    // Radius
    //==============================================================

    const float radius =
        juce::jmin(
            static_cast<float>(
                width),
            static_cast<float>(
                height))
        * 0.5f
        - 5.0f;

    //==============================================================
    // Center
    //==============================================================

    const float centreX =
        static_cast<float>(x)
        + static_cast<float>(width)
          * 0.5f;

    const float centreY =
        static_cast<float>(y)
        + static_cast<float>(height)
          * 0.5f;

    //==============================================================
    // Pointer angle
    //==============================================================

    const float angle =
        rotaryStartAngle
        + sliderPosProportional
          * (rotaryEndAngle -
             rotaryStartAngle);

    //==============================================================
    // OUTER KNOB
    //==============================================================

    g.setColour(
        juce::Colour(
            18,
            18,
            18));

    g.fillEllipse(
        centreX - radius,
        centreY - radius,
        radius * 2.0f,
        radius * 2.0f);

    //==============================================================
    // OUTER RING
    //==============================================================

    g.setColour(
        juce::Colour(
            80,
            80,
            80));

    g.drawEllipse(
        centreX - radius,
        centreY - radius,
        radius * 2.0f,
        radius * 2.0f,
        2.0f);

    //==============================================================
    // POINTER
    //==============================================================

    const float pointerLength =
        radius * 0.70f;

    const float pointerThickness =
        3.0f;

    juce::Path pointer;

    pointer.addRoundedRectangle(
        -pointerThickness * 0.5f,
        -pointerLength,
        pointerThickness,
        pointerLength,
        1.5f);

    g.setColour(
        juce::Colours::white);

    g.fillPath(
        pointer,
        juce::AffineTransform()
            .rotation(
                angle)
            .translated(
                centreX,
                centreY));

    //==============================================================
    // CENTER
    //==============================================================

    const float centerRadius =
        radius * 0.12f;

    g.setColour(
        juce::Colour(
            45,
            45,
            45));

    g.fillEllipse(
        centreX - centerRadius,
        centreY - centerRadius,
        centerRadius * 2.0f,
        centerRadius * 2.0f);
}
