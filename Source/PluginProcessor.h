#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_gui_basics/juce_gui_basics.h>

// NeuralAudio
#include "NeuralAudio/NeuralModel.h"

class AmpSimAudioProcessor : public juce::AudioProcessor
{
public:
    AmpSimAudioProcessor();
    ~AmpSimAudioProcessor() override;

    void prepareToPlay(
        double sampleRate,
        int samplesPerBlock) override;

    void releaseResources() override;

    bool isBusesLayoutSupported(
        const BusesLayout& layouts) const override;

    void processBlock(
        juce::AudioBuffer<float>& buffer,
        juce::MidiBuffer& midiMessages) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;

    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;

    void setCurrentProgram(int index) override;

    const juce::String getProgramName(
        int index) override;

    void changeProgramName(
        int index,
        const juce::String& newName) override;

    void getStateInformation(
        juce::MemoryBlock& destData) override;

    void setStateInformation(
        const void* data,
        int sizeInBytes) override;

    //==========================================================================
    // PARAMETERS
    //==========================================================================

    juce::AudioProcessorValueTreeState parameters;

    static juce::AudioProcessorValueTreeState::ParameterLayout
    createParameterLayout();

private:

    //==========================================================================
    // SAMPLE RATE
    //==========================================================================

    double currentSampleRate = 48000.0;

    //==========================================================================
    // PURE MONO WORKING BUFFER
    //
    // The entire amp + cab chain uses ONE channel.
    //==========================================================================

    juce::AudioBuffer<float> monoBuffer;

    //==========================================================================
    // NAM
    //==========================================================================

    std::unique_ptr<
        NeuralAudio::NeuralModelLoader> namLoader;

    std::unique_ptr<
        NeuralAudio::NeuralModel> namModel;

    juce::File namTempFile;

    bool namLoaded = false;

    //==========================================================================
    // MONO 4x12 CAB IR
    //==========================================================================

    juce::dsp::Convolution irConvolution;

    juce::File irTempFile;

    bool irLoaded = false;

    //==========================================================================
    // MONO DSP
    //==========================================================================

    juce::dsp::Gain<float> inputGain;
    juce::dsp::Gain<float> outputGain;

    juce::dsp::IIR::Filter<float> bassFilter;
    juce::dsp::IIR::Filter<float> midFilter;
    juce::dsp::IIR::Filter<float> highFilter;

    //==========================================================================
    // NAM TEMPORARY DATA
    //==========================================================================

    std::vector<float> namInputData;
    std::vector<float> namOutputData;

    //==========================================================================
    // PARAMETERS
    //==========================================================================

    std::atomic<float>* gainParameter = nullptr;
    std::atomic<float>* bassParameter = nullptr;
    std::atomic<float>* midParameter = nullptr;
    std::atomic<float>* highParameter = nullptr;
    std::atomic<float>* volumeParameter = nullptr;

    //==========================================================================
    // EMBEDDED FILES
    //==========================================================================

    bool createEmbeddedNAMFile();
    bool createEmbeddedIRFile();

    bool loadNAM();
    bool loadIR();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        AmpSimAudioProcessor)
};
