#include "fmt/format.h"
#include "utf8.h"


int main(int argc, char **argv)
{
    using namespace std;

#if defined _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif


    string u8 = u8"Ελληνικά -- Español -- 中国 --  ĐĄßĞĝ ";
    fmt::print("{}\n", u8);

    string input = getLineUtf8();
    fmt::print("{}\n", input);

    return 0;
}
