#ifndef ANIMATION_LOGGER_H
#define ANIMATION_LOGGER_H
#include <string>

class Logger {
private:

public:
    static const bool enable = true;
    static void error(std::string module, std::string message);
    static void info(std::string module, std::string message);
};

#endif
