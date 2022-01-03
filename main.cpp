#include <iostream>
#include <fmt/format.h>
#include <thread>

int main(int, char **)
{
    std::thread t1([]()
                   { fmt::print("HELLO\n"); });
    std::thread t2([]()
                   { fmt::print("WORLD\n"); });
    t1.join();
    t2.join();
    return 0;
}
