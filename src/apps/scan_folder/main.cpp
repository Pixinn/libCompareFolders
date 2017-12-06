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

/// @Brief custom error "live" logger
class LogError : public cf::ILogError
{
public:
    void log(const string& err) override {
        cerr << "ARRRRG! " << err << endl;
    }
};





/// @brief Compares the content of two folders and displays the result as JSON
int main(int argc, char* argv[])
{
	// Sanity
	if (argc != 2) {
        cout << "Usage: " << argv[0] << " [DIRECTORY]" << endl;
		return -1;
	}

    // Parse args
    const string path_folder{ argv[1] };

    cout << "\nSCANNING \"" << path_folder << '\"' << endl;
    
    try {
        
        LogError logErr;
        cout << cf::ScanFolder(path_folder, logErr);

    }
    catch (const exception& e)
    {
        cout << e.what() << endl;
    }

	return 0;
}
