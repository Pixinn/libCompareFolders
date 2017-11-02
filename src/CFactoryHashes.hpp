
//
//  CFactoryHashes.hpp
//  compare_folder
//
//  Created by Christophe Meneboeuf on 24/10/2017.
//
//

#ifndef _SRC_CFactoryHashes_hpp__
#define _SRC_CFactoryHashes_hpp__

#include <memory>
#include <boost/filesystem.hpp>

#include "CCollectionHash.hpp"

namespace fs = boost::filesystem;

namespace  cf {
    
    /// @brief Produces hashes
    class CFactoryHashes
    {
    public:
        CFactoryHashes() = default;
        ~CFactoryHashes() = default;
    
        /// @brief Builds a collection with all the directory's files' hashes
        /// @param root Root folder: all its files will be hashed
        CCollectionHash ComputeHashes(const fs::path& root) const;
    
    private:
        /// @brief Lists and returns all **files** entries located inside the provided directory
        const std::list<fs::path> listFiles(const fs::path&) const;
    };
    
}


#endif /* _SRC_CFactoryHashes_hpp__ */
