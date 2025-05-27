#include "Logger.h"
#include <iostream>

void Logger::error(std::string module, std::string message) {
    if (!enable) return;
    std::cerr << "ERROR::" << module << "::" << message << std::endl;
}

void Logger::info(std::string module, std::string message) {
    if (!enable) return;
    std::cout << "INFO::" << module << "::" << message << std::endl;
}
