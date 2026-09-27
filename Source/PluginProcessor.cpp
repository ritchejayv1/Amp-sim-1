#include "PluginProcessor.h"
#include "PluginEditor.h"

#include "BinaryData.h"

#include <cmath>

//==============================================================================
// Temporary extracted filenames
//==============================================================================

namespace
{
    constexpr const char* tempNAMName =
        "RG_AmpSim_Internal_Model.nam";

    constexpr const char* tempIRName =
        "RG_AmpSim_Internal_IR.wav";

    //==========================================================================
    // Try several possible JUCE BinaryData resource names.
    //
    // This avoids directly referencing a generated C++ symbol whose exact
    // name can change depending on JUCE's resource-name sanitisation.
    //==========================================================================

    const void* findEmbeddedResource(
        const char* const* names,
        int numberOfNames,
        int& dataSize)
    {
        dataSize = 0;

        for (int i = 0; i < numberOfNames; ++i)
        {
            const void* data =
                BinaryData::getNamedResource(
                    names[i],
                    dataSize);

            if (data != nullptr &&
                dataSize > 0)
            {
                return data;
            }
        }

        dataSize = 0;
        return nullptr;
    }
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
    //==============================================================
    // Parameter pointers
    //==============================================================

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
    // NeuralAudio loader
    //==============================================================

    namLoader =
        std::make_unique<
            NeuralAudio::NeuralModelLoader>();

    //==============================================================
    // Temporary files
    //==============================================================

    const auto tempDirectory =
        juce::File::getSpecialLocation(
            juce::File::tempDirectory);

    namTempFile =
        tempDirectory.getChildFile(
            tempNAMName);

    irTempFile =
        tempDirectory.getChildFile(
            tempIRName);

    //==============================================================
    // Extract embedded assets
    //==============================================================

    const bool namExtracted =
        createEmbeddedNAMFile();

    const bool irExtracted =
        createEmbeddedIRFile();

    //==============================================================
    // Load NAM
    //==============================================================

    if (namExtracted)
        loadNAM();

    //==============================================================
    // Load IR
    //==============================================================

    if (irExtracted)
        loadIR();
}

//==============================================================================
// Destructor
//==============================================================================

AmpSimAudioProcessor::~AmpSimAudioProcessor()
{
    namModel.reset();

    namLoader.reset();

    irConvolution.reset();

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
    std::vector<
        std::unique_ptr<
            juce::RangedAudioParameter>>
        params;

    //==============================================================
    // GAIN
    //==============================================================

    params.push_back(
        std::make_unique<
            juce::AudioParameterFloat>(
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
        std::make_unique<
            juce::AudioParameterFloat>(
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
        std::make_unique<
            juce::AudioParameterFloat>(
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
        std::make_unique<
            juce::AudioParameterFloat>(
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
        std::make_unique<
            juce::AudioParameterFloat>(
                "VOLUME",
                "Volume",

                juce::NormalisableRange<float>(
                    -24.0f,
                    12.0f,
                    0.01f),

                0.0f,

                "dB"));

    return
    {
        params.begin(),
        params.end()
    };
}

//==============================================================================
// Create Embedded NAM File
//==============================================================================

bool AmpSimAudioProcessor::createEmbeddedNAMFile()
{
    //==============================================================
    // Possible JUCE resource names.
    //
    // Original asset:
    //
    // RG MBDR+precision.nam
    //
    //==============================================================

    const char* names[] =
    {
        "RG_MBDR_precision_nam",
        "RG_MBDR_precision",
        "RGMBDRprecision_nam",
        "RGMBDRprecision"
    };

    int dataSize = 0;

    const void* data =
        findEmbeddedResource(
            names,
            static_cast<int>(
                sizeof(names) / sizeof(names[0])),
            dataSize);

    //==============================================================
    // Asset not found
    //==============================================================

    if (data == nullptr ||
        dataSize <= 0)
    {
        namLoaded = false;
        return false;
    }

    //==============================================================
    // Delete old temporary file
    //==============================================================

    if (namTempFile.existsAsFile())
        namTempFile.deleteFile();

    //==============================================================
    // Extract NAM
    //==============================================================

    const bool written =
        namTempFile.replaceWithData(
            data,
            static_cast<size_t>(
                dataSize));

    if (!written)
    {
        namLoaded = false;
        return false;
    }

    return true;
}

//==============================================================================
// Create Embedded IR File
//==============================================================================

bool AmpSimAudioProcessor::createEmbeddedIRFile()
{
    //==============================================================
    // Possible JUCE resource names.
    //
    // Original asset:
    //
    // RG 412 MB mic 1.wav
    //
    //==============================================================

    const char* names[] =
    {
        "RG_412_MB_mic_1_wav",
        "RG_412_MB_mic_1",
        "RG_412_MB_mic_1wav",
        "RG412MBmic1_wav",
        "RG412MBmic1"
    };

    int dataSize = 0;

    const void* data =
        findEmbeddedResource(
            names,
            static_cast<int>(
                sizeof(names) / sizeof(names[0])),
            dataSize);

    //==============================================================
    // Asset not found
    //==============================================================

    if (data == nullptr ||
        dataSize <= 0)
    {
        irLoaded = false;
        return false;
    }

    //==============================================================
    // Delete old temporary file
    //==============================================================

    if (irTempFile.existsAsFile())
        irTempFile.deleteFile();

    //==============================================================
    // Extract WAV
    //==============================================================

    const bool written =
        irTempFile.replaceWithData(
            data,
            static_cast<size_t>(
                dataSize));

    if (!written)
    {
        irLoaded = false;
        return false;
    }

    return true;
}

//==============================================================================
// Load NAM
//==============================================================================

bool AmpSimAudioProcessor::loadNAM()
{
    namLoaded = false;

    if (!namLoader)
        return false;

    if (!namTempFile.existsAsFile())
        return false;

    //==============================================================
    // NeuralAudio buffer configuration
    //==============================================================

    namLoader->SetDefaultMaxAudioBufferSize(
        2048);

    //==============================================================
    // Remove old model
    //==============================================================

    namModel.reset();

    //==============================================================
    // Load NAM model
    //==============================================================

    namModel =
        std::unique_ptr<
            NeuralAudio::NeuralModel>(
                namLoader->CreateFromFile(
                    namTempFile
                        .getFullPathName()
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
    irLoaded = false;

    if (!irTempFile.existsAsFile())
        return false;

    irLoaded = true;

    return true;
}

//==============================================================================
// Prepare To Play
//==============================================================================

void AmpSimAudioProcessor::prepareToPlay(
    double sampleRate,
    int samplesPerBlock)
{
    //==============================================================
    // DSP specification
    //==============================================================

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
    // INPUT GAIN
    //==============================================================

    inputGain.prepare(spec);

    inputGain.setRampDurationSeconds(
        0.02);

    inputGain.setGainDecibels(
        0.0f);

    //==============================================================
    // OUTPUT VOLUME
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
    // CABINET IR
    //==============================================================

    irConvolution.reset();

    irLoaded = false;

    if (irTempFile.existsAsFile())
    {
        irConvolution.loadImpulseResponse(
            irTempFile,
            juce::dsp::Convolution::Stereo::yes,
            juce::dsp::Convolution::Trim::yes,
            0,
            juce::dsp::Convolution::Normalise::yes);

        irLoaded = true;
    }

    //==============================================================
    // NAM buffers
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
        static_cast<size_t>(
            samplesPerBlock));

    namOutputData.resize(
        static_cast<size_t>(
            samplesPerBlock));

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
// Release Resources
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
// DSP PROCESSING
//
// INPUT
//   ↓
// GAIN
//   ↓
// NAM AMP MODEL
//   ↓
// BASS
//   ↓
// MID
//   ↓
// HI
//   ↓
// 4x12 IR CABINET
//   ↓
// VOLUME
//   ↓
// OUTPUT
//
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

    if (numSamples <= 0 ||
        numChannels <= 0)
    {
        return;
    }

    //==============================================================
    // Parameters
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
    // 1. INPUT GAIN
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
    // 2. NAM AMP MODEL
    //==============================================================

    if (namLoaded &&
        namModel != nullptr)
    {
        //==========================================================
        // Resize buffers when necessary
        //==========================================================

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
        // Stereo -> Mono
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
                const float left =
                    buffer.getSample(
                        0,
                        sample);

                const float right =
                    buffer.getSample(
                        1,
                        sample);

                mono =
                    0.5f *
                    (left + right);
            }

            namInputData[
                static_cast<size_t>(
                    sample)] = mono;
        }

        //==========================================================
        // Process NAM
        //==========================================================

        namModel->Process(
            namInputData.data(),
            namOutputData.data(),
            numSamples);

        //==========================================================
        // Mono -> Stereo
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
    // 3. BASS
    //==============================================================

    {
        auto bassCoefficients =
            juce::dsp::IIR::Coefficients<float>::
                makeLowShelf(
                    getSampleRate(),
                    120.0f,
                    0.7071f,
                    juce::Decibels::
                        decibelsToGain(
                            bass));

        bassFilter.coefficients =
            bassCoefficients;

        juce::dsp::AudioBlock<float>
            block(buffer);

        juce::dsp::ProcessContextReplacing<float>
            context(block);

        bassFilter.process(context);
    }

    //==============================================================
    // 4. MID
    //==============================================================

    {
        auto midCoefficients =
            juce::dsp::IIR::Coefficients<float>::
                makePeakFilter(
                    getSampleRate(),
                    750.0f,
                    0.8f,
                    juce::Decibels::
                        decibelsToGain(
                            mid));

        midFilter.coefficients =
            midCoefficients;

        juce::dsp::AudioBlock<float>
            block(buffer);

        juce::dsp::ProcessContextReplacing<float>
            context(block);

        midFilter.process(context);
    }

    //==============================================================
    // 5. HI
    //==============================================================

    {
        auto highCoefficients =
            juce::dsp::IIR::Coefficients<float>::
                makeHighShelf(
                    getSampleRate(),
                    4500.0f,
                    0.7071f,
                    juce::Decibels::
                        decibelsToGain(
                            high));

        highFilter.coefficients =
            highCoefficients;

        juce::dsp::AudioBlock<float>
            block(buffer);

        juce::dsp::ProcessContextReplacing<float>
            context(block);

        highFilter.process(context);
    }

    //==============================================================
    // 6. 4x12 CABINET / MICROPHONE IR
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
    // 7. MASTER VOLUME
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
// Programs
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

    std::unique_ptr<
        juce::XmlElement>
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
    std::unique_ptr<
        juce::XmlElement>
        xml(
            getXmlFromBinary(
                data,
                sizeInBytes));

    if (xml != nullptr &&
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
// Plugin Name
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
// Plugin Entry
//==============================================================================

juce::AudioProcessor*
JUCE_CALLTYPE createPluginFilter()
{
    return new AmpSimAudioProcessor();
}
