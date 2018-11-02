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
#include <boost/multiprecision/miller_rabin.hpp>

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

        static auto generate_random_prime(const unsigned int k = 100) {
            const auto rand = get_int_random();
            while(true) {
                const auto n = rand();
                for (unsigned int i = 0; i < math::max_prime; ++i)
                    if (n % math::prime(i) == 0)
                        goto next;
                if (mp::miller_rabin_test(n, k))
                    return n;
                next:
                continue;
            }
        }

        static auto bezout_identity(const signed_number& a, const signed_number& b) {
            if (a == 0)
                return std::make_pair(signed_number(0), signed_number(1));
            const auto buff = bezout_identity(b % a, a);
            return std::make_pair(buff.second - (b / a) * buff.first, buff.first);
        }

        template<typename It>
        static auto bytes_to_number(It begin, const It& end) {
            number result;
            mp::import_bits(result, begin, end, 8);
            return result;
        }

        static auto number_to_bytes(const number& number) {
            std::array<char, N / 8> result;
            result.fill(0);
            mp::export_bits(number, result.rbegin(), 8, false);
            return result;
        }

    };

}// namespace rsa
