//
//  CFactoryHashes.cpp
//  compare_folder
//
//  Created by Christophe Meneboeuf on 28/10/2017.
//
//

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
