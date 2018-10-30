#pragma once
#include <cstddef>
#include <random>
#include <functional>
#include <vector>
#include <ctime>
#include <limits>
#include <iostream>
#include <boost/multiprecision/number.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/math/special_functions/prime.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>

namespace mp = boost::multiprecision;
namespace math = boost::math;
namespace rnd = boost::random;

namespace rsa {

    template<unsigned int N>
    struct keys {

        using number = mp::number<mp::backends::cpp_int_backend<N, N, mp::unsigned_magnitude, mp::unchecked, void>>;

        static auto& random_generator() {
            static boost::mt19937 gen(static_cast<const uint32_t&>(std::time(nullptr)));
            return gen;
        }

        static auto get_int_random(const number min = std::numeric_limits<number>::min(),
                                   const number max = std::numeric_limits<number>::max()) {
            rnd::uniform_int_distribution<number> dist(min, max);
            return std::bind(dist, std::ref(random_generator()));

        }

        static auto generate_random_prime(const size_t k = 100) {
            const auto rand = get_int_random();
            while(true) {
                const auto n = rand();
                for (unsigned int i = 0; i < math::max_prime; ++i)
                    if (n % math::prime(i) == 0)
                        goto next;
                if (miller_rabin_test(n, k))
                    return n;
                next:
                continue;
            }
        }

        static auto miller_rabin_test(const number& n, const size_t k) {
            const auto rand = get_int_random(2, n - 2);
            auto d = n - 1;
            size_t r = 0;
            while (d % 2 == 0) {
                d /= 2;
                ++r;
            }
            for (size_t i = 0; i < k; ++i) {
                auto x = mp::powm(rand(), d, n);
                if (x == 1 || x == n - 1)
                    continue;
                for (size_t j = 0; j < r - 1; ++j) {
                    x = mp::powm(x, 2, n);
                    if (x == n - 1)
                        goto next;
                }
                return false;
                next:
                continue;
            }
            return true;
        }

    };

}// namespace rsa
