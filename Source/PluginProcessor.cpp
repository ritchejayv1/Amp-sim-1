#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "BinaryData.h"

#include <vector>
#include <cmath>

//==============================================================
// CONSTRUCTOR
//==============================================================

AmpSimAudioProcessor::AmpSimAudioProcessor()
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
          juce::Identifier("AmpSimParameters"),
          createParameterLayout())
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

    // false = CLEAN
    // true  = DRIVE
    params.push_back(
        std::make_unique<juce::AudioParameterBool>(
            "MODE",
            "Mode",
            false));

    // false = OFF
    // true  = ON
    params.push_back(
        std::make_unique<juce::AudioParameterBool>(
            "AMP",
            "Amp",
            true));

    return {
        params.begin(),
        params.end()
    };
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

    monoBuffer.setSize(
        1,
        samplesPerBlock);

    monoBuffer.clear();

    namInputData.resize(
        static_cast<size_t>(samplesPerBlock));

    namOutputData.resize(
        static_cast<size_t>(samplesPerBlock));

    monoSpec.sampleRate =
        sampleRate;

    monoSpec.maximumBlockSize =
        static_cast<juce::uint32>(
            samplesPerBlock);

    monoSpec.numChannels = 1;

    inputGain.prepare(monoSpec);
    inputGain.setRampDurationSeconds(0.02);

    outputGain.prepare(monoSpec);
    outputGain.setRampDurationSeconds(0.02);

    eqChain.prepare(monoSpec);

    //==========================================================
    // INITIAL BASS
    //==========================================================

    {
        auto bassCoefficients =
            juce::dsp::IIR::Coefficients<float>::makeLowShelf(
                sampleRate,
                120.0,
                0.707f,
                1.0f);

        eqChain.get<0>().coefficients =
            bassCoefficients;
    }

    //==========================================================
    // INITIAL MID
    //==========================================================

    {
        auto midCoefficients =
            juce::dsp::IIR::Coefficients<float>::makePeakFilter(
                sampleRate,
                750.0,
                0.707f,
                1.0f);

        eqChain.get<1>().coefficients =
            midCoefficients;
    }

    //==========================================================
    // INITIAL HIGH
    //==========================================================

    {
        auto highCoefficients =
            juce::dsp::IIR::Coefficients<float>::makeHighShelf(
                sampleRate,
                4500.0,
                0.707f,
                1.0f);

        eqChain.get<2>().coefficients =
            highCoefficients;
    }

    loadNAM();
    loadIR();
}

//==============================================================
// RELEASE RESOURCES
//==============================================================

void AmpSimAudioProcessor::releaseResources()
{
    namModel.reset();

    namLoaded = false;

    irConvolution.reset();

    monoBuffer.setSize(0, 0);

    namInputData.clear();
    namOutputData.clear();
}

//==============================================================
// BUS LAYOUT
//==============================================================

bool AmpSimAudioProcessor::isBusesLayoutSupported(
    const BusesLayout& layouts) const
{
    const auto mainInput =
        layouts.getChannelSet(true, 0);

    const auto mainOutput =
        layouts.getChannelSet(false, 0);

    if (mainOutput !=
        juce::AudioChannelSet::stereo())
    {
        return false;
    }

    if (mainInput !=
        juce::AudioChannelSet::stereo())
    {
        return false;
    }

    return true;
}

//==============================================================
// LOAD NAM
//==============================================================

void AmpSimAudioProcessor::loadNAM()
{
    namLoaded = false;
    namModel.reset();

    const auto namFile =
        juce::File::getSpecialLocation(
            juce::File::tempDirectory)
            .getChildFile(
                "RG_MBDR_precision.nam");

    const auto namData =
        BinaryData::RG_MBDRprecision_nam;

    const int namSize =
        BinaryData::RG_MBDRprecision_namSize;

    if (!namFile.replaceWithData(
            namData,
            static_cast<size_t>(namSize)))
    {
        return;
    }

    try
    {
        namModel.reset(
            namLoader.CreateFromFile(
                namFile.getFullPathName().toStdString()));

        if (namModel != nullptr)
        {
            namLoaded = true;

            if (maximumBlockSize > 0)
            {
                namModel->SetMaxAudioBufferSize(
                    maximumBlockSize);
            }
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
    const auto irFile =
        juce::File::getSpecialLocation(
            juce::File::tempDirectory)
            .getChildFile(
                "RG_412_MB_mic_1.wav");

    const auto irData =
        BinaryData::RG_412_MB_mic_1_wav;

    const int irSize =
        BinaryData::RG_412_MB_mic_1_wavSize;

    if (!irFile.replaceWithData(
            irData,
            static_cast<size_t>(irSize)))
    {
        return;
    }

    irConvolution.reset();

    try
    {
        irConvolution.loadImpulseResponse(
            irFile,
            juce::dsp::Convolution::Stereo::no,
            juce::dsp::Convolution::Trim::yes,
            0,
            juce::dsp::Convolution::Normalise::yes);
    }
    catch (...)
    {
        irConvolution.reset();
    }

    irConvolution.prepare(monoSpec);
}

//==============================================================
// PROCESS BLOCK
//==============================================================

void AmpSimAudioProcessor::processBlock(
    juce::AudioBuffer& buffer,
    juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    juce::ignoreUnused(midiMessages);

    const int numSamples =
        buffer.getNumSamples();

    const int numOutputChannels =
        getTotalNumOutputChannels();

    if (numSamples <= 0)
        return;

    //==========================================================
    // AMP STATE
    //==========================================================

    const bool ampIsOn =
        ampParameter != nullptr
            ? ampParameter->load() >= 0.5f
            : true;

    //==========================================================
    // TRUE DRY BYPASS
    //==========================================================

    if (!ampIsOn)
    {
        return;
    }

    //==========================================================
    // SAFETY
    //==========================================================

    if (monoBuffer.getNumSamples() < numSamples)
    {
        monoBuffer.setSize(
            1,
            numSamples,
            false,
            false,
            true);
    }

    if (static_cast<int>(namInputData.size()) < numSamples)
    {
        namInputData.resize(
            static_cast<size_t>(numSamples));

        namOutputData.resize(
            static_cast<size_t>(numSamples));
    }

    //==========================================================
    // READ PARAMETERS
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
            ? modeParameter->load() >= 0.5f
            : false;

    //==========================================================
    // INPUT -> MONO
    //==========================================================

    monoBuffer.clear();

    float* monoData =
        monoBuffer.getWritePointer(0);

    const float* left =
        buffer.getReadPointer(0);

    const float* right =
        buffer.getNumChannels() > 1
            ? buffer.getReadPointer(1)
            : nullptr;

    if (right != nullptr)
    {
        for (int i = 0;
             i < numSamples;
             ++i)
        {
            monoData[i] =
                0.5f * (left[i] + right[i]);
        }
    }
    else
    {
        for (int i = 0;
             i < numSamples;
             ++i)
        {
            monoData[i] =
                left[i];
        }
    }

    //==========================================================
    // INPUT GAIN
    //==========================================================

    const float inputGainDb =
        juce::jmap(
            gainValue,
            0.0f,
            10.0f,
            -12.0f,
            12.0f);

    inputGain.setGainDecibels(
        inputGainDb);

    {
        juce::dsp::AudioBlock<float> monoBlock(
            monoBuffer);

        auto processBlock =
            monoBlock.getSubBlock(
                0,
                static_cast<size_t>(numSamples));

        juce::dsp::ProcessContextReplacing<float> context(
            processBlock);

        inputGain.process(context);
    }

    //==========================================================
    // DRIVE MODE -> NAM
    //==========================================================

    const bool shouldProcessNAM =
        driveMode &&
        namLoaded &&
        namModel != nullptr;

    if (shouldProcessNAM)
    {
        for (int i = 0;
             i < numSamples;
             ++i)
        {
            namInputData[
                static_cast<size_t>(i)] =
                monoData[i];
        }

        try
        {
            namModel->Process(
                namInputData.data(),
                namOutputData.data(),
                static_cast<size_t>(numSamples));

            for (int i = 0;
                 i < numSamples;
                 ++i)
            {
                monoData[i] =
                    namOutputData[
                        static_cast<size_t>(i)];
            }
        }
        catch (...)
        {
            // Keep signal unchanged if NAM fails.
        }
    }

    //==========================================================
    // BASS
    //==========================================================

    {
        eqChain.get<0>().coefficients =
            juce::dsp::IIR::Coefficients<float>::makeLowShelf(
                currentSampleRate,
                120.0,
                0.707f,
                juce::Decibels::decibelsToGain(
                    bassValue));
    }

    //==========================================================
    // MID
    //==========================================================

    {
        eqChain.get<1>().coefficients =
            juce::dsp::IIR::Coefficients<float>::makePeakFilter(
                currentSampleRate,
                750.0,
                0.707f,
                juce::Decibels::decibelsToGain(
                    midValue));
    }

    //==========================================================
    // HIGH
    //==========================================================

    {
        eqChain.get<2>().coefficients =
            juce::dsp::IIR::Coefficients<float>::makeHighShelf(
                currentSampleRate,
                4500.0,
                0.707f,
                juce::Decibels::decibelsToGain(
                    highValue));
    }

    //==========================================================
    // EQ
    //==========================================================

    {
        juce::dsp::AudioBlock<float> monoBlock(
            monoBuffer);

        auto processBlock =
            monoBlock.getSubBlock(
                0,
                static_cast<size_t>(numSamples));

        juce::dsp::ProcessContextReplacing<float> context(
            processBlock);

        eqChain.process(context);
    }

    //==========================================================
    // 4x12 IR
    //==========================================================

    {
        juce::dsp::AudioBlock<float> monoBlock(
            monoBuffer);

        auto processBlock =
            monoBlock.getSubBlock(
                0,
                static_cast<size_t>(numSamples));

        juce::dsp::ProcessContextReplacing<float> context(
            processBlock);

        irConvolution.process(context);
    }

    //==========================================================
    // OUTPUT BOOST
    //==========================================================

    constexpr float fixedOutputBoostDb =
        24.0f;

    constexpr float cleanMakeupGainDb =
        3.5f;

    const float totalOutputBoostDb =
        driveMode
            ? fixedOutputBoostDb
            : fixedOutputBoostDb + cleanMakeupGainDb;

    outputGain.setGainDecibels(
        totalOutputBoostDb);

    {
        juce::dsp::AudioBlock<float> monoBlock(
            monoBuffer);

        auto processBlock =
            monoBlock.getSubBlock(
                0,
                static_cast<size_t>(numSamples));

        juce::dsp::ProcessContextReplacing<float> context(
            processBlock);

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

    for (int i = 0;
         i < numSamples;
         ++i)
    {
        monoData[i] *= masterGain;
    }

    //==========================================================
    // MONO -> STEREO
    //==========================================================

    for (int channel = 0;
         channel < numOutputChannels;
         ++channel)
    {
        if (channel < buffer.getNumChannels())
        {
            buffer.copyFrom(
                channel,
                0,
                monoData,
                numSamples);
        }
    }

    //==========================================================
    // CLEAR EXTRA CHANNELS
    //==========================================================

    for (int channel = numOutputChannels;
         channel < buffer.getNumChannels();
         ++channel)
    {
        buffer.clear(
            channel,
            0,
            numSamples);
    }
}

//==============================================================
// CREATE EDITOR
//==============================================================

juce::AudioProcessorEditor*
AmpSimAudioProcessor::createEditor()
{
    return new AmpSimAudioProcessorEditor(*this);
}

//==============================================================
// HAS EDITOR
//==============================================================

bool AmpSimAudioProcessor::hasEditor() const
{
    return true;
}

//==============================================================
// NAME
//==============================================================

const juce::String
AmpSimAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

//==============================================================
// MIDI
//==============================================================

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

//==============================================================
// TAIL
//==============================================================

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
    juce::ignoreUnused(
        index,
        newName);
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
    std::unique_ptr<juce::XmlElement> xmlState =
        getXmlFromBinary(
            data,
            sizeInBytes);

    if (xmlState != nullptr)
    {
        if (xmlState->hasTagName(
                parameters.state.getType()))
        {
            parameters.replaceState(
                juce::ValueTree::fromXml(
                    *xmlState));
        }
    }
}

//==============================================================
// JUCE PLUGIN FACTORY
//==============================================================

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AmpSimAudioProcessor();
}
