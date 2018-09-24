#ifndef __TEST_SRC_TEST_LIBRAY_HPP__
#define __TEST_SRC_TEST_LIBRAY_HPP__

#include <boost/filesystem.hpp>
#include <string>

extern const std::string FOLDER_ROOT;
extern std::pair<boost::filesystem::path, boost::filesystem::path> Folders;
extern std::pair<boost::filesystem::path, boost::filesystem::path> FoldersFast;

void CleanUp();
std::pair<boost::filesystem::path, boost::filesystem::path> Build_Test_Files();
std::pair<boost::filesystem::path, boost::filesystem::path> Build_Test_Fast_Files(unsigned nb_files, const boost::filesystem::path& dir_source);

#endif
