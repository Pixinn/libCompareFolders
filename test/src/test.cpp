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

#include <cstdlib>
#include <cstdint>
#include <stdexcept>
#include <list>
#include <array>
#include <boost\filesystem.hpp>

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"

namespace fs = boost::filesystem;
using namespace std;

/// @brief Returns a random string
string Random_String(const unsigned length)
{
    const auto len = (1 + length) & 0xFF;
    constexpr auto size_charset = 66u;
    constexpr array<char, size_charset> charset = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', \
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',\
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};
    unique_ptr<char[]> buffer{ new char[len + 1] };
    for (auto i = 0u; i < len; ++i) {
        buffer[i] = charset[rand() % size_charset];
    }
    buffer[len] = 0u;
    string str{ buffer.get() };
    return str;
}

/// @brief Creates a random named folder
/// @param parent Where to create the folder.
fs::path Create_Random_Folder(const fs::path parent)
{
    constexpr uint8_t max_len_name = 20u;
    const auto name = parent / Random_String(rand() % max_len_name);
    if (!fs::exists(name) && !fs::create_directory(name)) {
        throw runtime_error{ "Cannot create folder " + name.string() };
    }
    return name;
}

/// @brief  Creates random files inside the provided folder
///         Returns the paths of all created files.
/// @param  folder Folder to create the files in.
list<fs::path> Create_Random_Files(const fs::path folder)
{
    list<fs::path> paths;
    constexpr uint8_t max_len_filename = 20u;
    constexpr uint8_t max_nb_files = 100u;

    const unsigned nb_files = rand() % max_nb_files;
    for (auto i = 0u; i < nb_files; ++i)
    {
        auto filepath = folder / Random_String(rand() % max_len_filename);
        while (fs::exists(filepath)) {
            filepath = folder / Random_String(rand() % max_len_filename);
        }
        fs::ofstream stream{ filepath, fs::ofstream::binary };
        constexpr auto max_size_file = 65535u;
        const auto size_file = rand() % max_size_file;
        unique_ptr<char[]> buffer{ new char[size_file] };
        for (auto j = 0u; j < size_file; ++j) {
            buffer[j] = rand() % 0xff;
        }
        stream.write(buffer.get(), size_file);
        paths.push_back(filepath);
    }

    return paths;
}

/// @brief  Duplicates the source directory into the destination.
///         The destination must not pre-exists: it will be created.
//  from: https://stackoverflow.com/questions/8593608/how-can-i-copy-a-directory-using-boost-filesystem
bool Copy_Folder(fs::path const & source, fs::path const & destination )
{
    try
    {
        // Check whether the function call is valid
        if (
            !fs::exists(source) ||
            !fs::is_directory(source)
            )
        {
            std::cerr << "Source directory " << source.string()
                << " does not exist or is not a directory." << '\n'
                ;
            return false;
        }
        if (fs::exists(destination))
        {
            std::cerr << "Destination directory " << destination.string()
                << " already exists." << '\n'
                ;
            return false;
        }
        // Create the destination directory
        if (!fs::create_directory(destination))
        {
            std::cerr << "Unable to create destination directory"
                << destination.string() << '\n'
                ;
            return false;
        }
    }
    catch (fs::filesystem_error const & e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }
    // Iterate through the source directory
    for (
        fs::directory_iterator file(source);
        file != fs::directory_iterator(); ++file
        )
    {
        try
        {
            fs::path current(file->path());
            if (fs::is_directory(current))
            {
                // Found directory: Recursion
                if ( !Copy_Folder(current, destination / current.filename() ) )  {
                    return false;
                }
            }
            else
            {
                // Found file: Copy
                fs::copy_file(
                    current,
                    destination / current.filename()
                );
            }
        }
        catch (fs::filesystem_error const & e)
        {
            std::cerr << e.what() << '\n';
        }
    }
    return true;
}


/// @brief Builds a file tree to perform the tests on
///        Returns the files that were created
pair<fs::path, fs::path> Build_Test_Files()
{
    list<fs::path> files;

    const auto folder_tmp = fs::temp_directory_path();
    const auto folder_left = folder_tmp / "left";
    const auto folder_right = folder_tmp / "right";
    if (!fs::exists(folder_left) && !fs::create_directory(folder_left)) {
        throw runtime_error{ "Cannot create folder " + folder_left.string() };
    }

    srand(static_cast<unsigned>(time(nullptr)));

    constexpr uint8_t max_nb_folders = 10u;
    const unsigned nb_folders = (rand() % (max_nb_folders-1))+1;
    for (auto i = 0u; i < nb_folders; ++i) {
        const auto folder = Create_Random_Folder(folder_left);
        auto files_created = Create_Random_Files(folder);
        files.splice(files.end(), files_created);
    }

    Copy_Folder(folder_left, folder_right);

    return { folder_left, folder_right };
    
}


/// @brief Deletes all the test files
void CleanUp(const pair<fs::path, fs::path> paths)
{
    fs::remove_all(paths.first);
    fs::remove_all(paths.second);
}


TEST_CASE("NOMINAL")
{
    const auto files = Build_Test_Files();
    CleanUp(files);
}