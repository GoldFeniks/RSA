#pragma once
#include <type_traits>
#include <vector>
#include <string>
#include <array>
#include <exception>
#include <boost/multiprecision/cpp_int.hpp>
#include "keys.hpp"
#include "stream_handler.hpp"

namespace mp = boost::multiprecision;

namespace rsa {

    template<size_t E, size_t D>
    struct cipher {

        template<
                typename Handler,
                typename = std::enable_if_t<has_operation_v<Handler, read_number_padding_operation> && has_operation_v<Handler, eof_operation>>>
        static auto encrypt(Handler& handler, const keys<D * 8>& keys, unsigned int& n) {
            std::vector<typename num_utils<D * 8>::number> result;
            while (!handler.eof())
                result.push_back(mp::powm(
                        static_cast<typename num_utils<D * 8>::number>(handler.template read_number_padding<E * 8>(n)),
                        keys.get_e(), keys.get_n()));
            return result;
        }

        template<
                typename Handler,
                typename = std::enable_if_t<has_operation_v<Handler, read_number_operation> && has_operation_v<Handler, eof_operation>>>
        static auto decrypt(Handler& handler, const keys<D * 8>& keys) {
            std::vector<typename num_utils<E * 8>::number> result;
            while (!handler.eof())
                result.push_back(static_cast<typename num_utils<E * 8>::number>(mp::powm(
                        static_cast<typename num_utils<D * 8>::number>(handler.template read_number<D * 8>()),
                        keys.get_d(), keys.get_n())));
            return result;
        }

    };

}// namespace rsa
