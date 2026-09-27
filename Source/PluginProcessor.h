#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_gui_basics/juce_gui_basics.h>

// NeuralAudio
#include <NeuralAudio.h>

class AmpSimAudioProcessor : public juce::AudioProcessor
{
public:
    AmpSimAudioProcessor();
    ~AmpSimAudioProcessor() override;

    //==============================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(
        const BusesLayout& layouts) const override;

    void processBlock(
        juce::AudioBuffer<float>&,
        juce::MidiBuffer&) override;

    //==============================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;

    double getTailLengthSeconds() const override;

    //==============================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(
        int index,
        const juce::String& newName) override;

    //==============================================================
    void getStateInformation(
        juce::MemoryBlock& destData) override;

    void setStateInformation(
        const void* data,
        int sizeInBytes) override;

    //==============================================================
    juce::AudioProcessorValueTreeState parameters;

    static juce::AudioProcessorValueTreeState::ParameterLayout
    createParameterLayout();

private:

    //==============================================================
    // NAM
    //==============================================================

    std::unique_ptr<NeuralAudio::NeuralModelLoader> namLoader;

    std::unique_ptr<NeuralAudio::NeuralModel> namModel;

    juce::File namTempFile;

    bool namLoaded = false;

    //==============================================================
    // IR
    //==============================================================

    juce::dsp::Convolution irConvolution;

    juce::File irTempFile;

    bool irLoaded = false;

    //==============================================================
    // DSP
    //==============================================================

    juce::dsp::Gain<float> inputGain;
    juce::dsp::Gain<float> outputGain;

    juce::dsp::IIR::Filter<float> bassFilter;
    juce::dsp::IIR::Filter<float> midFilter;
    juce::dsp::IIR::Filter<float> highFilter;

    //==============================================================
    // NAM buffers
    //==============================================================

    juce::AudioBuffer<float> namInputBuffer;
    juce::AudioBuffer<float> namOutputBuffer;

    std::vector<float> namInputData;
    std::vector<float> namOutputData;

    //==============================================================
    // Parameters
    //==============================================================

    std::atomic<float>* gainParameter   = nullptr;
    std::atomic<float>* bassParameter   = nullptr;
    std::atomic<float>* midParameter    = nullptr;
    std::atomic<float>* highParameter   = nullptr;
    std::atomic<float>* volumeParameter = nullptr;

    //==============================================================
    // Helpers
    //==============================================================

    bool createEmbeddedNAMFile();

    bool createEmbeddedIRFile();

    bool loadNAM();

    bool loadIR();

    //==============================================================

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        AmpSimAudioProcessor)
};
