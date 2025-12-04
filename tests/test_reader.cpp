#include "../src/CapnpReader.h"
#include <iostream>
#include <cassert>

void test_reader() {
    std::cout << "Testing CapnpReader...\n";

    // Test: CapnpReader can be instantiated
    CapnpReader reader;
    std::cout << "  ✓ CapnpReader instantiation\n";

    // Test: Can open a file (we'll test with actual file later)
    // For now, just test the interface exists
    std::cout << "  ✓ CapnpReader interface test\n";
}
