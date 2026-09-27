#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "BinaryData.h"

#include <cmath>

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
    // Extract embedded NAM
    //==============================================================

    namTempFile =
        juce::File::getSpecialLocation(
            juce::File::tempDirectory)
            .getChildFile("RG_AmpSim_Internal_Model.nam");

    if (namTempFile.replaceWithData(
            BinaryData::RG_MBDRprecision_nam,
            static_cast<size_t>(
                BinaryData::RG_MBDRprecision_namSize)))
    {
        DBG("RG Amp SIM: Embedded NAM extracted.");
        DBG("NAM size: "
            + juce::String(BinaryData::RG_MBDRprecision_namSize));
    }
    else
    {
        DBG("RG Amp SIM: FAILED to extract NAM.");
    }

    //==============================================================
    // Extract embedded IR
    //==============================================================

    irTempFile =
        juce::File::getSpecialLocation(
            juce::File::tempDirectory)
            .getChildFile("RG_AmpSim_Internal_IR.wav");

    if (irTempFile.replaceWithData(
            BinaryData::RG_412_MB_mic_1_wav,
            static_cast<size_t>(
                BinaryData::RG_412_MB_mic_1_wavSize)))
    {
        DBG("RG Amp SIM: Embedded IR extracted.");
        DBG("IR size: "
            + juce::String(BinaryData::RG_412_MB_mic_1_wavSize));
    }
    else
    {
        DBG("RG Amp SIM: FAILED to extract IR.");
    }

    //==============================================================
    // Load NAM
    //==============================================================

    namLoaded = loadNAM();

    if (namLoaded)
        DBG("RG Amp SIM: NAM LOAD SUCCESS.");
    else
        DBG("RG Amp SIM: NAM LOAD FAILED.");

    //==============================================================
    // Load IR
    //==============================================================

    irLoaded = loadIR();

    if (irLoaded)
        DBG("RG Amp SIM: IR LOAD SUCCESS.");
    else
        DBG("RG Amp SIM: IR LOAD FAILED.");
}

//==============================================================================

AmpSimAudioProcessor::~AmpSimAudioProcessor()
{
    namModel.reset();
    namLoader.reset();

    namTempFile.deleteFile();
    irTempFile.deleteFile();
}

//==============================================================================
// Parameter Layout
//==============================================================================

juce::AudioProcessorValueTreeState::ParameterLayout
AmpSimAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // GAIN
    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "GAIN",
            "Gain",
            juce::NormalisableRange<float>(
                0.0f,
                10.0f,
                0.01f),
            5.0f));

    // BASS
    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "BASS",
            "Bass",
            juce::NormalisableRange<float>(
                -12.0f,
                12.0f,
                0.01f),
            0.0f));

    // MID
    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "MID",
            "Mid",
            juce::NormalisableRange<float>(
                -12.0f,
                12.0f,
                0.01f),
            0.0f));

    // HI
    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "HI",
            "High",
            juce::NormalisableRange<float>(
                -12.0f,
                12.0f,
                0.01f),
            0.0f));

    // VOLUME
    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "VOLUME",
            "Volume",
            juce::NormalisableRange<float>(
                -24.0f,
                12.0f,
                0.01f),
            0.0f));

    return { params.begin(), params.end() };
}

//==============================================================================
// Prepare
//==============================================================================

void AmpSimAudioProcessor::prepareToPlay(
    double sampleRate,
    int samplesPerBlock)
{
    DBG("RG Amp SIM: prepareToPlay");
    DBG("Sample rate: " + juce::String(sampleRate));
    DBG("Block size: " + juce::String(samplesPerBlock));

    //==============================================================
    // NAM buffer sizes
    //==============================================================

    namInputBuffer.setSize(
        1,
        samplesPerBlock);

    namOutputBuffer.setSize(
        1,
        samplesPerBlock);

    namInputData.resize(
        static_cast<size_t>(samplesPerBlock));

    namOutputData.resize(
        static_cast<size_t>(samplesPerBlock));

    //==============================================================
    // NAM sample-rate configuration
    //==============================================================

    if (namLoader != nullptr)
    {
        namLoader->SetExternalSampleRate(sampleRate);
    }

    if (namModel != nullptr)
    {
        namModel->SetMaxAudioBufferSize(samplesPerBlock);
    }

    //==============================================================
    // Input gain
    //==============================================================

    inputGain.prepare(
        juce::dsp::ProcessSpec{
            sampleRate,
            static_cast<juce::uint32>(samplesPerBlock),
            2
        });

    //==============================================================
    // Output gain
    //==============================================================

    outputGain.prepare(
        juce::dsp::ProcessSpec{
            sampleRate,
            static_cast<juce::uint32>(samplesPerBlock),
            2
        });

    //==============================================================
    // EQ filters
    //==============================================================

    bassFilter.prepare(
        juce::dsp::ProcessSpec{
            sampleRate,
            static_cast<juce::uint32>(samplesPerBlock),
            2
        });

    midFilter.prepare(
        juce::dsp::ProcessSpec{
            sampleRate,
            static_cast<juce::uint32>(samplesPerBlock),
            2
        });

    highFilter.prepare(
        juce::dsp::ProcessSpec{
            sampleRate,
            static_cast<juce::uint32>(samplesPerBlock),
            2
        });

    //==============================================================
    // Reset DSP
    //==============================================================

    inputGain.reset();
    outputGain.reset();

    bassFilter.reset();
    midFilter.reset();
    highFilter.reset();

    //==============================================================
    // IR convolution
    //==============================================================

    irConvolution.reset();

    if (irLoaded && irTempFile.existsAsFile())
    {
        DBG("RG Amp SIM: Preparing IR convolution...");

        try
        {
            irConvolution.loadImpulseResponse(
                irTempFile,
                juce::dsp::Convolution::Stereo::yes,
                juce::dsp::Convolution::Trim::yes,
                0,
                juce::dsp::Convolution::Normalise::yes);

            DBG("RG Amp SIM: IR convolution prepared.");
        }
        catch (...)
        {
            DBG("RG Amp SIM: IR convolution FAILED.");
            irLoaded = false;
        }
    }

    //==============================================================
    // Initial filter coefficients
    //==============================================================

    bassFilter.coefficients =
        juce::dsp::IIR::Coefficients<float>::makeLowShelf(
            sampleRate,
            120.0f,
            0.707f,
            juce::Decibels::decibelsToGain(
                bassParameter != nullptr
                    ? bassParameter->load()
                    : 0.0f));

    midFilter.coefficients =
        juce::dsp::IIR::Coefficients<float>::makePeakFilter(
            sampleRate,
            750.0f,
            0.8f,
            juce::Decibels::decibelsToGain(
                midParameter != nullptr
                    ? midParameter->load()
                    : 0.0f));

    highFilter.coefficients =
        juce::dsp::IIR::Coefficients<float>::makeHighShelf(
            sampleRate,
            4500.0f,
            0.707f,
            juce::Decibels::decibelsToGain(
                highParameter != nullptr
                    ? highParameter->load()
                    : 0.0f));
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
}

//==============================================================================

bool AmpSimAudioProcessor::isBusesLayoutSupported(
    const BusesLayout& layouts) const
{
    const auto mainInput =
        layouts.getChannelSet(
            true,
            0);

    const auto mainOutput =
        layouts.getChannelSet(
            false,
            0);

    if (mainInput != juce::AudioChannelSet::mono()
        && mainInput != juce::AudioChannelSet::stereo())
    {
        return false;
    }

    if (mainOutput != juce::AudioChannelSet::mono()
        && mainOutput != juce::AudioChannelSet::stereo())
    {
        return false;
    }

    return true;
}

//==============================================================================
// Audio Processing
//==============================================================================

void AmpSimAudioProcessor::processBlock(
    juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);

    const int numSamples =
        buffer.getNumSamples();

    const int numChannels =
        buffer.getNumChannels();

    if (numSamples <= 0 || numChannels <= 0)
        return;

    //==============================================================
    // INPUT GAIN
    //==============================================================

    const float gainValue =
        gainParameter != nullptr
            ? gainParameter->load()
            : 5.0f;

    // 0 -> -12 dB
    // 5 -> 0 dB
    // 10 -> +12 dB

    const float inputDb =
        juce::jmap(
            gainValue,
            0.0f,
            10.0f,
            -12.0f,
            12.0f);

    inputGain.setGainDecibels(inputDb);

    {
        juce::dsp::AudioBlock<float> block(buffer);

        juce::dsp::ProcessContextReplacing<float> context(block);

        inputGain.process(context);
    }

    //==============================================================
    // NAM
    //==============================================================

    if (namLoaded && namModel != nullptr)
    {
        // Convert stereo input to mono for NAM.
        for (int sample = 0;
             sample < numSamples;
             ++sample)
        {
            float value = 0.0f;

            if (numChannels == 1)
            {
                value =
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

                value =
                    0.5f *
                    (left + right);
            }

            namInputData[
                static_cast<size_t>(sample)]
                = value;
        }

        // Actual NeuralAudio processing.
        namModel->Process(
            namInputData.data(),
            namOutputData.data(),
            numSamples);

        // Return NAM mono output to stereo.
        if (numChannels == 1)
        {
            buffer.copyFrom(
                0,
                0,
                namOutputData.data(),
                numSamples);
        }
        else
        {
            buffer.copyFrom(
                0,
                0,
                namOutputData.data(),
                numSamples);

            buffer.copyFrom(
                1,
                0,
                namOutputData.data(),
                numSamples);
        }
    }

    //==============================================================
    // BASS
    //==============================================================

    const float bassDb =
        bassParameter != nullptr
            ? bassParameter->load()
            : 0.0f;

    {
        juce::dsp::AudioBlock<float> block(buffer);

        bassFilter.coefficients =
            juce::dsp::IIR::Coefficients<float>::makeLowShelf(
                getSampleRate(),
                120.0f,
                0.707f,
                juce::Decibels::decibelsToGain(bassDb));

        juce::dsp::ProcessContextReplacing<float> context(block);

        bassFilter.process(context);
    }

    //==============================================================
    // MID
    //==============================================================

    const float midDb =
        midParameter != nullptr
            ? midParameter->load()
            : 0.0f;

    {
        juce::dsp::AudioBlock<float> block(buffer);

        midFilter.coefficients =
            juce::dsp::IIR::Coefficients<float>::makePeakFilter(
                getSampleRate(),
                750.0f,
                0.8f,
                juce::Decibels::decibelsToGain(midDb));

        juce::dsp::ProcessContextReplacing<float> context(block);

        midFilter.process(context);
    }

    //==============================================================
    // HIGH
    //==============================================================

    const float highDb =
        highParameter != nullptr
            ? highParameter->load()
            : 0.0f;

    {
        juce::dsp::AudioBlock<float> block(buffer);

        highFilter.coefficients =
            juce::dsp::IIR::Coefficients<float>::makeHighShelf(
                getSampleRate(),
                4500.0f,
                0.707f,
                juce::Decibels::decibelsToGain(highDb));

        juce::dsp::ProcessContextReplacing<float> context(block);

        highFilter.process(context);
    }

    //==============================================================
    // 4x12 CAB IR
    //==============================================================

    if (irLoaded)
    {
        juce::dsp::AudioBlock<float> block(buffer);

        juce::dsp::ProcessContextReplacing<float> context(block);

        irConvolution.process(context);
    }

    //==============================================================
    // OUTPUT VOLUME
    //==============================================================

    const float volumeDb =
        volumeParameter != nullptr
            ? volumeParameter->load()
            : 0.0f;

    outputGain.setGainDecibels(volumeDb);

    {
        juce::dsp::AudioBlock<float> block(buffer);

        juce::dsp::ProcessContextReplacing<float> context(block);

        outputGain.process(context);
    }
}

//==============================================================================
// NAM Loading
//==============================================================================

bool AmpSimAudioProcessor::loadNAM()
{
    if (!namTempFile.existsAsFile())
    {
        DBG("RG Amp SIM: NAM file does not exist.");
        return false;
    }

    DBG("RG Amp SIM: NAM file:");
    DBG(namTempFile.getFullPathName());

    try
    {
        namLoader =
            std::make_unique<
                NeuralAudio::NeuralModelLoader>();

        // The NAM models are normally designed around 48 kHz.
        namLoader->SetExternalSampleRate(48000.0);

        namModel =
            namLoader->CreateFromFile(
                namTempFile
                    .getFullPathName()
                    .toStdString());

        if (namModel == nullptr)
        {
            DBG("RG Amp SIM: CreateFromFile returned NULL.");
            return false;
        }

        DBG("RG Amp SIM: Neural model created.");

        const auto inputAdjustment =
            namModel->GetRecommendedInputDBAdjustment();

        const auto outputAdjustment =
            namModel->GetRecommendedOutputDBAdjustment();

        DBG(
            "NAM recommended input adjustment: "
            + juce::String(inputAdjustment));

        DBG(
            "NAM recommended output adjustment: "
            + juce::String(outputAdjustment));

        DBG(
            "NAM model version: "
            + juce::String(
                namModel->GetModelVersion()));

        return true;
    }
    catch (...)
    {
        DBG("RG Amp SIM: Exception while loading NAM.");
        namModel.reset();
        namLoader.reset();
        return false;
    }
}

//==============================================================================
// IR Loading
//==============================================================================

bool AmpSimAudioProcessor::loadIR()
{
    if (!irTempFile.existsAsFile())
    {
        DBG("RG Amp SIM: IR file does not exist.");
        return false;
    }

    DBG("RG Amp SIM: IR file:");
    DBG(irTempFile.getFullPathName());

    // We only mark it loaded here.
    // Actual convolution loading happens in prepareToPlay().
    return true;
}

//==============================================================================
// State
//==============================================================================

void AmpSimAudioProcessor::getStateInformation(
    juce::MemoryBlock& destData)
{
    auto state =
        parameters.copyState();

    std::unique_ptr<juce::XmlElement> xml =
        parameters.stateToXml(state);

    copyXmlToBinary(
        *xml,
        destData);
}

//==============================================================================

void AmpSimAudioProcessor::setStateInformation(
    const void* data,
    int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState =
        getXmlFromBinary(
            data,
            sizeInBytes);

    if (xmlState != nullptr &&
        xmlState->hasTagName(
            parameters.state.getType()))
    {
        parameters.replaceState(
            parameters.xmlToState(
                *xmlState));
    }
}

//==============================================================================
// Plugin information
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

int AmpSimAudioProcessor::getNumPrograms()
{
    return 1;
}

//==============================================================================

int AmpSimAudioProcessor::getCurrentProgram()
{
    return 0;
}

//==============================================================================

void AmpSimAudioProcessor::setCurrentProgram(
    int index)
{
    juce::ignoreUnused(index);
}

//==============================================================================

const juce::String
AmpSimAudioProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);

    return {};
}

//==============================================================================

void AmpSimAudioProcessor::changeProgramName(
    int index,
    const juce::String& newName)
{
    juce::ignoreUnused(index);
    juce::ignoreUnused(newName);
}

//==============================================================================

bool AmpSimAudioProcessor::hasEditor() const
{
    return true;
}

//==============================================================================

juce::AudioProcessorEditor*
AmpSimAudioProcessor::createEditor()
{
    return new AmpSimAudioProcessorEditor(*this);
}

//==============================================================================
// JUCE factory
//==============================================================================

juce::AudioProcessor* JUCE_CALLTYPE
createPluginFilter()
{
    return new AmpSimAudioProcessor();
}
