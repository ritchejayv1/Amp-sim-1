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
    // PROPORTIONAL CONTROL POSITIONS
    //
    // INPUT → GAIN → MODE → BASS → MID → HI → VOLUME → AMP
    //==============================================================

    constexpr float inputX  = 100.0f;
    constexpr float gainX   = 190.0f;
    constexpr float modeX   = 280.0f;
    constexpr float bassX   = 370.0f;
    constexpr float midX    = 460.0f;
    constexpr float hiX     = 550.0f;
    constexpr float volumeX = 640.0f;
    constexpr float ampX    = 730.0f;

    constexpr float controlY = 330.0f;

    constexpr float knobSize = 36.0f;

    //==============================================================
    // KNOB ROTATION
    //
    // 270° total sweep
    // Minimum = approximately 7 o'clock
    // Center  = 12 o'clock
    // Maximum = approximately 5 o'clock
    //==============================================================

    constexpr float knobStartAngle =
        juce::MathConstants<float>::pi * 0.75f;

    constexpr float knobEndAngle =
        juce::MathConstants<float>::pi * 2.25f;

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
                button.getComponentID()
                    == "AMP_SWITCH";

            //======================================================
            // MODE
            //
            // Released = CLEAN
            // Pressed  = DRIVE
            //======================================================

            if (! isAmp)
            {
                constexpr float buttonSize = 16.0f;

                const float bx =
                    (w - buttonSize) * 0.5f;

                const float by =
                    (h - buttonSize) * 0.5f;

                // Shadow

                g.setColour(
                    juce::Colour(
                        0,
                        0,
                        0)
                        .withAlpha(0.9f));

                g.fillRoundedRectangle(
                    bx + 1.2f,
                    by + 1.8f,
                    buttonSize,
                    buttonSize,
                    2.0f);

                //==================================================
                // DRIVE / PRESSED
                //==================================================

                if (on)
                {
                    g.setColour(
                        juce::Colour(
                            20,
                            75,
                            145));

                    g.fillRoundedRectangle(
                        bx,
                        by + 1.0f,
                        buttonSize,
                        buttonSize - 1.0f,
                        2.0f);

                    // Blue edge

                    g.setColour(
                        juce::Colour(
                            85,
                            165,
                            255));

                    g.drawRoundedRectangle(
                        bx,
                        by + 1.0f,
                        buttonSize,
                        buttonSize - 1.0f,
                        2.0f,
                        0.8f);

                    // Inner surface

                    g.setColour(
                        juce::Colour(
                            45,
                            125,
                            225));

                    g.fillRoundedRectangle(
                        bx + 2.0f,
                        by + 3.0f,
                        buttonSize - 4.0f,
                        buttonSize - 5.0f,
                        1.2f);

                    // Highlight

                    g.setColour(
                        juce::Colour(
                            180,
                            220,
                            255)
                        .withAlpha(0.65f));

                    g.drawLine(
                        bx + 3.0f,
                        by + 3.0f,
                        bx + buttonSize - 3.0f,
                        by + 3.0f,
                        0.8f);
                }
                else
                {
                    //==================================================
                    // CLEAN / RELEASED
                    //==================================================

                    g.setColour(
                        juce::Colour(
                            45,
                            45,
                            45));

                    g.fillRoundedRectangle(
                        bx,
                        by,
                        buttonSize,
                        buttonSize,
                        2.0f);

                    // Metal edge

                    g.setColour(
                        juce::Colour(
                            145,
                            145,
                            145));

                    g.drawRoundedRectangle(
                        bx,
                        by,
                        buttonSize,
                        buttonSize,
                        2.0f,
                        0.8f);

                    // Highlight

                    g.setColour(
                        juce::Colour(
                            205,
                            205,
                            205)
                        .withAlpha(0.4f));

                    g.drawLine(
                        bx + 2.0f,
                        by + 2.0f,
                        bx + buttonSize - 2.0f,
                        by + 2.0f,
                        0.8f);
                }

                //==================================================
                // MODE CENTER LED
                //==================================================

                const float ledRadius = 2.0f;

                const float ledX =
                    bx + buttonSize * 0.5f;

                const float ledY =
                    by + buttonSize * 0.5f;

                g.setColour(
                    on
                        ? juce::Colour(
                            125,
                            210,
                            255)
                        : juce::Colour(
                            18,
                            18,
                            18));

                g.fillEllipse(
                    ledX - ledRadius,
                    ledY - ledRadius,
                    ledRadius * 2.0f,
                    ledRadius * 2.0f);

                if (on)
                {
                    g.setColour(
                        juce::Colour(
                            200,
                            235,
                            255)
                        .withAlpha(0.75f));

                    g.fillEllipse(
                        ledX - 0.8f,
                        ledY - 0.8f,
                        1.6f,
                        1.6f);
                }

                return;
            }

            //======================================================
            // AMP SWITCH
            //
            // Physical vertical toggle.
            // No ON/OFF text.
            // Red LED is the status indicator.
            //======================================================

            const float centerX =
                w * 0.5f;

            const float switchY =
                on
                    ? 11.0f
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
                centerX - 3.5f,
                5.0f,
                7.0f,
                h - 10.0f,
                3.5f);

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
                6.0f,
                centerX - 0.5f,
                h - 6.0f,
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
                centerX - 8.0f,
                switchY - 8.0f,
                16.0f,
                16.0f);

            g.setColour(
                juce::Colour(
                    180,
                    180,
                    180));

            g.drawEllipse(
                centerX - 8.0f,
                switchY - 8.0f,
                16.0f,
                16.0f,
                1.0f);

            //======================================================
            // METAL LEVER
            //======================================================

            const float leverLength =
                12.0f;

            const float endY =
                on
                    ? switchY - leverLength
                    : switchY + leverLength;

            // Shadow

            g.setColour(
                juce::Colour(
                    0,
                    0,
                    0)
                    .withAlpha(0.85f));

            g.drawLine(
                centerX + 1.3f,
                switchY + 1.3f,
                centerX + 1.3f,
                endY + 1.3f,
                4.4f);

            // Lever

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
                4.0f);

            // Highlight

            g.setColour(
                juce::Colour(
                    250,
                    250,
                    250));

            g.drawLine(
                centerX - 0.6f,
                switchY - 0.6f,
                centerX - 0.6f,
                endY - 0.6f,
                1.0f);

            //======================================================
            // LEVER TIP
            //======================================================

            g.setColour(
                juce::Colour(
                    150,
                    150,
                    150));

            g.fillEllipse(
                centerX - 4.5f,
                endY - 4.5f,
                9.0f,
                9.0f);

            g.setColour(
                juce::Colour(
                    235,
                    235,
                    235));

            g.fillEllipse(
                centerX - 2.7f,
                endY - 2.7f,
                5.4f,
                5.4f);
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

    setupLabel(inputLabel,  "INPUT");
    setupLabel(gainLabel,   "GAIN");
    setupLabel(modeLabel,   "MODE");
    setupLabel(bassLabel,   "BASS");
    setupLabel(midLabel,    "MID");
    setupLabel(hiLabel,     "HI");
    setupLabel(volumeLabel, "VOLUME");
    setupLabel(ampLabel,    "AMP");

    //==============================================================
    // MODE PUSH BUTTON
    //==============================================================

    modeSwitch.setButtonText("");

    modeSwitch.setClickingTogglesState(true);

    modeSwitch.setToggleState(
        false,
        juce::dontSendNotification);

    modeSwitch.setLookAndFeel(
        &toggleLookAndFeel);

    addAndMakeVisible(
        modeSwitch);

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

            modeSwitch.setButtonText("");

            modeSwitch.repaint();
            repaint();
        };

    //==============================================================
    // AMP SWITCH
    //==============================================================

    ampSwitch.setButtonText("");

    ampSwitch.setComponentID(
        "AMP_SWITCH");

    ampSwitch.setClickingTogglesState(true);

    ampSwitch.setToggleState(
        true,
        juce::dontSendNotification);

    ampSwitch.setLookAndFeel(
        &toggleLookAndFeel);

    addAndMakeVisible(
        ampSwitch);

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

    modeSwitch.setButtonText("");

    ampSwitch.setButtonText("");

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

    // TRUE = HARD STOP AT MIN/MAX

    slider.setRotaryParameters(
        knobStartAngle,
        knobEndAngle,
        true);

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
    // AMP RED LED
    //
    // Positioned to the RIGHT of the AMP switch.
    //==============================================================

    constexpr float ledX =
        ampX + 40.0f;

    constexpr float ledY =
        controlY;

    constexpr float ledRadius =
        7.0f;

    //==============================================================
    // OUTER LED RING
    //==============================================================

    g.setColour(
        juce::Colour(
            5,
            5,
            5));

    g.fillEllipse(
        ledX - ledRadius - 2.0f,
        ledY - ledRadius - 2.0f,
        (ledRadius + 2.0f) * 2.0f,
        (ledRadius + 2.0f) * 2.0f);

    //==============================================================
    // RED LED
    //==============================================================

    g.setColour(
        ampIsOn
            ? juce::Colour(
                235,
                25,
                25)
            : juce::Colour(
                65,
                10,
                10));

    g.fillEllipse(
        ledX - ledRadius,
        ledY - ledRadius,
        ledRadius * 2.0f,
        ledRadius * 2.0f);

    //==============================================================
    // BRIGHT RED HIGHLIGHT WHEN ON
    //==============================================================

    if (ampIsOn)
    {
        g.setColour(
            juce::Colour(
                255,
                150,
                150)
            .withAlpha(0.9f));

        g.fillEllipse(
            ledX - 2.2f,
            ledY - 2.2f,
            4.4f,
            4.4f);
    }

    //==============================================================
    // LED BORDER
    //==============================================================

    g.setColour(
        juce::Colour(
            20,
            0,
            0));

    g.drawEllipse(
        ledX - ledRadius,
        ledY - ledRadius,
        ledRadius * 2.0f,
        ledRadius * 2.0f,
        1.2f);
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
        static_cast<int>(
            knobSize),
        static_cast<int>(
            knobSize));

    bassKnob.setBounds(
        static_cast<int>(
            bassX - knobSize * 0.5f),
        static_cast<int>(
            controlY - knobSize * 0.5f),
        static_cast<int>(
            knobSize),
        static_cast<int>(
            knobSize));

    midKnob.setBounds(
        static_cast<int>(
            midX - knobSize * 0.5f),
        static_cast<int>(
            controlY - knobSize * 0.5f),
        static_cast<int>(
            knobSize),
        static_cast<int>(
            knobSize));

    hiKnob.setBounds(
        static_cast<int>(
            hiX - knobSize * 0.5f),
        static_cast<int>(
            controlY - knobSize * 0.5f),
        static_cast<int>(
            knobSize),
        static_cast<int>(
            knobSize));

    volumeKnob.setBounds(
        static_cast<int>(
            volumeX - knobSize * 0.5f),
        static_cast<int>(
            controlY - knobSize * 0.5f),
        static_cast<int>(
            knobSize),
        static_cast<int>(
            knobSize));

    //==============================================================
    // LABELS
    //
    // All labels use the same proportional control positions.
    // Positioned 8 px lower / closer to controls.
    //==============================================================

    constexpr int labelWidth =
        60;

    constexpr int labelHeight =
        18;

    constexpr int labelY =
        static_cast<int>(
            controlY - 34.0f);

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

    modeLabel.setBounds(
        static_cast<int>(
            modeX - labelWidth * 0.5f),
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

    ampLabel.setBounds(
        static_cast<int>(
            ampX - labelWidth * 0.5f),
        labelY,
        labelWidth,
        labelHeight);

    //==============================================================
    // MODE
    //
    // Now physically positioned between GAIN and BASS.
    //==============================================================

    modeSwitch.setBounds(
        static_cast<int>(
            modeX - 22.0f),
        static_cast<int>(
            controlY - 11.0f),
        44,
        22);

    //==============================================================
    // AMP SWITCH
    //==============================================================

    ampSwitch.setBounds(
        static_cast<int>(
            ampX - 30.0f),
        static_cast<int>(
            controlY - 17.0f),
        60,
        34);
}
