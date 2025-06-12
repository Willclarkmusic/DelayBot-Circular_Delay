/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
CircularDelay_01AudioProcessor::CircularDelay_01AudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), params (*this, nullptr, "Parameters", createParameters()) 
#endif
{
}

CircularDelay_01AudioProcessor::~CircularDelay_01AudioProcessor()
{
}

//==============================================================================
const juce::String CircularDelay_01AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool CircularDelay_01AudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool CircularDelay_01AudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool CircularDelay_01AudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double CircularDelay_01AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int CircularDelay_01AudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int CircularDelay_01AudioProcessor::getCurrentProgram()
{
    return 0;
}

void CircularDelay_01AudioProcessor::setCurrentProgram (int index)
{
}

const juce::String CircularDelay_01AudioProcessor::getProgramName (int index)
{
    return {};
}

void CircularDelay_01AudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void CircularDelay_01AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    delayMsSmooth.reset(sampleRate, 0.005f);
    feedbackSmooth.reset(sampleRate, 0.005f);

    //init delayBuffer
    auto delayBufferSize = sampleRate * 2.0;
    delayBuffer.setSize(getTotalNumOutputChannels(), (int)delayBufferSize);
    delayBuffer.clear();
}

void CircularDelay_01AudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool CircularDelay_01AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void CircularDelay_01AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        fillBuffer (buffer, channel);
        readFromBuffer (buffer, delayBuffer, channel);
        fillBuffer (buffer, channel);
    }

    updateBufferPostions(buffer, delayBuffer);

    }

void CircularDelay_01AudioProcessor::fillBuffer(juce::AudioBuffer<float>& buffer, int channel)
{
    auto bufferSize = buffer.getNumSamples();
    auto delayBufferSize = delayBuffer.getNumSamples();

    // Check to see if main buffer copies to delay buffer without overflow...
    if (delayBufferSize > bufferSize + writePosition)
    {
        // copy main buffer to delay buffer
        delayBuffer.copyFrom(channel, writePosition, buffer.getWritePointer(channel), bufferSize);
    }
    else
    {
        // Determine how much space is left at end of delay buffer
        auto numSamplesToEnd = delayBufferSize - writePosition;
        // Copy that amount from main buffer to delay buffer
        delayBuffer.copyFrom (channel, writePosition, buffer.getWritePointer(channel), numSamplesToEnd);

        // Calculate how much content is remaining to copy 
        auto numSamplesAtStart = bufferSize - numSamplesToEnd;
        // Copy remaining amount to begining of delay buffer
        delayBuffer.copyFrom (channel, 0, buffer.getWritePointer(channel, numSamplesToEnd), numSamplesAtStart);
    }
}

void CircularDelay_01AudioProcessor::readFromBuffer(juce::AudioBuffer<float>& buffer, juce::AudioBuffer<float>& delayBuffer, int channel)
{
    auto bufferSize = buffer.getNumSamples();
    auto delayBufferSize = delayBuffer.getNumSamples();

    auto* delayMs = params.getRawParameterValue("DELAYMS");  
    auto readPosition = writePosition - (delayMs->load());
    delayMsSmooth.setTargetValue(readPosition);

    DBG(delayMsSmooth.getNextValue());

    if (readPosition < 0)
        readPosition += delayBufferSize;        

    // feedback
    auto* feedback = params.getRawParameterValue("FEEDBACK");
    auto gain = feedback->load();
    feedbackSmooth.setTargetValue(gain);

    DBG(feedbackSmooth.getNextValue());

    if (readPosition + bufferSize < delayBufferSize)
    {
        buffer.addFromWithRamp(channel, 0, delayBuffer.getReadPointer(channel, readPosition), bufferSize, gain, gain);
    }
    else
    {
        auto numSamplesToEnd = delayBufferSize - readPosition;
        buffer.addFromWithRamp(channel, 0, delayBuffer.getReadPointer(channel, readPosition), numSamplesToEnd, gain, gain);

        auto numSamplesAtStart = bufferSize - numSamplesToEnd;
        buffer.addFromWithRamp(channel, numSamplesToEnd, delayBuffer.getReadPointer(channel, 0), numSamplesAtStart, gain, gain);
    }
}

void CircularDelay_01AudioProcessor::updateBufferPostions(juce::AudioBuffer<float>& buffer, juce::AudioBuffer<float>& delayBuffer)
{
    auto bufferSize = buffer.getNumSamples();
    auto delayBufferSize = delayBuffer.getNumSamples();

    writePosition += bufferSize;
    writePosition %= delayBufferSize;
}

//==============================================================================
bool CircularDelay_01AudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* CircularDelay_01AudioProcessor::createEditor()
{
    return new CircularDelay_01AudioProcessorEditor (*this);
}

//==============================================================================
void CircularDelay_01AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void CircularDelay_01AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CircularDelay_01AudioProcessor();
}

juce::AudioProcessorValueTreeState::ParameterLayout CircularDelay_01AudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat>("DELAYMS", "DelayMs", 0.0f, 96000.0f, 24000.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>("FEEDBACK", "Feedback", 0.0f, 1.0f, 0.4f));

    return { params.begin(), params.end()};
}
