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
        TCLAP::CmdLine cmd{ "Compares the content of two folders" };
        TCLAP::MultiArg<string> folders("f", "folder", "An actual folder to be compared", true, "path"); // at least one folder must be provided
        TCLAP::ValueArg<string> json("j", "json", "A JSON file containing the descrition of a preiously scanned folder", false, "", "Filepath");
        cmd.add(folders);
        cmd.add(json);
        cmd.parse(argc, argv);
        const auto path_folders = folders.getValue();
        const auto path_json = json.getValue();

        // Sanity
        if (path_folders.size() > 2u) {
            throw(TCLAP::ArgException{ "No more than two folders can be given as input."});
        }
        else if (path_folders.size() == 2u && path_json.length() != 0) {
            throw(TCLAP::ArgException{"You cannot compare the JSON content with more than one folder."});
        }

        // Execute       
        LogError logErr;
        if (path_folders.size() == 2u) // Compare two folders
        {
            cout << "\nCOMPARING\n\n" << '\"' << path_folders[0] << "\"\n\tand\n\"" << path_folders[1] << "\"\n" << endl;
            const auto diff = cf::CompareFolders(path_folders[0], path_folders[1], logErr);
            cout << cf::Json(diff);
        }

        else if (path_folders.size() == 1)
        {
            if (path_json.length() != 0) // Compare a folder with a JSON
            {

            }
            else {  // Analyse the content of the folder

            }
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
