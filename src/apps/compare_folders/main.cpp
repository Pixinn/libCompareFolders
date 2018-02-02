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
#include <fstream>
#include <array>
#include <string>

#include <tclap/CmdLine.h>

#include "CompareFolders.hpp"


using namespace std;

/// @Brief custom error "live" logger
class LogError : public cf::ILogError
{
public:
    void log(const string& err) override {
        cout << "ARRRRG! " << err << endl;
    }
};





/// @brief Compares the content of two folders and displays the result as JSON
int main(int argc, char* argv[])
{
    try
    {
        // Parsing args
        TCLAP::CmdLine cmd{ "Compares the content of two folders, given paths or JSON files. If no output is provided, differences will be displayed." };
        TCLAP::MultiArg<string> folders("f", "folder", "An actual folder to be compared", false, "Folder's path");
        TCLAP::MultiArg<string> json("j", "json", "A JSON file containing the descrition of a previously scanned folder", false, "JSON filepath");
        TCLAP::ValueArg<string> output("o", "output", "A JSON file that will contain the result of the comparison. If provided, no result is displayed on the screen.", false, "", "JSON filepath");
        cmd.add(folders);
        cmd.add(json);
        cmd.add(output);
        cmd.parse(argc, argv);
        const auto path_folders = folders.getValue();
        const auto path_json = json.getValue();
        const auto path_output = output.getValue();

        if (path_folders.size() + path_json.size() != 2u) {
            throw(TCLAP::ArgException{ "You shall give two entries (JSON or FOLDER) to be compared.\n\nType \"" + string{argv[0]} + " -h\" for help.\n"});
        }

        // Execute       
        LogError logErr;
        wstring result;

        // Compare two folders
        if (path_folders.size() == 2u) 
        {
            if (!path_output.empty()) {
                cout << "\nCOMPARING\n\n" << '\"' << path_folders[0] << "\"\n\tand\n\"" << path_folders[1] << "\"\n" << endl;
            }
            result = cf::Json(cf::CompareFolders(path_folders[0], path_folders[1], logErr));
        }
        // Compare one folder and one JSON file
        else if (path_folders.size() == 1)
        {
            if (!path_output.empty()) {
                cout << "\nCOMPARING\n\n" << '\"' << path_folders[0] << "\"\n\tand\n\"" << path_json[0] << "\"\n" << endl;
            }
            result = cf::Json(cf::CompareFolders(path_folders[0], cf::json_t{path_json [0]}, logErr));
        }
        // Compare two JSON files
        else
        {
            if (!path_output.empty()) {
                cout << "\nCOMPARING\n\n" << '\"' << path_json[0] << "\"\n\tand\n\"" << path_json[1] << "\"\n" << endl;
            }
            result = cf::Json(cf::CompareFolders(cf::json_t{ path_json[0] }, cf::json_t{ path_json[1] }));
        }

        // Output the result
        if (path_output.empty())  {
            wcout << result;
        }
        else
		{
            ofstream stream{ path_output , ios::out };
            if (!stream) {
                throw runtime_error{ "Cannot write to " + path_output };
            }
			
			cf::WriteWString(stream, result);
        }
    }
    catch (TCLAP::ArgException &e)  // catch any exceptions
    {
        std::cerr << "error: " << e.error() << std::endl;
        return -1;
    }
    catch (const exception& e)
    {
        cout << e.what() << endl;
        return -1;
    }


    return 0;

}
