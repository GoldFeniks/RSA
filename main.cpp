#include <ctime>
#include <string>
#include <fstream>
#include <iostream>
#include <boost/program_options.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include "keys.hpp"
#include "utils.hpp"
#include "cipher.hpp"
#include "stream_handler.hpp"

namespace mp = boost::multiprecision;
namespace po = boost::program_options;

template<size_t E, size_t D>
void encrypt_file(const std::string& in_file, const std::string& out_file, const rsa::keys<D * 8> keys) {
    std::ifstream inp(in_file, std::ios::binary);
    rsa::ifstream_handler<rsa::read_number_padding_operation, rsa::eof_operation> read_handler(inp);
    std::ofstream out(out_file, std::ios::binary);
    rsa::ofstream_handler<rsa::write_number_operation, rsa::write_bytes_operation> write_handler(out);
    write_handler.write_number<D * 8>(keys.get_d());
    write_handler.write_number<D * 8>(keys.get_n());
    unsigned int n = 0;
    for (const auto& it : rsa::cipher<E, D>::encrypt(read_handler, keys, n)) {
        write_handler.write_number<D * 8>(it);
    }
    if (n == E) {
        std::array<char, E> padding{};
        padding.fill(static_cast<const char&>(E));
        write_handler.write_number<D * 8>(mp::powm(
                static_cast<typename rsa::num_utils<D * 8>::number>(
                        rsa::num_utils<E * 8>::bytes_to_number(padding.begin(), padding.end())),
                keys.get_e(), keys.get_n()));
    }
}

template<size_t E, size_t D>
void decrypt_file(const std::string& in_file, const std::string& out_file) {
    std::ifstream inp(in_file, std::ios::binary);
    rsa::ifstream_handler<rsa::read_number_operation, rsa::eof_operation> read_handler(inp);
    std::ofstream out(out_file, std::ios::binary);
    rsa::ofstream_handler<rsa::write_number_operation> write_handler(out);
    const auto d = read_handler.read_number<D * 8>();
    const auto n = read_handler.read_number<D * 8>();
    const auto numbers = rsa::cipher<E, D>::decrypt(read_handler, rsa::keys<D * 8>(n, 0, d, 0));
    for (size_t i = 0; i < numbers.size() - 1; ++i)
        write_handler.write_number<E * 8>(numbers[i]);
    const auto bytes = rsa::num_utils<E * 8>::number_to_bytes(numbers.back());
    const auto p = bytes.back();
    bool ok = true;
    for (size_t i = 1; i < p; ++i)
        ok &= bytes[E - i - 1] == p;
    if (!ok)
        throw std::runtime_error("Could not decrypt file");
    out.write(bytes.data(), E - p);
}

using number = rsa::num_utils<256>::number;

int main(int argc, char* argv[]) {
    rsa::random::default_random().init_generator(static_cast<const uint64_t>(std::time(nullptr)));
    std::string in_file, out_file, enc_file;
    number n, e, d;
    po::options_description desc("Allowed options");
    desc.add_options()
            ("help,h", "Print this message")
            ("n", po::value(&n), "n key")
            ("e", po::value(&e), "e key")
            ("d", po::value(&d), "d key")
            ("input_file,i", po::value(&in_file)->default_value("input.txt"), "input filename")
            ("output_file,o", po::value(&out_file)->default_value("output.txt"), "output filename")
            ("encfile,c", po::value(&enc_file)->default_value("encrypted.txt"), "encrypted filename")
            ("random_keys", "use random keys")
            ("print_keys", "print keys");
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 0;
    }
    const auto keys = vm.count("random_keys") ? rsa::keys<256>() : rsa::keys<256>(n, e, d, 0);
    if (vm.count("print_keys")) {
        std::cout << "n  : " << keys.get_n() << '\n';
        std::cout << "e  : " << keys.get_e() << '\n';
        std::cout << "d  : " << keys.get_d() << '\n';
        std::cout << "phi: " << keys.get_phi() << std::endl;
    }
    encrypt_file<8, 32>(in_file, enc_file, keys);
    decrypt_file<8, 32>(enc_file, out_file);
}