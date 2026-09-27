
#include "PluginProcessor.h"
#include "PluginEditor.h"

#include "BinaryData.h"

#include <cmath>

//==============================================================================
namespace
{
    constexpr const char* namFileName =
        "RG MBDR+precision.nam";

    constexpr const char* irFileName =
        "RG 412 MB mic 1.wav";

    // Temporary filenames.
    constexpr const char* tempNAMName =
        "RG_AmpSim_Internal_Model.nam";

    constexpr const char* tempIRName =
        "RG_AmpSim_Internal_IR.wav";
}

//==============================================================================
// Constructor
//==============================================================================

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
          "PARAMETERS",
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

    //==============================================================
    // Create embedded NAM loader
    //==============================================================

    namLoader =
        std::make_unique<NeuralAudio::NeuralModelLoader>();

    //==============================================================
    // Temporary directory
    //==============================================================

    const auto tempDirectory =
        juce::File::getSpecialLocation(
            juce::File::tempDirectory);

    namTempFile =
        tempDirectory.getChildFile(tempNAMName);

    irTempFile =
        tempDirectory.getChildFile(tempIRName);

    //==============================================================
    // Extract embedded files
    //==============================================================

    createEmbeddedNAMFile();
    createEmbeddedIRFile();

    //==============================================================
    // Load actual NAM + IR
    //==============================================================

    loadNAM();
    loadIR();
}

//==============================================================================

AmpSimAudioProcessor::~AmpSimAudioProcessor()
{
    namModel.reset();
    namLoader.reset();

    irConvolution.reset();

    // Remove temporary files.
    if (namTempFile.existsAsFile())
        namTempFile.deleteFile();

    if (irTempFile.existsAsFile())
        irTempFile.deleteFile();
}

//==============================================================================
// Parameter Layout
//==============================================================================

juce::AudioProcessorValueTreeState::ParameterLayout
AmpSimAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>>
        params;

    //==============================================================
    // GAIN
    //==============================================================

    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "GAIN",
            "Gain",

            juce::NormalisableRange<float>(
                0.0f,
                10.0f,
                0.01f),

            5.0f));

    //==============================================================
    // BASS
    //==============================================================

    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "BASS",
            "Bass",

            juce::NormalisableRange<float>(
                -12.0f,
                12.0f,
                0.01f),

            0.0f,

            "dB"));

    //==============================================================
    // MID
    //==============================================================

    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "MID",
            "Mid",

            juce::NormalisableRange<float>(
                -12.0f,
                12.0f,
                0.01f),

            0.0f,

            "dB"));

    //==============================================================
    // HIGH
    //==============================================================

    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "HI",
            "High",

            juce::NormalisableRange<float>(
                -12.0f,
                12.0f,
                0.01f),

            0.0f,

            "dB"));

    //==============================================================
    // VOLUME
    //==============================================================

    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "VOLUME",
            "Volume",

            juce::NormalisableRange<float>(
                -24.0f,
                12.0f,
                0.01f),

            0.0f,

            "dB"));

    return {
        params.begin(),
        params.end()
    };
}

//==============================================================================
// Embedded NAM
//==============================================================================

bool AmpSimAudioProcessor::createEmbeddedNAMFile()
{
    int dataSize = 0;

    const void* data =
        BinaryData::getNamedResource(
            namFileName,
            dataSize);

    if (data == nullptr || dataSize <= 0)
        return false;

    if (namTempFile.existsAsFile())
        namTempFile.deleteFile();

    return namTempFile.replaceWithData(
        data,
        static_cast<size_t>(dataSize));
}

//==============================================================================
// Embedded IR
//==============================================================================

bool AmpSimAudioProcessor::createEmbeddedIRFile()
{
    int dataSize = 0;

    const void* data =
        BinaryData::getNamedResource(
            irFileName,
            dataSize);

    if (data == nullptr || dataSize <= 0)
        return false;

    if (irTempFile.existsAsFile())
        irTempFile.deleteFile();

    return irTempFile.replaceWithData(
        data,
        static_cast<size_t>(dataSize));
}

//==============================================================================
// Load NAM
//==============================================================================

bool AmpSimAudioProcessor::loadNAM()
{
    if (!namLoader)
        return false;

    if (!namTempFile.existsAsFile())
        return false;

    // Give NeuralAudio enough room for the host block.
    namLoader->SetDefaultMaxAudioBufferSize(2048);

    // Use the host's sample rate later in prepareToPlay.
    // NeuralAudio defaults to 48 kHz.

    namModel.reset();

    namModel =
        std::unique_ptr<NeuralAudio::NeuralModel>(
            namLoader->CreateFromFile(
                namTempFile.getFullPathName()
                    .toStdString()));

    if (namModel == nullptr)
    {
        namLoaded = false;
        return false;
    }

    namLoaded = true;

    return true;
}

//==============================================================================
// Load IR
//==============================================================================

bool AmpSimAudioProcessor::loadIR()
{
    if (!irTempFile.existsAsFile())
        return false;

    if (!irTempFile.existsAsFile())
    {
        irLoaded = false;
        return false;
    }

    irLoaded = true;

    return true;
}

//==============================================================================
// prepareToPlay
//==============================================================================

void AmpSimAudioProcessor::prepareToPlay(
    double sampleRate,
    int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;

    spec.sampleRate =
        sampleRate;

    spec.maximumBlockSize =
        static_cast<juce::uint32>(
            samplesPerBlock);

    spec.numChannels =
        static_cast<juce::uint32>(
            getTotalNumOutputChannels());

    //==============================================================
    // Input gain
    //==============================================================

    inputGain.prepare(spec);

    inputGain.setRampDurationSeconds(
        0.02);

    inputGain.setGainDecibels(
        0.0f);

    //==============================================================
    // Output gain
    //==============================================================

    outputGain.prepare(spec);

    outputGain.setRampDurationSeconds(
        0.02);

    outputGain.setGainDecibels(
        0.0f);

    //==============================================================
    // EQ
    //==============================================================

    bassFilter.prepare(spec);
    midFilter.prepare(spec);
    highFilter.prepare(spec);

    bassFilter.reset();
    midFilter.reset();
    highFilter.reset();

    //==============================================================
    // IR
    //==============================================================

    irConvolution.reset();

    if (irTempFile.existsAsFile())
    {
        // Load the actual embedded WAV IR.
        irConvolution.loadImpulseResponse(
            irTempFile,
            juce::dsp::Convolution::Stereo::yes,
            juce::dsp::Convolution::Trim::yes,
            juce::dsp::Convolution::Normalise::yes);
    }

    //==============================================================
    // NAM buffer
    //==============================================================

    namInputBuffer.setSize(
        1,
        samplesPerBlock,
        false,
        true,
        true);

    namOutputBuffer.setSize(
        1,
        samplesPerBlock,
        false,
        true,
        true);

    namInputData.resize(
        static_cast<size_t>(samplesPerBlock));

    namOutputData.resize(
        static_cast<size_t>(samplesPerBlock));

    //==============================================================
    // Prepare NAM
    //==============================================================

    if (namModel)
    {
        namModel->SetMaxAudioBufferSize(
            samplesPerBlock);
    }
}

//==============================================================================

void AmpSimAudioProcessor::releaseResources()
{
    inputGain.reset();
    outputGain.reset();

    bassFilter.reset();
    midFilter.reset();
    highFilter.reset();

    irConvolution.reset();

    namInputBuffer.setSize(
        1,
        0);

    namOutputBuffer.setSize(
        1,
        0);

    namInputData.clear();
    namOutputData.clear();
}

//==============================================================================
// Bus Layout
//==============================================================================

bool AmpSimAudioProcessor::isBusesLayoutSupported(
    const BusesLayout& layouts) const
{
    const auto& mainInput =
        layouts.getChannelSet(
            true,
            0);

    const auto& mainOutput =
        layouts.getChannelSet(
            false,
            0);

    if (mainOutput !=
            juce::AudioChannelSet::mono()
        &&
        mainOutput !=
            juce::AudioChannelSet::stereo())
    {
        return false;
    }

    if (mainInput != mainOutput)
        return false;

    return true;
}

//==============================================================================
// DSP
//==============================================================================

void AmpSimAudioProcessor::processBlock(
    juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer&)
{
    juce::ScopedNoDenormals
        noDenormals;

    const int numSamples =
        buffer.getNumSamples();

    const int numChannels =
        buffer.getNumChannels();

    if (numSamples <= 0)
        return;

    //==============================================================
    // Parameter values
    //==============================================================

    const float gain =
        gainParameter != nullptr
            ? gainParameter->load()
            : 5.0f;

    const float bass =
        bassParameter != nullptr
            ? bassParameter->load()
            : 0.0f;

    const float mid =
        midParameter != nullptr
            ? midParameter->load()
            : 0.0f;

    const float high =
        highParameter != nullptr
            ? highParameter->load()
            : 0.0f;

    const float volume =
        volumeParameter != nullptr
            ? volumeParameter->load()
            : 0.0f;

    //==============================================================
    // GAIN
    //==============================================================

    const float gainDb =
        juce::jmap(
            gain,
            0.0f,
            10.0f,
            -12.0f,
            24.0f);

    inputGain.setGainDecibels(
        gainDb);

    {
        juce::dsp::AudioBlock<float>
            block(buffer);

        juce::dsp::ProcessContextReplacing<float>
            context(block);

        inputGain.process(context);
    }

    //==============================================================
    // NAM
    //
    // Guitar signal is converted to mono for the amp model.
    // NAM output is then copied back to stereo.
    //==============================================================

    if (namLoaded && namModel)
    {
        // Make sure our temporary arrays are large enough.
        if (static_cast<int>(
                namInputData.size())
            < numSamples)
        {
            namInputData.resize(
                static_cast<size_t>(
                    numSamples));

            namOutputData.resize(
                static_cast<size_t>(
                    numSamples));
        }

        //==========================================================
        // Stereo -> mono
        //==========================================================

        for (int sample = 0;
             sample < numSamples;
             ++sample)
        {
            float mono = 0.0f;

            if (numChannels == 1)
            {
                mono =
                    buffer.getSample(
                        0,
                        sample);
            }
            else
            {
                mono =
                    0.5f *
                    (buffer.getSample(
                         0,
                         sample)
                     +
                     buffer.getSample(
                         1,
                         sample));
            }

            namInputData[
                static_cast<size_t>(
                    sample)] = mono;
        }

        //==========================================================
        // NAM PROCESS
        //==========================================================

        namModel->Process(
            namInputData.data(),
            namOutputData.data(),
            numSamples);

        //==========================================================
        // Mono -> stereo
        //==========================================================

        for (int sample = 0;
             sample < numSamples;
             ++sample)
        {
            const float output =
                namOutputData[
                    static_cast<size_t>(
                        sample)];

            for (int channel = 0;
                 channel < numChannels;
                 ++channel)
            {
                buffer.setSample(
                    channel,
                    sample,
                    output);
            }
        }
    }

    //==============================================================
    // BASS
    //==============================================================

    *bassFilter.state =
        *juce::dsp::IIR::Coefficients<float>::makeLowShelf(
            getSampleRate(),
            120.0,
            0.7071,
            juce::Decibels::decibelsToGain(
                bass));

    {
        juce::dsp::AudioBlock<float>
            block(buffer);

        juce::dsp::ProcessContextReplacing<float>
            context(block);

        bassFilter.process(context);
    }

    //==============================================================
    // MID
    //==============================================================

    *midFilter.state =
        *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
            getSampleRate(),
            750.0,
            0.8,
            juce::Decibels::decibelsToGain(
                mid));

    {
        juce::dsp::AudioBlock<float>
            block(buffer);

        juce::dsp::ProcessContextReplacing<float>
            context(block);

        midFilter.process(context);
    }

    //==============================================================
    // HIGH
    //==============================================================

    *highFilter.state =
        *juce::dsp::IIR::Coefficients<float>::makeHighShelf(
            getSampleRate(),
            4500.0,
            0.7071,
            juce::Decibels::decibelsToGain(
                high));

    {
        juce::dsp::AudioBlock<float>
            block(buffer);

        juce::dsp::ProcessContextReplacing<float>
            context(block);

        highFilter.process(context);
    }

    //==============================================================
    // FIXED CABINET IR
    //==============================================================

    if (irLoaded)
    {
        juce::dsp::AudioBlock<float>
            block(buffer);

        juce::dsp::ProcessContextReplacing<float>
            context(block);

        irConvolution.process(
            context);
    }

    //==============================================================
    // VOLUME
    //==============================================================

    outputGain.setGainDecibels(
        volume);

    {
        juce::dsp::AudioBlock<float>
            block(buffer);

        juce::dsp::ProcessContextReplacing<float>
            context(block);

        outputGain.process(context);
    }
}

//==============================================================================
// Program
//==============================================================================

int AmpSimAudioProcessor::getNumPrograms()
{
    return 1;
}

int AmpSimAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AmpSimAudioProcessor::setCurrentProgram(
    int)
{
}

const juce::String
AmpSimAudioProcessor::getProgramName(
    int)
{
    return {};
}

void AmpSimAudioProcessor::changeProgramName(
    int,
    const juce::String&)
{
}

//==============================================================================
// State
//==============================================================================

void AmpSimAudioProcessor::getStateInformation(
    juce::MemoryBlock& destData)
{
    auto state =
        parameters.copyState();

    std::unique_ptr<juce::XmlElement>
        xml(
            state.createXml());

    copyXmlToBinary(
        *xml,
        destData);
}

//==============================================================================

void AmpSimAudioProcessor::setStateInformation(
    const void* data,
    int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement>
        xml(
            getXmlFromBinary(
                data,
                sizeInBytes));

    if (xml != nullptr
        &&
        xml->hasTagName(
            parameters.state.getType()))
    {
        parameters.replaceState(
            juce::ValueTree::fromXml(
                *xml));
    }
}

//==============================================================================
// Editor
//==============================================================================

juce::AudioProcessorEditor*
AmpSimAudioProcessor::createEditor()
{
    return new AmpSimAudioProcessorEditor(
        *this);
}

//==============================================================================

bool AmpSimAudioProcessor::hasEditor() const
{
    return true;
}

//==============================================================================
// Plugin name
//==============================================================================

const juce::String
AmpSimAudioProcessor::getName() const
{
    return "RG Amp SIM";
}

//==============================================================================

bool AmpSimAudioProcessor::acceptsMidi() const
{
    return false;
}

//==============================================================================

bool AmpSimAudioProcessor::producesMidi() const
{
    return false;
}

//==============================================================================

bool AmpSimAudioProcessor::isMidiEffect() const
{
    return false;
}

//==============================================================================

double AmpSimAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

//==============================================================================
// Plugin entry
//==============================================================================

juce::AudioProcessor*
JUCE_CALLTYPE createPluginFilter()
{
    return new AmpSimAudioProcessor();
}
