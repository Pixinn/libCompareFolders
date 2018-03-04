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
#include <future>

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

    CCollectionInfo AFactoryInfo::ReadInfo(const fs::path& json_path)
    {
        // Prepare the wide string stream
        if (!fs::is_regular_file(json_path)) {
            throw ExceptionFatal{ json_path.string() + " is not a file." };
        }
        wifstream streamFile{ json_path.string(), ios_base::in };
        streamFile.imbue(locale(locale{}, new codecvt_utf8<wchar_t>));
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
        const auto algo_hash = root.get<wstring>(JSON_KEYS.ALGO_HASH);
        CCollectionInfo collection{ fs::path{ root.get<wstring>(JSON_KEYS.ROOT) },
                                    algo_hash == JSON_CONST_VALUES.ALGO_HASH_FAST ? eCollectingAlgorithm::FAST  : eCollectingAlgorithm::SECURE };

        try {
            for (const auto& file : root.get_child(JSON_KEYS.CONTENT.FILES)) {
                const auto hash = file.second.get_child(JSON_KEYS.CONTENT.HASH).data();
                const time_t time = std::stoll(file.second.get_child(JSON_KEYS.CONTENT.TIME).data());
                const auto size = std::stoull(file.second.get_child(JSON_KEYS.CONTENT.SIZE).data());
                collection.setInfo(fs::path{ file.first }, { codec_utf8.to_bytes(hash) , time, size });
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
    CCollectionInfo CFactoryInfoSecure::collectInfo(const fs::path& root, ILogError&) const
    {
        // split work for tasks
        const auto paths_files = listFiles(root);
        const auto works = splitPaths(paths_files, _nbThreads);
            
        // construct the tasks and launch threaded workers
        typedef struct resultWork_t {
            fs::path path_relative;
            CCollectionInfo::info_t info;
        } resultWork_t; ///< structure containing a file's info collected

        vector<thread> workers;
        vector<future<list<resultWork_t>>> future_results;
        for(const auto& paths : works)
        {
            packaged_task<list<resultWork_t>(const fs::path&, const list<fs::path>&)> task{
                [] (const fs::path& root, const list<fs::path>& paths)
                {
                    list<resultWork_t> results;
                    for(const auto& path : paths)
                    {
                        constexpr bool isUpperCase = true;
                        string hash;
                        CryptoPP::SHA1 hasher;
                        CryptoPP::FileSource(path.c_str(), true,
                            new CryptoPP::HashFilter(hasher, new CryptoPP::HexEncoder(new CryptoPP::StringSink(hash), isUpperCase))
                        );
                        results.emplace_back<resultWork_t>( {
                            fs::relative(path, root),
                            { hash, fs::last_write_time(path), fs::file_size(path) }
                        });
                    }
                    return results;
                } // lambda
            }; // packaged_task

            future_results.emplace_back(task.get_future()); // storing the future for the result
            workers.emplace_back(std::move(task), root, paths); // creating (and starting) the thread with the task
        }

        // Detaching the threads avoids terminate() in case of an exception
        for( auto& worker : workers ) {
            worker.detach();
        }

        // Collect the result
        CCollectionInfo info{ root, eCollectingAlgorithm::SECURE };
        for(auto& future_result : future_results) {
            // TODO try catch
            auto results = future_result.get();
            for(const auto& result : results) {
                info.setInfo(result.path_relative, result.info);
            }
        }

        return info;
    }


     vector<list<fs::path>> CFactoryInfoSecure::splitPaths(const list<fs::path>& paths, const unsigned nb_slices) const
     {
        vector<list<fs::path>> splitted;
        for(auto i = 0u; i < nb_slices; ++i) {
            splitted.push_back(list<fs::path>{});
        }

        auto slice = begin(splitted);
        for(const auto& path : paths)
        {
            slice->push_back(path);
            ++slice;
            if(slice == end(splitted)) {
                slice = begin(splitted);
            }
        }
         
        return splitted;
     }


    /// @detailed   All *pseudo-hashes* are computed by combining the size of the file and its last modification time. 
    ///             In a second pass, a real cryptographic hash is computed on *duplicates* that may arise because of the **weak** *pseudo-hashes* computed first.
    ///             Thus, the time consuming *secure* hash is only computed for those duplicates.
    ///             Collecting info can take some time. Thus, an external error logger must be provided
    ///             to give the opportunity to report the errors in real time.
    CCollectionInfo CFactoryInfoFast::collectInfo(const fs::path& root, ILogError& logger) const
    {
       
        CCollectionInfo collection_info{ root,  eCollectingAlgorithm::FAST};
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
