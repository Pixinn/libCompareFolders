/*
3  *  Copyright (C) 2017 Christophe Meneboeuf <christophe@xtof.info>
4  *
5  *  This program is free software: you can redistribute it and/or modify
6  *  it under the terms of the GNU General Public License as published by
7  *  the Free Software Foundation, either version 3 of the License, or
8  *  (at your option) any later version.
9  *
10  *  This program is distributed in the hope that it will be useful,
11  *  but WITHOUT ANY WARRANTY; without even the implied warranty of
12  *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
13  *  GNU General Public License for more details.
14  *
15  *  You should have received a copy of the GNU General Public License
16  *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
17 */


#ifndef _SRC_EXCEPTIONS_H__
#define _SRC_EXCEPTIONS_H__

#include <exception>
#include <list>
#include <string>

namespace cf
{

    /// @brief The root of the exception hierarchy
    class IException : public std::runtime_error
    {
    public:
        /// @brief Constructor
        /// @param msg Message explaining the origin of the exception
        IException(const std::string& msg) :
            std::runtime_error(msg)
        {   }
    };

    /// @brief A recoverable exception
    class ExceptionMinor : public IException
    {
    public:
        /// @brief Constructor
        /// @param msg Message explaining the origin of the exception
        ExceptionMinor(const std::string& msg);
    };

    /// @brief A fatal error occured
    class ExceptionFatal : public IException
    {
    public:
        /// @brief Constructor
        /// @param msg Message explaining the origin of the exception
        ExceptionFatal(const std::string& msg);
    };


    /// @brief Interface for an error logger
    class ILogError
    {
    public: 
        ILogError() = default;
        virtual ~ILogError() = default;
        /// @brief Logs the provided error message
        virtual void log(const std::string& message) = 0;
    };

    /// @brief Null error logger (Singleton)
    class SLogErrorNull : public ILogError
    {
    public:
        void log(const std::string&) override { }
        static SLogErrorNull& GetInstance() {
            return _Instance;
        }
    private:
        static SLogErrorNull _Instance;
    };


    /// @brief Holds the differences between two folders named *left* and *right*
    typedef struct diff_t
    {

        /// @brief Files with different path but the very same content.
        /// Those files can have **duplicates** both left and right
        typedef struct renamed_t {
            std::list<std::string> left;  ///< Left files with the same content
            std::list<std::string> right; ///< Right files with the same content
        }renamed_t;

        const std::string root_left;               ///< Left directory root path
        const std::string root_right;              ///< Right directory root path
        const std::list<std::string> identical;    ///< Identical files
        const std::list<std::string> different;    ///< Different files
        const std::list<std::string> unique_left;  ///< Files that are unique to the left directory
        const std::list<std::string> unique_right; ///< Files that are unique to the right directory
        const std::list<renamed_t> renamed;     ///< Files with different reative path that have the same content
    } diff_t;


    /// @brief Compares the content of two folders
    /// @param left First folder's path
    /// @param right Second folder's path
    /// @param logErrors A logger to catch minor errors that could happen. By default, the NULL logger will ignore them.
    /// @details Returns the differences between the two folders. Files can be identical or unique.
    ///          Identical files but with a different names are also detected.
    diff_t CompareFolders(const std::string& left, const std::string right, ILogError& logErrors = SLogErrorNull::GetInstance());
}

#endif
