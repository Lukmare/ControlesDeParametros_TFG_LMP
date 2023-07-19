/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ControlesDeParametrosAudioProcessor::ControlesDeParametrosAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    ataque = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("Ataque"));
    jassert(ataque != nullptr);

    libera = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("Libera"));
    jassert(libera != nullptr);

    limite = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("Limite"));
    jassert(limite != nullptr);

    ratio = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("Ratio"));
    jassert(ratio != nullptr);
}

ControlesDeParametrosAudioProcessor::~ControlesDeParametrosAudioProcessor()
{
}

//==============================================================================
const juce::String ControlesDeParametrosAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ControlesDeParametrosAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ControlesDeParametrosAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ControlesDeParametrosAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ControlesDeParametrosAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ControlesDeParametrosAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ControlesDeParametrosAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ControlesDeParametrosAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ControlesDeParametrosAudioProcessor::getProgramName (int index)
{
    return {};
}

void ControlesDeParametrosAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void ControlesDeParametrosAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    
    juce::dsp::ProcessSpec especificaciones;
    especificaciones.maximumBlockSize = samplesPerBlock;
    especificaciones.numChannels = getTotalNumOutputChannels();
    especificaciones.sampleRate = sampleRate;

    compresor.prepare(especificaciones);

}

void ControlesDeParametrosAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ControlesDeParametrosAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void ControlesDeParametrosAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

   for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());


   compresor.setAttack(ataque->get());
   compresor.setRelease(libera->get());
   compresor.setThreshold(limite->get());
   compresor.setRatio(ratio->getCurrentChoiceName().getFloatValue());

   auto bloque = juce::dsp::AudioBlock<float>(buffer);
   auto contexto = juce::dsp::ProcessContextReplacing<float>(bloque);

   compresor.process(contexto);
}

//==============================================================================
bool ControlesDeParametrosAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ControlesDeParametrosAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void ControlesDeParametrosAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void ControlesDeParametrosAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

juce::AudioProcessorValueTreeState::ParameterLayout ControlesDeParametrosAudioProcessor::creaLayoutDeParametros()
{
    APVTS::ParameterLayout layout;

    using namespace juce;

    layout.add(std::make_unique<AudioParameterFloat>("Limite", "Limite", NormalisableRange<float>(-60, 12, 1, 1), 0));

    layout.add(std::make_unique<AudioParameterFloat>("Ataque", "Ataque", NormalisableRange<float>(5, 500, 1, 1), 50));
    
    layout.add(std::make_unique<AudioParameterFloat>("Libera", "Libera", NormalisableRange<float>(5, 500, 1, 1), 250));


    auto choices = std::vector<double>{ 1,1.5,2,3,4,5,6,7,8,10,15,20,50,100 };

    juce::StringArray sa;
    for (auto choice : choices)
    {
        sa.add(juce::String(choice, 1));
    }

    layout.add(std::make_unique <AudioParameterChoice>("Ratio", "Ratio", sa, 3));

    return layout;
}
//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ControlesDeParametrosAudioProcessor();
}
