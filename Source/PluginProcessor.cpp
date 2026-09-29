#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "BinaryData.h"

#include <vector>
#include <cmath>

//==============================================================
// CONSTRUCTOR
//==============================================================

AmpSimAudioProcessor::AmpSimAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(
        BusesProperties()
            .withInput(
                "Input",
                juce::AudioChannelSet::stereo(),
                true)
            .withOutput(
                "Output",
                juce::AudioChannelSet::stereo(),
                true)),
      parameters(
          *this,
          nullptr,
          juce::Identifier("PARAMETERS"),
          createParameterLayout())
#else
    : parameters(
          *this,
          nullptr,
          juce::Identifier("PARAMETERS"),
          createParameterLayout())
#endif
{
    gainParameter =
        parameters.getRawParameterValue("GAIN");

    bassParameter =
        parameters.getRawParameterValue("BASS");

    midParameter =
        parameters.getRawParameterValue("MID");

    highParameter =
        parameters.getRawParameterValue("HI");

    volumeParameter =
        parameters.getRawParameterValue("VOLUME");

    modeParameter =
        parameters.getRawParameterValue("MODE");

    ampParameter =
        parameters.getRawParameterValue("AMP");
}

//==============================================================
// DESTRUCTOR
//==============================================================

AmpSimAudioProcessor::~AmpSimAudioProcessor()
{
}

//==============================================================
// PARAMETER LAYOUT
//==============================================================

juce::AudioProcessorValueTreeState::ParameterLayout
AmpSimAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "GAIN",
            "Gain",
            juce::NormalisableRange<float>(
                0.0f,
                10.0f,
                0.01f),
            5.0f));

    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "BASS",
            "Bass",
            juce::NormalisableRange<float>(
                -12.0f,
                12.0f,
                0.01f),
            0.0f));

    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "MID",
            "Mid",
            juce::NormalisableRange<float>(
                -12.0f,
                12.0f,
                0.01f),
            0.0f));

    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "HI",
            "High",
            juce::NormalisableRange<float>(
                -12.0f,
                12.0f,
                0.01f),
            0.0f));

    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "VOLUME",
            "Volume",
            juce::NormalisableRange<float>(
                0.0f,
                10.0f,
                0.01f),
            5.0f));

    params.push_back(
        std::make_unique<juce::AudioParameterBool>(
            "MODE",
            "Mode",
            false));

    params.push_back(
        std::make_unique<juce::AudioParameterBool>(
            "AMP",
            "Amp",
            true));

    return { params.begin(), params.end() };
}

//==============================================================
// PREPARE TO PLAY
//==============================================================

void AmpSimAudioProcessor::prepareToPlay(
    double sampleRate,
    int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    maximumBlockSize = samplesPerBlock;

    //==========================================================
    // MONO BUFFER
    //==========================================================

    monoBuffer.setSize(
        1,
        samplesPerBlock);

    monoBuffer.clear();

    //==========================================================
    // NAM BUFFERS
    //==========================================================

    namInputData.resize(
        static_cast<size_t>(samplesPerBlock));

    namOutputData.resize(
        static_cast<size_t>(samplesPerBlock));

    //==========================================================
    // DSP PROCESS SPEC
    //==========================================================

    monoSpec.sampleRate = sampleRate;

    monoSpec.maximumBlockSize =
        static_cast<juce::uint32>(samplesPerBlock);

    monoSpec.numChannels = 1;

    //==========================================================
    // INPUT GAIN
    //==========================================================

    inputGain.prepare(monoSpec);

    //==========================================================
    // OUTPUT GAIN
    //==========================================================

    outputGain.prepare(monoSpec);

    //==========================================================
    // EQ
    //==========================================================

    eqChain.prepare(monoSpec);

    //==========================================================
    // LOAD NAM
    //==========================================================

    loadNAM();

    //==========================================================
    // LOAD 4x12 IR
    //==========================================================

    loadIR();
}

//==============================================================
// RELEASE RESOURCES
//==============================================================

void AmpSimAudioProcessor::releaseResources()
{
    namModel.reset();

    namLoaded = false;

    namInputData.clear();
    namOutputData.clear();

    monoBuffer.setSize(0, 0);

    irConvolution.reset();
}

//==============================================================
// BUS LAYOUT
//==============================================================

bool AmpSimAudioProcessor::isBusesLayoutSupported(
    const BusesLayout& layouts) const
{
    const auto input =
        layouts.getMainInputChannelSet();

    const auto output =
        layouts.getMainOutputChannelSet();

    if (input != juce::AudioChannelSet::stereo())
        return false;

    if (output != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

//==============================================================
// LOAD NAM
//==============================================================

void AmpSimAudioProcessor::loadNAM()
{
    namLoaded = false;
    namModel.reset();

    const auto tempFile =
        juce::File::getSpecialLocation(
            juce::File::tempDirectory)
            .getChildFile(
                "RG_MBDR_precision.nam");

    tempFile.replaceWithData(
        BinaryData::RG_MBDRprecision_nam,
        BinaryData::RG_MBDRprecision_namSize);

    try
    {
        namModel.reset(
            namLoader.CreateFromFile(
                tempFile.getFullPathName().toStdString()));

        if (namModel != nullptr)
        {
            namLoaded = true;

            namModel->SetMaxAudioBufferSize(
                static_cast<size_t>(maximumBlockSize));
        }
    }
    catch (...)
    {
        namModel.reset();
        namLoaded = false;
    }
}

//==============================================================
// LOAD IR
//==============================================================

void AmpSimAudioProcessor::loadIR()
{
    const auto tempFile =
        juce::File::getSpecialLocation(
            juce::File::tempDirectory)
            .getChildFile(
                "RG_412_MB_mic_1.wav");

    tempFile.replaceWithData(
        BinaryData::RG_412_MB_mic_1_wav,
        BinaryData::RG_412_MB_mic_1_wavSize);

    irConvolution.reset();

    irConvolution.loadImpulseResponse(
        tempFile,
        juce::dsp::Convolution::Stereo::no,
        juce::dsp::Convolution::Trim::yes,
        0,
        juce::dsp::Convolution::Normalise::yes);

    irConvolution.prepare(monoSpec);
}

//==============================================================
// PROCESS BLOCK
//==============================================================

void AmpSimAudioProcessor::processBlock(
    juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);

    //==========================================================
    // AMP POWER
    //==========================================================

    const bool ampIsOn =
        ampParameter != nullptr
            ? ampParameter->load() > 0.5f
            : true;

    //==========================================================
    // TRUE BYPASS
    //==========================================================

    if (!ampIsOn)
        return;

    //==========================================================
    // PARAMETER VALUES
    //==========================================================

    const float gainValue =
        gainParameter != nullptr
            ? gainParameter->load()
            : 5.0f;

    const float bassValue =
        bassParameter != nullptr
            ? bassParameter->load()
            : 0.0f;

    const float midValue =
        midParameter != nullptr
            ? midParameter->load()
            : 0.0f;

    const float highValue =
        highParameter != nullptr
            ? highParameter->load()
            : 0.0f;

    const float volumeValue =
        volumeParameter != nullptr
            ? volumeParameter->load()
            : 5.0f;

    const bool driveMode =
        modeParameter != nullptr
            ? modeParameter->load() > 0.5f
            : false;

    //==========================================================
    // CLEAR EXTRA CHANNELS
    //==========================================================

    for (int channel = 2;
         channel < buffer.getNumChannels();
         ++channel)
    {
        buffer.clear(
            channel,
            0,
            buffer.getNumSamples());
    }

    const int numSamples =
        buffer.getNumSamples();

    if (numSamples <= 0)
        return;

    //==========================================================
    // STEREO -> MONO
    //==========================================================

    monoBuffer.setSize(
        1,
        numSamples,
        false,
        false,
        true);

    auto* monoData =
        monoBuffer.getWritePointer(0);

    const auto* leftData =
        buffer.getReadPointer(0);

    const auto* rightData =
        buffer.getNumChannels() > 1
            ? buffer.getReadPointer(1)
            : leftData;

    for (int sample = 0;
         sample < numSamples;
         ++sample)
    {
        monoData[sample] =
            0.5f *
            (leftData[sample] +
             rightData[sample]);
    }

    //==========================================================
    // INPUT GAIN
    //==========================================================

    float inputGainDb =
        juce::jmap(
            gainValue,
            0.0f,
            10.0f,
            -22.0f,
            2.0f);

    //==========================================================
    // CLEAN MODE MAXIMUM GAIN
    //==========================================================

    if (!driveMode)
    {
        inputGainDb =
            juce::jmin(
                inputGainDb,
                0.0f);
    }

    inputGain.setGainDecibels(
        inputGainDb);

    {
        juce::dsp::AudioBlock<float> audioBlock(
            monoBuffer);

        auto subBlock =
            audioBlock.getSubBlock(
                0,
                static_cast<size_t>(numSamples));

        juce::dsp::ProcessContextReplacing<float> context(
            subBlock);

        inputGain.process(context);
    }

    //==========================================================
    // CLEAN +3 dB BOOST
    //==========================================================

    if (!driveMode)
    {
        monoBuffer.applyGain(
            1.4125f);
    }

    //==========================================================
    // NAM
    //==========================================================

    if (driveMode &&
        namLoaded &&
        namModel != nullptr)
    {
        for (int sample = 0;
             sample < numSamples;
             ++sample)
        {
            namInputData[
                static_cast<size_t>(sample)] =
                monoData[sample];
        }

        try
        {
            namModel->Process(
                namInputData.data(),
                namOutputData.data(),
                static_cast<size_t>(numSamples));

            for (int sample = 0;
                 sample < numSamples;
                 ++sample)
            {
                monoData[sample] =
                    namOutputData[
                        static_cast<size_t>(sample)];
            }
        }
        catch (...)
        {
            // Keep input signal if NAM processing fails.
        }
    }

    //==========================================================
    // 3-BAND EQ
    //==========================================================

    auto& bassFilter =
        eqChain.get<0>();

    auto& midFilter =
        eqChain.get<1>();

    auto& highFilter =
        eqChain.get<2>();

    //==========================================================
    // BASS
    //==========================================================

    bassFilter.coefficients =
        juce::dsp::IIR::Coefficients<float>::makeLowShelf(
            currentSampleRate,
            120.0f,
            0.7071f,
            juce::Decibels::decibelsToGain(
                bassValue));

    //==========================================================
    // MID
    //==========================================================

    midFilter.coefficients =
        juce::dsp::IIR::Coefficients<float>::makePeakFilter(
            currentSampleRate,
            800.0f,
            0.8f,
            juce::Decibels::decibelsToGain(
                midValue));

    //==========================================================
    // HIGH
    //==========================================================

    highFilter.coefficients =
        juce::dsp::IIR::Coefficients<float>::makeHighShelf(
            currentSampleRate,
            4500.0f,
            0.7071f,
            juce::Decibels::decibelsToGain(
                highValue));

    //==========================================================
    // PROCESS EQ
    //==========================================================

    {
        juce::dsp::AudioBlock<float> audioBlock(
            monoBuffer);

        auto subBlock =
            audioBlock.getSubBlock(
                0,
                static_cast<size_t>(numSamples));

        juce::dsp::ProcessContextReplacing<float> context(
            subBlock);

        eqChain.process(context);
    }

    //==========================================================
    // 4x12 CAB IR
    //==========================================================

    {
        juce::dsp::AudioBlock<float> audioBlock(
            monoBuffer);

        auto subBlock =
            audioBlock.getSubBlock(
                0,
                static_cast<size_t>(numSamples));

        juce::dsp::ProcessContextReplacing<float> context(
            subBlock);

        irConvolution.process(context);
    }

    //==========================================================
    // OUTPUT BOOST
    //==========================================================

    constexpr float fixedOutputBoostDb = 24.0f;
    constexpr float cleanMakeupGainDb = 8.0f;

    const float totalOutputBoostDb =
        driveMode
            ? fixedOutputBoostDb
            : fixedOutputBoostDb +
              cleanMakeupGainDb;

    outputGain.setGainDecibels(
        totalOutputBoostDb);

    {
        juce::dsp::AudioBlock<float> audioBlock(
            monoBuffer);

        auto subBlock =
            audioBlock.getSubBlock(
                0,
                static_cast<size_t>(numSamples));

        juce::dsp::ProcessContextReplacing<float> context(
            subBlock);

        outputGain.process(context);
    }

    //==========================================================
    // MASTER VOLUME
    //==========================================================

    const float masterGain =
        juce::jmap(
            volumeValue,
            0.0f,
            10.0f,
            0.0f,
            1.0f);

    monoBuffer.applyGain(
        masterGain);

    //==========================================================
    // MONO -> STEREO
    //==========================================================

    buffer.copyFrom(
        0,
        0,
        monoBuffer,
        0,
        0,
        numSamples);

    if (buffer.getNumChannels() > 1)
    {
        buffer.copyFrom(
            1,
            0,
            monoBuffer,
            0,
            0,
            numSamples);
    }
}

//==============================================================
// EDITOR
//==============================================================

juce::AudioProcessorEditor*
AmpSimAudioProcessor::createEditor()
{
    return new AmpSimAudioProcessorEditor(*this);
}

bool AmpSimAudioProcessor::hasEditor() const
{
    return true;
}

//==============================================================
// BASIC INFORMATION
//==============================================================

const juce::String
AmpSimAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AmpSimAudioProcessor::acceptsMidi() const
{
    return false;
}

bool AmpSimAudioProcessor::producesMidi() const
{
    return false;
}

bool AmpSimAudioProcessor::isMidiEffect() const
{
    return false;
}

double AmpSimAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

//==============================================================
// PROGRAMS
//==============================================================

int AmpSimAudioProcessor::getNumPrograms()
{
    return 1;
}

int AmpSimAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AmpSimAudioProcessor::setCurrentProgram(
    int index)
{
    juce::ignoreUnused(index);
}

const juce::String
AmpSimAudioProcessor::getProgramName(
    int index)
{
    juce::ignoreUnused(index);
    return {};
}

void AmpSimAudioProcessor::changeProgramName(
    int index,
    const juce::String& newName)
{
    juce::ignoreUnused(index);
    juce::ignoreUnused(newName);
}

//==============================================================
// SAVE STATE
//==============================================================

void AmpSimAudioProcessor::getStateInformation(
    juce::MemoryBlock& destData)
{
    std::unique_ptr<juce::XmlElement> xml =
        parameters.state.createXml();

    if (xml != nullptr)
    {
        copyXmlToBinary(
            *xml,
            destData);
    }
}

//==============================================================
// LOAD STATE
//==============================================================

void AmpSimAudioProcessor::setStateInformation(
    const void* data,
    int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml =
        getXmlFromBinary(
            data,
            sizeInBytes);

    if (xml != nullptr)
    {
        if (xml->hasTagName(
                parameters.state.getType()))
        {
            parameters.state =
                juce::ValueTree::fromXml(
                    *xml);
        }
    }
}

//==============================================================
// CREATE PLUGIN
//==============================================================

juce::AudioProcessor*
JUCE_CALLTYPE createPluginFilter()
{
    return new AmpSimAudioProcessor();
}
