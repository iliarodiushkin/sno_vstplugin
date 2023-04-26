#pragma once
#include <JuceHeader.h>
#include <juce_dsp/juce_dsp.h>
#include "ProcessorBase.h"


class CompressorProcessor : public ProcessorBase
{
public:
    CompressorProcessor() {
        threshold = -20.0f;
        ratio = 4.0f;
        attack = 1.0f;
        release = 250.0f;
    }


    void prepareToPlay(double sampleRate, int samplesPerBlock) override {
        compressor.setThreshold(threshold);
        compressor.setRatio(ratio);
        compressor.setAttack(attack);
        compressor.setRelease(release);
        juce::dsp::ProcessSpec spec{ sampleRate, static_cast<juce::uint32> (samplesPerBlock), 2 };
        compressor.prepare(spec);
    }


    void processBlock(juce::AudioSampleBuffer& buffer, juce::MidiBuffer&) override {
        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        compressor.process(context);
    }

    void reset() override
    {
        compressor.reset();
    }

    const juce::String getName() const override { return {"Compressor"}; }

private:
    juce::dsp::Compressor<float> compressor;
    float threshold;
    float ratio;
    float attack;
    float release;
};

