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
#include <iostream>
#include <codecvt>
#include <array>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "CompareFolders.hpp"



using namespace std;
namespace pt = boost::property_tree;



namespace cf
{
	/// @details The JSON is coded in wstring in order to handle any special character in the filepaths
    wstring Json(const diff_t diff)
    {
        pt::wptree root;
        root.put(JSON_KEYS.GENERATOR, JSON_CONST_VALUES.GENERATOR);

        // identical files
        if (!diff.identical.empty()) {
            pt::wptree node_identical;
            for (const auto& entry : diff.identical) {
                pt::wptree node_entry;
                node_entry.put(wstring{}, entry);
                node_identical.push_back(make_pair(wstring{}, node_entry));
            }
            root.add_child(JSON_KEYS.DIFF.IDENTICAL, node_identical);
        }

        // different files
        if (!diff.different.empty()) {
            pt::wptree node_different;
            for (const auto& entry : diff.different) {
                pt::wptree node_entry;
                node_entry.put(wstring{}, entry);
                node_different.push_back(make_pair(wstring{}, node_entry));
            }
            root.add_child(JSON_KEYS.DIFF.DIFFERENT, node_different);
        }

        // files unique to the left
        if (!diff.unique_left.empty()) {
            pt::wptree node_unique_left;
            for (const auto& entry : diff.unique_left) {
                pt::wptree node_entry;
                node_entry.put(wstring{}, entry);
                node_unique_left.push_back(make_pair(wstring{}, node_entry));
            }
            root.add_child(JSON_KEYS.DIFF.UNIQUE_LEFT, node_unique_left);
        }

        // files unique to the right
        if (!diff.unique_right.empty()) {
            pt::wptree node_unique_right;
            for (const auto& entry : diff.unique_right) {
                pt::wptree node_entry;
                node_entry.put(wstring{}, entry);
                node_unique_right.push_back(make_pair(wstring{}, node_entry));
            }
            root.add_child(JSON_KEYS.DIFF.UNIQUE_RIGHT, node_unique_right);
        }

        // renamed and duplicates
        if (!diff.renamed.empty()) {
            pt::wptree renamed;
            for (const auto& entry : diff.renamed)
            {
                pt::wptree names;
                pt::wptree left;
                for (const auto& entry_left : entry.left) {
                    pt::wptree node_entry;
                    node_entry.put(wstring{}, entry_left);
                    left.push_back(make_pair(wstring{}, node_entry));
                }
                names.push_back(make_pair(JSON_KEYS.DIFF.LEFT, left));

                pt::wptree right;
                for (const auto& entry_right : entry.right) {
                    pt::wptree node_entry;
                    node_entry.put(wstring{}, entry_right);
                    right.push_back(make_pair(wstring{}, node_entry));
                }
                names.push_back(make_pair(JSON_KEYS.DIFF.RIGHT, right));
                renamed.push_back(make_pair(wstring{begin(entry.hash), end(entry.hash)}, names)); // Conversion ok because, hashes is plains 7-bit ASCII
            }
            root.add_child(JSON_KEYS.DIFF.RENAMED, renamed);
        }

        // Get the string and returns
        wstringstream stream{ ios_base::out };
        pt::write_json(stream, root);
        return stream.str();
    }
	
	
	
	/// --------
	void WriteWString(std::ofstream& stream, const std::wstring& str, const bool withBOM)
	{
		if(withBOM) {
			static const array<uint8_t, 3u> BOM = { 0xEF, 0xBB, 0xBF };
			stream.write(reinterpret_cast<char*>(const_cast<uint8_t*>(BOM.data())), BOM.size());
		}
		wstring_convert<std::codecvt_utf8<wchar_t>> codec_to_utf8;
		stream << codec_to_utf8.to_bytes(str);
	}
}