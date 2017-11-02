
//
//  CCollectionHash.hpp
//  compare_folder
//
//  Created by Christophe Meneboeuf on 24/10/2017.
//
//

#ifndef _SRC_CCollectionHash_hpp__
#define _SRC_CCollectionHash_hpp__

#include <map>
#include <string>
#include <mutex>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace  cf {
    
    /// @brief This is a collection of hashes
    /// @details Internally it is built on two symetrical maps that can give the hash of a file or
    /// the files producing a given hash (useful if some files are duplicated).
    /// All operations are **not** thread safe!
    class CCollectionHash
    {
    public:
        /// @brief Files with different path but the very same content.
        /// Those files can have **duplicates** both left and right
        typedef struct renamed_t {
            std::list<fs::path> left;  ///< Left files with the same content
            std::list<fs::path> right; ///< Right files with the same content
        }renamed_t;
        
        /// @brief Hosts the differences between the *left* and the *right* directories
        typedef struct diff_t {
            const fs::path root_left;               ///< Left directory root path
            const fs::path root_right;              ///< Right directory root path
            const std::list<fs::path> identical;    ///< Identical files
            const std::list<fs::path> different;    ///< Different files
            const std::list<fs::path> unique_left;  ///< Files that are unique to the left directory
            const std::list<fs::path> unique_right; ///< Files that are unique to the right directory
            const std::list<renamed_t> renamed;     ///< Files with different reative path that have the same content
        } diff_t;
        
        /// @brief Constructor
        /// @param root Root folder containing the hashed files
        CCollectionHash(const fs::path& root) :
            _root{ root }
        {   }
        ~CCollectionHash() = default;
        
        /// @brief Adds a hash corresponding to a given path
        void setHash(const fs::path& path, const std::string& hash);
        
        /// @brief Removes the path from the collection
        void removePath(const fs::path& path);
        
        /// @brief Compares the collection to another one.
        diff_t compare(CCollectionHash rhs) const;
        
        /// @brief Converts the stored paths and their hashes to a string
        std::string toString() const;
        
    private:
        std::map<fs::path, std::string> _file_hashes;           ///< File pathes and their corresponding hash
        std::map<std::string, std::list<fs::path>> _hash_files; ///< Hash with the corresponding files. Useful for duplicate files.
        const fs::path _root;                                   ///< Root folder containing all the files hashed 
    };
    
}


#endif /* _SRC_CCollectionHash_hpp__ */
