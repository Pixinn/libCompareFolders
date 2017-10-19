#include <iostream>
#include <list>
#include <boost/filesystem.hpp>

#include "cryptopp/sha.h"
#include "cryptopp/hex.h"
#include "cryptopp/files.h"

#include "CCollectionHash.hpp"
#include "CFactoryHashes.hpp"

namespace fs = boost::filesystem;


/// @brief Constructs a path object from a C string
const fs::path path_folder(const char* const str_path)
{
    const fs::path path{ str_path };
    // Sanity
    if (!fs::is_directory(path)) {
        std::cout << "Error: " << path.string() << " is not a valid directory." << std::endl;
        exit(-1);
    }
    return path;
}




int main(int argc, char* argv[])
{
	// Sanity
	if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " [DIRECTORY_1] [DIRECTORY_2]" << std::endl;
		return -1;
	}

    // Parse args
    const auto path_folder_1 = path_folder(argv[1]);
    const auto path_folder_2 = path_folder(argv[2]);
    
    // Compute the hashes
    const cf::CFactoryHashes factoryHashes;
    try {
        
        const auto hashesDir1 = factoryHashes.ComputeHashes(path_folder_1);
        const auto hashesDir2 = factoryHashes.ComputeHashes(path_folder_2);
        
        const auto diff = hashesDir1->compare(*hashesDir2);
        // ++++++++++ dbg
        for(const auto& file : diff.identical) {
            std::cout << file << '\n';
        }
        std::cout << diff.identical.size() << " files are identical.\n====" << std::endl;
        
        for(const auto& file : diff.different) {
            std::cout << file << '\n';
        }
        std::cout << diff.different.size() << " files are different.\n====" << std::endl;
        
        for(const auto& file : diff.unique_left) {
            std::cout << file << '\n';
        }
        std::cout << diff.unique_left.size() << " files are unique to the left.\n====" << std::endl;
        
        for(const auto& file : diff.unique_right) {
            std::cout << file << '\n';
        }
        std::cout << diff.unique_right.size() << " files are unique to the right.\n====" << std::endl;
        
        for(const auto& files : diff.renamed) {
            std::cout << "====\n";
            for(const auto renamed : files.left) {
                std::cout << renamed << '\n';
            }
            for(const auto renamed : files.right) {
                std::cout << "\t\t" << renamed << '\n';
            }
        }
        std::cout << std::endl;
        // ---------- dbg

    }
    catch (const std::exception& e)
    {
        std::cout << "Error: " << e.what();
    }

	return 0;
}
