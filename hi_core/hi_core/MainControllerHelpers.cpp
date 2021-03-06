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
*   Commercial licenses for using HISE in an closed source project are
*   available on request. Please visit the project's website to get more
*   information about commercial licensing:
*
*   http://www.hise.audio/
*
*   HISE is based on the JUCE library,
*   which must be separately licensed for closed source applications:
*
*   http://www.juce.com
*
*   ===========================================================================
*/

namespace hise { using namespace juce;


MidiControllerAutomationHandler::MidiControllerAutomationHandler(MainController *mc_) :
anyUsed(false),
mc(mc_)
{
	tempBuffer.ensureSize(2048);

	clear();
}

void MidiControllerAutomationHandler::addMidiControlledParameter(Processor *interfaceProcessor, int attributeIndex, NormalisableRange<double> parameterRange, int macroIndex)
{
	ScopedLock sl(mc->getLock());

	unlearnedData.processor = interfaceProcessor;
	unlearnedData.attribute = attributeIndex;
	unlearnedData.parameterRange = parameterRange;
	unlearnedData.fullRange = parameterRange;
	unlearnedData.macroIndex = macroIndex;
	unlearnedData.used = true;

}

bool MidiControllerAutomationHandler::isLearningActive() const
{
	return unlearnedData.used;
}

bool MidiControllerAutomationHandler::isLearningActive(Processor *interfaceProcessor, int attributeIndex) const
{
	return unlearnedData.processor == interfaceProcessor && unlearnedData.attribute == attributeIndex;
}

void MidiControllerAutomationHandler::deactivateMidiLearning()
{
	ScopedLock sl(mc->getLock());

	unlearnedData = AutomationData();
}

void MidiControllerAutomationHandler::setUnlearndedMidiControlNumber(int ccNumber, NotificationType notifyListeners)
{
	jassert(isLearningActive());

	ScopedLock sl(mc->getLock());

	unlearnedData.ccNumber = ccNumber;

	automationData[ccNumber].addIfNotAlreadyThere(unlearnedData);
	unlearnedData = AutomationData();

	anyUsed = true;

	if (notifyListeners)
		sendChangeMessage();
}

int MidiControllerAutomationHandler::getMidiControllerNumber(Processor *interfaceProcessor, int attributeIndex) const
{
	for (int i = 0; i < 128; i++)
	{
		for (auto& a : automationData[i])
		{
			if (a.processor == interfaceProcessor && a.attribute == attributeIndex)
			{
				return i;
			}
		}
	}

	return -1;
}

void MidiControllerAutomationHandler::refreshAnyUsedState()
{
	ScopedLock sl(mc->getLock());

	anyUsed = false;

	for (int i = 0; i < 128; i++)
	{
		for (auto& a : automationData[i])
		{
			if (a.used)
			{
				anyUsed = true;
				return;
			}
		}
	}
}

void MidiControllerAutomationHandler::clear()
{
	for (int i = 0; i < 128; i++)
	{
		automationData[i].clearQuick();
	};

	unlearnedData = AutomationData();

	anyUsed = false;
}

void MidiControllerAutomationHandler::removeMidiControlledParameter(Processor *interfaceProcessor, int attributeIndex, NotificationType notifyListeners)
{
	ScopedLock sl(mc->getLock());

	for (int i = 0; i < 128; i++)
	{
		for (auto& a : automationData[i])
		{
			if (a.processor == interfaceProcessor && a.attribute == attributeIndex)
			{
				automationData[i].removeAllInstancesOf(a);
				break;
			}
		}
	}

	refreshAnyUsedState();

	if (notifyListeners == sendNotification)
		sendChangeMessage();
}

MidiControllerAutomationHandler::AutomationData::AutomationData() :
processor(nullptr),
attribute(-1),
parameterRange(NormalisableRange<double>()),
fullRange(NormalisableRange<double>()),
macroIndex(-1),
used(false)
{

}



void MidiControllerAutomationHandler::AutomationData::clear()
{
	processor = nullptr;
	attribute = -1;
	parameterRange = NormalisableRange<double>();
	fullRange = NormalisableRange<double>();
	macroIndex = -1;
	ccNumber = -1;
	inverted = false;
	used = false;
}

bool MidiControllerAutomationHandler::AutomationData::operator==(const AutomationData& other) const
{
	return other.processor == processor && other.attribute == attribute;
}

ValueTree MidiControllerAutomationHandler::exportAsValueTree() const
{
	ValueTree v("MidiAutomation");

	for (int i = 0; i < 128; i++)
	{
		for (auto& a : automationData[i])
		{
			if (a.used && a.processor != nullptr)
			{
				ValueTree cc("Controller");

				cc.setProperty("Controller", i, nullptr);
				cc.setProperty("Processor", a.processor->getId(), nullptr);
				cc.setProperty("MacroIndex", a.macroIndex, nullptr);
				cc.setProperty("Start", a.parameterRange.start, nullptr);
				cc.setProperty("End", a.parameterRange.end, nullptr);
				cc.setProperty("FullStart", a.fullRange.start, nullptr);
				cc.setProperty("FullEnd", a.fullRange.end, nullptr);
				cc.setProperty("Skew", a.parameterRange.skew, nullptr);
				cc.setProperty("Interval", a.parameterRange.interval, nullptr);
				cc.setProperty("Attribute", a.attribute, nullptr);
				cc.setProperty("Inverted", a.inverted, nullptr);
				v.addChild(cc, -1, nullptr);
			}
		}
	}

	return v;
}

void MidiControllerAutomationHandler::restoreFromValueTree(const ValueTree &v)
{
	if (v.getType() != Identifier("MidiAutomation")) return;

	clear();

	for (int i = 0; i < v.getNumChildren(); i++)
	{
		ValueTree cc = v.getChild(i);

		int controller = cc.getProperty("Controller", 1);

		auto& aArray = automationData[controller];

		AutomationData a;

		a.ccNumber = controller;
		a.processor = ProcessorHelpers::getFirstProcessorWithName(mc->getMainSynthChain(), cc.getProperty("Processor"));
		a.macroIndex = cc.getProperty("MacroIndex");
		a.attribute = cc.getProperty("Attribute", a.attribute);

		double start = cc.getProperty("Start");
		double end = cc.getProperty("End");
		double skew = cc.getProperty("Skew", a.parameterRange.skew);
		double interval = cc.getProperty("Interval", a.parameterRange.interval);

		auto fullStart = cc.getProperty("FullStart", start);
		auto fullEnd = cc.getProperty("FullEnd", end);

		a.parameterRange = NormalisableRange<double>(start, end, interval, skew);
		a.fullRange = NormalisableRange<double>(fullStart, fullEnd, interval, skew);
		
		a.used = true;
		a.inverted = cc.getProperty("Inverted", false);

		aArray.addIfNotAlreadyThere(a);
	}

	sendChangeMessage();

	refreshAnyUsedState();
}

void MidiControllerAutomationHandler::handleParameterData(MidiBuffer &b)
{
	const bool bufferEmpty = b.isEmpty();
	const bool noCCsUsed = !anyUsed && !unlearnedData.used;

	if (bufferEmpty || noCCsUsed) return;

	tempBuffer.clear();

	MidiBuffer::Iterator mb(b);
	MidiMessage m;

	int samplePos;

	while (mb.getNextEvent(m, samplePos))
	{
		bool consumed = false;

		if (m.isController())
		{
			const int number = m.getControllerNumber();

			if (isLearningActive())
			{
				setUnlearndedMidiControlNumber(number, sendNotification);
			}

			for (auto& a : automationData[number])
			{
				if (a.used)
				{
					jassert(a.processor.get() != nullptr);

					auto normalizedValue = (double)m.getControllerValue() / 127.0;

					if (a.inverted) normalizedValue = 1.0 - normalizedValue;

					const double value = a.parameterRange.convertFrom0to1(normalizedValue);

					const float snappedValue = (float)a.parameterRange.snapToLegalValue(value);

					if (a.macroIndex != -1)
					{
						a.processor->getMainController()->getMacroManager().getMacroChain()->setMacroControl(a.macroIndex, (float)m.getControllerValue(), sendNotification);
					}
					else
					{
						if (a.lastValue != snappedValue)
						{
							a.processor->setAttribute(a.attribute, snappedValue, sendNotification);
							a.lastValue = snappedValue;
						}
					}

					consumed = true;
				}
			}
		}

		if (!consumed) tempBuffer.addEvent(m, samplePos);
	}

	b.clear();
	b.addEvents(tempBuffer, 0, -1, 0);
}


hise::MidiControllerAutomationHandler::AutomationData MidiControllerAutomationHandler::getDataFromIndex(int index) const
{
	int currentIndex = 0;

	for (int i = 0; i < 128; i++)
	{
		for (const auto& a: automationData[i])
		{
			if (index == currentIndex)
				return AutomationData(a);

			currentIndex++;
		}
	}

	return AutomationData();
}

int MidiControllerAutomationHandler::getNumActiveConnections() const
{
	int numActive = 0;

	for (int i = 0; i < 128; i++)
	{
		numActive += automationData[i].size();
	}

	return numActive;
}

bool MidiControllerAutomationHandler::setNewRangeForParameter(int index, NormalisableRange<double> range)
{
	int currentIndex = 0;

	for (int i = 0; i < 128; i++)
	{
		for (auto& a : automationData[i])
		{
			if (index == currentIndex)
			{
				a.parameterRange = range;
				return true;
			}
			
			currentIndex++;
		}
	}

	return false;
}

bool MidiControllerAutomationHandler::setParameterInverted(int index, bool value)
{
	int currentIndex = 0;

	for (int i = 0; i < 128; i++)
	{
		for (auto& a : automationData[i])
		{
			if (index == currentIndex)
			{
				a.inverted = value;
				return true;
			}

			currentIndex++;
		}
	}

	return false;
}

void ConsoleLogger::logMessage(const String &message)
{
	if (message.startsWith("!"))
	{
		debugError(processor, message.substring(1));
	}
	else
	{
		debugToConsole(processor, message);
	}

	
}

ControlledObject::ControlledObject(MainController *m) :
	controller(m) {
	jassert(m != nullptr);
};

ControlledObject::~ControlledObject()
{
	// Oops, this ControlledObject was not connected to a MainController
	jassert(controller != nullptr);

	masterReference.clear();
};

class DelayedRenderer::Pimpl
{
public:

	Pimpl() {}

	bool shouldDelayRendering() const 
	{
#if IS_STANDALONE_APP || IS_STANDALONE_FRONTEND
		return false;
#else
		return hostType.isFruityLoops();
#endif
	}

#if !(IS_STANDALONE_APP || IS_STANDALONE_FRONTEND)
	PluginHostType hostType;
#endif

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Pimpl)
};

DelayedRenderer::DelayedRenderer(MainController* mc_) :
	pimpl(new Pimpl()),
	mc(mc_)
{
}

DelayedRenderer::~DelayedRenderer()
{
	pimpl = nullptr;
}

bool DelayedRenderer::shouldDelayRendering() const
{
	return pimpl->shouldDelayRendering();
}

void DelayedRenderer::processWrapped(AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
	if (shouldDelayRendering())
	{
		const int thisNumSamples = buffer.getNumSamples();
		const int blockSize = writeBuffer->getNumSamples();
		const bool bufferOverflow = sampleIndex + thisNumSamples >= blockSize;

		if (bufferOverflow)
		{
			const int remainingSamples = blockSize - sampleIndex;

#if FRONTEND_IS_PLUGIN
			FloatVectorOperations::copy(writeBuffer->getWritePointer(0, sampleIndex), buffer.getReadPointer(0, 0), remainingSamples);
			FloatVectorOperations::copy(writeBuffer->getWritePointer(1, sampleIndex), buffer.getReadPointer(1, 0), remainingSamples);
#else
			delayedMidiBuffer.addEvents(midiMessages, 0, remainingSamples, sampleIndex);
#endif

			FloatVectorOperations::copy(buffer.getWritePointer(0, 0), readBuffer->getReadPointer(0, sampleIndex), remainingSamples);
			FloatVectorOperations::copy(buffer.getWritePointer(1, 0), readBuffer->getReadPointer(1, sampleIndex), remainingSamples);
			

			mc->processBlockCommon(*writeBuffer, delayedMidiBuffer);

			AudioSampleBuffer* temp = readBuffer;
			readBuffer = writeBuffer;
			writeBuffer = temp;
			delayedMidiBuffer.clear();
			sampleIndex = 0;

			const int samplesAfterWrap = thisNumSamples - remainingSamples;

			if (samplesAfterWrap > 0)
			{
				FloatVectorOperations::copy(buffer.getWritePointer(0, remainingSamples), readBuffer->getReadPointer(0, 0), samplesAfterWrap);
				FloatVectorOperations::copy(buffer.getWritePointer(1, remainingSamples), readBuffer->getReadPointer(1, 0), samplesAfterWrap);
			}

			sampleIndex += samplesAfterWrap;
		}
		else
		{

#if FRONTEND_IS_PLUGIN
			FloatVectorOperations::copy(writeBuffer->getWritePointer(0, sampleIndex), buffer.getReadPointer(0, 0), thisNumSamples);
			FloatVectorOperations::copy(writeBuffer->getWritePointer(1, sampleIndex), buffer.getReadPointer(1, 0), thisNumSamples);
#else
			delayedMidiBuffer.addEvents(midiMessages, 0, thisNumSamples, sampleIndex);
#endif

			FloatVectorOperations::copy(buffer.getWritePointer(0, 0), readBuffer->getReadPointer(0, sampleIndex), thisNumSamples);
			FloatVectorOperations::copy(buffer.getWritePointer(1, 0), readBuffer->getReadPointer(1, sampleIndex), thisNumSamples);
			
			sampleIndex += thisNumSamples;
		}
	}
	else
	{
		mc->processBlockCommon(buffer, midiMessages);
	}
}

void DelayedRenderer::prepareToPlayWrapped(double sampleRate, int samplesPerBlock)
{
	if (shouldDelayRendering())
	{
		fullBlockSize = jmin<int>(256, samplesPerBlock);

		b1.setSize(2, fullBlockSize);
		b2.setSize(2, fullBlockSize);

		b1.clear();
		b2.clear();

		delayedMidiBuffer.ensureSize(1024);

		readBuffer = &b1;
		writeBuffer = &b2;

		sampleIndex = 0;

		dynamic_cast<AudioProcessor*>(mc)->setLatencySamples(fullBlockSize);

		mc->prepareToPlay(sampleRate, fullBlockSize);
	}
	else
	{
		mc->prepareToPlay(sampleRate, samplesPerBlock);
	}
}


void OverlayMessageBroadcaster::sendOverlayMessage(int newState, const String& newCustomMessage/*=String()*/)
{
	if (currentState == DeactiveOverlay::State::CriticalCustomErrorMessage)
		return;

#if USE_BACKEND

	ignoreUnused(newState);

	// Just print it on the console
	Logger::getCurrentLogger()->writeToLog("!" + newCustomMessage);
#else

	currentState = newState;
	customMessage = newCustomMessage;

	internalUpdater.triggerAsyncUpdate();
#endif
}

} // namespace hise