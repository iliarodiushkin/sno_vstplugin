#pragma once
#include <juce_dsp/juce_dsp.h>
#include "ProcessorBase.h"


class HighPassFilterProcessor : public ProcessorBase
{
public:
    HighPassFilterProcessor() {}

    //! [FilterProcessor prepareToPlay]
    void prepareToPlay(double sampleRate, int samplesPerBlock) override
    {
        *filter.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 1000.0f);

        juce::dsp::ProcessSpec spec{ sampleRate, static_cast<juce::uint32> (samplesPerBlock), 2 };
        filter.prepare(spec);
    }
    //! [FilterProcessor prepareToPlay]

    //! [FilterProcessor processBlock]
    void processBlock(juce::AudioSampleBuffer& buffer, juce::MidiBuffer&) override
    {
        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        filter.process(context);
    }
    //! [FilterProcessor processBlock]

    //! [FilterProcessor reset]
    void reset() override
    {
        filter.reset();
    }
    //! [FilterProcessor reset]

    const juce::String getName() const override { return "HighPassFilter"; }

private:
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> filter;
};


