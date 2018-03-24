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
     typedef struct {
         /// @brief Differences betwwen two folders
         typedef struct {
             std::wstring IDENTICAL;///< Identical files
             std::wstring DIFFERENT;///< Different files
             std::wstring UNIQUE_LEFT;///< Files that are unique to the left
             std::wstring UNIQUE_RIGHT;///< Files that are unique to the right
             std::wstring RENAMED;///< Files that are identical but where renamed, moved or duplicated
             std::wstring LEFT;
             std::wstring RIGHT;
         } DIFF_t;
         /// @brief Description of a folder's content
         typedef struct {
             std::wstring FILES;//< Files inside a folder
             std::wstring HASH;///< Hash of a file's content
             std::wstring TIME;///< Time of file's last modification
             std::wstring SIZE;///< Size of the file
         } CONTENT_t;
         std::wstring GENERATOR; ///< Program used to generate the JSON file
         std::wstring ROOT;///< Root folder
         std::wstring ALGO_HASH;///< Algorithm used to compute hashes
         DIFF_t DIFF;
         CONTENT_t CONTENT;
     } JSON_KEYS_t;

     /// @brief Description of the const values that may be used in JSON files
     typedef struct {
         std::wstring GENERATOR;
         std::wstring ALGO_HASH_FAST;
         std::wstring ALGO_HASH_SECURE;
     } JSON_CONST_VALUES_t;

     static const JSON_KEYS_t JSON_KEYS {
         L"Generator",                   // GENERATOR
         L"root",                        // ROOT
         L"hash",                        // ALGO_HASH
         {   // DIFF
             L"identical",               // IDENTICAL
             L"different",               // DIFFERENT
             L"unique left",             // UNIQUE_LEFT
             L"unique right",            // UNIQUE_RIGHT
             L"renamed and duplicates",  // RENAMED
             L"left",                    // LEFT
             L"right"                    // RIGHT
         },
         {   // CONTENT
             L"files",                   // FILES
             L"hash",                    // HASH
             L"last_modified",           // TIME
             L"size"                     // SIZE
         }
     };

     static const JSON_CONST_VALUES_t JSON_CONST_VALUES {
         L"info.xtof.COMPARE_FOLDERS",   // GENERATOR
         L"fast",                        // ALGO_HASH_FAST
         L"secure"                       // ALGO_HASH_SECURE
     };

    /// @brief A fatal error occured
    class ExceptionFatal : public std::runtime_error
    {
    public:
        /// @brief Constructor
        /// @param msg Message explaining the origin of the exception
        ExceptionFatal(const std::string& msg);
    };

    /// @brief A an error occured which **may** be recoverable
    class Exception : public std::runtime_error
    {
    public:
        /// @brief Constructor
        /// @param msg Message explaining the origin of the exception
        Exception(const std::string& msg);
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
	/// @details This is a mere facade to the path
    typedef struct json_t {
        explicit json_t(const std::string& p_path) :
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
            std::list<std::wstring> left;  ///< Left files with the same content
            std::list<std::wstring> right; ///< Right files with the same content
        } renamed_t;

        std::wstring root_left;               ///< Left directory root path
        std::wstring root_right;              ///< Right directory root path
        std::list<std::wstring> identical;    ///< Identical files
        std::list<std::wstring> different;    ///< Different files
        std::list<std::wstring> unique_left;  ///< Files that are unique to the left directory
        std::list<std::wstring> unique_right; ///< Files that are unique to the right directory
        std::list<renamed_t> renamed;        ///< Files with different reative path that have the same content
    } diff_t;


    /// @brief Algorithm used to compute file's hashes
    /// @detailled Hashes are used to find which files are different, or were renamed / moved.
    typedef enum eHashingAlgorithm {
        FAST,       ///< **Faster** algorithm. Reliable and sufficient in *most* situation.
        SECURE      ///< **100% reliable**. Using a *slow* but secure cryptographic hashing function.
    } eCollectingAlgorithm;


    /// @brief Compares the content of two folders
    /// @param left First folder's path
    /// @param right Second folder's path
    /// @param logErrors A logger to catch minor errors that could happen. By default, the NULL logger will ignore them.
    /// @details Returns the differences between the two folders.
    ///          Identical files but with a different names are also detected.
    diff_t CompareFolders(const std::string& left, const std::string& right, const eHashingAlgorithm algo, ILogError& logErrors = SLogErrorNull::GetInstance());

    /// @brief Compares the content of two JSON files
    /// @param left First JSON file
    /// @param right Second JSON file
    /// @details Returns the differences between the content described by the JSON files.
    ///          Identical files but with a different names are also detected.
	///			 **Please note** that a json file in UTF-8 **with BOM** won't be correctly parsed!!
    diff_t CompareFolders(const json_t left, const json_t right);

    /// @brief Compares the content of two JSON files
    /// @param folder The folder path
    /// @param json The JSON file
    /// @param logErrors A logger to catch minor errors that could happen. By default, the NULL logger will ignore them.
    /// @details Returns the differences between the content described by the JSON files.
    ///          Identical files but with a different names are also detected.
	///			 **Please note** that a json file in UTF-8 **with BOM** won't be correctly parsed!!
    diff_t CompareFolders(const std::string& folder, const json_t json, ILogError& logErrors = SLogErrorNull::GetInstance());

    /// @brief Produces a JSON wstring with the difference between two folders
    /// @param diff Difference between two folders
    std::wstring Json(const diff_t diff);

    /// @brief Analyzes the content of a folder and returns a JSON string
    /// @param path          Path of the folder to be analyzed
    /// @param method        Algorithm used to collect info about the files
    /// @param logErrors     Error logger
    std::wstring ScanFolder(const std::string& path, const eHashingAlgorithm algo,  ILogError& logErrors = SLogErrorNull::GetInstance());

	/// @brief 	Creates a new file containing an UTF-8 representation of the provided wstring
	/// @details The resulting file will be UTF-8 which *may* be headed by a **BOM**
	/// @param stream 	Stream handling the file to create
	/// @param str 		wide string to encode in UTF-8 then write to disk
	/// @param withBOM 	Is writing a BOM header to the UTF-8 file required? (default: false)
	void WriteWString(std::ofstream& stream, const std::wstring& str, const bool withBOM = false);
}

#endif
