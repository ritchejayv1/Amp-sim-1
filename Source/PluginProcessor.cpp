#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "BinaryData.h"

#include <algorithm>
#include <cmath>

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

    namLoader =
        std::make_unique<NeuralAudio::NeuralModelLoader>();
}

//==============================================================================
AmpSimAudioProcessor::~AmpSimAudioProcessor()
{
    namModel.reset();
    namLoader.reset();
}

//==============================================================================
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
            "Hi",
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
                -24.0f,
                12.0f,
                0.01f),
            0.0f));

    return { params.begin(), params.end() };
}

//==============================================================================
void AmpSimAudioProcessor::prepareToPlay(
    double sampleRate,
    int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    //==========================================================================
    // PURE MONO WORKING SIGNAL
    //==========================================================================

    monoBuffer.setSize(
        1,
        samplesPerBlock,
        false,
        true,
        true);

    namInputData.resize(
        static_cast<size_t>(samplesPerBlock));

    namOutputData.resize(
        static_cast<size_t>(samplesPerBlock));

    //==========================================================================
    // MONO DSP SPEC
    //==========================================================================

    juce::dsp::ProcessSpec monoSpec;
    monoSpec.sampleRate = sampleRate;
    monoSpec.maximumBlockSize =
        static_cast<juce::uint32>(samplesPerBlock);
    monoSpec.numChannels = 1;

    //==========================================================================
    // INPUT / OUTPUT GAIN
    //==========================================================================

    inputGain.prepare(monoSpec);
    inputGain.reset();

    outputGain.prepare(monoSpec);
    outputGain.reset();

    //==========================================================================
    // MONO EQ
    //==========================================================================

    bassFilter.prepare(monoSpec);
    bassFilter.reset();

    midFilter.prepare(monoSpec);
    midFilter.reset();

    highFilter.prepare(monoSpec);
    highFilter.reset();

    //==========================================================================
    // MONO 4x12 CAB IR
    //==========================================================================

    irConvolution.prepare(monoSpec);
    irConvolution.reset();

    //==========================================================================
    // LOAD EMBEDDED NAM
    //==========================================================================

    loadNAM();

    //==========================================================================
    // LOAD EMBEDDED 4x12 IR
    //==========================================================================

    loadIR();
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

    monoBuffer.setSize(
        1,
        0,
        false,
        false,
        true);
}

//==============================================================================
bool AmpSimAudioProcessor::isBusesLayoutSupported(
    const BusesLayout& layouts) const
{
    const auto inputLayout =
        layouts.getChannelSet(
            true,
            0);

    const auto outputLayout =
        layouts.getChannelSet(
            false,
            0);

    if (inputLayout != juce::AudioChannelSet::mono()
        && inputLayout != juce::AudioChannelSet::stereo())
    {
        return false;
    }

    if (outputLayout != juce::AudioChannelSet::mono()
        && outputLayout != juce::AudioChannelSet::stereo())
    {
        return false;
    }

    return true;
}

//==============================================================================
void AmpSimAudioProcessor::processBlock(
    juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);

    const int numSamples =
        buffer.getNumSamples();

    if (numSamples <= 0)
        return;

    //==========================================================================
    // MAKE SURE MONO BUFFER IS LARGE ENOUGH
    //==========================================================================

    if (monoBuffer.getNumSamples() < numSamples)
    {
        monoBuffer.setSize(
            1,
            numSamples,
            false,
            true,
            true);
    }

    float* monoData =
        monoBuffer.getWritePointer(0);

    //==========================================================================
    // 1. STEREO INPUT -> PURE MONO
    //==========================================================================

    const int inputChannels =
        buffer.getNumChannels();

    if (inputChannels >= 2)
    {
        const float* left =
            buffer.getReadPointer(0);

        const float* right =
            buffer.getReadPointer(1);

        for (int i = 0; i < numSamples; ++i)
        {
            monoData[i] =
                0.5f * (left[i] + right[i]);
        }
    }
    else if (inputChannels == 1)
    {
        const float* input =
            buffer.getReadPointer(0);

        std::copy(
            input,
            input + numSamples,
            monoData);
    }
    else
    {
        monoBuffer.clear();
        return;
    }

    //==========================================================================
    // 2. INPUT GAIN
    //
    // 0  = -12 dB
    // 10 = +12 dB
    //==========================================================================

    const float gainValue =
        gainParameter != nullptr
            ? gainParameter->load()
            : 5.0f;

    const float inputDb =
        juce::jmap(
            gainValue,
            0.0f,
            10.0f,
            -12.0f,
            12.0f);

    inputGain.setGainDecibels(inputDb);

    {
        juce::dsp::AudioBlock<float> monoBlock(
            monoBuffer);

        juce::dsp::ProcessContextReplacing<float> context(
            monoBlock);

        inputGain.process(context);
    }

    //==========================================================================
    // 3. NAM AMP SIMULATION
    //==========================================================================

    if (namLoaded && namModel != nullptr)
    {
        if (static_cast<int>(namInputData.size()) < numSamples)
        {
            namInputData.resize(
                static_cast<size_t>(numSamples));

            namOutputData.resize(
                static_cast<size_t>(numSamples));
        }

        for (int i = 0; i < numSamples; ++i)
        {
            namInputData[
                static_cast<size_t>(i)] =
                monoData[i];
        }

        namModel->Process(
            namInputData.data(),
            namOutputData.data(),
            numSamples);

        for (int i = 0; i < numSamples; ++i)
        {
            monoData[i] =
                namOutputData[
                    static_cast<size_t>(i)];
        }
    }

    //==========================================================================
    // 4. BASS
    // 120 Hz LOW SHELF
    //==========================================================================

    const float bassValue =
        bassParameter != nullptr
            ? bassParameter->load()
            : 0.0f;

    *bassFilter.coefficients =
        *juce::dsp::IIR::Coefficients<float>::makeLowShelf(
            currentSampleRate,
            120.0,
            0.7071f,
            juce::Decibels::decibelsToGain(
                bassValue));

    {
        juce::dsp::AudioBlock<float> monoBlock(
            monoBuffer);

        juce::dsp::ProcessContextReplacing<float> context(
            monoBlock);

        bassFilter.process(context);
    }

    //==========================================================================
    // 5. MID
    // 750 Hz PEAK
    //==========================================================================

    const float midValue =
        midParameter != nullptr
            ? midParameter->load()
            : 0.0f;

    *midFilter.coefficients =
        *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
            currentSampleRate,
            750.0,
            0.8f,
            juce::Decibels::decibelsToGain(
                midValue));

    {
        juce::dsp::AudioBlock<float> monoBlock(
            monoBuffer);

        juce::dsp::ProcessContextReplacing<float> context(
            monoBlock);

        midFilter.process(context);
    }

    //==========================================================================
    // 6. HI
    // 4500 Hz HIGH SHELF
    //==========================================================================

    const float highValue =
        highParameter != nullptr
            ? highParameter->load()
            : 0.0f;

    *highFilter.coefficients =
        *juce::dsp::IIR::Coefficients<float>::makeHighShelf(
            currentSampleRate,
            4500.0,
            0.7071f,
            juce::Decibels::decibelsToGain(
                highValue));

    {
        juce::dsp::AudioBlock<float> monoBlock(
            monoBuffer);

        juce::dsp::ProcessContextReplacing<float> context(
            monoBlock);

        highFilter.process(context);
    }

    //==========================================================================
    // 7. 4x12 CAB IR
    // PURE MONO
    //==========================================================================

    if (irLoaded)
    {
        juce::dsp::AudioBlock<float> monoBlock(
            monoBuffer);

        juce::dsp::ProcessContextReplacing<float> context(
            monoBlock);

        irConvolution.process(context);
    }

    //==========================================================================
    // 8. OUTPUT VOLUME
    //==========================================================================

    const float volumeValue =
        volumeParameter != nullptr
            ? volumeParameter->load()
            : 0.0f;

    outputGain.setGainDecibels(
        volumeValue);

    {
        juce::dsp::AudioBlock<float> monoBlock(
            monoBuffer);

        juce::dsp::ProcessContextReplacing<float> context(
            monoBlock);

        outputGain.process(context);
    }

    //==========================================================================
    // 9. MONO -> IDENTICAL L/R
    //==========================================================================

    for (int channel = 0;
         channel < buffer.getNumChannels();
         ++channel)
    {
        float* output =
            buffer.getWritePointer(channel);

        std::copy(
            monoData,
            monoData + numSamples,
            output);
    }
}

//==============================================================================
bool AmpSimAudioProcessor::createEmbeddedNAMFile()
{
    int dataSize = 0;

    const void* data =
        BinaryData::getNamedResource(
            "RG_MBDRprecision_nam",
            dataSize);

    if (data == nullptr || dataSize <= 0)
        return false;

    namTempFile =
        juce::File::getSpecialLocation(
            juce::File::tempDirectory)
            .getChildFile(
                "RG_MBDR_precision.nam");

    if (namTempFile.existsAsFile())
        namTempFile.deleteFile();

    return namTempFile.replaceWithData(
        data,
        static_cast<size_t>(dataSize));
}

//==============================================================================
bool AmpSimAudioProcessor::createEmbeddedIRFile()
{
    int dataSize = 0;

    const void* data =
        BinaryData::getNamedResource(
            "RG_412_MB_mic_1_wav",
            dataSize);

    if (data == nullptr || dataSize <= 0)
        return false;

    irTempFile =
        juce::File::getSpecialLocation(
            juce::File::tempDirectory)
            .getChildFile(
                "RG_412_MB_mic_1.wav");

    if (irTempFile.existsAsFile())
        irTempFile.deleteFile();

    return irTempFile.replaceWithData(
        data,
        static_cast<size_t>(dataSize));
}

//==============================================================================
bool AmpSimAudioProcessor::loadNAM()
{
    namLoaded = false;

    if (!createEmbeddedNAMFile())
        return false;

    if (namLoader == nullptr)
    {
        namLoader =
            std::make_unique<
                NeuralAudio::NeuralModelLoader>();
    }

    namLoader->SetExternalSampleRate(
        static_cast<int>(currentSampleRate));

    namModel.reset(
        namLoader->CreateFromFile(
            namTempFile
                .getFullPathName()
                .toStdString()));

    if (namModel == nullptr)
        return false;

    namModel->SetMaxAudioBufferSize(
        8192);

    namLoaded = true;

    return true;
}

//==============================================================================
bool AmpSimAudioProcessor::loadIR()
{
    irLoaded = false;

    if (!createEmbeddedIRFile())
        return false;

    irConvolution.reset();

    irConvolution.loadImpulseResponse(
        irTempFile,
        juce::dsp::Convolution::Stereo::no,
        juce::dsp::Convolution::Trim::yes,
        0,
        juce::dsp::Convolution::Normalise::yes);

    irLoaded = true;

    return true;
}

//==============================================================================
const juce::String AmpSimAudioProcessor::getName() const
{
    return JucePlugin_Name;
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
const juce::String AmpSimAudioProcessor::getProgramName(
    int index)
{
    juce::ignoreUnused(index);
    return {};
}

//==============================================================================
void AmpSimAudioProcessor::changeProgramName(
    int index,
    const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

//==============================================================================
void AmpSimAudioProcessor::getStateInformation(
    juce::MemoryBlock& destData)
{
    auto state =
        parameters.copyState();

    std::unique_ptr<juce::XmlElement> xml =
        state.createXml();

    if (xml != nullptr)
    {
        copyXmlToBinary(
            *xml,
            destData);
    }
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
            juce::ValueTree::fromXml(
                *xmlState));
    }
}

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
// JUCE VST3 FACTORY
//==============================================================================

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AmpSimAudioProcessor();
}
