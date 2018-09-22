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

#include <memory>

#include <boost/filesystem.hpp>

#include "CCollectionInfo.hpp"
#include "CFactoryInfo.hpp"
#include "CLoggerConcurrent.hpp"

#include "CompareFolders.hpp"

namespace fs = boost::filesystem;
using namespace std;
using namespace cf;


// ==== EXCEPTIONS


ExceptionFatal::ExceptionFatal(const std::string& msg) :
    runtime_error{ "Fatal error: " + msg }
{   }

Exception::Exception(const std::string& msg) :
    runtime_error{ "Error: " + msg }
{   }


// ===== PRIVATE FACILITIES


bool cf::diff_t::operator==(const cf::diff_t& rhs) const noexcept
{
    if (root_left != rhs.root_left || root_right != rhs.root_right) {
        return false;
    }

    if (identical.size() != rhs.identical.size()) {
        return false;
    }
    for (const auto& entry : identical) {
        if (find(begin(rhs.identical), end(rhs.identical), entry) == end(rhs.identical)) {
            return false;
        }
    }

    if (different.size() != rhs.different.size()) {
        return false;
    }
    for (const auto& entry : different) {
        if (find(begin(rhs.different), end(rhs.different), entry) == end(rhs.different)) {
            return false;
        }
    }

    if (unique_left.size() != rhs.unique_left.size()) {
        return false;
    }
    for (const auto& entry : unique_left) {
        if (find(begin(rhs.unique_left), end(rhs.unique_left), entry) == end(rhs.unique_left)) {
            return false;
        }
    }

    if (unique_right.size() != rhs.unique_right.size()) {
        return false;
    }
    for (const auto& entry : unique_right) {
        if (find(begin(rhs.unique_right), end(rhs.unique_right), entry) == end(rhs.unique_right)) {
            return false;
        }
    }

    return true;
}


bool cf::diff_t::renamed_t::operator==(const cf::diff_t::renamed_t& rhs) const noexcept
{
    if (hash != rhs.hash || left.size() != rhs.left.size() || right.size() != rhs.right.size()) {
        return false;
    }
    for (const auto& entry : left) {
        if (find(begin(rhs.left), end(rhs.left), entry) == end(rhs.left)) {
            return false;
        }
    }
    for (const auto& entry : right) {
        if (find(begin(rhs.right), end(rhs.right), entry) == end(rhs.right)) {
            return false;
        }
    }
    return true;
}

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


diff_t cf::CompareFolders(const std::string& root_left, const std::string& root_right, const eHashingAlgorithm algo, unique_ptr<ILogError> logger)
{
    const auto path_folder_1 = path_folder(root_left);
    const auto path_folder_2 = path_folder(root_right);

    // Compute the hashes
    unique_ptr<AFactoryInfo> factoryInfo = (algo == eCollectingAlgorithm::SECURE) ?
        unique_ptr<AFactoryInfo>(dynamic_cast<AFactoryInfo*>(new CFactoryInfoSecure{ std::move(logger) })) :
        unique_ptr<AFactoryInfo>(dynamic_cast<AFactoryInfo*>(new CFactoryInfoFast{ std::move(logger) }));

    const auto infoDir1 = factoryInfo->collectInfo(path_folder_1);
    const auto infoDir2 = factoryInfo->collectInfo(path_folder_2);
        
    const auto diff = infoDir1.compare(infoDir2);

    return diff;
}


diff_t cf::CompareFolders(const json_t left, const json_t right)
{
    const auto infoDir1 = AFactoryInfo::ReadInfo(left.path);
    const auto infoDir2 = AFactoryInfo::ReadInfo(right.path);

    try {
        const auto diff = infoDir1.compare(infoDir2);
        return diff;
    }
    catch (const Exception& e) {
        throw ExceptionFatal{ e.what() };
    }
   
}



diff_t cf::CompareFolders(const std::string& folder, const json_t json, unique_ptr<ILogError> logger)
{
    const auto path_folder_1 = path_folder(folder);

    const auto infoDir1 = AFactoryInfo::ReadInfo(json.path);
    unique_ptr<AFactoryInfo> factoryInfo = (infoDir1.hasher() == eCollectingAlgorithm::SECURE) ?
        unique_ptr<AFactoryInfo>(dynamic_cast<AFactoryInfo*>(new CFactoryInfoSecure{ std::move(logger) })) :
        unique_ptr<AFactoryInfo>(dynamic_cast<AFactoryInfo*>(new CFactoryInfoFast{ std::move(logger) }));
    const auto infoDir2 = factoryInfo->collectInfo(path_folder_1);
    
    const auto diff = infoDir2.compare(infoDir1);

    return diff;
}


wstring cf::ScanFolder(const string& path, const cf::eCollectingAlgorithm algo, unique_ptr<ILogError> logger)
{
    const auto folder = path_folder(path);
    unique_ptr<AFactoryInfo> factoryInfo = (algo == eCollectingAlgorithm::SECURE) ?
        unique_ptr<AFactoryInfo>(dynamic_cast<AFactoryInfo*>(new CFactoryInfoSecure{ std::move(logger) })) :
        unique_ptr<AFactoryInfo>(dynamic_cast<AFactoryInfo*>(new CFactoryInfoFast{ std::move(logger) }));
    const auto properties = factoryInfo->collectInfo(folder);
    return properties.json();
}
