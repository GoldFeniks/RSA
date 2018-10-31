#pragma once
#include <cstddef>
#include <random>
#include <functional>
#include <vector>
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

    struct random {

        boost::mt19937_64 generator;

        void init_generator(const uint64_t seed) {
            generator = boost::mt19937_64(seed);
        }

        static auto& default_random() {
            static random rand;
            return rand;
        }

    };

    template<unsigned int N>
    struct num_utils {

        using number = mp::number<mp::backends::cpp_int_backend<N, N, mp::unsigned_magnitude, mp::unchecked, void>>;
        using signed_number = mp::number<mp::backends::cpp_int_backend<2 * N, 2 * N, mp::signed_magnitude, mp::unchecked, void>>;

        static auto get_int_random(const number min = std::numeric_limits<number>::min(),
                                   const number max = std::numeric_limits<number>::max()) {
            rnd::uniform_int_distribution<number> dist(min, max);
            return std::bind(dist, std::ref(random::default_random().generator));

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

        static auto bezout_identity(const signed_number& a, const signed_number& b) {
            if (a == 0)
                return std::make_pair(signed_number(0), signed_number(1));
            const auto buff = bezout_identity(b % a, a);
            return std::make_pair(buff.second - (b / a) * buff.first, buff.first);
        }

    };

}// namespace rsa
