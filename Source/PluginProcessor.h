/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "GainProcessor.h"
#include "HighPassFilterProcessor.h"
#include "CompressorProcessor.h"



using AudioGraphIOProcessor = juce::AudioProcessorGraph::AudioGraphIOProcessor;
using Node = juce::AudioProcessorGraph::Node;


class Sno_vstpluginAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    Sno_vstpluginAudioProcessor();
    ~Sno_vstpluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override { return "SNO vst"; }

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;


    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    juce::StringArray processorChoices{ "Empty", "Gain", "HighPassFilter", "Compressor" };

    void initialiseGraph()
    {
        mainProcessor->clear();

        audioInputNode = mainProcessor->addNode(std::make_unique<AudioGraphIOProcessor>(AudioGraphIOProcessor::audioInputNode));
        audioOutputNode = mainProcessor->addNode(std::make_unique<AudioGraphIOProcessor>(AudioGraphIOProcessor::audioOutputNode));
        midiInputNode = mainProcessor->addNode(std::make_unique<AudioGraphIOProcessor>(AudioGraphIOProcessor::midiInputNode));
        midiOutputNode = mainProcessor->addNode(std::make_unique<AudioGraphIOProcessor>(AudioGraphIOProcessor::midiOutputNode));

        connectAudioNodes();
        connectMidiNodes();
    }
    //! [initialiseGraph]

    //! [updateGraph]
    void updateGraph()
    {
        bool hasChanged = false;

        juce::Array<juce::AudioParameterChoice*> choices{ processorSlot1,
                                                           processorSlot2,
                                                           processorSlot3 };

        juce::Array<juce::AudioParameterBool*> bypasses{ bypassSlot1,
                                                          bypassSlot2,
                                                          bypassSlot3 };

        juce::ReferenceCountedArray<Node> slots;
        slots.add(slot1Node);
        slots.add(slot2Node);
        slots.add(slot3Node);
        //! [updateGraph]

        //! [updateGraph loop]
        for (int i = 0; i < 2; ++i)
        {
            auto& choice = choices.getReference(i);
            auto  slot = slots.getUnchecked(i);

            if (choice->getIndex() == 0)            // [1]
            {
                if (slot != nullptr)
                {
                    mainProcessor->removeNode(slot.get());
                    slots.set(i, nullptr);
                    hasChanged = true;
                }
            }
            //else if (choice->getIndex() == 1)       // [2]
            //{
            //    if (slot != nullptr)
            //    {
            //        if (slot->getProcessor()->getName() == "Oscillator")
            //            continue;

            //        mainProcessor->removeNode(slot.get());
            //    }

            //    slots.set(i, mainProcessor->addNode(std::make_unique<OscillatorProcessor>()));
            //    hasChanged = true;
            //}
            else if (choice->getIndex() == 1)       // [3]
            {
                if (slot != nullptr)
                {
                    if (slot->getProcessor()->getName() == "Gain")
                        continue;

                    mainProcessor->removeNode(slot.get());
                }

                slots.set(i, mainProcessor->addNode(std::make_unique<GainProcessor>()));
                hasChanged = true;
            }
            else if (choice->getIndex() == 2)
            {
                if (slot != nullptr)
                {
                    if (slot->getProcessor()->getName() == "HighPassFilter")
                        continue;

                    mainProcessor->removeNode(slot.get());
                }
                slots.set(i, mainProcessor->addNode(std::make_unique<HighPassFilterProcessor>()));
                hasChanged = true;
            }
            else if (choice->getIndex() == 3)       // [4]
            {
                if (slot != nullptr)
                {
                    if (slot->getProcessor()->getName() == "Compressor")
                        continue;

                    mainProcessor->removeNode(slot.get());
                }

                slots.set(i, mainProcessor->addNode(std::make_unique<CompressorProcessor>()));
                hasChanged = true;
            }
        }
        //! [updateGraph loop]

        //! [updateGraph connect]
        if (hasChanged)
        {
            for (auto connection : mainProcessor->getConnections())     // [5]
                mainProcessor->removeConnection(connection);

            juce::ReferenceCountedArray<Node> activeSlots;

            for (auto slot : slots)
            {
                if (slot != nullptr)
                {
                    activeSlots.add(slot);                             // [6]

                    slot->getProcessor()->setPlayConfigDetails(getMainBusNumInputChannels(),
                        getMainBusNumOutputChannels(),
                        getSampleRate(), getBlockSize());
                }
            }

            if (activeSlots.isEmpty())                                  // [7]
            {
                connectAudioNodes();
            }
            else
            {
                for (int i = 0; i < activeSlots.size() - 1; ++i)        // [8]
                {
                    for (int channel = 0; channel < 2; ++channel)
                        mainProcessor->addConnection({ { activeSlots.getUnchecked(i)->nodeID,      channel },
                                                        { activeSlots.getUnchecked(i + 1)->nodeID,  channel } });
                }

                for (int channel = 0; channel < 2; ++channel)           // [9]
                {
                    mainProcessor->addConnection({ { audioInputNode->nodeID,         channel },
                                                    { activeSlots.getFirst()->nodeID, channel } });
                    mainProcessor->addConnection({ { activeSlots.getLast()->nodeID,  channel },
                                                    { audioOutputNode->nodeID,        channel } });
                }
            }

            connectMidiNodes();

            for (auto node : mainProcessor->getNodes())                 // [10]
                node->getProcessor()->enableAllBuses();
        }
        //! [updateGraph connect]

        //! [updateGraph bypass]
        for (int i = 0; i < 3; ++i)
        {
            auto  slot = slots.getUnchecked(i);
            auto& bypass = bypasses.getReference(i);

            if (slot != nullptr)
                slot->setBypassed(bypass->get());
        }

        audioInputNode->setBypassed(muteInput->get());

        slot1Node = slots.getUnchecked(0);
        slot2Node = slots.getUnchecked(1);
        slot3Node = slots.getUnchecked(2);
    }
    //! [updateGraph bypass]

    //! [connectAudioNodes]
    void connectAudioNodes()
    {
        for (int channel = 0; channel < 2; ++channel)
            mainProcessor->addConnection({ { audioInputNode->nodeID,  channel },
                                            { audioOutputNode->nodeID, channel } });
    }
    //! [connectAudioNodes]

    //! [connectMidiNodes]
    void connectMidiNodes()
    {
        mainProcessor->addConnection({ { midiInputNode->nodeID,  juce::AudioProcessorGraph::midiChannelIndex },
                                        { midiOutputNode->nodeID, juce::AudioProcessorGraph::midiChannelIndex } });
    }
    //! [connectMidiNodes]

        //==============================================================================
    //! [members]

    //! [graph member]
    std::unique_ptr<juce::AudioProcessorGraph> mainProcessor;
    //! [graph member]

    juce::AudioParameterBool* muteInput;

    juce::AudioParameterChoice* processorSlot1;
    juce::AudioParameterChoice* processorSlot2;
    juce::AudioParameterChoice* processorSlot3;

    juce::AudioParameterBool* bypassSlot1;
    juce::AudioParameterBool* bypassSlot2;
    juce::AudioParameterBool* bypassSlot3;

    //! [io members]
    Node::Ptr audioInputNode;
    Node::Ptr audioOutputNode;
    Node::Ptr midiInputNode;
    Node::Ptr midiOutputNode;
    //! [io members]

    Node::Ptr slot1Node;
    Node::Ptr slot2Node;
    Node::Ptr slot3Node;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Sno_vstpluginAudioProcessor)
};
