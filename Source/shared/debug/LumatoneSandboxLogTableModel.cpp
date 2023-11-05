
#include "./LumatoneSandboxLogTableModel.h"
#include "LumatoneSandboxLogTableModel.h"

LumatoneSandboxLogTableModel::~LumatoneSandboxLogTableModel()
{

}

void LumatoneSandboxLogTableModel::logMessage(const juce::String& message)
{
    if (numLogs == maxLogs)
        logs.remove(0);
    else
        numLogs++;
    
    LumatoneSandboxLog logInfo = LumatoneSandboxLog::FromString(message);
    if (logInfo.time == errorTime)
    {
        logInfo.className = "LumatoneSandboxLogTableModel";
        logInfo.method = "logMessage";
        logInfo.message = "Unable to parse message: " + message;
    }

    logs.add(logInfo);
    
    juce::MessageManager::callAsync([=]() { sendChangeMessage(); });
}

const LumatoneSandboxLog& LumatoneSandboxLogTableModel::getLog(int logNum) const
{
    if (logNum < logs.size())
        return logs.getReference(logNum);
    
    return defaultLog;
}

juce::Colour LumatoneSandboxLogTableModel::getRowColour(int rowNumber, LumatoneSandboxLogStatus status)
{
    juce::Colour c = rowNumber % 2 == 0 ? juce::Colours::lightslategrey : juce::Colours::lightgrey;

    if (status == LumatoneSandboxLogStatus::WARNING)
        c.overlaidWith(juce::Colours::yellow);
    else if (status == LumatoneSandboxLogStatus::ERROR)
        c.overlaidWith(juce::Colours::red);

    return c;
}
void LumatoneSandboxLogTableModel::paintRowBackground(juce::Graphics &g, int rowNumber, int width, int height, bool rowIsSelected)
{   
    auto log = getLog(rowNumber);
    auto c = getRowColour(rowNumber, log.status);
    g.fillAll(c);
}

void LumatoneSandboxLogTableModel::paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{
    if (rowNumber < 0)
        return;
    
    const LumatoneSandboxLog& log = getLog(rowNumber);
    juce::String value;

    switch (columnId)
    {
    case LumatoneSandboxLogTableColumn::Date:
        value = log.time.toString(true, true, true, true);
        break;
    case LumatoneSandboxLogTableColumn::Class:
        value = log.className;
        break;
    case LumatoneSandboxLogTableColumn::Status:
        value = log.getStatusString();
        break;
    case LumatoneSandboxLogTableColumn::Method:
        value = log.method;
        break;
    case LumatoneSandboxLogTableColumn::Message:
        value = log.message;
        break;
    default:
        break;
    }

    juce::Colour textColour = getRowColour(rowNumber, log.status).contrasting(0.95f);
    g.setColour(textColour);
    g.drawText(value, 0, 0, width, height, juce::Justification::centredLeft);
}

