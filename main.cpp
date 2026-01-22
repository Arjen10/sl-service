#include "src/sl/app.hpp"

int main(int argc, char* argv[]) {

#ifdef _WIN32
    // 控制台切换为 UTF-8
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    app a(argc, argv);
    return a.run();
}
