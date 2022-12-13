/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Sno_vstpluginAudioProcessor::Sno_vstpluginAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
    mainProcessor(new juce::AudioProcessorGraph()),
    //! [constructor]
    muteInput(new juce::AudioParameterBool("mute", "Mute Input", true)),
    processorSlot1(new juce::AudioParameterChoice("slot1", "Slot 1", processorChoices, 0)),
    processorSlot2(new juce::AudioParameterChoice("slot2", "Slot 2", processorChoices, 0)),
    processorSlot3(new juce::AudioParameterChoice("slot3", "Slot 3", processorChoices, 0)),
    bypassSlot1(new juce::AudioParameterBool("bypass1", "Bypass 1", false)),
    bypassSlot2(new juce::AudioParameterBool("bypass2", "Bypass 2", false)),
    bypassSlot3(new juce::AudioParameterBool("bypass3", "Bypass 3", false))
{
    addParameter(muteInput);

    addParameter(processorSlot1);
    addParameter(processorSlot2);
    addParameter(processorSlot3);

    addParameter(bypassSlot1);
    addParameter(bypassSlot2);
    addParameter(bypassSlot3);
}

Sno_vstpluginAudioProcessor::~Sno_vstpluginAudioProcessor()
{
}

//==============================================================================
//const juce::String Sno_vstpluginAudioProcessor::getName() const
//{
//    return JucePlugin_Name;
//}

bool Sno_vstpluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Sno_vstpluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Sno_vstpluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double Sno_vstpluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Sno_vstpluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Sno_vstpluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void Sno_vstpluginAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String Sno_vstpluginAudioProcessor::getProgramName (int index)
{
    return {};
}

void Sno_vstpluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void Sno_vstpluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    mainProcessor->setPlayConfigDetails(getMainBusNumInputChannels(),
        getMainBusNumOutputChannels(),
        sampleRate, samplesPerBlock);

    mainProcessor->prepareToPlay(sampleRate, samplesPerBlock);

    initialiseGraph();
}

void Sno_vstpluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    mainProcessor->releaseResources();
}

bool Sno_vstpluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainInputChannelSet() == juce::AudioChannelSet::disabled()
        || layouts.getMainOutputChannelSet() == juce::AudioChannelSet::disabled())
        return false;

    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return layouts.getMainInputChannelSet() == layouts.getMainOutputChannelSet();
}

void Sno_vstpluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    for (int i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    updateGraph();

    mainProcessor->processBlock(buffer, midiMessages);
}

//==============================================================================
bool Sno_vstpluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* Sno_vstpluginAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor(*this);
    //return new Sno_vstpluginAudioProcessorEditor (*this);
}

//==============================================================================
void Sno_vstpluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void Sno_vstpluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Sno_vstpluginAudioProcessor();
}

