#include "../src/RootWriter.h"
#include <iostream>
#include <cassert>

void test_writer() {
    std::cout << "Testing RootWriter...\n";

    // Test: RootWriter can be instantiated
    RootWriter writer("test_output.root");
    std::cout << "  ✓ RootWriter instantiation\n";

    // Test: Can write data
    TreeData data;
    data.Mod = 1;
    data.Ch = 2;
    data.TimeStamp = 12345;
    data.ChargeLong = 100;

    writer.Fill(data);
    std::cout << "  ✓ RootWriter Fill test\n";

    writer.Close();
    std::cout << "  ✓ RootWriter Close test\n";
}
