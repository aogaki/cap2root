#include <iostream>

int main(int argc, char** argv) {
    std::cout << "Running tests...\n";

    // Simple test runner - calls will be added by individual test files
    extern void test_reader();
    extern void test_writer();

    try {
        test_reader();
        test_writer();
        std::cout << "All tests passed!\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << "\n";
        return 1;
    }
}
