#ifndef _SRC_EXCEPTIONS_H__
#define _SRC_EXCEPTIONS_H__

#include <exception>
#include <string>

namespace cf
{

    class ExceptionMinor : public std::runtime_error
    {
    public:
        ExceptionMinor(const std::string& str)  : 
            std::runtime_error(str)
        {   }
    };

}

#endif
