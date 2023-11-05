#include "LumatoneSandboxDebugWindow.h"

#include "LumatoneSandboxLogTableModel.h"

LumatoneSandboxDebugWindow::LumatoneSandboxDebugWindow(LumatoneSandboxLogTableModel *logTableModelIn)
    : juce::DocumentWindow("LumatoneSandboxDebugWindow", 
                           juce::Colours::darkslategrey, 
                           juce::DocumentWindow::TitleBarButtons::minimiseButton,
                           true)
{

    logModel = logTableModelIn;

    auto logTable = new juce::TableListBox("LumatoneSandboxLogTable", static_cast<juce::TableListBoxModel*>(logModel));
    logTable->setHeader(std::make_unique<juce::TableHeaderComponent>());
    logTable->getHeader().addColumn("Date",     LumatoneSandboxLogTableColumn::Date,    160);
    logTable->getHeader().addColumn("Class",    LumatoneSandboxLogTableColumn::Class,   128);
    logTable->getHeader().addColumn("Status",   LumatoneSandboxLogTableColumn::Status,  72);
    logTable->getHeader().addColumn("Method",   LumatoneSandboxLogTableColumn::Method,  160);
    logTable->getHeader().addColumn("Message",  LumatoneSandboxLogTableColumn::Message, 1024);

    logModel->addChangeListener(this);

    logTable->setSize(1024, 1024);
    setContentOwned(logTable, true);
}

void LumatoneSandboxDebugWindow::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == logModel)
    {
        getContentComponent()->resized();
    }
}
