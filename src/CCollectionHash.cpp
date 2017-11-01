
#include "CCollectionHash.hpp"


namespace  cf
{
    
    std::mutex CCollectionHash::_Mutex;
    
    
    ///////////////////////

    void CCollectionHash::setHash(const boost::filesystem::path& path, const std::string& hash)
    {
        std::lock_guard<std::mutex> lock{_Mutex};
        
        // Is it the first time a hash is provided for this file?
        {
            const auto idx =_file_hashes.find(path);
            if(idx == _file_hashes.end()) {
                _file_hashes.emplace(std::make_pair(path, hash));
            }
            else {
                idx->second = hash;
            }
        }
        
        // Is it first file with this hash?
        {
            const auto idx = _hash_files.find(hash);
            if(idx == _hash_files.end()) {
                _hash_files.emplace(std::make_pair(hash, std::list<fs::path>{path}));
            }
            else {
                idx->second.push_back(path);
            }
        }
    }
    
    
    
    ///////////////////////
    
    void CCollectionHash::removePath(const fs::path& path)
    {
        const auto file_hash = _file_hashes.find(path);
        if(file_hash == _file_hashes.end()) { // not found
            return;
        }
        
        // Removing the entry in the collection of files
        const auto file = file_hash->first;
        const auto hash = file_hash->second;
        _file_hashes.erase(file_hash);
        
        // Removing the file from the files with this hash
        const auto hash_files = _hash_files.find(hash);
        assert(hash_files != _hash_files.end());
        auto& files_with_hash = hash_files->second;
        const auto idx_file = std::find(files_with_hash.begin(), files_with_hash.end(), file);
        assert(idx_file != files_with_hash.end());
        files_with_hash.erase(idx_file);
        if(files_with_hash.empty()) { // No more files with the same hash: erasing the entry
            _hash_files.erase(hash_files);
        }
    }
    
    
    
    ///////////////////////
    
    CCollectionHash::diff_t CCollectionHash::compare(CCollectionHash rhs) const
    {
        // Find identical, different and unique_left
        std::list<fs::path> identical;
        std::list<fs::path> different;
        std::list<fs::path> unique_left;
        for(const auto& file_hash : _file_hashes)
        {
            const auto& file_hash_right = rhs._file_hashes.find(file_hash.first);
            if(file_hash_right != rhs._file_hashes.end()) // file with the same relative path
            {
                if(file_hash_right->second == file_hash.second) { // identical
                    identical.push_back(file_hash.first);
                }
                else { // different
                    different.push_back(file_hash.first);
                }
                rhs.removePath(file_hash.first);
            }
            else    // no file with the same relative path
            {       // any other filename with the same content (hash)??
                const auto it_hash = rhs._hash_files.find(file_hash.second);
                if(it_hash == rhs._hash_files.end()) { // file is unique to the left
                     unique_left.push_back(file_hash.first);
                }
            }
        }
        
        // Find unique_right
        std::list<fs::path> unique_right;
        for(const auto& file_hash : rhs._file_hashes)
        {
            if(_file_hashes.find(file_hash.first) == _file_hashes.end() &&   // left has no file with the same relative path
               _hash_files.find(file_hash.second) == _hash_files.end())      // nor the same content (hash)
            {
                unique_right.push_back(file_hash.first);
            }
        }
        for(const auto& file : unique_right) {
            rhs.removePath(file);
        }
        
        // Find renamed files with the same content
        std::list<renamed_t> list_renamed;
        for(const auto& hash_files : rhs._hash_files)
        {
            const auto& hash = hash_files.first;
            const auto& files_right = hash_files.second;
            const auto hash_files_left = _hash_files.find(hash);
            assert(hash_files_left != _hash_files.end());
            renamed_t renamed;
            for(const auto& file : hash_files_left->second) {
                renamed.left.push_back(file);
            }
            for(const auto& file : files_right) {
                renamed.right.push_back(file);
            }
            list_renamed.push_back(renamed);
        }
        
        
        return diff_t{
            _root,
            rhs._root,
            identical,
            different,
            unique_left,
            unique_right,
            list_renamed
        };
    }
    
    
    
    ///////////////////////
    
    std::string CCollectionHash::toString() const
    {
        std::string str;
        std::lock_guard<std::mutex> lock{_Mutex};
        for(auto it = _file_hashes.begin(); it != _file_hashes.end(); ++it) {
            str += it->first.string() + " : " + it->second + "\n";
        }
        return str;
    }

}
