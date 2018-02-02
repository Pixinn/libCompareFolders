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

#include <stdexcept>
#include <list>
#include <string>

namespace cf
{
    /// @brief Description of the keys used in JSON files
    static const struct  {
        const std::string GENERATOR{ "Generator" };                 ///< Program used to generate the JSON file
        const std::string ROOT{ "root" };                           ///< Root folder
        const struct  {
            const std::string IDENTICAL{ "identical" };             ///< Identical files
            const std::string DIFFERENT{ "different" };             ///< Different files
            const std::string UNIQUE_LEFT{ "unique left" };         ///< Files that are unique to the left
            const std::string UNIQUE_RIGHT{ "unique right" };       ///< Files that are unique to the right
            const std::string RENAMED{ "renamed and duplicates" };  ///< Files that are identical but where renamed, moved or duplicated
            const std::string LEFT{ "left" };
            const std::string RIGHT{ "right" };
        } DIFF;                                                     ///< Differences betwwen two folders
        const struct {
            const std::string FILES{ "files" };                     ///< Files inside a folder
            const std::string HASH{ "hash" };                       ///< Hash of a file's conte,t
            const std::string TIME{ "last_modified" };              ///< Time of file's last modification
        } CONTENT;                                                  ///< Description of a filder's content
    } JSON_KEYS;

    /// @brief Description of the const values that pmay be used in JSON files
    static const struct {
        const std::string GENERATOR{ "info.xtof.COMPARE_FOLDERS" };
    } JSON_CONST_VALUES;
	
	
	    /// @brief Description of the keys used in JSON files
    static const struct  {
        const std::wstring GENERATOR{ L"Generator" };                 ///< Program used to generate the JSON file
        const std::wstring ROOT{ L"root" };                           ///< Root folder
        const struct  {
            const std::wstring IDENTICAL{ L"identical" };             ///< Identical files
            const std::wstring DIFFERENT{ L"different" };             ///< Different files
            const std::wstring UNIQUE_LEFT{ L"unique left" };         ///< Files that are unique to the left
            const std::wstring UNIQUE_RIGHT{ L"unique right" };       ///< Files that are unique to the right
            const std::wstring RENAMED{ L"renamed and duplicates" };  ///< Files that are identical but where renamed, moved or duplicated
            const std::wstring LEFT{ L"left" };
            const std::wstring RIGHT{ L"right" };
        } DIFF;                                                     ///< Differences betwwen two folders
        const struct {
            const std::wstring FILES{ L"files" };                     ///< Files inside a folder
            const std::wstring HASH{ L"hash" };                       ///< Hash of a file's conte,t
            const std::wstring TIME{ L"last_modified" };              ///< Time of file's last modification
        } CONTENT;                                                  ///< Description of a filder's content
    } WJSON_KEYS;

    /// @brief Description of the const values that pmay be used in JSON files
    static const struct {
        const std::wstring GENERATOR{ L"info.xtof.COMPARE_FOLDERS" };
    } WJSON_CONST_VALUES;

    /// @brief A fatal error occured
    class ExceptionFatal : public std::runtime_error
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

    /// @brief JSON file
    typedef struct json_t {
        json_t(const std::string& p_path) :
            path{ p_path }
        {     }
        
        const std::string path;
    } json_t;


    /// @brief Holds the differences between two folders named *left* and *right*
    typedef struct diff_t
    {
        /// @brief Custom operator== as std::list does not provide any
        bool operator==(const diff_t& rhs) const noexcept;
       
        /// @brief Files with different path but the very same content.
        /// Those files can have **duplicates** both left and right
        typedef struct renamed_t
        {
            bool operator==(const renamed_t& rhs) const noexcept;
            std::string hash;             ///< hash of the files
            std::list<std::string> left;  ///< Left files with the same content
            std::list<std::string> right; ///< Right files with the same content
        } renamed_t;

        std::string root_left;               ///< Left directory root path
        std::string root_right;              ///< Right directory root path
        std::list<std::string> identical;    ///< Identical files
        std::list<std::string> different;    ///< Different files
        std::list<std::string> unique_left;  ///< Files that are unique to the left directory
        std::list<std::string> unique_right; ///< Files that are unique to the right directory
        std::list<renamed_t> renamed;        ///< Files with different reative path that have the same content
    } diff_t;


    /// @brief Compares the content of two folders
    /// @param left First folder's path
    /// @param right Second folder's path
    /// @param logErrors A logger to catch minor errors that could happen. By default, the NULL logger will ignore them.
    /// @details Returns the differences between the two folders.
    ///          Identical files but with a different names are also detected.
    diff_t CompareFolders(const std::string& left, const std::string& right, ILogError& logErrors = SLogErrorNull::GetInstance());

    /// @brief Compares the content of two JSON files
    /// @param left First JSON file
    /// @param right Second JSON file
    /// @details Returns the differences between the content described by the JSON files.
    ///          Identical files but with a different names are also detected.
    diff_t CompareFolders(const json_t left, const json_t right);

    /// @brief Compares the content of two JSON files
    /// @param folder The folder path
    /// @param json The JSON file
    /// @param logErrors A logger to catch minor errors that could happen. By default, the NULL logger will ignore them.
    /// @details Returns the differences between the content described by the JSON files.
    ///          Identical files but with a different names are also detected.
    diff_t CompareFolders(const std::string& folder, const json_t json, ILogError& logErrors = SLogErrorNull::GetInstance());

    /// @brief Produces a JSON string with the difference between two folders
    /// @param diff Difference between two folders
    std::string Json(const diff_t diff);

    /// @brief Analyzes the content of a folder and returns a JSON string
    /// @param path Path of the folder to be analyzed
    std::wstring ScanFolder(const std::string& path, ILogError& logErrors = SLogErrorNull::GetInstance());
}

#endif
