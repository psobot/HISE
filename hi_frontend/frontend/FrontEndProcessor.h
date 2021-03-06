/*  ===========================================================================
*
*   This file is part of HISE.
*   Copyright 2016 Christoph Hart
*
*   HISE is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   HISE is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*
*   Commercial licenses for using HISE in an closed source project are
*   available on request. Please visit the project's website to get more
*   information about commercial licensing:
*
*   http://www.hartinstruments.net/hise/
*
*   HISE is based on the JUCE library,
*   which must be separately licensed for closed source applications:
*
*   http://www.juce.com
*
*   ===========================================================================
*/

#ifndef FRONTENDPROCESSOR_H_INCLUDED
#define FRONTENDPROCESSOR_H_INCLUDED

namespace hise { using namespace juce;

#if USE_COPY_PROTECTION
class Unlocker
{
public:

	Unlocker();

	~Unlocker();

	var loadKeyFile();

	var isUnlocked() const;

	static RSAKey getPublicKey();

	String getEmailAdress() const;

	var isValidMachine(const String& machineId) const;

	static void showActivationWindow(Component* overlay);

	static void resolveLicenseFile(Component* overlay);

	String getProductErrorMessage() const;

private:

	friend class OnlineActivator;

	class Pimpl;
	Pimpl* pimpl;
};
#endif



/** This class lets you take your exported HISE presets and wrap them into a hardcoded plugin (VST / AU, x86/x64, Win / OSX)
*
*	It is connected to a FrontendProcessorEditor, which will display all script interfaces that are brought to the front using 'Synth.addToFront(true)'.
*	It also checks for a licence file to allow minimal protection against the most stupid crackers.
*/
class FrontendProcessor: public PluginParameterAudioProcessor,
						 public AudioProcessorDriver,
						 public MainController,
						 public FrontendDataHolder,
						 public FrontendSampleManager
{
public:
	FrontendProcessor(ValueTree &synthData, AudioDeviceManager* manager, AudioProcessorPlayer* callback_, ValueTree *imageData_ = nullptr, ValueTree *impulseData = nullptr, ValueTree *externalScriptData = nullptr, ValueTree *userPresets = nullptr);

	const String getName(void) const override;

	void changeProgramName(int /*index*/, const String &/*newName*/) override {};

	~FrontendProcessor()
	{
		setEnabledMidiChannels(synthChain->getActiveChannelData()->exportData());

		clearPreset();

		synthChain = nullptr;

		storeAllSamplesFound(areSamplesLoadedCorrectly());

	};

	bool shouldLoadSamplesAfterSetup() {
		return areSamplesLoadedCorrectly() && keyFileCorrectlyLoaded;
	}

	void updateUnlockedSuspendStatus()
	{

	}

	void prepareToPlay (double sampleRate, int samplesPerBlock);
	void releaseResources() {};

	void loadSamplesAfterRegistration()
    {
#if USE_COPY_PROTECTION || USE_TURBO_ACTIVATE
        keyFileCorrectlyLoaded = unlocker.isUnlocked();
#else
        keyFileCorrectlyLoaded = true;
#endif
        
        loadSamplesAfterSetup();
    }

	void getStateInformation	(MemoryBlock &destData) override
	{
		MemoryOutputStream output(destData, false);

		
		ValueTree v("ControlData");
		
		//synthChain->saveMacroValuesToValueTree(v);
		
		v.addChild(getMacroManager().getMidiControlAutomationHandler()->exportAsValueTree(), -1, nullptr);

		synthChain->saveInterfaceValues(v);
		
		v.setProperty("MidiChannelFilterData", getMainSynthChain()->getActiveChannelData()->exportData(), nullptr);

		v.setProperty("Program", currentlyLoadedProgram, nullptr);

		v.setProperty("UserPreset", getUserPresetHandler().getCurrentlyLoadedFile().getFullPathName(), nullptr);

		v.writeToStream(output);
	};
    
    void setStateInformation(const void *data,int sizeInBytes) override
	{
		ValueTree v = ValueTree::readFromData(data, sizeInBytes);

		currentlyLoadedProgram = v.getProperty("Program");

		getMacroManager().getMidiControlAutomationHandler()->restoreFromValueTree(v.getChildWithName("MidiAutomation"));

		channelData = v.getProperty("MidiChannelFilterData", -1);
		if (channelData != -1) synthChain->getActiveChannelData()->restoreFromData(channelData);
			

		const String userPresetName = v.getProperty("UserPreset").toString();

		if (userPresetName.isNotEmpty())
		{
			getUserPresetHandler().setCurrentlyLoadedFile(File(userPresetName));
		}

		synthChain->restoreInterfaceValues(v.getChildWithName("InterfaceData"));
	}

	

	void processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages);

	virtual void processBlockBypassed (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
	{
#if !FRONTEND_IS_PLUGIN
		buffer.clear();
		midiMessages.clear();
		allNotesOff();
#else
		ignoreUnused(buffer, midiMessages);
#endif
		
	};

    void createSampleMapValueTreeFromPreset(ValueTree& treeToSearch)
    {
        static const Identifier sm("samplemap");
        
        for(int i = 0; i < treeToSearch.getNumChildren(); i++)
        {
            ValueTree child = treeToSearch.getChild(i);
            
            if(child.hasType(sm))
            {
                treeToSearch.removeChild(child, nullptr);
                
                sampleMaps.addChild(child, -1, nullptr);
                
                i--;
            }
            else
            {
                createSampleMapValueTreeFromPreset(child);
            }
        }
    };
    
	void handleControllersForMacroKnobs(const MidiBuffer &midiMessages);
 
	AudioProcessorEditor* createEditor();
	bool hasEditor() const {return true;};

	bool acceptsMidi() const {return true;};
	bool producesMidi() const {return false;};
	
	double getTailLengthSeconds() const {return 0.0;};

	ModulatorSynthChain *getMainSynthChain() override {return synthChain; };

	const ModulatorSynthChain *getMainSynthChain() const override { return synthChain; }

	int getNumPrograms() override
	{
		return 1;// presets.getNumChildren() + 1;
	}

	const String getProgramName(int /*index*/) override
	{
		return "Default";
	}

	int getCurrentProgram() override
	{
		return 0;

		//return currentlyLoadedProgram;
	}

	File getSampleLocation() const override { return ProjectHandler::Frontend::getSampleLocationForCompiledPlugin(); }

	void setCurrentProgram(int index) override;
	
#if USE_COPY_PROTECTION
	Unlocker unlocker;
#endif

#if USE_TURBO_ACTIVATE
	TurboActivateUnlocker unlocker;
#endif

	const ValueTree getValueTree(ProjectHandler::SubDirectories type) const override
	{
		if (type == ProjectHandler::SubDirectories::SampleMaps) return sampleMaps;
		else if (type == ProjectHandler::SubDirectories::UserPresets) return presets;
		else return ValueTree();
	}


private:

	void loadImages(ValueTree *imageData);
	
	friend class FrontendProcessorEditor;
	friend class DefaultFrontendBar;

	

    bool keyFileCorrectlyLoaded = true;

	int numParameters;

	const ValueTree presets;

	ValueTree sampleMaps;

	AudioPlayHead::CurrentPositionInfo lastPosInfo;

	friend class FrontendProcessorEditor;

	ScopedPointer<ModulatorSynthChain> synthChain;

	ScopedPointer<AudioSampleBufferPool> audioSampleBufferPool;

	int currentlyLoadedProgram;
	
	int unlockCounter;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FrontendProcessor)	
};


class FrontendStandaloneApplication : public JUCEApplication
{
public:
	//==============================================================================
	FrontendStandaloneApplication() {}

	const String getApplicationName() override;
	const String getApplicationVersion() override;
	bool moreThanOneInstanceAllowed() override       { return true; }


	void initialise(const String& /*commandLine*/) override { mainWindow = new MainWindow(getApplicationName()); }
	void shutdown() override { mainWindow = nullptr; }
	void systemRequestedQuit() override { quit(); }

	void anotherInstanceStarted(const String& /*commandLine*/) override {}

	class AudioWrapper : public Component,
						 public Timer
	{
	public:

		void init();

		void timerCallback() override
		{
			stopTimer();
			init();
		}

		AudioWrapper();

        void paint(Graphics& g) override
        {
            g.fillAll(Colours::black);
        }
    
		~AudioWrapper();

		void resized()
		{
			if (splashScreen != nullptr)
				splashScreen->setBounds(getLocalBounds());

			if(editor != nullptr)
				editor->setBounds(0, 0, editor->getWidth(), editor->getHeight());
		}

	private:

		ScopedPointer<ImageComponent> splashScreen;

		ScopedPointer<AudioProcessorEditor> editor;
		ScopedPointer<StandaloneProcessor> standaloneProcessor;
        
		ScopedPointer<OpenGLContext> context;
        
	};


	class MainWindow : public DocumentWindow
	{
	public:
		MainWindow(String name);

		~MainWindow()
        {
			

            audioWrapper = nullptr;
        }

		void closeButtonPressed() override
		{
			JUCEApplication::getInstance()->systemRequestedQuit();
		}

	private:

		
        
		ScopedPointer<AudioWrapper> audioWrapper;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
	};


private:

	ScopedPointer<MainWindow> mainWindow;
};

} // namespace hise

#endif  // FRONTENDPROCESSOR_H_INCLUDED
