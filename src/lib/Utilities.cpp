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



using namespace std;
namespace pt = boost::property_tree;



namespace cf
{
    string Json(const diff_t diff)
    {
        pt::ptree root;
        root.put(JSON_KEYS.GENERATOR, JSON_CONST_VALUES.GENERATOR);

        // identical files
        pt::ptree node_identical;
        for (const auto& entry : diff.identical) {
            pt::ptree node_entry;
            node_entry.put("", entry);
            node_identical.push_back(make_pair("", node_entry));
        }
        root.add_child(JSON_KEYS.DIFF.IDENTICAL, node_identical);

        // different files
        pt::ptree node_different;
        for (const auto& entry : diff.different) {
            pt::ptree node_entry;
            node_entry.put("", entry);
            node_different.push_back(make_pair("", node_entry));
        }
        root.add_child(JSON_KEYS.DIFF.DIFFERENT, node_different);

        // files unique to the left
        pt::ptree node_unique_left;
        for (const auto& entry : diff.unique_left) {
            pt::ptree node_entry;
            node_entry.put("", entry);
            node_unique_left.push_back(make_pair("", node_entry));
        }
        root.add_child(JSON_KEYS.DIFF.UNIQUE_LEFT, node_unique_left);

        // files unique to the right
        pt::ptree node_unique_right;
        for (const auto& entry : diff.unique_right) {
            pt::ptree node_entry;
            node_entry.put("", entry);
            node_unique_right.push_back(make_pair("", node_entry));
        }
        root.add_child(JSON_KEYS.DIFF.UNIQUE_RIGHT, node_unique_right);

        // renamed and duplicates
        pt::ptree renamed;
        for (const auto& entry : diff.renamed)
        {
            pt::ptree left;
            for (const auto& entry_left : entry.left) {
                pt::ptree node_entry;
                node_entry.put("", entry_left);
                left.push_back(make_pair("", node_entry));
            }
            renamed.push_back(make_pair(JSON_KEYS.DIFF.LEFT, left));

            pt::ptree right;
            for (const auto& entry_right : entry.right) {
                pt::ptree node_entry;
                node_entry.put("", entry_right);
                right.push_back(make_pair("", node_entry));
            }
            renamed.push_back(make_pair(JSON_KEYS.DIFF.RIGHT, right));
        }
        root.add_child(JSON_KEYS.DIFF.RENAMED, renamed);

        // Get the string and returns
        stringstream stream{ ios_base::out };
        pt::write_json(stream, root);
        return stream.str();
    }
}