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
#include <string>
#include <fstream>


#include <tclap/CmdLine.h>

#include "CompareFolders.hpp"


using namespace std;

/// @Brief custom error "live" logger
class LogError : public cf::ILogger
{
public:
    void error(const string& err) override {
        cerr << "ARRRRG! " << err << endl;
    }
    void message(const string&) override {}
};





/// @brief Compares the content of two folders and displays the result as JSON
int main(int argc, char* argv[])
{

    // Parsing args
    TCLAP::CmdLine cmd{ "Scans the content of a directory and export the result to a JSON file." };
    TCLAP::ValueArg<string> folder("d", "dir", "The directory to be analyzed", true, "", "Directory's path");
    TCLAP::ValueArg<string> output("o", "output", "The JSON file that will contain the descrition of the scanned folder", true, "",  "JSON filepath");
    TCLAP::SwitchArg fast("f", "fast", "Use the fast algorithm to represent the files' content. Way faster, but less reliable than the default algorithm.");
    cmd.add(folder);
    cmd.add(output);
    cmd.add(fast);
    cmd.parse(argc, argv);
    const auto path_folder = folder.getValue();
    const auto path_output = output.getValue();
    const auto fast_hash = fast.getValue();
    const auto algo = fast_hash ? cf::eCollectingAlgorithm::FAST : cf::eCollectingAlgorithm::SECURE;
    

    cout << "\nSCANNING \"" << path_folder << '\"' << endl;
    
    try {
        
        LogError logErr;
        const auto json = cf::ScanFolder(path_folder, algo, make_unique< LogError>());
        ofstream stream{ path_output , ios::out };
        if (!stream) {
            throw runtime_error{ "Cannot write to " + path_output };
        }
		
		cf::WriteWString(stream, json);
    }
    catch (const exception& e) {
        cout << e.what() << endl;
    }

	return 0;
}
