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

#include "CompareFolders.hpp"
#include "CCollectionHash.hpp"


using namespace std;
namespace pt = boost::property_tree;



namespace  cf
{
    
    ///////////////////////

    void CCollectionHash::setHash(const fs::path& path, const info_t& info)
    {        
        // Is it the first time a hash is provided for this file?
        {
            const auto idx =_file_infos.find(path);
            if(idx == _file_infos.end()) {
                _file_infos.emplace(make_pair(path, info));
            }
            else {
                idx->second = info;
            }
        }
        
        // Is it first file with this hash?
        {
            const auto idx = _hash_files.find(info.hash);
            if(idx == _hash_files.end()) {
                _hash_files.emplace(make_pair(info.hash, list<fs::path>{path}));
            }
            else {
                idx->second.push_back(path);
            }
        }
    }
    
    
    
    ///////////////////////
    
    void CCollectionHash::removePath(const fs::path& path)
    {
        const auto file_info = _file_infos.find(path);
        if(file_info == _file_infos.end()) { // not found
            return;
        }
        
        // Removing the entry in the collection of files
        const auto file = file_info->first;
        const auto info = file_info->second;
        _file_infos.erase(file_info);
        
        // Removing the file from the files with this hash
        const auto hash_files = _hash_files.find(info.hash);
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

        list<string> identical;
        list<string> different;
        list<string> unique_left;
        list<string> unique_right;
        list<diff_t::renamed_t> list_renamed;

        for (const auto& file_info : _file_infos)
        {
            const auto& file_hash_right = rhs._file_infos.find(file_info.first);
            if (file_hash_right != rhs._file_infos.end()) // file with the same relative path
            {
                if (file_hash_right->second == file_info.second) { // identical
                    identical.push_back(file_info.first.string());
                }
                else { // different
                    different.push_back(file_info.first.string());
                }
            }
            else
            {
                // Search in the right hashes if files with same content are found
                const auto& hash = file_info.second.hash;
                const auto hash_files_right = rhs._hash_files.find(hash);
                if (hash_files_right == std::end(rhs._hash_files)) {
                    unique_left.push_back(file_info.first.string()); // Nope: it's unique to the left
                }
                else
                {  // Found some match: same file with a different relative path / filename
                    diff_t::renamed_t renamed;
                    renamed.hash = hash;
                    const auto& hash_files_left = _hash_files.find(hash);
                    for (const auto& file : hash_files_left->second) {
                        renamed.left.push_back(file.string());
                    }
                    for (const auto& file : hash_files_right->second) {
                        renamed.right.push_back(file.string());
                    }
                    list_renamed.push_back(renamed);
                }
            }
        }

        for (const auto& file_info : rhs._file_infos)
        {
            const auto& path = file_info.first.string();
            //Is the file in the identical or different lists?
            const auto isIdentical = (find(begin(identical), end(identical), path) != end(identical));
            const auto isDifferent = (find(begin(different), end(different), path) != end(different));
            if (!isIdentical && !isDifferent) // Nope
            { 
                // Has this file a twin with the same content here?
                const auto& hash = file_info.second.hash;
                const auto hasSomeTwins = (_hash_files.find(hash) != end(_hash_files));
                if (!hasSomeTwins) {
                    unique_right.push_back(path);
                }
            }
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
        root.put(JSON_KEYS.GENERATOR, JSON_CONST_VALUES.GENERATOR);
        root.put(JSON_KEYS.ROOT, _root.string());
        pt::ptree node_files;
        for (const auto& entry : _file_infos) {
            pt::ptree node_info;
            node_info.put(JSON_KEYS.CONTENT.HASH, entry.second.hash);
            node_info.put(JSON_KEYS.CONTENT.TIME, entry.second.time_modified);
            node_files.push_back(pt::ptree::value_type(entry.first.string(), node_info)); // not using "put()" as '.' is its delimiter
        }
        root.add_child(JSON_KEYS.CONTENT.FILES, node_files);

        // Get the string and returns
        stringstream stream{ ios_base::out };
        pt::write_json(stream, root);
        return stream.str();
    }
    
    

}
