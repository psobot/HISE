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

#ifndef SAMPLEPOOLTABLE_H_INCLUDED
#define SAMPLEPOOLTABLE_H_INCLUDED

namespace hise { using namespace juce;

class ModulatorSamplerSoundPool;

/** A table component containing all samples of a ModulatorSampler.
*	@ingroup debugComponents
*/
class SamplePoolTable      : public Component,
                             public TableListBoxModel,
							 public SafeChangeListener
{
public:

	enum ColumnId
	{
		FileName = 1,
		Memory,
		State,
		References,
		numColumns
	};

	SamplePoolTable(BackendRootWindow *rootWindow) ;

	SET_GENERIC_PANEL_ID("SamplePoolTable");

	~SamplePoolTable();

	void changeListenerCallback(SafeChangeBroadcaster *) override
	{
		setName(getHeadline());
		table.updateContent();
		if(getParentComponent() != nullptr) getParentComponent()->repaint();
	}

    int getNumRows() override;

    void paintRowBackground (Graphics& g, int rowNumber, int /*width*/, int /*height*/, bool rowIsSelected) override;

	void selectedRowsChanged(int /*lastRowSelected*/) override;

    void paintCell (Graphics& g, int rowNumber, int columnId,
                    int width, int height, bool /*rowIsSelected*/) override;
    
	String getHeadline() const;

    void resized() override;

	void mouseDown(const MouseEvent &e) override;

private:
    TableListBox table;     // the table component itself
    Font font;

	ModulatorSamplerSoundPool *pool;
	ScopedPointer<TableHeaderLookAndFeel> laf;
	PopupLookAndFeel plaf;

    int numRows;            // The number of rows of data we've got

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SamplePoolTable)
};

class MainController;

/** A table component containing all samples of a ModulatorSampler.
*	@ingroup components
*/
class ExternalFileTableBase      : public Component,
                             public TableListBoxModel,
							 public SafeChangeListener,
							 public DragAndDropContainer
{
public:

	enum ColumnId
	{
		FileName = 1,
		Memory,
		Type,
		References,
		numColumns
	};

	ExternalFileTableBase(SharedPoolBase *pool) ;

	~ExternalFileTableBase();

	
	void changeListenerCallback(SafeChangeBroadcaster *) override
	{
		setName(getHeadline());
		table.updateContent();
		if(getParentComponent() != nullptr) getParentComponent()->repaint();
	}

    int getNumRows() override;

    void paintRowBackground (Graphics& g, int rowNumber, int /*width*/, int /*height*/, bool rowIsSelected) override;

	void selectedRowsChanged(int /*lastRowSelected*/) override;

    void paintCell (Graphics& g, int rowNumber, int columnId,
                    int width, int height, bool /*rowIsSelected*/) override;
    
	String getHeadline() const;

    void resized() override;

	

	String getTextForTableCell(int rowNumber, int columnNumber);

	var getDragSourceDescription(const SparseSet< int > &set) override
	{
		var id;

		if(set.getNumRanges() > 0)
		{
			const int index = set[0];

			Identifier name = pool->getIdForIndex(index);

			String x = pool->getFileNameForId(name);

			id = x;
			
		}

		return id;
	};

protected:

	SharedPoolBase *pool;

private:
    TableListBox table;     // the table component itself
    Font font;

	int selectedRow;

	var currentlyDraggedId;

	

	
	ScopedPointer<TableHeaderLookAndFeel> laf;
	PopupLookAndFeel plaf;

    int numRows;            // The number of rows of data we've got

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ExternalFileTableBase)
};

struct PoolTableSubTypes
{
	struct AudioFilePoolTable : public ExternalFileTableBase
	{
	public:

		AudioFilePoolTable(BackendRootWindow* rootWindow);

		void cellClicked(int /*rowNumber*/, int /*columnId*/, const MouseEvent&)
		{

		}

		virtual void cellDoubleClicked(int rowNumber, int columnId, const MouseEvent&);

		SET_GENERIC_PANEL_ID("AudioFilePoolTable");
	};

	struct ImageFilePoolTable : public ExternalFileTableBase
	{
	public:

		ImageFilePoolTable(BackendRootWindow* rootWindow);

		virtual void cellDoubleClicked(int rowNumber, int columnId, const MouseEvent&);

		SET_GENERIC_PANEL_ID("ImagePoolTable");
	};
};



} // namespace hise

#endif  // SAMPLEPOOLTABLE_H_INCLUDED
