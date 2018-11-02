#pragma once

#include <boost/multiprecision/cpp_int.hpp>
#include <type_traits>
#include <string>
#include <fstream>
#include <array>
#include <cstdio>
#include "utils.hpp"

namespace mp = boost::multiprecision;

namespace rsa {

    template<typename Stream, template<typename> typename... Operations>
    class stream_handler : public Operations<stream_handler<Stream, Operations...>>... {

    public:

        stream_handler() = delete;
        stream_handler(const stream_handler&) = delete;
        stream_handler(stream_handler&&) = delete;

        explicit stream_handler(Stream& stream) : _stream(stream),  Operations<stream_handler<Stream, Operations...>>(this)... {}

        auto& get_stream() {
            return _stream;
        }

    private:

        Stream& _stream;

    };

    template<typename OwnerT>
    class operation {

    public:

        explicit operation(OwnerT* owner) : _owner(owner) {}

    protected:

        OwnerT* _owner;

    };

    template<typename OwnerT>
    class read_bytes_operation : private operation<OwnerT> {

    public:

        explicit read_bytes_operation(OwnerT* owner) : operation<OwnerT>(owner) {}

        template<unsigned int N>
        auto read_bytes() {
            static unsigned int n;
            return read_bytes(n);
        }

        template<unsigned int N>
        auto read_bytes(unsigned int& n) {
            std::array<char, N> result;
            this->_owner->get_stream().read(result.data(), N);
            n = this->_owner->get_stream().gcount();
            return result;
        }

    };

    template<typename OwnerT>
    class read_number_operation {

    public:

        explicit read_number_operation(OwnerT* owner) : _read_bytes(owner) {}

        template<unsigned int N>
        auto read_number() {
            unsigned int n;
            const auto bytes = _read_bytes.template read_bytes<N / 8>(n);
            return num_utils<N>::bytes_to_number(bytes.begin(), bytes.begin() + n);
        }

    private:

        read_bytes_operation<OwnerT> _read_bytes;

    };

    template<typename OwnerT>
    class read_number_padding_operation {

    public:

        explicit read_number_padding_operation(OwnerT* owner) : _read_bytes(owner) {}

        template<unsigned int N>
        auto read_number_padding(unsigned int& n) {
            auto bytes = _read_bytes.template read_bytes<N / 8>(n);
            for (unsigned int i = 0; i < N / 8 - n; ++i)
                bytes[n + i] = static_cast<char>(N / 8 - n);
            return num_utils<N>::bytes_to_number(bytes.begin(), bytes.end());
        }

        template<unsigned int N>
        auto read_number_padding() {
            static unsigned int n;
            return read_number_padding(n);
        }

    private:

        read_bytes_operation<OwnerT> _read_bytes;

    };

    template<typename OwnerT>
    class write_bytes_operation : private operation<OwnerT> {

    public:

        explicit write_bytes_operation(OwnerT* owner) : operation<OwnerT>(owner) {}

        template<unsigned int N>
        void write_bytes(const std::array<char, N>& bytes) {
            this->_owner->get_stream().write(bytes.data(), N);
        }

    };

    template<typename OwnerT>
    class write_number_operation {

    public:

        explicit write_number_operation(OwnerT* owner) : _write_bytes(owner) {}

        template<unsigned int N>
        void write_number(const typename num_utils<N>::number number) {
            _write_bytes.template write_bytes<N / 8>(num_utils<N>::number_to_bytes(number));
        }

    private:

        write_bytes_operation<OwnerT> _write_bytes;

    };

    template<typename OwnerT>
    class write_number_nlz_operation : operation<OwnerT> {

    public:

        explicit write_number_nlz_operation(OwnerT* owner) : operation<OwnerT>(owner) {}

        template<unsigned int N>
        void write_number_nlz(const typename num_utils<N>::number& number) {
            const auto msb = mp::msb(number);
            const auto n = msb % 8 == 0 ? msb / 8 : msb / 8 + 1;
            const auto bytes = num_utils<N>::number_to_bytes(number);
            this->_owner->get_stream().write(bytes.data() + N - n, n);
        }

    };

    template<typename OwnerT>
    class eof_operation : operation<OwnerT> {

    public:

        explicit eof_operation(OwnerT* owner) : operation<OwnerT>(owner) {}

        bool eof() {
            return this->_owner->get_stream().peek() == EOF;
        }

    };

    template<template<typename> typename... Operations>
    using ifstream_handler = stream_handler<std::ifstream, Operations...>;

    template<template<typename> typename... Operations>
    using ofstream_handler = stream_handler<std::ofstream, Operations...>;

    template<typename Handler, template<typename> typename Operation>
    struct has_operation {

        static constexpr bool value = std::is_base_of_v<Operation<Handler>, Handler>;

    };

    template<typename Handler, template<typename> typename Operation>
    inline constexpr bool has_operation_v = has_operation<Handler, Operation>::value;

};// namespace rsa