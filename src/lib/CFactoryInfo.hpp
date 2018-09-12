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

#ifndef _SRC_CFactoryInfo_hpp__
#define _SRC_CFactoryInfo_hpp__

#include <memory>
#include <future>
#include <thread>
#include <algorithm>
#include <vector>
#include <boost/filesystem.hpp>


namespace fs = boost::filesystem;

namespace  cf {

    class CCollectionInfo;
    class ILogError;
    
    /// @brief Abstract class, root of the FactoryInfo class hierarchy
    class AFactoryInfo
    {
    public:
        virtual ~AFactoryInfo() = default;
        AFactoryInfo(const AFactoryInfo&) = delete;
        void operator=(const AFactoryInfo&) = delete;

        /// @brief This **static** operation builds a collection from the values stored in a JSON file
        /// @param json_path Pth to the JSON file storing the hashes
        static CCollectionInfo ReadInfo(const fs::path& json_path);

        /// @brief Builds a collection with all the directory's files' hashes
        /// @param root Root folder: all its files will be hashed
        /// @param loggerErr Will log eventual errors
        virtual CCollectionInfo collectInfo(const fs::path& root, ILogError& loggerErr) const = 0;
    
    protected:
        AFactoryInfo() = default;

        /// @brief Lists and returns all **files** entries located inside the provided directory
        const std::list<fs::path> listFiles(const fs::path&) const;
    };

    
    /// @brief      *Seculrely* collects info about files.
    /// @detailed   All hashes are produced by a real hashing function.
    class CFactoryInfoSecure : public AFactoryInfo
    {
    public:
        explicit CFactoryInfoSecure() :
            _nbThreads{std::max(1u, std::thread::hardware_concurrency())}
        {   }
        ~CFactoryInfoSecure() = default;
    
        /// @brief Builds a collection with all the directory's files' hashes
        /// @param root Root folder: all its files will be hashed
        /// @param loggerErr Will log eventual errors
        CCollectionInfo collectInfo(const fs::path& root, ILogError& loggerErr) const override;

    private:
        /// @brief Splits the paths in vectors (*slices*)
        std::vector<std::list<fs::path>> splitPaths(const std::list<fs::path>& paths, const unsigned nb_slices) const;
        const unsigned _nbThreads;
    };

    /// @brief      *Quickly* collects info about files.
    /// @detailed   Hashes are computed using **modification time** and **size**. 
    ///             If *duplicates* are found, **a real secure hash is computed** to confirm.
    class CFactoryInfoFast : public AFactoryInfo
    {
    public:
        explicit CFactoryInfoFast() = default;
        ~CFactoryInfoFast() = default;

        /// @brief Builds a collection with all the directory's files' hashes
        /// @param root Root folder: all its files will be hashed
        /// @param loggerErr Will log eventual errors
        CCollectionInfo collectInfo(const fs::path& root, ILogError& loggerErr) const override;

    private:
        /// @brief Computes and returns the *fast hash* from the info provided
        std::string hasherFast(const std::time_t time_modified, const std::uintmax_t size) const;
    };
    
}


#endif /* _SRC_CFactoryInfo_hpp__ */
