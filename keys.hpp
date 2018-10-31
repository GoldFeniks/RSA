#pragma once
#include <boost/multiprecision/cpp_int.hpp>
#include "utils.hpp"

namespace mp = boost::multiprecision;

namespace rsa {

    template<unsigned int N>
    class keys {

    public:

        using number = typename num_utils<N>::number;

        keys() {
            const auto p = static_cast<number>(num_utils<N / 2>::generate_random_prime());
            const auto q = static_cast<number>(num_utils<N / 2>::generate_random_prime());
            _n = p * q;
            _phi = (p - 1) * (q - 1);
            const auto rand = num_utils<N>::get_int_random(2, _phi - 1);
            _e = rand();
            while (mp::gcd(_phi, _e) != 1)
                _e = rand();
            const auto d = num_utils<N>::bezout_identity(_e, _phi).first % _phi;
            _d = static_cast<number>(d > 0 ? d : _phi + d);
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
