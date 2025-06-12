/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
CircularDelay_01AudioProcessorEditor::CircularDelay_01AudioProcessorEditor (CircularDelay_01AudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p) //, jif(BinaryData::DuckyDance_gif, BinaryData::DuckyDance_gifSize)
{
    delayMsSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    delayMsSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 25);
    addAndMakeVisible(delayMsSlider);

    feedbackSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    feedbackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 25);
    addAndMakeVisible(feedbackSlider);

    delayMsSliderAttachment = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.params, "DELAYMS", delayMsSlider);
    feedbackSliderAttachment = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.params, "FEEDBACK", feedbackSlider);

    setSize (400, 400);
    //startTimerHz(8);
}

CircularDelay_01AudioProcessorEditor::~CircularDelay_01AudioProcessorEditor()
{
}

//==============================================================================
void CircularDelay_01AudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour(juce::Colours::aquamarine);
    g.setFont(50.0f);
    g.drawFittedText("Duck Delay", getLocalBounds(), juce::Justification::topRight - 50, 5);

    auto ducky = juce::ImageCache::getFromMemory(BinaryData::DuckyDance_gif, BinaryData::DuckyDance_gifSize);
    g.drawImageWithin(ducky, 0, 0, getWidth() * 0.4, getHeight() * 0.4, juce::RectanglePlacement::centred, false);

}

void CircularDelay_01AudioProcessorEditor::resized()
{
    delayMsSlider.setBounds(getWidth() / 2 + 50, getHeight() / 2, 100, 100);
    feedbackSlider.setBounds(getWidth() / 2 - 50, getHeight() / 2, 100, 100);

    //jif.resetAnimation();
}

//void CircularDelay_01AudioProcessorEditor::timerCallBack()
//{
//    ++jif;
//    repaint();
//}