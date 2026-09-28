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
    // REALISTIC TOGGLE LOOK
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

            //======================================================
            // SWITCH PLATE
            //======================================================

            const float plateX = 2.0f;
            const float plateY = 3.0f;
            const float plateW = w - 4.0f;
            const float plateH = h - 6.0f;

            g.setColour(
                juce::Colour(
                    8,
                    8,
                    8));

            g.fillRoundedRectangle(
                plateX,
                plateY,
                plateW,
                plateH,
                4.0f);

            //======================================================
            // METAL EDGE
            //======================================================

            g.setColour(
                juce::Colour(
                    70,
                    70,
                    70));

            g.drawRoundedRectangle(
                plateX,
                plateY,
                plateW,
                plateH,
                4.0f,
                1.0f);

            //======================================================
            // INNER SLOT
            //======================================================

            const float slotX = 8.0f;
            const float slotY = h * 0.5f - 4.0f;
            const float slotW = w - 16.0f;
            const float slotH = 8.0f;

            g.setColour(
                juce::Colour(
                    2,
                    2,
                    2));

            g.fillRoundedRectangle(
                slotX,
                slotY,
                slotW,
                slotH,
                3.0f);

            //======================================================
            // TOGGLE POSITION
            //======================================================

            const bool on =
                button.getToggleState();

            // MODE = horizontal movement
            // AMP  = vertical-looking metal switch,
            //        but rendered inside the same component.

            const bool isAmp =
                button.getButtonText()
                    .equalsIgnoreCase("ON")
                || button.getButtonText()
                    .equalsIgnoreCase("OFF");

            if (! isAmp)
            {
                //==================================================
                // MODE TOGGLE
                // CLEAN = LEFT
                // DRIVE = RIGHT
                //==================================================

                const float centerY =
                    h * 0.5f;

                const float leftX =
                    w * 0.34f;

                const float rightX =
                    w * 0.66f;

                const float centerX =
                    on ? rightX : leftX;

                // Metal base
                g.setColour(
                    juce::Colour(
                        105,
                        105,
                        105));

                g.fillEllipse(
                    centerX - 5.0f,
                    centerY - 5.0f,
                    10.0f,
                    10.0f);

                // Metal lever
                g.setColour(
                    juce::Colour(
                        205,
                        205,
                        205));

                const float leverAngle =
                    on
                        ? -0.45f
                        : 0.45f;

                const float leverLength = 9.0f;

                const float endX =
                    centerX
                    + std::sin(leverAngle)
                      * leverLength;

                const float endY =
                    centerY
                    - std::cos(leverAngle)
                      * leverLength;

                g.drawLine(
                    centerX,
                    centerY,
                    endX,
                    endY,
                    3.0f);

                // Lever tip
                g.setColour(
                    juce::Colour(
                        225,
                        225,
                        225));

                g.fillEllipse(
                    endX - 3.0f,
                    endY - 3.0f,
                    6.0f,
                    6.0f);

                //==================================================
                // MODE TEXT
                //==================================================

                g.setFont(
                    juce::Font(
                        5.5f,
                        juce::Font::bold));

                g.setColour(
                    juce::Colour(
                        185,
                        185,
                        185));

                g.drawText(
                    "CLEAN",
                    2,
                    static_cast<int>(h - 8.0f),
                    static_cast<int>(w * 0.5f - 2.0f),
                    7,
                    juce::Justification::centred);

                g.drawText(
                    "DRIVE",
                    static_cast<int>(w * 0.5f),
                    static_cast<int>(h - 8.0f),
                    static_cast<int>(w * 0.5f - 2.0f),
                    7,
                    juce::Justification::centred);
            }
            else
            {
                //==================================================
                // AMP TOGGLE
                // OFF = DOWN
                // ON  = UP
                //==================================================

                const float centerX =
                    w * 0.5f;

                const float centerY =
                    h * 0.5f;

                const float switchY =
                    on
                        ? centerY - 4.0f
                        : centerY + 4.0f;

                // Metal mounting nut
                g.setColour(
                    juce::Colour(
                        95,
                        95,
                        95));

                g.fillEllipse(
                    centerX - 5.0f,
                    switchY - 5.0f,
                    10.0f,
                    10.0f);

                // Metal lever
                g.setColour(
                    juce::Colour(
                        215,
                        215,
                        215));

                const float leverTop =
                    on
                        ? switchY - 8.0f
                        : switchY + 8.0f;

                g.drawLine(
                    centerX,
                    switchY,
                    centerX,
                    leverTop,
                    3.0f);

                // Lever tip
                g.setColour(
                    juce::Colour(
                        235,
                        235,
                        235));

                g.fillEllipse(
                    centerX - 3.0f,
                    leverTop - 3.0f,
                    6.0f,
                    6.0f);

                //==================================================
                // ON / OFF TEXT
                //==================================================

                g.setFont(
                    juce::Font(
                        5.5f,
                        juce::Font::bold));

                g.setColour(
                    on
                        ? juce::Colour(
                            100,
                            220,
                            110)
                        : juce::Colour(
                            150,
                            150,
                            150));

                g.drawText(
                    on ? "ON" : "OFF",
                    2,
                    static_cast<int>(h - 8.0f),
                    static_cast<int>(w - 4.0f),
                    7,
                    juce::Justification::centred);
            }
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

    setupKnob(gainKnob,   0.0,  10.0, 0.01);
    setupKnob(bassKnob,  -12.0, 12.0, 0.01);
    setupKnob(midKnob,   -12.0, 12.0, 0.01);
    setupKnob(hiKnob,    -12.0, 12.0, 0.01);
    setupKnob(volumeKnob, 0.0,  10.0, 0.01);

    //==============================================================
    // LABELS
    //==============================================================

    setupLabel(inputLabel,  "INPUT");
    setupLabel(gainLabel,   "GAIN");
    setupLabel(bassLabel,   "BASS");
    setupLabel(midLabel,    "MID");
    setupLabel(hiLabel,     "HI");
    setupLabel(volumeLabel, "VOLUME");
    setupLabel(modeLabel,   "MODE");
    setupLabel(ampLabel,    "AMP");

    //==============================================================
    // MODE SWITCH
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

            repaint();
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
    juce::ignoreUnused(
        rotaryStartAngle,
        rotaryEndAngle,
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
        knobStartAngle
        + sliderPosProportional
          * (knobEndAngle - knobStartAngle);

    const float pointerLength =
        knobRadius * 0.65f;

    const float pointerX =
        cx
        + std::cos(angle)
          * pointerLength;

    const float pointerY =
        cy
        + std::sin(angle)
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

    constexpr float jackRadius = 11.0f;

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

    constexpr float ledX = ampX;
    constexpr float ledY = controlY - 17.0f;
    constexpr float ledRadius = 5.0f;

    g.setColour(
        ampIsOn
            ? juce::Colour(70, 220, 90)
            : juce::Colour(45, 45, 45));

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
        static_cast<int>(gainX - knobSize * 0.5f),
        static_cast<int>(controlY - knobSize * 0.5f),
        static_cast<int>(knobSize),
        static_cast<int>(knobSize));

    bassKnob.setBounds(
        static_cast<int>(bassX - knobSize * 0.5f),
        static_cast<int>(controlY - knobSize * 0.5f),
        static_cast<int>(knobSize),
        static_cast<int>(knobSize));

    midKnob.setBounds(
        static_cast<int>(midX - knobSize * 0.5f),
        static_cast<int>(controlY - knobSize * 0.5f),
        static_cast<int>(knobSize),
        static_cast<int>(knobSize));

    hiKnob.setBounds(
        static_cast<int>(hiX - knobSize * 0.5f),
        static_cast<int>(controlY - knobSize * 0.5f),
        static_cast<int>(knobSize),
        static_cast<int>(knobSize));

    volumeKnob.setBounds(
        static_cast<int>(volumeX - knobSize * 0.5f),
        static_cast<int>(controlY - knobSize * 0.5f),
        static_cast<int>(knobSize),
        static_cast<int>(knobSize));

    //==============================================================
    // LABELS — ABOVE COMPONENTS
    //==============================================================

    constexpr int labelWidth  = 60;
    constexpr int labelHeight = 18;

    constexpr int labelY =
        static_cast<int>(
            controlY - 42.0f);

    inputLabel.setBounds(
        static_cast<int>(inputX - labelWidth * 0.5f),
        labelY,
        labelWidth,
        labelHeight);

    gainLabel.setBounds(
        static_cast<int>(gainX - labelWidth * 0.5f),
        labelY,
        labelWidth,
        labelHeight);

    bassLabel.setBounds(
        static_cast<int>(bassX - labelWidth * 0.5f),
        labelY,
        labelWidth,
        labelHeight);

    midLabel.setBounds(
        static_cast<int>(midX - labelWidth * 0.5f),
        labelY,
        labelWidth,
        labelHeight);

    hiLabel.setBounds(
        static_cast<int>(hiX - labelWidth * 0.5f),
        labelY,
        labelWidth,
        labelHeight);

    volumeLabel.setBounds(
        static_cast<int>(volumeX - labelWidth * 0.5f),
        labelY,
        labelWidth,
        labelHeight);

    modeLabel.setBounds(
        static_cast<int>(modeX - labelWidth * 0.5f),
        labelY,
        labelWidth,
        labelHeight);

    ampLabel.setBounds(
        static_cast<int>(ampX - labelWidth * 0.5f),
        labelY,
        labelWidth,
        labelHeight);

    //==============================================================
    // MODE REALISTIC SWITCH
    //==============================================================

    modeSwitch.setBounds(
        static_cast<int>(modeX - 32.0f),
        static_cast<int>(controlY - 15.0f),
        64,
        30);

    //==============================================================
    // AMP REALISTIC SWITCH
    //==============================================================

    ampSwitch.setBounds(
        static_cast<int>(ampX - 26.0f),
        static_cast<int>(controlY - 15.0f),
        52,
        30);
}
