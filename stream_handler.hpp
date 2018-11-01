#pragma once

#include <boost/multiprecision/cpp_int.hpp>
#include <type_traits>
#include <string>
#include <fstream>
#include <array>
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
            typename num_utils<N>::number result;
            unsigned int n;
            const auto bytes = _read_bytes.template read_bytes<N / 8>(n);
            mp::import_bits(result, bytes.begin(), bytes.begin() + n, 8);
            return result;
        }

    private:

        read_bytes_operation<OwnerT> _read_bytes;

    };

    template<typename OwnerT>
    class write_bytes_operation : operation<OwnerT> {

    public:

        explicit write_bytes_operation(OwnerT* owner) : operation<OwnerT>(owner) {}

        template<size_t N>
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
            std::array<char, N / 8> bytes;
            mp::export_bits(number, bytes.rbegin(), 8, false);
            _write_bytes.write_bytes(bytes);
        }

    private:

        write_bytes_operation<OwnerT> _write_bytes;
    };

    template<template<typename> typename... Operations>
    using ifstream_handler = stream_handler<std::ifstream, Operations...>;

    template<template<typename> typename... Operations>
    using ofstream_handler = stream_handler<std::ofstream, Operations...>;

};// namespace rsa