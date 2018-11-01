#pragma once
#include <boost/multiprecision/cpp_int.hpp>
#include <tuple>
#include "utils.hpp"

namespace mp = boost::multiprecision;

namespace rsa {

    template<unsigned int N>
    class keys {

    public:

        using number = typename num_utils<N>::number;

        keys() {
            std::tie(_n, _e, _d, _phi) = generate_keys();
        }

        keys(const number& n , const number& e, const number& d, const number& phi) :
            _n(n), _e(e), _d(d), _phi(phi) {

        }

        static auto generate_keys() {
            const auto p = static_cast<number>(num_utils<N / 2>::generate_random_prime());
            const auto q = static_cast<number>(num_utils<N / 2>::generate_random_prime());
            const auto n = p * q;
            const auto phi = (p - 1) * (q - 1);
            const auto rand = num_utils<N>::get_int_random(2, phi - 1);
            auto e = rand();
            while (mp::gcd(phi, e) != 1)
                e = rand();
            const auto d = num_utils<N>::bezout_identity(e, phi).first % phi;
            return std::make_tuple(n, e, static_cast<number>(d > 0 ? d : phi + d), phi);
        }

        const auto& get_n() const {
            return _n;
        }

        const auto& get_e() const {
            return _e;
        }
        const auto& get_d() const {
            return _d;
        }
        const auto& get_phi() const {
            return _phi;
        }

    private:

        number _n, _e, _d, _phi;

    };

}// namespace rsa
