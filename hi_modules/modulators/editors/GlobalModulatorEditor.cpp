/*
  ==============================================================================

  This is an automatically generated GUI class created by the Introjucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Introjucer version: 3.2.0

  ------------------------------------------------------------------------------

  The Introjucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright (c) 2015 - ROLI Ltd.

  ==============================================================================
*/

//[Headers] You can add your own extra header files here...
namespace hise { using namespace juce;
//[/Headers]

#include "GlobalModulatorEditor.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
GlobalModulatorEditor::GlobalModulatorEditor (ProcessorEditor *p)
    : ProcessorEditorBody(p)
{
    //[Constructor_pre] You can add your own custom stuff here..
    //[/Constructor_pre]

    addAndMakeVisible (midiTable = new TableEditor (dynamic_cast<GlobalModulator*>(getProcessor())->getTable(0)));
    midiTable->setName ("new component");

    addAndMakeVisible (useTableButton = new HiToggleButton ("new toggle button"));
    useTableButton->setTooltip (TRANS("Use a table to calculate the value"));
    useTableButton->setButtonText (TRANS("UseTable"));
    useTableButton->addListener (this);
    useTableButton->setColour (ToggleButton::textColourId, Colours::white);

    addAndMakeVisible (globalModSelector = new ComboBox ("new combo box"));
    globalModSelector->setEditableText (false);
    globalModSelector->setJustificationType (Justification::centredLeft);
    globalModSelector->setTextWhenNothingSelected (TRANS("Select Global Modulator"));
    globalModSelector->setTextWhenNoChoicesAvailable (TRANS("(no choices)"));
    globalModSelector->addListener (this);


    //[UserPreSize]

	getProcessor()->getMainController()->skin(*useTableButton);

	tableUsed = getProcessor()->getAttribute(GlobalModulator::UseTable) == 1.0f;

	midiTable->connectToLookupTableProcessor(getProcessor());

	useTableButton->setup(getProcessor(), GlobalModulator::UseTable, "Use Table");

	getProcessor()->getMainController()->skin(*globalModSelector);

	setItemEntry();
    
    

    //[/UserPreSize]

    setSize (900, 200);


    //[Constructor] You can add your own custom stuff here..

	h = getHeight();


    //[/Constructor]
}

GlobalModulatorEditor::~GlobalModulatorEditor()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    midiTable = nullptr;
    useTableButton = nullptr;
    globalModSelector = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void GlobalModulatorEditor::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

   
	ProcessorEditorLookAndFeel::fillEditorBackgroundRect(g, this);

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void GlobalModulatorEditor::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

    midiTable->setBounds ((getWidth() / 2) + -2 - ((getWidth() - 116) / 2), 63, getWidth() - 116, 120);
    useTableButton->setBounds (56, 16, 128, 32);
    globalModSelector->setBounds (getWidth() - 60 - (getWidth() - 268), 16, getWidth() - 268, 32);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void GlobalModulatorEditor::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == useTableButton)
    {
        //[UserButtonCode_useTableButton] -- add your button handler code here..
		tableUsed = useTableButton->getToggleState();
		getProcessor()->setAttribute(GlobalModulator::UseTable, tableUsed ? 1.0f : 0.0f, dontSendNotification);

		refreshBodySize();
        //[/UserButtonCode_useTableButton]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}

void GlobalModulatorEditor::comboBoxChanged (ComboBox* comboBoxThatHasChanged)
{
    //[UsercomboBoxChanged_Pre]
    //[/UsercomboBoxChanged_Pre]

    if (comboBoxThatHasChanged == globalModSelector)
    {
        //[UserComboBoxCode_globalModSelector] -- add your combo box handling code here..

		const String text = globalModSelector->getText();

		dynamic_cast<GlobalModulator*>(getProcessor())->connectToGlobalModulator(text);

        //[/UserComboBoxCode_globalModSelector]
    }

    //[UsercomboBoxChanged_Post]
    //[/UsercomboBoxChanged_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...


void GlobalModulatorEditor::setItemEntry()
{
	GlobalModulator *gm = dynamic_cast<GlobalModulator*>(getProcessor());

	StringArray itemList = gm->getListOfAllModulatorsWithType();

	String item = gm->getItemEntryFor(gm->getConnectedContainer(), gm->getOriginalModulator());

	globalModSelector->clear(dontSendNotification);

	globalModSelector->addItemList(itemList, 1);

	const int index = itemList.indexOf(item);



	if (index != -1)
	{
		globalModSelector->setSelectedItemIndex(index, dontSendNotification);
	}
}

//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="GlobalModulatorEditor" componentName=""
                 parentClasses="public ProcessorEditorBody" constructorParams="ProcessorEditor *p"
                 variableInitialisers="ProcessorEditorBody(p)&#10;" snapPixels="8"
                 snapActive="1" snapShown="1" overlayOpacity="0.330" fixedSize="1"
                 initialWidth="900" initialHeight="200">
  <BACKGROUND backgroundColour="ffffff">
    <ROUNDRECT pos="0Cc 3 84M 6M" cornerSize="6" fill="solid: 30000000" hasStroke="1"
               stroke="2, mitered, butt" strokeColour="solid: 25ffffff"/>
  </BACKGROUND>
  <GENERICCOMPONENT name="new component" id="e2252e55bedecdc5" memberName="midiTable"
                    virtualName="" explicitFocusOrder="0" pos="-2Cc 63 116M 120"
                    class="TableEditor" params="dynamic_cast&lt;GlobalModulator*&gt;(getProcessor())-&gt;getTable(0)"/>
  <TOGGLEBUTTON name="new toggle button" id="e77edc03c117de85" memberName="useTableButton"
                virtualName="HiToggleButton" explicitFocusOrder="0" pos="56 16 128 32"
                tooltip="Use a table to calculate the value" txtcol="ffffffff"
                buttonText="UseTable" connectedEdges="0" needsCallback="1" radioGroupId="0"
                state="0"/>
  <COMBOBOX name="new combo box" id="f9053c2b9246bbfc" memberName="globalModSelector"
            virtualName="ComboBox" explicitFocusOrder="0" pos="60Rr 16 268M 32"
            posRelativeX="3b242d8d6cab6cc3" editable="0" layout="33" items=""
            textWhenNonSelected="Select Global Modulator" textWhenNoItems="(no choices)"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
} // namespace hise
//[/EndFile]
