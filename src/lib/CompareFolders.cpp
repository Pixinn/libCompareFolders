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

#include <boost/filesystem.hpp>

#include "CCollectionHash.hpp"
#include "CFactoryHashes.hpp"

#include "CompareFolders.hpp"

namespace fs = boost::filesystem;
using namespace std;
using namespace cf;


// === STATIC INSTANCES

SLogErrorNull SLogErrorNull::_Instance;

// ==== EXCEPTIONS


ExceptionFatal::ExceptionFatal(const std::string& msg) :
    runtime_error{ "Fatal error: " + msg }
{   }


// ===== PRIVATE FACILITIES

/// @brief Constructs a path object from a sanitized string
inline const fs::path path_folder(const string str_path)
{
    const fs::path path{ str_path };
    // Sanity
    if (!fs::is_directory(path)) {
        throw ExceptionFatal{ path.string() + " is not a valid directory." };
    }
    return path;
}


// ===== PUBLIC FUNCTIONS


diff_t cf::CompareFolders(const std::string& root_left, const std::string root_right, ILogError& logger)
{
    const auto path_folder_1 = path_folder(root_left);
    const auto path_folder_2 = path_folder(root_right);

    // Compute the hashes
    const cf::CFactoryHashes factoryHashes{};

    const auto hashesDir1 = factoryHashes.ComputeHashes(path_folder_1, logger);
    const auto hashesDir2 = factoryHashes.ComputeHashes(path_folder_2, logger);
        
    const auto diff = hashesDir1.compare(hashesDir2);


    return diff;
}


string cf::ScanFolder(const string& path, ILogError& logger)
{
    const auto folder = path_folder(path);
    const cf::CFactoryHashes factoryHashes{};
    const auto properties = factoryHashes.ComputeHashes(folder, logger);
    return properties.json();
}
