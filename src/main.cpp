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



#include <iostream>
#include <list>
#include <string>

#include "CompareFolders.hpp"


using namespace std;



int main(int argc, char* argv[])
{
	// Sanity
	if (argc != 3) {
        cout << "Usage: " << argv[0] << " [DIRECTORY_1] [DIRECTORY_2]" << endl;
		return -1;
	}

    // Parse args
    const string path_folder_1{ argv[1] };
    const string path_folder_2{ argv[2] };

    cout << "\nCOMPARING\n" << '\"' << path_folder_1 << "\"\nand\n\"" << path_folder_2 << "\"\n" << endl;
    
    try {
        
        const auto diff = cf::CompareFolders(path_folder_1, path_folder_2);

        // ++++++++++ display result
        cout << diff.identical.size() << " files are identical:\n\n";
        for(const auto& file : diff.identical) {
            cout << '\"' << file << "\"\n";
        }
        cout << "\n====\n\n";
        
        cout << diff.different.size() << " files are different:\n\n";
        for(const auto& file : diff.different) {
            cout << '\"' << file << "\"\n";
        }
        cout << "\n====\n\n";
        
        cout << diff.unique_left.size() << " files are unique to \"" << diff.root_left << "\":\n\n";
        for(const auto& file : diff.unique_left) {
            cout << '\"' << file << "\"\n";
        }
        cout << "\n====\n\n";
        
        cout << diff.unique_right.size() << " files are unique to \"" << diff.root_right << "\":\n\n";
        for(const auto& file : diff.unique_right) {
            cout << '\"' << file << "\"\n";;
        }
        cout << "\n====\n\n";
        
        cout << diff.renamed.size() << " files with identical content but different name/location:\n";
        for(const auto& files : diff.renamed) {
            cout << "\n---\n";
            for(const auto renamed : files.left) {
                cout << '\"' << diff.root_left << renamed << "\"\n";
            }
            for(const auto renamed : files.right) {
                cout << " -> \t\"" << diff.root_right  << renamed << "\"\n";
            }
        }
        cout << endl;
        // ---------- display

    }
    catch (const exception& e)
    {
        cout << "Error: " << e.what();
    }

	return 0;
}
