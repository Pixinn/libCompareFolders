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

#include <ctime>
#include <map>
#include <list>
#include <vector>
#include <locale>
#include <codecvt>
#include <iostream>
#include <sstream>

#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <cryptopp/sha.h>
#include <cryptopp/hex.h>
#include <cryptopp/files.h>

#include "CompareFolders.hpp"
#include "CCollectionInfo.hpp"

#include "CFactoryInfo.hpp"



using namespace std;
namespace pt = boost::property_tree;

namespace cf {

    ///////////////////////

    const std::list<fs::path> AFactoryInfo::listFiles(const fs::path& dir) const
    {
        // Collect the files list
        std::list<fs::path> paths;
        try
        {
            for (const auto& entry : fs::recursive_directory_iterator(dir)) {
                if (fs::is_regular_file(entry.path())) {
                    paths.push_back(entry.path());
                }
            }
        }
        catch (const fs::filesystem_error& e) {
            throw(ExceptionFatal{ e.what() });
        }
        return paths;
    }



    
    ///////////////////////

    CCollectionInfo AFactoryInfo::readHashes(const fs::path& json_path) const
    {
        // Prepare the wide string stream
        if (!fs::is_regular_file(json_path)) {
            throw ExceptionFatal{ json_path.string() + " is not a file." };
        }
        wifstream streamFile{ json_path.string(), ios_base::in };
        streamFile.imbue(locale(locale::empty(), new codecvt_utf8<wchar_t>));
        wstringstream streamStr;
        streamStr << streamFile.rdbuf();
        // Populates the PTree from the stream
        wstring_convert<codecvt_utf8_utf16<wchar_t>> codec_utf8;
        pt::wptree root;
        pt::read_json(streamStr, root);
        const auto generator = root.get<wstring>(JSON_KEYS.GENERATOR);
        if (generator != JSON_CONST_VALUES.GENERATOR) {
            throw ExceptionFatal{ "This is not a proper file." };
        }
        CCollectionInfo collection{ fs::path{ root.get<wstring>(JSON_KEYS.ROOT) } };

        try {
            for (const auto& file : root.get_child(JSON_KEYS.CONTENT.FILES)) {
                const auto hash = file.second.get_child(JSON_KEYS.CONTENT.HASH).data();
                const time_t time = std::stoll(file.second.get_child(JSON_KEYS.CONTENT.TIME).data());
                const auto size = std::stoull(file.second.get_child(JSON_KEYS.CONTENT.SIZE).data());
                collection.setInfo(fs::path{ file.first }, { codec_utf8.to_bytes(hash) , time });
            }
        }
        catch (const pt::ptree_error& e)
        {
            throw ExceptionFatal{ "An error occured while parsing " + json_path.string() + " : " + e.what() };
        }

        return collection;
    }





    ////////////////////////

    /// @detailed   All hashes are computed using a cryptographic hasher. 
    ///             Collecting info can take some time. Thus, an external error logger must be provided
    ///             to give the opportunity to report the errors in real time.
    CCollectionInfo CFactoryInfoSecure::computeHashes(const fs::path& root, ILogError& logger) const
    {
        CCollectionInfo info{ root };

        try
        {
            const auto paths = listFiles(root);
            for (const auto& path : paths)
            {
                constexpr bool isUpperCase = true;
                string hash;
                CryptoPP::SHA1 hasher;
                CryptoPP::FileSource(path.c_str(), true, \
                    new CryptoPP::HashFilter(hasher, new CryptoPP::HexEncoder(new CryptoPP::StringSink(hash), isUpperCase))
                );
                const auto path_relative = fs::relative(path, root);
                const auto time_modified = fs::last_write_time(path);
                const auto size = fs::file_size(path);

                info.setInfo(path_relative, { hash, time_modified, size });
            }
        }
        catch (const CryptoPP::Exception& e) {
            const string message = string{ "Hashing error: " } +e.what();
            logger.log(message);
            throw;  // TODO handle and don't rethrow
        }
        catch (const fs::filesystem_error& e) {
            const string message = string{ "Filesystem error: " } +e.what();
            logger.log(message);
            throw; // TODO handle and don't rethrow
        }

        return info;
    }


    /// @detailed   All *pseudo-hashes* are computed by combining the size of the file and its last modification time. 
    ///             In a second pass, a real cryptographic hash is computed on *duplicates* that may arise because of the **weak** *pseudo-hashes* computed first.
    ///             Thus, the time consuming *secure* hash is only computed for those duplicates.
    ///             Collecting info can take some time. Thus, an external error logger must be provided
    ///             to give the opportunity to report the errors in real time.
    CCollectionInfo CFactoryInfoFast::computeHashes(const fs::path& root, ILogError& logger) const
    {
       
        CCollectionInfo collection_info{ root };
        try
        {
            const auto paths = listFiles(root);
            for (const auto& path : paths)
            {
                const auto path_relative = fs::relative(path, root);
                const auto time_modified = fs::last_write_time(path);
                const auto size = fs::file_size(path);
                const auto hash = hasherFast(time_modified, size);

                collection_info.setInfo(path_relative, { hash, time_modified, size });
            }
        }
        catch (const fs::filesystem_error& e) {
            const string message = string{ "Filesystem error: " } +e.what();
            logger.log(message);
            throw; // TODO handle and don't rethrow
        }

        return collection_info;
    }


    string CFactoryInfoFast::hasherFast(const std::time_t time_modified, const std::uintmax_t size) const
    {
        // Time to string
        //constexpr unsigned MAX_SIZE_TIME = 32u;
        //char str_time[MAX_SIZE_TIME];
        //const auto time = localtime(&time_modified);
        //strftime(str_time, MAX_SIZE_TIME,"", time);
        stringstream stream;
        stream << std::uppercase << std::hex << time_modified << std::uppercase << std::hex << size;
        return stream.str();
    }

}
