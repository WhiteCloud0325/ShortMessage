#ifndef LOGGER_H_
#define LOGGER_H_

#include <log4cplus/logger.h>
#include <log4cplus/layout.h>
#include <log4cplus/configurator.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/helpers/stringhelper.h>

class Slog{
public:
    bool Init(const char* properties_filename);
    ~Slog();
    static inline Slog& GetInstance() {
        static Slog instance_;
        return instance_;
    }
    void Debug(const char* pFormat, ...);
    void Error(const char* pFormat, ...);
    void Fatal(const char* pFormat, ...);
    void Info(const char* pFormat, ...);
    void Warn(const char* pFormat, ...); 
    void Trace(const char* pFormat, ...);

private:
    Slog(){};
    void Stop();
};

#define Log (Slog::GetInstance())

#define LOG_DEBUG(...)    Log.Debug(__VA_ARGS__);
#define LOG_ERROR(...)    Log.Error(__VA_ARGS__);
#define LOG_FATAL(...)    Log.Fatal(__VA_ARGS__);
#define LOG_INFO(...)     Log.Info(__VA_ARGS__);
#define LOG_WARN(...)     Log.Warn(__VA_ARGS__);
#define LOG_TRACE(...)    Log.Trace(__VA_ARGS__);

#endif  /* define logger.h*/
