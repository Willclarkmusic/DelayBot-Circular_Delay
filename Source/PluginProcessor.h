/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class CircularDelay_01AudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    CircularDelay_01AudioProcessor();
    ~CircularDelay_01AudioProcessor() override;

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
    const juce::String getName() const override;

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

    juce::AudioProcessorValueTreeState params;

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();
    juce::LinearSmoothedValue<float> delayMsSmooth;
    juce::LinearSmoothedValue<float> feedbackSmooth;


    void fillBuffer(juce::AudioBuffer<float>& buffer, int channel);
    void readFromBuffer(juce::AudioBuffer<float>& buffer, juce::AudioBuffer<float>& delayBuffer,
        int channel);
    void updateBufferPostions(juce::AudioBuffer<float>& buffer, juce::AudioBuffer<float>& delayBuffer);

    juce::AudioBuffer<float> delayBuffer;
    int writePosition{ 0 };



    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CircularDelay_01AudioProcessor)
};
