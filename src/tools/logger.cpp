#include "tools/logger.h"
#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/tchar.h>
#include <string>



bool Slog::Init(const char* properties_filename) {
    if (properties_filename == NULL) {
        return false;
    }
    log4cplus::initialize();
    log4cplus::PropertyConfigurator::doConfigure(LOG4CPLUS_TEXT( properties_filename));
    log4cplus::Logger logger_ = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("global"));
    LOG4CPLUS_INFO(logger_, "Logger System start finish"); 
    return true;
}

Slog::~Slog() {
    /*log4cplus::Logger logger_ = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("global"));
    LOG4CPLUS_INFO(logger_, "Logger System Stop Finish...");
    logger_.shutdown();*/
    Stop();
}

void Slog::Stop() {
    log4cplus::Logger logger_ = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("global"));
    LOG4CPLUS_INFO(logger_, "Logger System Stop Finish...");
    logger_.shutdown();
}

#define DO_LOGGER(logLevel, pFormat, bufSize) \
    log4cplus::Logger logger_ = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("global"));\
    \
    if(logger_.isEnabledFor(logLevel))\
{                \
    va_list args;            \
    va_start(args, pFormat);        \
    char buf[bufSize] = {0};        \
    vsnprintf(buf, sizeof(buf), pFormat, args);    \
    va_end(args);           \
    logger_.forcedLog(logLevel, buf); \
}

void Slog::Debug(const char* pFormat, ...) {
    DO_LOGGER(log4cplus::DEBUG_LOG_LEVEL, pFormat, 1024);
}

void Slog::Error(const char* pFormat,  ...) {
    DO_LOGGER(log4cplus::ERROR_LOG_LEVEL, pFormat, 256);
}
 
void Slog::Fatal(const char* pFormat, ... ) {
    DO_LOGGER(log4cplus::FATAL_LOG_LEVEL, pFormat, 256);
}
 
void Slog::Info( const char* pFormat,  ...) {
    DO_LOGGER(log4cplus::INFO_LOG_LEVEL, pFormat, 512);
}
 
void Slog::Warn( const char* pFormat,  ...) {
    DO_LOGGER(log4cplus::WARN_LOG_LEVEL, pFormat, 256);
}
 
void Slog::Trace(const char* pFormat,  ...) {
    DO_LOGGER(log4cplus::TRACE_LOG_LEVEL, pFormat, 1024);

}
