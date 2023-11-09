#ifndef LUMATONE_SANDBOX_LOGGER_H
#define LUMATONE_SANDBOX_LOGGER_H

#include <JuceHeader.h>

enum class LumatoneSandboxLogStatus
{
    ERROR   = -1,
    INFO    = 0,
    WARNING = 1
};

struct LumatoneSandboxLog
{
    juce::Time time;
    juce::String className;
    LumatoneSandboxLogStatus status = LumatoneSandboxLogStatus::ERROR;
    juce::String method = "undefined";
    juce::String message = "";
    juce::StringPairArray info = juce::StringPairArray();

    juce::String toFullString() const;
    juce::String toShortString() const;

    juce::String getStatusString() const;

    static LumatoneSandboxLog FromString(juce::String logString);

    static LumatoneSandboxLogStatus CodeToStatus(int statusCode);
    static LumatoneSandboxLogStatus LogStringToStatus(juce::StringRef statusString);
};

class LumatoneSandboxLogger : public juce::Logger
{
public:
    typedef enum
    {
        NONE = 0,
        ERROR,
        WARNING,
        INFO,
        VERBOSE
    } LogLevel;

    static LogLevel StatusToLogLevel(LumatoneSandboxLogStatus status)
    {
        int code = static_cast<int>(status);
        if (code == 0)
            return LogLevel::INFO;
        if (code > 0)
            return LogLevel::WARNING;
        return LogLevel::ERROR;
    }

public:

    LumatoneSandboxLogger(juce::String className);
    virtual ~LumatoneSandboxLogger() override { }

    // Basic log message with status parameter
    void log(LumatoneSandboxLogStatus status, juce::String method, juce::String message) const;

    // Status-based helper methods
    void logInfo(juce::String method, juce::String message) const;
    void logWarning(juce::String method, juce::String message) const;
    void logError(juce::String method, juce::String message) const;

    // Helper to build struct, mainly to add more info
    LumatoneSandboxLog getLog(LumatoneSandboxLogStatus status, juce::String method, juce::String message) const;

public:

    // Send logs to juce::Logger registered with applicatoin
    static void Log(LumatoneSandboxLog logInfo);
    
private:

    virtual void logMessage(const juce::String& message) override;

protected:

    juce::String className = "undefined";

    LogLevel logLevel = LogLevel::INFO;

    JUCE_DECLARE_NON_COPYABLE(LumatoneSandboxLogger)
};

#endif // LUMATONE_SANDBOX_LOGGER_H
