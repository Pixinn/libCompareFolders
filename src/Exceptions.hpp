#ifndef _SRC_EXCEPTIONS_H__
#define _SRC_EXCEPTIONS_H__

#include <exception>
#include <string>

namespace cf
{

    /// @brief A recoverable exception
    class ExceptionMinor : public std::runtime_error
    {
    public:
        /// @brief Constructor
        /// @param msg Message explaining the origin of the exception
        ExceptionMinor(const std::string& msg)  : 
            std::runtime_error(msg)
        {   }
    };

}

#endif
