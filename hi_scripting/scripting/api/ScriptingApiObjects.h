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
*   along with HISE.  If not, see <http://www.gnu.org/licenses/>.
*
*   Commercial licences for using HISE in an closed source project are
*   available on request. Please visit the project's website to get more
*   information about commercial licencing:
*
*   http://www.hartinstruments.net/hise/
*
*   HISE is based on the JUCE library,
*   which also must be licenced for commercial applications:
*
*   http://www.juce.com
*
*   ===========================================================================
*/


#ifndef SCRIPTINGAPIOBJECTS_H_INCLUDED
#define SCRIPTINGAPIOBJECTS_H_INCLUDED

class ScriptCreatedComponentWrapper;
class ScriptContentComponent;
class ScriptedControlAudioParameter;
class AudioProcessorWrapper;

/** This class wrapps all available objects that can be created by a script.
*	@ingroup scripting
*/
class ScriptingObjects
{
public:

	class MidiList : public ConstScriptingObject,
					 public AssignableObject
	{
	public:

		// ============================================================================================================

		MidiList(ProcessorWithScriptingContent *p);
		~MidiList() {};

		Identifier getObjectName() const override { RETURN_STATIC_IDENTIFIER("MidiList"); }

		void assign(const int index, var newValue);
		int getCachedIndex(const var &indexExpression) const override;
		var getAssignedValue(int index) const override;

		// ================================================================================================ API METHODS

		/** Fills the MidiList with a number specified with valueToFill. */
		void fill(int valueToFill);;

		/** Clears the MidiList to -1. */
		void clear();

		/** Returns the value at the given number. */
		int getValue(int index) const;

		/** Returns the number of occurences of 'valueToCheck' */
		int getValueAmount(int valueToCheck);;

		/** Returns the first index that contains this value. */
		int getIndex(int value) const;

		/** Checks if the list contains any data. */
		bool isEmpty() const { return empty; }

		/** Returns the number of values that are not -1. */
		int getNumSetValues() const { return numValues; }

		/** Sets the number to something between -127 and 128. */
		void setValue(int index, int value);;

		/** Encodes all values into a base64 encoded string for storage. */
		String getBase64String() const;

		/** Restore the values from a String that was created with getBase64String(). */
		void restoreFromBase64String(String base64encodedValues);

		// ============================================================================================================

		struct Wrapper;

	private:

		int16 data[128];
		bool empty;
		int numValues;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiList);

		// ============================================================================================================
	};

	/** A scripting objects that wraps an existing Modulator.
	*/
	class ScriptingModulator : public ConstScriptingObject,
							   public AssignableObject,
							   public DebugableObject
	{
	public:

		ScriptingModulator(ProcessorWithScriptingContent *p, Modulator *m_);;
		~ScriptingModulator() {};

		// ============================================================================================================

		Identifier getObjectName() const override { RETURN_STATIC_IDENTIFIER("Modulator"); }
		bool objectDeleted() const override { return mod.get() == nullptr; }
		bool objectExists() const override { return mod != nullptr;	}

		String getDebugName() const override;
		String getDebugValue() const override;
		void doubleClickCallback(Component *componentToNotify) override;

		// ============================================================================================================

		int getCachedIndex(const var &indexExpression) const override;
		void assign(const int index, var newValue) override;
		var getAssignedValue(int /*index*/) const override;

		// ============================================================================================================ API Methods

		/** Checks if the Object exists and prints a error message on the console if not. */
		bool exists() { return checkValidObject(); };

		/** Sets the attribute of the Modulator. You can look up the specific parameter indexes in the manual. */
		void setAttribute(int index, float value);

		/** Bypasses the Modulator. */
		void setBypassed(bool shouldBeBypassed);;

		/** Changes the Intensity of the Modulator. Ranges: Gain Mode 0 ... 1, PitchMode -12 ... 12. */
		void setIntensity(float newIntensity);

		// ============================================================================================================

		struct Wrapper;
		
	private:

		WeakReference<Modulator> mod;
		Modulation *m;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ScriptingModulator);

		// ============================================================================================================
	};



	class ScriptingEffect : public ConstScriptingObject
	{
	public:

		// ============================================================================================================

		ScriptingEffect(ProcessorWithScriptingContent *p, EffectProcessor *fx);
		~ScriptingEffect() {};

		Identifier getObjectName() const override { RETURN_STATIC_IDENTIFIER("Effect"); }
		bool objectDeleted() const override { return effect.get() == nullptr; }
		bool objectExists() const override { return effect != nullptr; }

		// ============================================================================================================ API Methods

		/** Checks if the Object exists and prints a error message on the console if not. */
		bool exists() { return checkValidObject(); };

		/** Changes one of the Parameter. Look in the manual for the index numbers of each effect. */
		void setAttribute(int parameterIndex, float newValue);

		/** Bypasses the effect. */
		void setBypassed(bool shouldBeBypassed);

		// ============================================================================================================

		struct Wrapper;

	private:

		WeakReference<Processor> effect;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ScriptingEffect);

		// ============================================================================================================
	};



	class ScriptingSynth : public ConstScriptingObject
	{
	public:

		// ============================================================================================================ 

		ScriptingSynth(ProcessorWithScriptingContent *p, ModulatorSynth *synth_);
		~ScriptingSynth() {};

		Identifier getObjectName() const override { RETURN_STATIC_IDENTIFIER("ChildSynth"); };
		bool objectDeleted() const override { return synth.get() == nullptr; };
		bool objectExists() const override { return synth != nullptr; };

		// ============================================================================================================ API Methods

		/** Checks if the Object exists and prints a error message on the console if not. */
		bool exists() { return checkValidObject(); };

		/** Changes one of the Parameter. Look in the manual for the index numbers of each effect. */
		void setAttribute(int parameterIndex, float newValue);;

		/** Bypasses the effect. */
		void setBypassed(bool shouldBeBypassed);

		// ============================================================================================================ 

		struct Wrapper;

	private:

		WeakReference<Processor> synth;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ScriptingSynth);

		// ============================================================================================================
	};


	class ScriptingMidiProcessor : public ConstScriptingObject,
								   public AssignableObject
	{
	public:

		// ============================================================================================================ 

		ScriptingMidiProcessor(ProcessorWithScriptingContent *p, MidiProcessor *mp_);;
		~ScriptingMidiProcessor() {};

		Identifier getObjectName() const override { RETURN_STATIC_IDENTIFIER("MidiProcessor"); }
		bool objectDeleted() const override { return mp.get() == nullptr; }
		bool objectExists() const override { return mp != nullptr; }

		int getCachedIndex(const var &indexExpression) const override;
		void assign(const int index, var newValue) override;
		var getAssignedValue(int /*index*/) const override;

		// ============================================================================================================ API Methods

		/** Checks if the Object exists and prints a error message on the console if not. */
		bool exists() { return checkValidObject(); };

		/** Sets the attribute of the MidiProcessor. If it is a script, then the index of the component is used. */
		void setAttribute(int index, float value);

		/** Bypasses the MidiProcessor. */
		void setBypassed(bool shouldBeBypassed);;

		// ============================================================================================================

		struct Wrapper;

	private:

		WeakReference<MidiProcessor> mp;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ScriptingMidiProcessor)

		// ============================================================================================================
	};

	class ScriptingAudioSampleProcessor : public ConstScriptingObject
	{
	public:

		// ============================================================================================================

		ScriptingAudioSampleProcessor(ProcessorWithScriptingContent *p, AudioSampleProcessor *sampleProcessor);
		~ScriptingAudioSampleProcessor() {};

		Identifier getObjectName() const override { RETURN_STATIC_IDENTIFIER("AudioSampleProcessor"); };
		bool objectDeleted() const override { return audioSampleProcessor.get() == nullptr; }
		bool objectExists() const override { return audioSampleProcessor != nullptr; }

		// ============================================================================================================ API Methods

		/** Checks if the Object exists and prints a error message on the console if not. */
		bool exists() { return checkValidObject(); };

		/** Changes one of the Parameter. Look in the manual for the index numbers of each effect. */
		void setAttribute(int parameterIndex, float newValue);;

		/** Bypasses the effect. */
		void setBypassed(bool shouldBeBypassed);

		/** loads the file. You can use the wildcard {PROJECT_FOLDER} to get the audio file folder for the current project. */
		void setFile(String fileName);

		/** Returns the length of the current sample selection in samples. */
		int getSampleLength() const;

		/** Sets the length of the current sample selection in samples. */
		void setSampleRange(int startSample, int endSample);

		// ============================================================================================================

		struct Wrapper; 

	private:

		WeakReference<Processor> audioSampleProcessor;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ScriptingAudioSampleProcessor);

		// ============================================================================================================
	};

	class ScriptingTableProcessor : public ConstScriptingObject
	{
	public:

		// ============================================================================================================

		ScriptingTableProcessor(ProcessorWithScriptingContent *p, LookupTableProcessor *tableProcessor);
		~ScriptingTableProcessor() {};

		Identifier getObjectName() const override {	RETURN_STATIC_IDENTIFIER("TableProcessor"); };
		bool objectDeleted() const override { return tableProcessor.get() == nullptr; }
		bool objectExists() const override { return tableProcessor != nullptr; }

		// ============================================================================================================ API Methods

		/** Checks if the Object exists and prints a error message on the console if not. */
		bool exists() { return checkValidObject(); };

		/** Sets the point with the given index to the values. */
		void setTablePoint(int tableIndex, int pointIndex, float x, float y, float curve);

		/** Adds a new table point (x and y are normalized coordinates). */
		void addTablePoint(int tableIndex, float x, float y);

		/** Resets the table with the given index to a 0..1 line. */
		void reset(int tableIndex);

		// ============================================================================================================
		
		struct Wrapper;

	private:

		WeakReference<Processor> tableProcessor;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ScriptingTableProcessor);

		// ============================================================================================================
	};


	class TimerObject : public Timer,
						public DynamicScriptingObject
	{
	public:

		TimerObject(ProcessorWithScriptingContent *p);;

		Identifier getObjectName() const override { return "Timer"; }
		bool objectDeleted() const override { return false; }
		bool objectExists() const override { return false; }

		/** the timer callback. */
		void timerCallback() override;;

		struct Wrapper;
	};
};




#endif  // SCRIPTINGAPIOBJECTS_H_INCLUDED