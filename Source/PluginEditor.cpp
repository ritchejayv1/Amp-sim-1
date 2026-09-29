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

    constexpr float inputX  = 205.0f;
    constexpr float gainX   = 255.0f;
    constexpr float modeX   = 305.0f;
    constexpr float bassX   = 355.0f;
    constexpr float midX    = 405.0f;
    constexpr float hiX     = 455.0f;
    constexpr float volumeX = 505.0f;
    constexpr float ampX    = 555.0f;

    constexpr float controlY = 330.0f;
    constexpr float knobSize = 36.0f;

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
            // MODE SWITCH
            //======================================================

            if (! isAmp)
            {
                constexpr float buttonSize = 11.5f;

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
                    bx + 1.0f,
                    by + 1.4f,
                    buttonSize,
                    buttonSize,
                    1.8f);

                //==================================================
                // MODE ON
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
                        by + 0.7f,
                        buttonSize,
                        buttonSize - 0.7f,
                        1.8f);

                    g.setColour(
                        juce::Colour(
                            85,
                            165,
                            255));

                    g.drawRoundedRectangle(
                        bx,
                        by + 0.7f,
                        buttonSize,
                        buttonSize - 0.7f,
                        1.8f,
                        0.7f);

                    g.setColour(
                        juce::Colour(
                            45,
                            125,
                            225));

                    g.fillRoundedRectangle(
                        bx + 1.8f,
                        by + 2.2f,
                        buttonSize - 3.6f,
                        buttonSize - 3.8f,
                        1.0f);

                    g.setColour(
                        juce::Colour(
                            180,
                            220,
                            255)
                        .withAlpha(0.65f));

                    g.drawLine(
                        bx + 2.2f,
                        by + 2.0f,
                        bx + buttonSize - 2.2f,
                        by + 2.0f,
                        0.7f);
                }
                else
                {
                    //================================================
                    // MODE OFF - DARK BLUE
                    //================================================

                    g.setColour(
                        juce::Colour(
                            15,
                            35,
                            60));

                    g.fillRoundedRectangle(
                        bx,
                        by,
                        buttonSize,
                        buttonSize,
                        1.8f);

                    g.setColour(
                        juce::Colour(
                            55,
                            90,
                            125));

                    g.drawRoundedRectangle(
                        bx,
                        by,
                        buttonSize,
                        buttonSize,
                        1.8f,
                        0.7f);

                    g.setColour(
                        juce::Colour(
                            20,
                            50,
                            80));

                    g.fillRoundedRectangle(
                        bx + 1.8f,
                        by + 1.8f,
                        buttonSize - 3.6f,
                        buttonSize - 3.6f,
                        1.0f);

                    g.setColour(
                        juce::Colour(
                            90,
                            130,
                            165)
                        .withAlpha(0.45f));

                    g.drawLine(
                        bx + 2.0f,
                        by + 2.0f,
                        bx + buttonSize - 2.0f,
                        by + 2.0f,
                        0.7f);
                }

                //==================================================
                // MODE INDICATOR
                //==================================================

                const float ledRadius = 1.65f;

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
                            8,
                            20,
                            35));

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
                        ledX - 0.65f,
                        ledY - 0.65f,
                        1.3f,
                        1.3f);
                }

                return;
            }

            //======================================================
            // POWER SWITCH
            //
            // Outer housing removed.
            // Metallic center retained.
            // ON  = lever UP
            // OFF = lever DOWN
            //======================================================

            const float centerX =
                w * 0.5f;

            const float centerY =
                h * 0.5f;

            constexpr float centerRadius = 11.5f;

            //======================================================
            // METALLIC CENTER
            //======================================================

            g.setColour(
                juce::Colour(
                    70,
                    70,
                    70));

            g.fillEllipse(
                centerX - centerRadius,
                centerY - centerRadius,
                centerRadius * 2.0f,
                centerRadius * 2.0f);

            g.setColour(
                juce::Colour(
                    145,
                    145,
                    145));

            g.fillEllipse(
                centerX - 10.3f,
                centerY - 10.3f,
                20.6f,
                20.6f);

            g.setColour(
                juce::Colour(
                    205,
                    205,
                    205)
                    .withAlpha(0.8f));

            g.drawEllipse(
                centerX - 9.5f,
                centerY - 9.5f,
                19.0f,
                19.0f,
                0.8f);

            //======================================================
            // INNER SHADING
            //======================================================

            g.setColour(
                juce::Colour(
                    65,
                    65,
                    65)
                    .withAlpha(0.45f));

            g.fillEllipse(
                centerX - 6.0f,
                centerY - 6.0f,
                12.0f,
                12.0f);

            //======================================================
            // LEVER
            //======================================================

            const float leverPivotY =
                centerY;

            const float leverTipY =
                on
                    ? centerY - 8.0f
                    : centerY + 8.0f;

            // Lever shadow
            g.setColour(
                juce::Colour(
                    0,
                    0,
                    0)
                    .withAlpha(0.75f));

            g.drawLine(
                centerX + 1.2f,
                leverPivotY + 1.5f,
                centerX + 1.2f,
                leverTipY + 1.5f,
                4.5f);

            // Main lever
            g.setColour(
                juce::Colour(
                    215,
                    215,
                    215));

            g.drawLine(
                centerX,
                leverPivotY,
                centerX,
                leverTipY,
                4.0f);

            // Bright edge
            g.setColour(
                juce::Colour(
                    250,
                    250,
                    250)
                    .withAlpha(0.85f));

            g.drawLine(
                centerX - 0.8f,
                leverPivotY - 0.5f,
                centerX - 0.8f,
                leverTipY - 0.5f,
                1.0f);

            // Lever tip
            g.setColour(
                juce::Colour(
                    105,
                    105,
                    105));

            g.fillEllipse(
                centerX - 4.2f,
                leverTipY - 4.2f,
                8.4f,
                8.4f);

            g.setColour(
                juce::Colour(
                    225,
                    225,
                    225));

            g.fillEllipse(
                centerX - 3.0f,
                leverTipY - 3.0f,
                6.0f,
                6.0f);

            // Tip highlight
            g.setColour(
                juce::Colour(
                    255,
                    255,
                    255)
                    .withAlpha(0.65f));

            g.fillEllipse(
                centerX - 1.4f,
                leverTipY - 2.0f,
                2.2f,
                1.8f);
        }
    };

    RGToggleLookAndFeel toggleLookAndFeel;
}

//==============================================================
// CONSTRUCTOR
//==============================================================

AmpSimAudioProcessorEditor::
AmpSimAudioProcessorEditor(
    AmpSimAudioProcessor& p)
    : AudioProcessorEditor(&p),
      audioProcessor(p)
{
    setSize(
        editorWidth,
        editorHeight);

    //============================================================
    // BACKGROUND
    //============================================================

    backgroundImage =
        juce::ImageCache::getFromMemory(
            BinaryData::rg100_jpg,
            BinaryData::rg100_jpgSize);

    //============================================================
    // KNOBS
    //============================================================

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

    //============================================================
    // LABELS
    //============================================================

    setupLabel(
        inputLabel,
        "INPUT");

    setupLabel(
        gainLabel,
        "GAIN");

    setupLabel(
        modeLabel,
        "MODE");

    setupLabel(
        bassLabel,
        "BASS");

    setupLabel(
        midLabel,
        "MIDDLE");

    setupLabel(
        hiLabel,
        "TREBLE");

    setupLabel(
        volumeLabel,
        "VOLUME");

    setupLabel(
        ampLabel,
        "POWER");

    //============================================================
    // MODE SWITCH
    //============================================================

    modeSwitch.setButtonText("");

    modeSwitch.setClickingTogglesState(
        true);

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

    //============================================================
    // POWER SWITCH
    //============================================================

    ampSwitch.setButtonText("");

    ampSwitch.setComponentID(
        "AMP_SWITCH");

    ampSwitch.setClickingTogglesState(
        true);

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

    //============================================================
    // PARAMETER ATTACHMENTS
    //============================================================

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

    //============================================================
    // INITIAL STATE
    //============================================================

    isDriveMode =
        modeSwitch.getToggleState();

    ampIsOn =
        ampSwitch.getToggleState();

    modeSwitch.setButtonText("");
    ampSwitch.setButtonText("");

    repaint();
}

//==============================================================
// DESTRUCTOR
//==============================================================

AmpSimAudioProcessorEditor::
~AmpSimAudioProcessorEditor()
{
    modeSwitch.setLookAndFeel(nullptr);
    ampSwitch.setLookAndFeel(nullptr);
}

//==============================================================
// KNOB SETUP
//==============================================================

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

    // KEEP VALUE POPUP
    slider.setPopupDisplayEnabled(
        true,
        true,
        this);

    addAndMakeVisible(
        slider);
}

//==============================================================
// LABEL SETUP
//==============================================================

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

//==============================================================
// KNOB DRAWING
//==============================================================

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

    //==============================================================
    // KNOB DIMENSIONS
    //==============================================================

    const float radius =
        size * 0.5f - 1.5f;

    const float bevelRadius =
        radius - 1.0f;

    const float knobRadius =
        bevelRadius - 2.0f;

    //==============================================================
    // OUTER SHADOW / BEZEL
    //==============================================================

    g.setColour(
        juce::Colour(
            8,
            8,
            8));

    g.fillEllipse(
        cx - radius,
        cy - radius,
        radius * 2.0f,
        radius * 2.0f);

    //==============================================================
    // DARK METALLIC BEVEL
    //==============================================================

    g.setColour(
        juce::Colour(
            70,
            70,
            70));

    g.fillEllipse(
        cx - bevelRadius,
        cy - bevelRadius,
        bevelRadius * 2.0f,
        bevelRadius * 2.0f);

    //==============================================================
    // MAIN KNOB BODY
    //==============================================================

    g.setColour(
        juce::Colour(
            225,
            225,
            225));

    g.fillEllipse(
        cx - knobRadius,
        cy - knobRadius,
        knobRadius * 2.0f,
        knobRadius * 2.0f);

    //==============================================================
    // LOWER SHADING
    //==============================================================

    g.setColour(
        juce::Colour(
            155,
            155,
            155)
        .withAlpha(0.35f));

    g.fillEllipse(
        cx - knobRadius + 1.0f,
        cy - knobRadius + 2.0f,
        knobRadius * 2.0f - 2.0f,
        knobRadius * 2.0f - 1.0f);

    //==============================================================
    // TOP HIGHLIGHT
    //==============================================================

    g.setColour(
        juce::Colour(
            255,
            255,
            255)
        .withAlpha(0.75f));

    g.fillEllipse(
        cx - knobRadius + 2.0f,
        cy - knobRadius + 1.0f,
        knobRadius * 2.0f - 4.0f,
        knobRadius * 0.75f);

    //==============================================================
    // OUTER BEVEL HIGHLIGHT
    //==============================================================

    g.setColour(
        juce::Colour(
            245,
            245,
            245)
        .withAlpha(0.8f));

    g.drawEllipse(
        cx - knobRadius,
        cy - knobRadius,
        knobRadius * 2.0f,
        knobRadius * 2.0f,
        0.8f);

    //==============================================================
    // POINTER
    //==============================================================

    const float angle =
        rotaryStartAngle
        + sliderPosProportional
          * (rotaryEndAngle - rotaryStartAngle);

    const float pointerStart =
        knobRadius * 0.18f;

    const float pointerLength =
        knobRadius * 0.72f;

    const float pointerStartX =
        cx
        + std::cos(angle)
          * pointerStart;

    const float pointerStartY =
        cy
        + std::sin(angle)
          * pointerStart;

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
        pointerStartX + 0.7f,
        pointerStartY + 0.7f,
        pointerX + 0.7f,
        pointerY + 0.7f,
        2.8f);

    //==============================================================
    // MAIN POINTER
    //==============================================================

    g.setColour(
        juce::Colour(
            25,
            25,
            25));

    g.drawLine(
        pointerStartX,
        pointerStartY,
        pointerX,
        pointerY,
        2.2f);

    //==============================================================
    // POINTER HIGHLIGHT
    //==============================================================

    g.setColour(
        juce::Colour(
            255,
            255,
            255)
        .withAlpha(0.45f));

    g.drawLine(
        pointerStartX - 0.5f,
        pointerStartY - 0.5f,
        pointerX - 0.5f,
        pointerY - 0.5f,
        0.7f);
}

//==============================================================
// PAINT
//==============================================================

void AmpSimAudioProcessorEditor::paint(
    juce::Graphics& g)
{
    //============================================================
    // BACKGROUND
    //============================================================

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

    //============================================================
    // INPUT JACK
    //============================================================

    constexpr float jackRadius =
        5.5f;

    //============================================================
    // OUTER DARK MOUNTING SHADOW
    //============================================================

    g.setColour(
        juce::Colour(
            3,
            3,
            3));

    g.fillEllipse(
        inputX - 5.5f,
        controlY - 5.5f,
        11.0f,
        11.0f);

    //============================================================
    // METALLIC OUTER BEZEL
    //============================================================

    g.setColour(
        juce::Colour(
            110,
            110,
            110));

    g.fillEllipse(
        inputX - 5.75f,
        controlY - 5.75f,
        11.5f,
        11.5f);

    //============================================================
    // METALLIC UPPER-LEFT BEVEL
    //============================================================

    g.setColour(
        juce::Colour(
            190,
            190,
            190)
        .withAlpha(0.85f));

    g.drawEllipse(
        inputX - 4.9f,
        controlY - 4.9f,
        9.8f,
        9.8f,
        0.8f);

    //============================================================
    // DARK RECESSED FACE
    //============================================================

    g.setColour(
        juce::Colour(
            35,
            35,
            35));

    g.fillEllipse(
        inputX - 4.0f,
        controlY - 4.0f,
        8.0f,
        8.0f);

    //============================================================
    // BLACK JACK OPENING
    //============================================================

    g.setColour(
        juce::Colour(
            5,
            5,
            5));

    g.fillEllipse(
        inputX - 2.8f,
        controlY - 2.8f,
        5.6f,
        5.6f);

    //============================================================
    // DEEP CENTER
    //============================================================

    g.setColour(
        juce::Colour(
            0,
            0,
            0));

    g.fillEllipse(
        inputX - 1.8f,
        controlY - 1.8f,
        3.6f,
        3.6f);

    //============================================================
    // METALLIC HIGHLIGHT ARC
    //============================================================

    juce::Path jackHighlight;

    jackHighlight.addArc(
        inputX - 4.4f,
        controlY - 4.4f,
        8.8f,
        8.8f,
        juce::MathConstants<float>::pi * 1.12f,
        juce::MathConstants<float>::pi * 1.72f,
        true);

    g.setColour(
        juce::Colour(
            235,
            235,
            235)
        .withAlpha(0.72f));

    g.strokePath(
        jackHighlight,
        juce::PathStrokeType(
            0.7f));

    //============================================================
    // SMALL LOWER SHADOW
    //============================================================

    g.setColour(
        juce::Colour(
            0,
            0,
            0)
        .withAlpha(0.65f));

    g.drawEllipse(
        inputX - jackRadius + 0.7f,
        controlY - jackRadius + 0.7f,
        jackRadius * 2.0f - 1.4f,
        jackRadius * 2.0f - 1.4f,
        0.5f);

    //============================================================
    // POWER RED LED
    //============================================================

    constexpr float ledX =
        ampX + 40.0f;

    constexpr float ledY =
        controlY;

    constexpr float ledRadius =
        7.0f;

    //============================================================
    // OUTER SHADOW
    //============================================================

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

    //============================================================
    // LED
    //============================================================

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

    //============================================================
    // LED HIGHLIGHT
    //============================================================

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

    //============================================================
    // LED BORDER
    //============================================================

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

    //============================================================
    // COPYRIGHT FOOTER
    //============================================================

    g.setColour(
        juce::Colours::white.withAlpha(0.55f));

    g.setFont(
        juce::Font(
            9.0f,
            juce::Font::plain));

    g.drawText(
        "© RG Electronics. All Rights Reserved.",
        getWidth() - 250,
        getHeight() - 24,
        235,
        16,
        juce::Justification::right,
        false);
}

//==============================================================
// RESIZED
//==============================================================

void AmpSimAudioProcessorEditor::resized()
{
    //============================================================
    // KNOBS
    //============================================================

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

    //============================================================
    // LABELS
    //============================================================

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

    //============================================================
    // MODE SWITCH
    //
    // Visual button = 11.5 x 11.5 px
    // Clickable area = 44 x 22 px
    //============================================================

    modeSwitch.setBounds(
        static_cast<int>(
            modeX - 22.0f),
        static_cast<int>(
            controlY - 11.0f),
        44,
        22);

    //============================================================
    // POWER SWITCH
    //
    // Visual metallic center = 23 px
    // Clickable area = 60 x 34 px
    //============================================================

    ampSwitch.setBounds(
        static_cast<int>(
            ampX - 30.0f),
        static_cast<int>(
            controlY - 17.0f),
        60,
        34);
}
