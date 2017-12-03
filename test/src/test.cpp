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

#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

#include "CompareFolders.hpp"



namespace fs = boost::filesystem;
using namespace std;



constexpr unsigned MAX_NB_FOLDERS = 10u;
constexpr unsigned MAX_NB_FILES_PER_FOLDER = 100u;
constexpr unsigned MAX_NB_FILES_TO_MODIFY = 10u;
constexpr unsigned MAX_NB_FILES_TO_ADD = 10u;
constexpr unsigned MAX_NB_FILES_TO_RENAME = 10u;


static pair<fs::path, fs::path> Folders;


/// @rbrief returns the identical files
list<fs::path> Get_Identical_Files()
{
    list<fs::path> identical;
    
    fs::recursive_directory_iterator it{ Folders.first };
    fs::recursive_directory_iterator it_end;
    while (it != it_end) { 
        if (fs::is_regular_file(*it)) {
            identical.push_back(fs::relative(*it, Folders.first));
        }
        ++it;
    }
    
    return identical;
}

/// @brief Returns a random string
string Random_String(const unsigned length)
{
    const auto len = (1 + length) & 0xFF;
    constexpr auto size_charset = 62u;;
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
    const auto name = parent / Random_String(1 + (rand() % (max_len_name-1)));
    if (!fs::exists(name) && !fs::create_directory(name)) {
        throw runtime_error{ "Cannot create folder " + name.string() };
    }
    return name;
}

/// @brief  Creates random files inside the provided folder
///         Returns the paths of all created files.
/// @param  folder Folder to create the files in.
/// @param  max_nb_files Max number of files to create inside the folder (default = 100)
vector<fs::path> Create_Random_Files(const fs::path folder, const uint8_t max_nb_files = MAX_NB_FILES_PER_FOLDER)
{
    vector<fs::path> paths;
    constexpr uint8_t max_len_filename = 20u;

    const auto parent = folder.parent_path();

    const unsigned nb_files = max_nb_files == 1 ? 1 : 1 + (rand() % (max_nb_files-1));
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
        paths.push_back(fs::relative(filepath, parent));
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

    const unsigned nb_folders = (rand() % (MAX_NB_FOLDERS -1))+1;
    for (auto i = 0u; i < nb_folders; ++i) {
        const auto folder = Create_Random_Folder(folder_left);
        auto files_created = Create_Random_Files(folder);
        files.insert(end(files), begin(files_created), end(files_created));
    }

    Copy_Folder(folder_left, folder_right);

    return { folder_left, folder_right };
    
}


/// @brief Modifies some "identical' files in the provided folder then remove them from the list.
///        Returns a vector of the modified files
vector<fs::path> Modify_Files(const fs::path & folder, list<fs::path>& files_identical)
{
    vector<fs::path> files_modified;

    // modify random files
    const auto nb_files = min(1 + (rand() % (MAX_NB_FILES_TO_MODIFY - 1)), files_identical.size());
    for (auto i = 0u; i < nb_files; ++i)
    {
        // find the file
        auto idx = rand() % files_identical.size();
        auto it = begin(files_identical);
        for (auto j = 0u; j < idx; ++j) { ++it;  }
        auto file = *it;

        // modify it
        fs::fstream stream{ folder / file, ios::in | ios::out | ios::binary | ios::ate};
        const auto len = static_cast<int>(stream.tellg());
        vector<char> buffer(len);
        stream.seekg(0, stream.beg);
        stream.read(buffer.data(), len);
        buffer.resize(len);
        const auto position = rand() % len;
        buffer[position] = ~buffer[position];
        stream.seekg(0, stream.beg);
        stream.write(buffer.data(), len);

        files_identical.erase(it);

        files_modified.push_back(file);
    }

    return files_modified;
}


/// @brief Adds some files into a folder. Un to 1 level of subfolders
vector<fs::path> Add_Files(const fs::path& folder)
{
    vector<fs::path> files_added;

    // list all subfolders
    vector<fs::path> subfolders;
    fs::directory_iterator it{ folder };
    const auto idx_end = fs::directory_iterator{};
    while (it != idx_end) {
        if (fs::is_directory(*it)) {
            subfolders.push_back(*it);
        }
        ++it;
    }
    // add files
    const uint8_t nb_files = 1 + (rand() % (MAX_NB_FILES_TO_ADD - 1));
    for (auto i = 0u; i < nb_files; ++i) {
        auto idx = rand() % subfolders.size();
        auto files = Create_Random_Files(subfolders[idx], 1);
        files_added.insert(end(files_added), begin(files), end(files));
    }

    return files_added;
}

/// @brief Renames  some random files in the provided folder
vector<fs::path> Rename_Files(const fs::path& folder, list<fs::path>& files_identical)
{
    vector<fs::path> files_renamed;

    // rename random files
    const auto nb_files = 1 + (rand() % (MAX_NB_FILES_TO_RENAME - 1));
    for (auto i = 0u; i < nb_files && !files_identical.empty(); ++i)
    {
        auto idx = rand() % files_identical.size();
        auto it = begin(files_identical);
        for (auto j = 0u; j < idx; ++j) { ++it; }
        auto path_file = folder / *it;
        files_identical.erase(it);
        const auto parent = path_file.parent_path();
        const auto filename_old = path_file.filename();
        auto filename_new = Random_String(20u);
        while (filename_new == filename_old.string()) {
            filename_new = Random_String(20u);
        }
        const auto path_new = parent / filename_new;
        fs::rename(path_file, path_new);
        files_renamed.push_back(fs::relative(path_new, folder));
    }

    return files_renamed;
}




/// @brief Deletes all the test files
void CleanUp(const pair<fs::path, fs::path> & paths)
{
    fs::remove_all(paths.first);
    fs::remove_all(paths.second);
}


TEST_CASE("BAD FOLDER TO COMPARE")
{
    const auto folder_random = Random_String(20u) + Random_String(20u) + Random_String(20u);
    bool bad_left_folder = false;
    bool bad_right_foldder = false;

    try {
        cf::CompareFolders(".", folder_random);
    }
    catch (const cf::ExceptionFatal&) {
        bad_right_foldder = true;
    }
    REQUIRE(bad_right_foldder == true);

    try {
        cf::CompareFolders(folder_random, ".");
    }
    catch (const cf::ExceptionFatal&) {
        bad_left_folder = true;
    }
    REQUIRE(bad_left_folder == true);
}



TEST_CASE("NOMINAL")
{
    // Get the identical files list
    auto files_identical = Get_Identical_Files();

    // Modify some
    auto files_modified = Modify_Files(Folders.first, files_identical);
    auto files_modified_right = Modify_Files(Folders.second, files_identical);
    files_modified.insert(end(files_modified), begin(files_modified_right), end(files_modified_right));
    
    // Add unique files
    // NOTE:    There is an **extremely** thin chance that the files added left == the files added right.
    //          Then the test would fail!
    auto files_unique_left = Add_Files(Folders.first);
    auto files_unique_right = Add_Files(Folders.second);

    // Rename
    auto files_renamed = Rename_Files(Folders.first, files_identical);

    // Compare
    const auto diff = cf::CompareFolders(Folders.first.string(), Folders.second.string());

    // Check results
    REQUIRE(diff.identical.size() == files_identical.size());
    for (auto& entry : diff.identical) {
        REQUIRE(find(begin(files_identical), end(files_identical), entry) != end(files_identical));
    }
    REQUIRE(diff.different.size() == files_modified.size());
    for (auto& entry : diff.different) {
        REQUIRE(find(begin(files_modified), end(files_modified), entry) != end(files_modified));
    }
    REQUIRE(diff.unique_left.size() == files_unique_left.size());
    for (auto& entry : diff.unique_left) {
        REQUIRE(find(begin(files_unique_left), end(files_unique_left), entry) != end(files_unique_left));
    }
    REQUIRE(diff.unique_right.size() == files_unique_right.size());
    for (auto& entry : diff.unique_right) {
        REQUIRE(find(begin(files_unique_right), end(files_unique_right), entry) != end(files_unique_right));
    }
    REQUIRE(diff.renamed.size() == files_renamed.size());
    for (auto& entry : diff.renamed) {
        REQUIRE(find(begin(files_renamed), end(files_renamed), *begin(entry.left)) != end(files_renamed)); // Works because file were renamed in LEFT and there was no duplication
    }

   
}





int main(int argc, char* argv[]) {
    
    // global setup...
    Folders = Build_Test_Files();

    int result = Catch::Session().run(argc, argv);

    // global clean-up...
    CleanUp(Folders);

    return result;
}