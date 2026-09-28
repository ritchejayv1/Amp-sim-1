#include "PluginEditor.h"

#include <cmath>

#include "BinaryData.h"

namespace
{
    //==============================================================
    // WINDOW
    //==============================================================

    constexpr int editorWidth  = 800;
    constexpr int editorHeight = 500;

    //==============================================================
    // CONTROL POSITIONS
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

    constexpr float knobSize = 36.0f;

    //==============================================================
    // KNOB ROTATION
    //==============================================================

    constexpr float knobStartAngle =
        juce::MathConstants<float>::pi * 1.25f;

    constexpr float knobEndAngle =
        juce::MathConstants<float>::pi * 2.75f;

    //==============================================================
    // TOGGLE LOOK AND FEEL
    //==============================================================

    class RGToggleLookAndFeel
        : public juce::LookAndFeel_V4
    {
    public:

        void drawToggleButton(
            juce::Graphics& g,
            juce::ToggleButton& button,
            bool shouldDrawButtonAsHighlighted,
            bool shouldDrawButtonAsDown) override
        {
            juce::ignoreUnused(
                shouldDrawButtonAsHighlighted,
                shouldDrawButtonAsDown);

            const auto bounds =
                button.getLocalBounds().toFloat();

            const float w = bounds.getWidth();
            const float h = bounds.getHeight();

            const bool on =
                button.getToggleState();

            const bool isAmp =
                button.getButtonText()
                    .equalsIgnoreCase("ON")
                || button.getButtonText()
                    .equalsIgnoreCase("OFF");

            //======================================================
            // MODE = SQUARE PUSH BUTTON
            //======================================================

            if (! isAmp)
            {
                const float buttonSize =
                    juce::jmin(
                        28.0f,
                        h - 2.0f);

                const float bx =
                    (w - buttonSize) * 0.5f;

                const float by =
                    (h - buttonSize) * 0.5f;

                //==================================================
                // RECESSED METAL PANEL
                //==================================================

                g.setColour(
                    juce::Colour(
                        7,
                        7,
                        7));

                g.fillRoundedRectangle(
                    1.0f,
                    2.0f,
                    w - 2.0f,
                    h - 4.0f,
                    3.0f);

                g.setColour(
                    juce::Colour(
                        105,
                        105,
                        105));

                g.drawRoundedRectangle(
                    1.0f,
                    2.0f,
                    w - 2.0f,
                    h - 4.0f,
                    3.0f,
                    1.0f);

                //==================================================
                // BUTTON SHADOW / RECESS
                //==================================================

                g.setColour(
                    juce::Colour(
                        0,
                        0,
                        0));

                g.fillRoundedRectangle(
                    bx - 1.5f,
                    by + 2.0f,
                    buttonSize + 3.0f,
                    buttonSize + 2.0f,
                    3.0f);

                //==================================================
                // PUSH BUTTON BODY
                //==================================================

                if (on)
                {
                    // DRIVE ENGAGED

                    g.setColour(
                        juce::Colour(
                            18,
                            75,
                            145));

                    g.fillRoundedRectangle(
                        bx,
                        by + 1.5f,
                        buttonSize,
                        buttonSize - 1.5f,
                        2.5f);

                    // Blue edge

                    g.setColour(
                        juce::Colour(
                            70,
                            150,
                            255));

                    g.drawRoundedRectangle(
                        bx,
                        by + 1.5f,
                        buttonSize,
                        buttonSize - 1.5f,
                        2.5f,
                        1.0f);

                    // Active center

                    g.setColour(
                        juce::Colour(
                            45,
                            125,
                            235));

                    g.fillRoundedRectangle(
                        bx + 4.0f,
                        by + 4.0f,
                        buttonSize - 8.0f,
                        buttonSize - 8.0f,
                        1.5f);

                    // Highlight

                    g.setColour(
                        juce::Colour(
                            130,
                            195,
                            255)
                        .withAlpha(0.65f));

                    g.drawLine(
                        bx + 5.0f,
                        by + 5.0f,
                        bx + buttonSize - 5.0f,
                        by + 5.0f,
                        1.0f);
                }
                else
                {
                    // CLEAN / RELEASED

                    g.setColour(
                        juce::Colour(
                            42,
                            42,
                            42));

                    g.fillRoundedRectangle(
                        bx,
                        by,
                        buttonSize,
                        buttonSize,
                        2.5f);

                    g.setColour(
                        juce::Colour(
                            125,
                            125,
                            125));

                    g.drawRoundedRectangle(
                        bx,
                        by,
                        buttonSize,
                        buttonSize,
                        2.5f,
                        1.0f);

                    // Top highlight

                    g.setColour(
                        juce::Colour(
                            180,
                            180,
                            180)
                        .withAlpha(0.45f));

                    g.drawLine(
                        bx + 4.0f,
                        by + 3.0f,
                        bx + buttonSize - 4.0f,
                        by + 3.0f,
                        1.0f);
                }

                //==================================================
                // CENTER INDICATOR
                //==================================================

                const float ledRadius =
                    3.0f;

                const float ledX =
                    w * 0.5f;

                const float ledY =
                    by + buttonSize * 0.5f;

                g.setColour(
                    on
                        ? juce::Colour(
                            105,
                            190,
                            255)
                        : juce::Colour(
                            25,
                            25,
                            25));

                g.fillEllipse(
                    ledX - ledRadius,
                    ledY - ledRadius,
                    ledRadius * 2.0f,
                    ledRadius * 2.0f);

                if (on)
                {
                    g.setColour(
                        juce::Colour(
                            170,
                            225,
                            255)
                        .withAlpha(0.8f));

                    g.fillEllipse(
                        ledX - 1.3f,
                        ledY - 1.3f,
                        2.6f,
                        2.6f);
                }

                //==================================================
                // CLEAN / DRIVE TEXT
                //==================================================

                g.setFont(
                    juce::Font(
                        5.5f,
                        juce::Font::bold));

                g.setColour(
                    on
                        ? juce::Colour(
                            100,
                            100,
                            100)
                        : juce::Colours::white);

                g.drawText(
                    "CLEAN",
                    2,
                    static_cast<int>(h - 8.0f),
                    static_cast<int>(w * 0.5f - 2.0f),
                    7,
                    juce::Justification::centred);

                g.setColour(
                    on
                        ? juce::Colours::white
                        : juce::Colour(
                            100,
                            100,
                            100));

                g.drawText(
                    "DRIVE",
                    static_cast<int>(w * 0.5f + 2.0f),
                    static_cast<int>(h - 8.0f),
                    static_cast<int>(w * 0.5f - 4.0f),
                    7,
                    juce::Justification::centred);

                return;
            }

            //======================================================
            // AMP SWITCH
            //======================================================

            const float centerX =
                w * 0.5f;

            const float switchY =
                on
                    ? 10.0f
                    : h - 11.0f;

            //======================================================
            // VERTICAL SLOT
            //======================================================

            g.setColour(
                juce::Colour(
                    2,
                    2,
                    2));

            g.fillRoundedRectangle(
                centerX - 3.0f,
                7.0f,
                6.0f,
                h - 14.0f,
                3.0f);

            //======================================================
            // SLOT HIGHLIGHT
            //======================================================

            g.setColour(
                juce::Colour(
                    75,
                    75,
                    75));

            g.drawLine(
                centerX - 0.5f,
                8.0f,
                centerX - 0.5f,
                h - 8.0f,
                1.0f);

            //======================================================
            // METAL WASHER
            //======================================================

            g.setColour(
                juce::Colour(
                    55,
                    55,
                    55));

            g.fillEllipse(
                centerX - 7.0f,
                switchY - 7.0f,
                14.0f,
                14.0f);

            g.setColour(
                juce::Colour(
                    180,
                    180,
                    180));

            g.drawEllipse(
                centerX - 7.0f,
                switchY - 7.0f,
                14.0f,
                14.0f,
                1.0f);

            //======================================================
            // METAL LEVER
            //======================================================

            const float leverLength =
                11.0f;

            const float endY =
                on
                    ? switchY - leverLength
                    : switchY + leverLength;

            //======================================================
            // LEVER SHADOW
            //======================================================

            g.setColour(
                juce::Colour(
                    0,
                    0,
                    0)
                    .withAlpha(0.85f));

            g.drawLine(
                centerX + 1.2f,
                switchY + 1.2f,
                centerX + 1.2f,
                endY + 1.2f,
                4.0f);

            //======================================================
            // LEVER
            //======================================================

            g.setColour(
                juce::Colour(
                    210,
                    210,
                    210));

            g.drawLine(
                centerX,
                switchY,
                centerX,
                endY,
                3.6f);

            //======================================================
            // HIGHLIGHT
            //======================================================

            g.setColour(
                juce::Colour(
                    250,
                    250,
                    250));

            g.drawLine(
                centerX - 0.5f,
                switchY - 0.5f,
                centerX - 0.5f,
                endY - 0.5f,
                0.9f);

            //======================================================
            // LEVER TIP
            //======================================================

            g.setColour(
                juce::Colour(
                    150,
                    150,
                    150));

            g.fillEllipse(
                centerX - 4.0f,
                endY - 4.0f,
                8.0f,
                8.0f);

            g.setColour(
                juce::Colour(
                    235,
                    235,
                    235));

            g.fillEllipse(
                centerX - 2.4f,
                endY - 2.4f,
                4.8f,
                4.8f);

            //======================================================
            // ON / OFF
            //======================================================

            g.setFont(
                juce::Font(
                    5.5f,
                    juce::Font::bold));

            g.setColour(
                on
                    ? juce::Colour(
                        95,
                        225,
                        105)
                    : juce::Colour(
                        135,
                        135,
                        135));

            g.drawText(
                on ? "ON" : "OFF",
                2,
                static_cast<int>(h - 9.0f),
                static_cast<int>(w - 4.0f),
                7,
                juce::Justification::centred);
        }
    };

    RGToggleLookAndFeel toggleLookAndFeel;
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
    setSize(
        editorWidth,
        editorHeight);

    //==============================================================
    // BACKGROUND
    //==============================================================

    backgroundImage =
        juce::ImageCache::getFromMemory(
            BinaryData::rg100_jpg,
            BinaryData::rg100_jpgSize);

    //==============================================================
    // KNOBS
    //==============================================================

    setupKnob(gainKnob, 0.0, 10.0, 0.01);
    setupKnob(bassKnob, -12.0, 12.0, 0.01);
    setupKnob(midKnob, -12.0, 12.0, 0.01);
    setupKnob(hiKnob, -12.0, 12.0, 0.01);
    setupKnob(volumeKnob, 0.0, 10.0, 0.01);

    //==============================================================
    // LABELS
    //==============================================================

    setupLabel(inputLabel, "INPUT");
    setupLabel(gainLabel, "GAIN");
    setupLabel(bassLabel, "BASS");
    setupLabel(midLabel, "MID");
    setupLabel(hiLabel, "HI");
    setupLabel(volumeLabel, "VOLUME");
    setupLabel(modeLabel, "MODE");
    setupLabel(ampLabel, "AMP");

    //==============================================================
    // MODE PUSH BUTTON
    //==============================================================

    modeSwitch.setButtonText("CLEAN");

    modeSwitch.setClickingTogglesState(true);

    modeSwitch.setToggleState(
        false,
        juce::dontSendNotification);

    modeSwitch.setLookAndFeel(
        &toggleLookAndFeel);

    addAndMakeVisible(modeSwitch);

    modeAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::ButtonAttachment>(
                audioProcessor.parameters,
                "MODE",
                modeSwitch);

    modeSwitch.onClick =
        [this]()
        {
            isDriveMode =
                modeSwitch.getToggleState();

            modeSwitch.setButtonText(
                isDriveMode
                    ? "DRIVE"
                    : "CLEAN");

            modeSwitch.repaint();
        };

    //==============================================================
    // AMP SWITCH
    //==============================================================

    ampSwitch.setButtonText("ON");

    ampSwitch.setClickingTogglesState(true);

    ampSwitch.setToggleState(
        true,
        juce::dontSendNotification);

    ampSwitch.setLookAndFeel(
        &toggleLookAndFeel);

    addAndMakeVisible(ampSwitch);

    ampAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::ButtonAttachment>(
                audioProcessor.parameters,
                "AMP",
                ampSwitch);

    ampSwitch.onClick =
        [this]()
        {
            ampIsOn =
                ampSwitch.getToggleState();

            ampSwitch.setButtonText(
                ampIsOn
                    ? "ON"
                    : "OFF");

            ampSwitch.repaint();
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
    // INITIAL STATE
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
    modeSwitch.setLookAndFeel(nullptr);
    ampSwitch.setLookAndFeel(nullptr);
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

    slider.setRotaryParameters(
        knobStartAngle,
        knobEndAngle,
        false);

    slider.setTextBoxStyle(
        juce::Slider::NoTextBox,
        false,
        0,
        0);

    slider.setRange(
        min,
        max,
        interval);

    slider.setLookAndFeel(
        &knobLookAndFeel);

    slider.setPopupDisplayEnabled(
        true,
        true,
        this);

    addAndMakeVisible(slider);
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

    addAndMakeVisible(label);
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
    juce::ignoreUnused(slider);

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
        knobRadius * 0.68f;

    const float pointerX =
        cx
        + std::cos(angle)
          * pointerLength;

    const float pointerY =
        cy
        + std::sin(angle)
          * pointerLength;

    //==============================================================
    // POINTER SHADOW
    //==============================================================

    g.setColour(
        juce::Colour(
            0,
            0,
            0)
            .withAlpha(0.35f));

    g.drawLine(
        cx + 0.8f,
        cy + 0.8f,
        pointerX + 0.8f,
        pointerY + 0.8f,
        2.8f);

    //==============================================================
    // POINTER
    //==============================================================

    g.setColour(
        juce::Colours::black);

    g.drawLine(
        cx,
        cy,
        pointerX,
        pointerY,
        2.2f);

    //==============================================================
    // CENTER CAP
    //==============================================================

    g.setColour(
        juce::Colour(
            35,
            35,
            35));

    g.fillEllipse(
        cx - 2.0f,
        cy - 2.0f,
        4.0f,
        4.0f);
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

    g.setColour(
        ampIsOn
            ? juce::Colour(
                70,
                220,
                90)
            : juce::Colour(
                45,
                45,
                45));

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
    // LABELS ABOVE CONTROLS
    //==============================================================

    constexpr int labelWidth =
        60;

    constexpr int labelHeight =
        18;

    constexpr int labelY =
        static_cast<int>(
            controlY - 42.0f);

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
    // MODE SQUARE PUSH BUTTON
    //==============================================================

    modeSwitch.setBounds(
        static_cast<int>(
            modeX - 32.0f),
        static_cast<int>(
            controlY - 15.0f),
        64,
        30);

    //==============================================================
    // AMP TOGGLE
    //==============================================================

    ampSwitch.setBounds(
        static_cast<int>(
            ampX - 26.0f),
        static_cast<int>(
            controlY - 15.0f),
        52,
        30);
}
