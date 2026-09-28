#pragma once

#include <JuceHeader.h>

// NeuralAudio
#include <NeuralAudio/NeuralAudio.h>

class AmpSimAudioProcessor : public juce::AudioProcessor
{
public:

    //==============================================================
    AmpSimAudioProcessor();
    ~AmpSimAudioProcessor() override;

    //==============================================================
    void prepareToPlay(
        double sampleRate,
        int samplesPerBlock) override;

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

    //==============================================================
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

    // MODE
    // 0 = CLEAN
    // 1 = DRIVE
    std::atomic<float>* modeParameter = nullptr;

    // AMP
    // 0 = OFF
    // 1 = ON
    std::atomic<float>* ampParameter = nullptr;

    //==============================================================
    // NAM
    //==============================================================

    std::unique_ptr<NeuralAudio::NeuralModelLoader> namLoader;

    std::unique_ptr<NeuralAudio::NeuralModel> namModel;

    bool namLoaded = false;

    std::vector<float> namInputData;
    std::vector<float> namOutputData;

    //==============================================================
    // AUDIO
    //==============================================================

    juce::AudioBuffer<float> monoBuffer;

    //==============================================================
    // FILTERS
    //==============================================================

    using Filter =
        juce::dsp::IIR::Filter<float>;

    juce::dsp::ProcessorChain<
        Filter,
        Filter,
        Filter> eqChain;

    //==============================================================
    // IR
    //==============================================================

    juce::dsp::Convolution irConvolution;

    //==============================================================
    // DSP
    //==============================================================

    juce::dsp::Gain<float> inputGain;
    juce::dsp::Gain<float> outputGain;

    juce::dsp::ProcessSpec monoSpec;

    double currentSampleRate = 44100.0;

    int maximumBlockSize = 0;

    //==============================================================
    // RESOURCE LOADING
    //==============================================================

    void loadNAM();

    void loadIR();

    //==============================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        AmpSimAudioProcessor)
};
