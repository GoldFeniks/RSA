#include <cstdio>
#include <iostream>
#include "keys.hpp"


int main() {
    std::cout << rsa::keys<256>::generate_random_prime();
}