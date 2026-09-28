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
        10.0,
        0.01);

    //==============================================================
    // VOLUME POPUP DISPLAY
    //==============================================================

    volumeKnob.textFromValueFunction =
        [](double value)
        {
            return juce::String(
                value / 10.0,
                1);
        };

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
        ampLabel,
        "AMP");

    //==============================================================
    // AMP SWITCH
    //==============================================================

    ampSwitch.setButtonText(
        "ON");

    ampSwitch.setToggleState(
        true,
        juce::dontSendNotification);

    ampSwitch.setClickingTogglesState(
        true);

    ampSwitch.setColour(
        juce::ToggleButton::textColourId,
        juce::Colours::white);

    ampSwitch.setColour(
        juce::ToggleButton::tickColourId,
        juce::Colours::white);

    ampSwitch.setColour(
        juce::ToggleButton::tickDisabledColourId,
        juce::Colours::darkgrey);

    ampSwitch.onClick =
        [this]()
        {
            ampIsOn =
                ampSwitch.getToggleState();

            ampSwitch.setButtonText(
                ampIsOn ? "ON" : "OFF");

            repaint();
        };

    addAndMakeVisible(
        ampSwitch);

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

    //==============================================================
    // POPUP VALUE DISPLAY
    //==============================================================

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

    //==============================================================
    // SMALL LABEL FONT
    //==============================================================

    label.setFont(
        juce::Font(
            8.0f,
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
                20,
                20,
                20));
    }

    //==============================================================
    // SCALE
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
    // INPUT JACK SOCKET
    //==============================================================

    const float jackCentreX =
        130.0f * scale;

    const float jackCentreY =
        330.0f * scale;

    const float jackRadius =
        11.0f * scale;

    //==============================================================
    // OUTER METAL RING
    //==============================================================

    g.setColour(
        juce::Colour(
            125,
            125,
            125));

    g.fillEllipse(
        jackCentreX - jackRadius,
        jackCentreY - jackRadius,
        jackRadius * 2.0f,
        jackRadius * 2.0f);

    //==============================================================
    // DARK INNER RING
    //==============================================================

    g.setColour(
        juce::Colour(
            45,
            45,
            45));

    g.fillEllipse(
        jackCentreX - jackRadius + 2.5f * scale,
        jackCentreY - jackRadius + 2.5f * scale,
        jackRadius * 2.0f - 5.0f * scale,
        jackRadius * 2.0f - 5.0f * scale);

    //==============================================================
    // CENTER HOLE
    //==============================================================

    g.setColour(
        juce::Colours::black);

    g.fillEllipse(
        jackCentreX - 4.0f * scale,
        jackCentreY - 4.0f * scale,
        8.0f * scale,
        8.0f * scale);

    //==============================================================
    // METALLIC HIGHLIGHT
    //==============================================================

    g.setColour(
        juce::Colour(
            200,
            200,
            200));

    g.fillEllipse(
        jackCentreX - 5.5f * scale,
        jackCentreY - 7.0f * scale,
        3.0f * scale,
        3.0f * scale);

    //==============================================================
    // AMP INDICATOR LED
    //
    // Right side of VOLUME
    // X = 570
    // Y = 330
    //==============================================================

    const float ledX =
        570.0f * scale;

    const float ledY =
        313.0f * scale;

    const float ledRadius =
        5.0f * scale;

    // LED outer ring
    g.setColour(
        juce::Colour(
            35,
            35,
            35));

    g.fillEllipse(
        ledX - ledRadius - 2.0f * scale,
        ledY - ledRadius - 2.0f * scale,
        (ledRadius + 2.0f * scale) * 2.0f,
        (ledRadius + 2.0f * scale) * 2.0f);

    // LED
    if (ampIsOn)
    {
        g.setColour(
            juce::Colour(
                70,
                255,
                100));
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
}

//==============================================================================
// Resized
//==============================================================================

void AmpSimAudioProcessorEditor::resized()
{
    //==============================================================
    // 800 x 500 REFERENCE
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
            36.0f * scale);

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
    // KNOBS
    //==============================================================

    setKnobPosition(
        gainKnob,
        200.0f,
        330.0f);

    setKnobPosition(
        bassKnob,
        270.0f,
        330.0f);

    setKnobPosition(
        midKnob,
        340.0f,
        330.0f);

    setKnobPosition(
        hiKnob,
        410.0f,
        330.0f);

    setKnobPosition(
        volumeKnob,
        480.0f,
        330.0f);

    //==============================================================
    // LABEL SIZE
    //==============================================================

    const int labelWidth =
        static_cast<int>(
            52.0f * scale);

    const int labelHeight =
        static_cast<int>(
            13.0f * scale);

    //==============================================================
    // LABEL Y
    //==============================================================

    const int labelY =
        static_cast<int>(
            291.0f * scale);

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
    // INPUT LABEL
    //==============================================================

    setLabelPosition(
        inputLabel,
        130.0f);

    //==============================================================
    // KNOB LABELS
    //==============================================================

    setLabelPosition(
        gainLabel,
        200.0f);

    setLabelPosition(
        bassLabel,
        270.0f);

    setLabelPosition(
        midLabel,
        340.0f);

    setLabelPosition(
        hiLabel,
        410.0f);

    setLabelPosition(
        volumeLabel,
        480.0f);

    //==============================================================
    // AMP LABEL
    //==============================================================

    setLabelPosition(
        ampLabel,
        570.0f);

    //==============================================================
    // AMP SWITCH
    //
    // Right side of VOLUME
    //==============================================================

    const int ampSwitchWidth =
        static_cast<int>(
            42.0f * scale);

    const int ampSwitchHeight =
        static_cast<int>(
            24.0f * scale);

    const int ampSwitchX =
        static_cast<int>(
            570.0f * scale
            - ampSwitchWidth * 0.5f);

    const int ampSwitchY =
        static_cast<int>(
            330.0f * scale
            - ampSwitchHeight * 0.5f);

    ampSwitch.setBounds(
        ampSwitchX,
        ampSwitchY,
        ampSwitchWidth,
        ampSwitchHeight);
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
    // RADIUS
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
    // CENTER
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
    // POINTER ANGLE
    //==============================================================

    const float angle =
        rotaryStartAngle
        + sliderPosProportional
          * (rotaryEndAngle -
             rotaryStartAngle);

    //==============================================================
    // WHITE KNOB
    //==============================================================

    g.setColour(
        juce::Colours::white);

    g.fillEllipse(
        centreX - radius,
        centreY - radius,
        radius * 2.0f,
        radius * 2.0f);

    //==============================================================
    // DARK OUTER RING
    //==============================================================

    g.setColour(
        juce::Colour(
            60,
            60,
            60));

    g.drawEllipse(
        centreX - radius,
        centreY - radius,
        radius * 2.0f,
        radius * 2.0f,
        2.0f);

    //==============================================================
    // BLACK POINTER
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
        juce::Colours::black);

    g.fillPath(
        pointer,
        juce::AffineTransform()
            .rotation(
                angle)
            .translated(
                centreX,
                centreY));
}
