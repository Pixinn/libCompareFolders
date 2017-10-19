//
//  CFactoryHashes.cpp
//  compare_folder
//
//  Created by Christophe Meneboeuf on 28/10/2017.
//
//

#include <list>
#include <boost/filesystem.hpp>
#include <cryptopp/sha.h>
#include <cryptopp/hex.h>
#include <cryptopp/files.h>

#include "CFactoryHashes.hpp"

using namespace std;

namespace cf {
    
    
    unique_ptr<CCollectionHash> CFactoryHashes::ComputeHashes(const fs::path& root) const
    {
        const auto paths = listFiles(root);
        
        auto hashes = make_unique<CCollectionHash>();
        // TODO Some multithreading! Blocking until all jobs are done.
        for(const auto& path : paths)
        {
            constexpr bool isUpperCase = true;
            string hash;
            CryptoPP::SHA1 hasher;
            CryptoPP::FileSource(path.c_str(), true, \
                                 new CryptoPP::HashFilter(hasher, new CryptoPP::HexEncoder(new CryptoPP::StringSink(hash), isUpperCase))
                                 );
            const auto path_relative = fs::relative(path, root);
            hashes->setHash(path_relative, hash);
        }
        return hashes;
    }
    
    
    /// @brief Lists and returns all **file** entries located inside the provided directory
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
