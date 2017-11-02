#include <iostream>
#include <list>
#include <boost/filesystem.hpp>

#include "cryptopp/sha.h"
#include "cryptopp/hex.h"
#include "cryptopp/files.h"

#include "CCollectionHash.hpp"
#include "CFactoryHashes.hpp"


namespace fs = boost::filesystem;
using namespace std;

/// @brief Constructs a path object from a C string
const fs::path path_folder(const char* const str_path)
{
    const fs::path path{ str_path };
    // Sanity
    if (!fs::is_directory(path)) {
        cout << "Error: " << path.string() << " is not a valid directory." << endl;
        exit(-1);
    }
    return path;
}




int main(int argc, char* argv[])
{
	// Sanity
	if (argc != 3) {
        cout << "Usage: " << argv[0] << " [DIRECTORY_1] [DIRECTORY_2]" << endl;
		return -1;
	}

    // Parse args
    const auto path_folder_1 = path_folder(argv[1]);
    const auto path_folder_2 = path_folder(argv[2]);

    cout << "\nCOMPARING\n" << path_folder_1 << "\nand\n" << path_folder_2 << "\n" << endl;
    
    // Compute the hashes
    const cf::CFactoryHashes factoryHashes;
    try {
        
        const auto hashesDir1 = factoryHashes.ComputeHashes(path_folder_1);
        const auto hashesDir2 = factoryHashes.ComputeHashes(path_folder_2);
        
        const auto diff = hashesDir1.compare(hashesDir2);
        // ++++++++++ display result
        cout << diff.identical.size() << " files are identical:\n\n";
        for(const auto& file : diff.identical) {
            cout << file << '\n';
        }
        cout << "\n====\n\n";
        
        cout << diff.different.size() << " files are different:\n\n";
        for(const auto& file : diff.different) {
            cout << file << '\n';
        }
        cout << "\n====\n\n";
        
        cout << diff.unique_left.size() << " files are unique to " << diff.root_left << ":\n\n";
        for(const auto& file : diff.unique_left) {
            cout << file << '\n';
        }
        cout << "\n====\n\n";
        
        cout << diff.unique_right.size() << " files are unique to " << diff.root_right << ":\n\n";
        for(const auto& file : diff.unique_right) {
            cout << file << '\n';
        }
        cout << "\n====\n\n";
        
        cout << diff.renamed.size() << " files with identical content but different name/location:\n";
        for(const auto& files : diff.renamed) {
            cout << "\n---\n";
            for(const auto renamed : files.left) {
                cout << diff.root_left / renamed << '\n';
            }
            for(const auto renamed : files.right) {
                cout << " -> \t" << diff.root_right  / renamed << '\n';
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
