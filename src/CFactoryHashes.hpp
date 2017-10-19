
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
    
    class CFactoryHashes
    {
    public:
        CFactoryHashes(){}
        ~CFactoryHashes(){}
    
        /// @brief Builds a collection with all the directory's files' hashes
        std::unique_ptr<CCollectionHash> ComputeHashes(const fs::path& root) const;
    
    private:
        /// @brief Lists and returns all **file** entries located inside the provided directory
        const std::list<fs::path> listFiles(const fs::path&) const;
        
        fs::path _root;
    };
    
}


#endif /* _SRC_CFactoryHashes_hpp__ */
