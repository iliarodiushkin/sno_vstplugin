#pragma once
#include <JuceHeader.h>
#include <juce_dsp/juce_dsp.h>
#include "ProcessorBase.h"

class DistortionProcessor : public ProcessorBase
{
public:
	DistortionProcessor() {
		gain = 1.0f;
		threshold = 0.5f;
	}

    void prepareToPlay(double sampleRate, int samplesPerBlock) override {
        juce::dsp::ProcessSpec spec{ sampleRate, static_cast<juce::uint32> (samplesPerBlock), 2 };
        distortion.prepare(spec);
    }


    void processBlock(juce::AudioSampleBuffer& buffer, juce::MidiBuffer&) override {
        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        distortion.process(context);
    }

    void reset() override
    {
        distortion.reset();
    }

    void setGain(float newGain)
    {
        gain = newGain;
    }

    void setThreshold(float newThreshold)
    {
        threshold = newThreshold;
    }

    const juce::String getName() const override { return { "Distortion" }; }

private:
    juce::dsp::WaveShaper<float> distortion{ [](float x)
                                   {
                                       return juce::jlimit(float(-0.1), float(0.1), x); // [6]
                                   } };
    float gain;
    float threshold;
};