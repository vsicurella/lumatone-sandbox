
#include "LumatoneSandboxLogger.h"

LumatoneSandboxLogger::LumatoneSandboxLogger(juce::String classNameIn)
    : className(classNameIn)
{

}

void LumatoneSandboxLogger::Log(LumatoneSandboxLog info)
{
    juce::String logMessage = info.toFullString();
    juce::Logger::writeToLog(logMessage);

    juce::String simpleMsg = info.toShortString();
    juce::Logger::outputDebugString(simpleMsg);
}

void LumatoneSandboxLogger::logMessage(const juce::String &message)
{
    juce::Logger::writeToLog(message);
}

void LumatoneSandboxLogger::log(LumatoneSandboxLogStatus status, juce::String method, juce::String message) const
{
    LumatoneSandboxLog info = getLog(status, method, message);
    LumatoneSandboxLogger::Log(info);
}

void LumatoneSandboxLogger::logInfo(juce::String method, juce::String message) const
{
    log(LumatoneSandboxLogStatus::INFO, method, message);
}

void LumatoneSandboxLogger::logWarning(juce::String method, juce::String message) const
{
    log(LumatoneSandboxLogStatus::WARNING, method, message);
}

void LumatoneSandboxLogger::logError(juce::String method, juce::String message) const
{
    log(LumatoneSandboxLogStatus::ERROR, method, message);
}

LumatoneSandboxLog LumatoneSandboxLogger::getLog(LumatoneSandboxLogStatus status, juce::String method, juce::String message) const
{
    juce::Time now = juce::Time::getCurrentTime();

    LumatoneSandboxLog info = 
    {
        now,
        className,
        status,
        method,
        message
    };

    return info;
}

juce::String LumatoneSandboxLog::toFullString() const
{
    juce::StringArray tokens;

    auto dateStamp = time.toISO8601(true);
    tokens.add(dateStamp);

    tokens.add(className);

    switch (status)
    {
    case LumatoneSandboxLogStatus::ERROR:
        tokens.add("Error");
        break;
    case LumatoneSandboxLogStatus::INFO:
        tokens.add("Info");
        break;
    case LumatoneSandboxLogStatus::WARNING:
        tokens.add("Warning");
        break;
    default:
        jassertfalse;
    }

    tokens.add(method + "()");
    tokens.add(message);
    
    if (info.size() > 0)
    {
        auto infoKeys = info.getAllKeys();
        auto infoValues = info.getAllValues();
        auto infoPacked = infoKeys.joinIntoString(",") + juce::String(":") + infoValues.joinIntoString(",");
        tokens.add(infoPacked);
    }

    juce::String logMsg = tokens.joinIntoString("|");
    return logMsg;
}

juce::String LumatoneSandboxLog::toShortString() const
{
    juce::StringArray tokens;

    tokens.add(className);

    juce::String statusString = getStatusString();
    tokens.add(statusString);

    tokens.add(method + "()");
    tokens.add(message);

    juce::String logMsg = tokens.joinIntoString(": ");
    return logMsg;
}

juce::String LumatoneSandboxLog::getStatusString() const
{
    switch (status)
    {
    default:
        jassertfalse;
    case LumatoneSandboxLogStatus::ERROR:
        return "Error";

    case LumatoneSandboxLogStatus::INFO:
        return "Info";
    case LumatoneSandboxLogStatus::WARNING:
        return "Warning";
    }

    return juce::String();
}

LumatoneSandboxLog LumatoneSandboxLog::FromString(juce::String logString)
{
    LumatoneSandboxLog logInfo;

    juce::String properties;

    juce::juce_wchar delim('|');
    const int lastIndex = logString.length() - 1;
    int wordStart = 0;
    int wordNum = 0;

    for (int i = 1; i < logString.length(); i++)
    {
        if (logString[i] == delim || i == lastIndex)
        {
            if (i == lastIndex)
                i++;

            auto word = logString.substring(wordStart, i);
            switch (wordNum)
            {
            case 0:
                logInfo.time = juce::Time::fromISO8601(word);
                break;
            case 1:
                logInfo.className = word;
                break;
            case 2:
                logInfo.status = LumatoneSandboxLog::LogStringToStatus(word);
                break;
            case 3:
                logInfo.method = word;
                break;
            case 4:
                logInfo.message = word;
                break;
            case 5:
                properties = word;
                break;
            default:
                jassertfalse;
                break;
            }

        wordStart = i + 1;
        i += 2;
        wordNum++;
        }
    }
    
    if (properties.length() > 0)
    {
        // auto propStrings = juce::StringArray::fromTokens(properties, "|");
        // auto keys = juce::StringArray::fromTokens(propStrings[0], ",");
        // auto values = juce::StringArray::fromTokens(propStrings[1], ",");
        
        // juce::StringPairArray props;
        // for (int i = 0; i < keys.size(); i++)
        // {
        //     props.set(keys[i], values[i]);
        // }
        
        // logInfo.info = props;
    }
    
    return logInfo;
}

LumatoneSandboxLogStatus LumatoneSandboxLog::CodeToStatus(int statusCode)
{
    if (statusCode == 0)
        return LumatoneSandboxLogStatus::INFO;
    if (statusCode > 0)
        return LumatoneSandboxLogStatus::WARNING;
    return LumatoneSandboxLogStatus::ERROR;
}

LumatoneSandboxLogStatus LumatoneSandboxLog::LogStringToStatus(juce::StringRef statusString)
{
    if (juce::String(statusString).startsWith("Info"))
        return LumatoneSandboxLogStatus::INFO;
    if (juce::String(statusString).startsWith("Warning"))
        return LumatoneSandboxLogStatus::WARNING;
    
    return LumatoneSandboxLogStatus::ERROR;
}
