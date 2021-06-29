#include <iostream>

int main()
{
    std::cout << "1-st line" << std::endl;
    std::cout << "2-nd line" << std::endl;
    std::cerr << "3-rd line (stderr)" << std::endl;
    std::cerr << "4-th line (stderr)" << std::endl;

    std::cout.flush();
    std::cerr.flush();

    std::cout << "Flushed" << std::endl;
    std::cout.flush();

    std::cout << "5-th line" << std::endl;
    std::cout << "6-th line" << std::endl;
    std::cerr << "7-th line (stderr)" << std::endl;
    std::cerr << "8-th line (stderr)" << std::endl;
    return 0;
}
