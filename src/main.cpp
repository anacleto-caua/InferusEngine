#include <exception>
#include <iostream>

int main() {
    try {
        // app.run();
        std::cout << "Hello World!!!";
    } catch (const std::exception &e) {
        std::cerr << e.what() << "\n";
        return 1;
    }

    return 0;
}
