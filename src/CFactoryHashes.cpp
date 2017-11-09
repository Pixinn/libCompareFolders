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

#include <list>
#include <vector>
#include <future>

#include <boost/filesystem.hpp>
#include <cryptopp/sha.h>
#include <cryptopp/hex.h>
#include <cryptopp/files.h>

#include "CFactoryHashes.hpp"

using namespace std;

namespace cf {
    
    ////////////////////////
    
    CCollectionHash CFactoryHashes::ComputeHashes(const fs::path& root) const
    {
        CCollectionHash hashes{ root };
        mutex mtex;

        const auto paths = listFiles(root);
        
        {
            vector<future<void>> jobs_scheduled;
            jobs_scheduled.reserve(paths.size());

            for (const auto& path : paths)
            {
                jobs_scheduled.push_back(async([&hashes, &root, &mtex, path]
                {
                    constexpr bool isUpperCase = true;
                    string hash;
                    CryptoPP::SHA1 hasher;
                    CryptoPP::FileSource(path.c_str(), true, \
                        new CryptoPP::HashFilter(hasher, new CryptoPP::HexEncoder(new CryptoPP::StringSink(hash), isUpperCase))
                    );
                    const auto path_relative = fs::relative(path, root);
                    
                    {
                        std::lock_guard<std::mutex> lock{ mtex };
                        hashes.setHash(path_relative, hash);
                    }
                }));
            }

        } // Calling the destructor of jobs_scheduled will wait for all the jobs to be executed
        
        return hashes;
    }
    

    ///////////////////////

    const std::list<fs::path> CFactoryHashes::listFiles(const fs::path& dir) const
    {
        // Collect the files list
        std::list<fs::path> paths;
        try
        {
            for (const auto& entry : fs::recursive_directory_iterator(dir)) {
                if(fs::is_regular_file(entry.path())) {
                    paths.push_back(entry.path());
                }
            }
        }
        catch (const std::runtime_error& e) {
            std::cout << "Error: " << e.what();
        }
        return paths;
    }
    
    
}
