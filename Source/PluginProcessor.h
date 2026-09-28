#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

//==============================================================
// NeuralAudio
//
// NeuralModel.h contains:
//   NeuralAudio::NeuralModel
//   NeuralAudio::NeuralModelLoader
//==============================================================

#include <NeuralModel.h>

//==============================================================
// RG AMP SIM AUDIO PROCESSOR
//==============================================================

class AmpSimAudioProcessor : public juce::AudioProcessor
{
public:

    //==============================================================
    // CONSTRUCTOR / DESTRUCTOR
    //==============================================================

    AmpSimAudioProcessor();
    ~AmpSimAudioProcessor() override;

    //==============================================================
    // AUDIO
    //==============================================================

    void prepareToPlay(
        double sampleRate,
        int samplesPerBlock) override;

    void releaseResources() override;

    bool isBusesLayoutSupported(
        const BusesLayout& layouts) const override;

    void processBlock(
        juce::AudioBuffer<float>& buffer,
        juce::MidiBuffer& midiMessages) override;

    //==============================================================
    // EDITOR
    //==============================================================

    juce::AudioProcessorEditor* createEditor() override;

    bool hasEditor() const override;

    //==============================================================
    // PLUGIN INFORMATION
    //==============================================================

    const juce::String getName() const override;

    bool acceptsMidi() const override;

    bool producesMidi() const override;

    bool isMidiEffect() const override;

    double getTailLengthSeconds() const override;

    //==============================================================
    // PROGRAMS
    //==============================================================

    int getNumPrograms() override;

    int getCurrentProgram() override;

    void setCurrentProgram(
        int index) override;

    const juce::String getProgramName(
        int index) override;

    void changeProgramName(
        int index,
        const juce::String& newName) override;

    //==============================================================
    // STATE
    //==============================================================

    void getStateInformation(
        juce::MemoryBlock& destData) override;

    void setStateInformation(
        const void* data,
        int sizeInBytes) override;

    //==============================================================
    // PARAMETERS
    //==============================================================

    juce::AudioProcessorValueTreeState parameters;

    static juce::AudioProcessorValueTreeState::ParameterLayout
    createParameterLayout();

private:

    //==============================================================
    // PARAMETERS
    //==============================================================

    std::atomic<float>* gainParameter   = nullptr;
    std::atomic<float>* bassParameter   = nullptr;
    std::atomic<float>* midParameter    = nullptr;
    std::atomic<float>* highParameter   = nullptr;
    std::atomic<float>* volumeParameter = nullptr;

    //==============================================================
    // MODE
    //
    // 0 = CLEAN
    // 1 = DRIVE
    //==============================================================

    std::atomic<float>* modeParameter = nullptr;

    //==============================================================
    // AMP
    //
    // 0 = OFF
    // 1 = ON
    //
    // AMP OFF = TRUE DRY BYPASS
    //==============================================================

    std::atomic<float>* ampParameter = nullptr;

    //==============================================================
    // NEURAL AMP MODEL
    //==============================================================

    // Direct object.
    // NeuralAudio's API is used as:
    //
    // NeuralModelLoader loader;
    // loader.CreateFromFile(...);

    NeuralAudio::NeuralModelLoader namLoader;

    // Actual loaded NAM model.
    std::unique_ptr<NeuralAudio::NeuralModel>
        namModel;

    bool namLoaded = false;

    //==============================================================
    // NAM AUDIO BUFFERS
    //==============================================================

    std::vector<float> namInputData;

    std::vector<float> namOutputData;

    //==============================================================
    // MONO AUDIO BUFFER
    //==============================================================

    juce::AudioBuffer<float> monoBuffer;

    //==============================================================
    // EQ FILTERS
    //
    // 0 = BASS
    // 1 = MID
    // 2 = HIGH
    //==============================================================

    using Filter =
        juce::dsp::IIR::Filter<float>;

    juce::dsp::ProcessorChain<
        Filter,
        Filter,
        Filter> eqChain;

    //==============================================================
    // 4x12 CABINET IR
    //==============================================================

    juce::dsp::Convolution irConvolution;

    //==============================================================
    // INPUT / OUTPUT GAIN
    //==============================================================

    juce::dsp::Gain<float> inputGain;

    juce::dsp::Gain<float> outputGain;

    //==============================================================
    // DSP PROCESS SPEC
    //==============================================================

    juce::dsp::ProcessSpec monoSpec;

    //==============================================================
    // AUDIO SETTINGS
    //==============================================================

    double currentSampleRate = 44100.0;

    int maximumBlockSize = 0;

    //==============================================================
    // RESOURCE LOADING
    //==============================================================

    void loadNAM();

    void loadIR();

    //==============================================================
    // JUCE
    //==============================================================

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        AmpSimAudioProcessor)
};
