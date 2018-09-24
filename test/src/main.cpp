#include "test_library.hpp"

#define CATCH_CONFIG_RUNNER
#include "catch.hpp"
#include <exception>



using namespace std;





int main(int argc, char* argv[])
{
    CleanUp();
    srand(static_cast<unsigned>(time(nullptr)));

    try {
        // global setup...
        Folders = Build_Test_Files();
        FoldersFast = Build_Test_Fast_Files(20, Folders.first);
    }
    catch (const exception& e) {
        cout << e.what() << endl;
        // global clean-up...
        CleanUp();
        return -1;
    }

    auto result = Catch::Session().run(argc, argv);

    // global clean-up...
    CleanUp();

    return result;
}
