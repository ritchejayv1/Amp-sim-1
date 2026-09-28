//==============================================================
// PROCESS BLOCK
//==============================================================

void AmpSimAudioProcessor::processBlock(
    juce::AudioBuffer<float>& buffer,
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
    // READ AMP STATE FIRST
    //
    // AMP ON  = full processing
    // AMP OFF = completely dry / true bypass
    //==========================================================

    const bool ampIsOn =
        ampParameter != nullptr
            ? ampParameter->load() >= 0.5f
            : true;

    //==========================================================
    // MASTER AMP BYPASS
    //
    // IMPORTANT:
    // Nothing before this point modifies the audio.
    //
    // AMP OFF:
    // INPUT -> OUTPUT
    //
    // No GAIN
    // No NAM
    // No EQ
    // No IR
    // No output boost
    // No VOLUME
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

    //==========================================================
    // MODE
    //
    // 0 = CLEAN
    // 1 = DRIVE
    //==========================================================

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
        for (int i = 0; i < numSamples; ++i)
        {
            monoData[i] =
                0.5f *
                (left[i] + right[i]);
        }
    }
    else
    {
        for (int i = 0; i < numSamples; ++i)
        {
            monoData[i] =
                left[i];
        }
    }

    //==========================================================
    // INPUT GAIN
    //
    // 0..10
    // -12 dB .. +12 dB
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
        juce::dsp::AudioBlock<float>
            monoBlock(
                monoData,
                static_cast<size_t>(numSamples));

        juce::dsp::ProcessContextReplacing<float>
            context(monoBlock);

        inputGain.process(context);
    }

    //==========================================================
    // DRIVE MODE
    //
    // CLEAN  = NAM bypassed
    // DRIVE  = NAM active
    //==========================================================

    const bool shouldProcessNAM =
        driveMode &&
        namLoaded &&
        namModel != nullptr;

    if (shouldProcessNAM)
    {
        //======================================================
        // COPY TO NAM INPUT
        //======================================================

        for (int i = 0; i < numSamples; ++i)
        {
            namInputData[
                static_cast<size_t>(i)]
                = monoData[i];
        }

        //======================================================
        // NAM
        //======================================================

        try
        {
            namModel->Process(
                namInputData.data(),
                namOutputData.data(),
                numSamples);

            //==================================================
            // NAM OUTPUT
            //==================================================

            for (int i = 0; i < numSamples; ++i)
            {
                monoData[i] =
                    namOutputData[
                        static_cast<size_t>(i)];
            }
        }
        catch (...)
        {
            // Keep signal before NAM if processing fails.
        }
    }

    //==========================================================
    // BASS
    //==========================================================

    {
        auto* coefficients =
            eqChain.get<0>().state;

        *coefficients =
            *juce::dsp::IIR::Coefficients<float>::makeLowShelf(
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
        auto* coefficients =
            eqChain.get<1>().state;

        *coefficients =
            *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
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
        auto* coefficients =
            eqChain.get<2>().state;

        *coefficients =
            *juce::dsp::IIR::Coefficients<float>::makeHighShelf(
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
        juce::dsp::AudioBlock<float>
            monoBlock(
                monoData,
                static_cast<size_t>(numSamples));

        juce::dsp::ProcessContextReplacing<float>
            context(monoBlock);

        eqChain.process(context);
    }

    //==========================================================
    // 4x12 IR
    //==========================================================

    {
        juce::dsp::AudioBlock<float>
            monoBlock(
                monoData,
                static_cast<size_t>(numSamples));

        juce::dsp::ProcessContextReplacing<float>
            context(monoBlock);

        irConvolution.process(context);
    }

    //==========================================================
    // FIXED OUTPUT BOOST
    //
    // Existing design = +18 dB
    //==========================================================

    constexpr float fixedOutputBoostDb =
        18.0f;

    outputGain.setGainDecibels(
        fixedOutputBoostDb);

    {
        juce::dsp::AudioBlock<float>
            monoBlock(
                monoData,
                static_cast<size_t>(numSamples));

        juce::dsp::ProcessContextReplacing<float>
            context(monoBlock);

        outputGain.process(context);
    }

    //==========================================================
    // MASTER VOLUME
    //
    // 0..10 -> 0..1
    //==========================================================

    const float masterGain =
        juce::jmap(
            volumeValue,
            0.0f,
            10.0f,
            0.0f,
            1.0f);

    for (int i = 0; i < numSamples; ++i)
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
