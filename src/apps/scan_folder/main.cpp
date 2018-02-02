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


#include <cstdint>
#include <array>
#include <list>
#include <string>
#include <iostream>
#include <fstream>
#include <codecvt>


#include <tclap/CmdLine.h>

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

    // Parsing args
    TCLAP::CmdLine cmd{ "Analyzes the content of a filder and export the result to a JSON file." };
    TCLAP::ValueArg<string> folder("f", "folder", "The folder to be analyzed", true, "", "Folder's path");
    TCLAP::ValueArg<string> output("o", "output", "The JSON file that will contain the descrition of the scanned folder", true, "",  "JSON filepath");
    cmd.add(folder);
    cmd.add(output);
    cmd.parse(argc, argv);
    const auto path_folder = folder.getValue();
    const auto path_output = output.getValue();

    cout << "\nSCANNING \"" << path_folder << '\"' << endl;
    
    try {
        
        LogError logErr;
        const auto json = cf::ScanFolder(path_folder, logErr);
        ofstream stream{ path_output , ios::out };
        if (!stream) {
            throw runtime_error{ "Cannot write to " + path_output };
        }
		
		array<uint8_t, 3u> bom = { 0xEF, 0xBB, 0xBF };
		wstring_convert<std::codecvt_utf8<wchar_t>> codec_to_utf8;
		stream.write(reinterpret_cast<char*>(bom.data()), 3);
        stream << codec_to_utf8.to_bytes(json);
        stream.close();
    }
    catch (const exception& e)
    {
        cout << e.what() << endl;
    }

	return 0;
}
