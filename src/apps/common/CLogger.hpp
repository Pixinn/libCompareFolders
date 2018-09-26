#ifndef __SRC_APP_COMMON_CLOGGER_HPP__
#define __SRC_APP_COMMON_CLOGGER_HPP__

#include <iostream>
#include <string>

#include <rlutil/rlutil.h>
#include "CompareFolders.hpp"


/// @brief Logging class displaying colored text to the console
class CLogger : public cf::ILogger
{
public:
    CLogger() {        
        rlutil::saveDefaultColor();        
    }
    ~CLogger() {
        rlutil::resetColor();
    }

    void message(const std::string& message) override final {
        rlutil::setColor(rlutil::CYAN);
        std::cout << message;
        rlutil::resetColor();
    }
    void error(const std::string& message) override final {
        rlutil::setColor(rlutil::LIGHTRED);
        std::cout << message << std::endl;
        rlutil::resetColor();
    }
};


#endif
