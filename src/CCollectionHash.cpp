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

#include <sstream>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "CCollectionHash.hpp"


using namespace std;
namespace pt = boost::property_tree;



namespace  cf
{
    
    ///////////////////////

    void CCollectionHash::setHash(const boost::filesystem::path& path, const string& hash)
    {        
        // Is it the first time a hash is provided for this file?
        {
            const auto idx =_file_hashes.find(path);
            if(idx == _file_hashes.end()) {
                _file_hashes.emplace(make_pair(path, hash));
            }
            else {
                idx->second = hash;
            }
        }
        
        // Is it first file with this hash?
        {
            const auto idx = _hash_files.find(hash);
            if(idx == _hash_files.end()) {
                _hash_files.emplace(make_pair(hash, list<fs::path>{path}));
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
        const auto idx_file = find(files_with_hash.begin(), files_with_hash.end(), file);
        assert(idx_file != files_with_hash.end());
        files_with_hash.erase(idx_file);
        if(files_with_hash.empty()) { // No more files with the same hash: erasing the entry
            _hash_files.erase(hash_files);
        }
    }
    
    
    
    ///////////////////////
    
    diff_t CCollectionHash::compare(CCollectionHash rhs) const
    {
        // Find identical, different and unique_left
        list<string> identical;
        list<string> different;
        list<string> unique_left;
        for(const auto& file_hash : _file_hashes)
        {
            const auto& file_hash_right = rhs._file_hashes.find(file_hash.first);
            if(file_hash_right != rhs._file_hashes.end()) // file with the same relative path
            {
                if(file_hash_right->second == file_hash.second) { // identical
                    identical.push_back(file_hash.first.string());
                }
                else { // different
                    different.push_back(file_hash.first.string());
                }
                rhs.removePath(file_hash.first);
            }
            else    // no file with the same relative path
            {       // any other filename with the same content (hash)??
                const auto it_hash = rhs._hash_files.find(file_hash.second);
                if(it_hash == rhs._hash_files.end()) { // file is unique to the left
                     unique_left.push_back(file_hash.first.string());
                }
            }
        }
        
        // Find unique_right
        list<string> unique_right;
        for(const auto& file_hash : rhs._file_hashes)
        {
            if(_file_hashes.find(file_hash.first) == _file_hashes.end() &&   // left has no file with the same relative path
               _hash_files.find(file_hash.second) == _hash_files.end())      // nor the same content (hash)
            {
                unique_right.push_back(file_hash.first.string());
            }
        }
        for(const auto& file : unique_right) {
            rhs.removePath(file);
        }
        
        // Find renamed files with the same content
        list<diff_t::renamed_t> list_renamed;
        for(const auto& hash_files : rhs._hash_files)
        {
            const auto& hash = hash_files.first;
            const auto& files_right = hash_files.second;
            const auto hash_files_left = _hash_files.find(hash);
            assert(hash_files_left != _hash_files.end());
            diff_t::renamed_t renamed;
            for(const auto& file : hash_files_left->second) {
                renamed.left.push_back(file.string());
            }
            for(const auto& file : files_right) {
                renamed.right.push_back(file.string());
            }
            list_renamed.push_back(renamed);
        }
        
        
        return diff_t{
            _root.string(),
            rhs._root.string(),
            identical,
            different,
            unique_left,
            unique_right,
            list_renamed
        };
    }


    string CCollectionHash::json() const
    {
        pt::ptree root;
        root.put("Generator", "info.xtof.COMPARE_FOLDERS");

        pt::ptree node_hashes;
        for (const auto& entry : _file_hashes) {
            pt::ptree node_entry;
            node_entry.put(entry.first.string(), entry.second);
            node_hashes.push_back(make_pair("", node_entry));
        }
        root.add_child("hashes", node_hashes);

        // Get the string and returns
        stringstream stream{ ios_base::out };
        pt::write_json(stream, root);
        return stream.str();
    }
    
    
    ///////////////////////
    
    string CCollectionHash::toString() const
    {
        string str;
        for(auto it = _file_hashes.begin(); it != _file_hashes.end(); ++it) {
            str += it->first.string() + " : " + it->second + "\n";
        }
        return str;
    }

}
