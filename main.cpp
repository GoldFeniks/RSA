#include <cstdio>
#include <iostream>
#include <ctime>
#include <type_traits>
#include <boost/multiprecision/cpp_int.hpp>
#include "utils.hpp"
#include "keys.hpp"
#include "stream_handler.hpp"
#include <fstream>

namespace mp = boost::multiprecision;

int main() {
    std::ifstream inp("test.txt", std::ios::binary);
    rsa::ifstream_handler<rsa::read_number_operation> read_handler(inp);
    const auto n = read_handler.read_number<32>();
    std::ofstream out("out.txt", std::ios::binary);
    rsa::ofstream_handler<rsa::write_number_operation> write_handler(out);
    write_handler.write_number<32>(n);
}