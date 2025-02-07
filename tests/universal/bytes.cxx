//
// Copyright (c) 2024 ZettaScale Technology
//
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// http://www.eclipse.org/legal/epl-2.0, or the Apache License, Version 2.0
// which is available at https://www.apache.org/licenses/LICENSE-2.0.
//
// SPDX-License-Identifier: EPL-2.0 OR Apache-2.0
//
// Contributors:
//   ZettaScale Zenoh Team, <zenoh@zettascale.tech>
//

#include <iostream>

#include "zenoh.hxx"
#undef NDEBUG
#include <assert.h>

using namespace zenoh;

void reader_writer() {
    std::cout << "running reader_writer\n";
    std::vector<uint8_t> data = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    Bytes b;
    {
        auto writer = b.writer();
        writer.write_all(data.data(), 5);
        writer.write_all(data.data() + 5, 5);
    }

    auto reader = b.reader();
    std::vector<uint8_t> out(3);
    assert(reader.read(out.data(), 3) == 3);
    assert(out == std::vector<uint8_t>(data.begin(), data.begin() + 3));
    out = std::vector<uint8_t>(7);
    assert(reader.read(out.data(), 10) == 7);
    assert(out == std::vector<uint8_t>(data.begin() + 3, data.end()));
    assert(reader.read(out.data(), 10) == 0);
}

void reader_seek_tell() {
    std::cout << "running reader_seek_tell\n";
    std::vector<uint8_t> data = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    Bytes b;
    {
        auto writer = b.writer();
        writer.write_all(data.data(), 5);
        writer.write_all(data.data() + 5, 5);
    }

    auto reader = b.reader();
    assert(reader.tell() == 0);
    uint8_t i = 255;
    reader.read(&i, 1);
    assert(i == 0);
    assert(reader.tell() == 1);
    reader.seek_from_current(5);
    assert(reader.tell() == 6);
    reader.read(&i, 1);
    assert(i == 6);
    reader.seek_from_start(3);
    assert(reader.tell() == 3);
    reader.read(&i, 1);
    assert(i == 3);

    reader.seek_from_end(-3);
    assert(reader.tell() == 7);
    reader.read(&i, 1);
    assert(i == 7);

    reader.seek_from_current(-2);
    assert(reader.tell() == 6);
    reader.read(&i, 1);
    assert(i == 6);
}

void serde_basic() {
    std::cout << "running serde_basic\n";
    std::vector<uint8_t> data = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    Bytes b = Bytes::serialize(data);
    assert(b.deserialize<std::vector<uint8_t>>() == data);
    assert(b.size() == 10);

    std::vector<uint8_t> data2 = data;
    b = Bytes::serialize(std::move(data));
    assert(data.empty());
    assert(b.deserialize<std::vector<uint8_t>>() == data2);

    std::string s = "abc";
    b = Bytes::serialize(s);
    assert(b.deserialize<std::string>() == s);
    assert(!s.empty());

    std::string s2 = s;
    b = Bytes::serialize(std::move(s));
    assert(s.empty());
    assert(b.deserialize<std::string>() == s2);

#define __ZENOH_TEST_ARITHMETIC(TYPE, VALUE) \
    {                                        \
        TYPE t = VALUE;                      \
        Bytes b = Bytes::serialize(t);       \
        assert(b.deserialize<TYPE>() == t);  \
    }

    __ZENOH_TEST_ARITHMETIC(uint8_t, 5);
    __ZENOH_TEST_ARITHMETIC(uint16_t, 500);
    __ZENOH_TEST_ARITHMETIC(uint32_t, 50000);
    __ZENOH_TEST_ARITHMETIC(uint64_t, 500000000000);

    __ZENOH_TEST_ARITHMETIC(int8_t, -5);
    __ZENOH_TEST_ARITHMETIC(int16_t, 500);
    __ZENOH_TEST_ARITHMETIC(int32_t, -50000);
    __ZENOH_TEST_ARITHMETIC(int64_t, -500000000000);

    __ZENOH_TEST_ARITHMETIC(float, 0.5f);
    __ZENOH_TEST_ARITHMETIC(double, 123.45);

    auto p = std::make_pair(-12, std::string("123"));
    b = Bytes::serialize(p);
    assert(b.deserialize<decltype(p)>() == p);
}

void serde_iter() {
    std::cout << "running serde_iter\n";
    std::vector<uint8_t> data = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    auto b = Bytes::serialize_from_iter(data.begin(), data.end());
    auto it = b.iter();
    std::vector<uint8_t> out;
    for (auto bb = it.next(); bb.has_value(); bb = it.next()) {
        out.push_back(bb->deserialize<uint8_t>());
    }
    assert(data == out);
}

void serde_advanced() {
    std::cout << "running serde_advanced\n";
    std::vector<float> v = {0.1f, 0.2f, 0.3f};
    auto b = Bytes::serialize(v);
    assert(b.deserialize<decltype(v)>() == v);

    std::vector<float> v2 = v;
    b = Bytes::serialize(std::move(v));
    assert(v.empty());
    assert(b.deserialize<decltype(v)>() == v2);

    std::unordered_map<std::string, double> mu = {{"a", 0.5}, {"b", -123.45}, {"abc", 3.1415926}};
    b = Bytes::serialize(mu);
    assert(b.deserialize<decltype(mu)>() == mu);

    std::unordered_map<std::string, double> mu2 = mu;
    b = Bytes::serialize(std::move(mu));
    assert(mu.empty());
    assert(b.deserialize<decltype(mu)>() == mu2);

    std::set<uint8_t> s = {1, 2, 3, 4, 0};
    b = Bytes::serialize(s);
    assert(b.deserialize<decltype(s)>() == s);

    std::set<uint8_t> s2 = s;
    b = Bytes::serialize(std::move(s));
    assert(s.empty());
    assert(b.deserialize<decltype(s)>() == s2);

    std::map<std::string, std::deque<double>> mo = {
        {"a", {0.5, 0.2}}, {"b", {-123.45, 0.4}}, {"abc", {3.1415926, -1.0}}};

    b = Bytes::serialize(mo);
    assert(b.deserialize<decltype(mo)>() == mo);

    std::map<std::string, std::deque<double>> mo2 = mo;
    b = Bytes::serialize(std::move(mo));
    assert(mo.empty());
    assert(b.deserialize<decltype(mo)>() == mo2);
}

void serde_shared() {
    std::cout << "running serde_shared\n";
    std::vector<uint8_t> v = {1, 2, 3, 4, 5};
    auto v_ptr = std::make_shared<std::vector<uint8_t>>(std::move(v));
    auto b = Bytes::serialize(v_ptr);
    assert(v_ptr.use_count() == 2);
    assert(b.deserialize<decltype(v)>() == *v_ptr);
    b = Bytes();
    assert(v_ptr.use_count() == 1);

    std::unordered_map<std::string, double> mu = {{"a", 0.5}, {"b", -123.45}, {"abc", 3.1415926}};
    auto mu_ptr = std::make_shared<std::unordered_map<std::string, double>>(std::move(mu));
    b = Bytes::serialize(mu_ptr);
    assert(mu_ptr.use_count() == 4);
    auto m = b.deserialize<decltype(mu)>();
    assert(b.deserialize<decltype(mu)>() == *mu_ptr);
    b = Bytes();
    assert(mu_ptr.use_count() == 1);
}

void reader_writer_append() {
    std::cout << "running reader_writer_append\n";
    std::vector<uint8_t> data = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    std::vector<uint8_t> data2 = {11, 12, 13, 14};
    Bytes b;
    {
        auto writer = b.writer();
        writer.write_all(data.data(), 5);
        writer.write_all(data.data() + 5, 5);
        writer.append(data2);
    }

    auto reader = b.reader();
    std::vector<uint8_t> out(3);
    assert(reader.read(out.data(), 3) == 3);
    assert(out == std::vector<uint8_t>(data.begin(), data.begin() + 3));
    out = std::vector<uint8_t>(7);
    assert(reader.read(out.data(), 7) == 7);
    assert(out == std::vector<uint8_t>(data.begin() + 3, data.end()));

    out = std::vector<uint8_t>(4);
    assert(reader.read(out.data(), 4) == 4);
    assert(out == data2);
    assert(reader.read(out.data(), 1) == 0);  // reached the end of the payload
}

void reader_writer_append_bounded() {
    std::cout << "running reader_writer_append_bounded\n";
    std::vector<uint8_t> data = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    std::string s = "abcd";
    float f = 0.5f;
    Bytes b;
    {
        auto writer = b.writer();
        writer.write_all(data.data(), 5);
        writer.write_all(data.data() + 5, 5);
        writer.append_bounded(s);
        writer.append_bounded(f);
    }

    auto reader = b.reader();
    std::vector<uint8_t> out(3);
    assert(reader.read(out.data(), 3) == 3);
    assert(out == std::vector<uint8_t>(data.begin(), data.begin() + 3));
    out = std::vector<uint8_t>(7);
    assert(reader.read(out.data(), 7) == 7);
    assert(out == std::vector<uint8_t>(data.begin() + 3, data.end()));

    assert(reader.read_bounded().deserialize<std::string>() == s);
    assert(reader.read_bounded().deserialize<float>() == f);
    assert(reader.read(out.data(), 1) == 0);  // reached the end of the payload
}

struct CustomStruct {
    uint32_t u = 0;
    double d = 0;
    std::string s = {};
};

// Example of codec for a custom class / struct
// We need to define corresponding serialize and deserialize methods
struct CustomCodec {
    static Bytes serialize(const CustomStruct& s) {
        Bytes b;
        auto writer = b.writer();
        writer.write_all(serialize_arithmetic(s.u).data(), 4);
        writer.write_all(serialize_arithmetic(s.d).data(), 8);
        writer.write_all(reinterpret_cast<const uint8_t*>(s.s.data()), s.s.size());
        return b;
    }

    static Bytes serialize(CustomStruct&& s) {
        Bytes b;
        auto writer = b.writer();
        writer.write_all(serialize_arithmetic(s.u).data(), 4);
        writer.write_all(serialize_arithmetic(s.d).data(), 8);
        writer.append(std::move(s.s));
        return b;
    }

    static Bytes serialize(std::shared_ptr<CustomStruct> s) {
        Bytes b;
        auto writer = b.writer();
        writer.write_all(serialize_arithmetic(s->u).data(), 4);
        writer.write_all(serialize_arithmetic(s->d).data(), 8);
        writer.append(Bytes::serialize(s->s, ZenohCodec(s)));
        return b;
    }

    // deserialize should be a template method
    template <class T>
    static T deserialize(const Bytes& b, ZResult* err = nullptr);

   private:
    template <std::uint8_t T_numBytes>
    using UintType = typename std::conditional<
        T_numBytes == 1, std::uint8_t,
        typename std::conditional<T_numBytes == 2, std::uint16_t,
                                  typename std::conditional<T_numBytes == 3 || T_numBytes == 4, std::uint32_t,
                                                            std::uint64_t>::type>::type>::type;

    template <class T>
    static std::enable_if_t<std::is_arithmetic_v<T>, std::array<uint8_t, sizeof(T)>> serialize_arithmetic(T t) {
        // use simple little endian encoding
        std::array<uint8_t, sizeof(T)> out;
        uint8_t mask = 0b11111111u;
        UintType<sizeof(T)> u = reinterpret_cast<UintType<sizeof(T)>&>(t);
        for (size_t i = 0; i < out.size(); i++) {
            out[i] = static_cast<uint8_t>(u & mask);
            u = u >> 8;
        }
        return out;
    }

    template <class T>
    static std::enable_if_t<std::is_arithmetic_v<T>, T> deserialize_arithmetic(const uint8_t* buf) {
        // use simple little endian encoding
        UintType<sizeof(T)> out = 0;
        for (size_t i = 0; i < sizeof(T); i++) {
            out = out << 8;
            out = out | buf[sizeof(T) - i - 1];
        }
        return reinterpret_cast<const T&>(out);
    }
};

template <>
CustomStruct CustomCodec::deserialize<CustomStruct>(const Bytes& b, ZResult* err) {
    CustomStruct out;
    if (b.size() < 12) {  // we should have at least 12 bytes in the payload
        if (err != nullptr) {
            *err = -1;
            return out;
        } else {
            throw std::runtime_error("Insufficient payload size");
        }
    }

    std::array<uint8_t, 8> buf;
    auto reader = b.reader();

    reader.read(buf.data(), 4);
    out.u = deserialize_arithmetic<uint32_t>(buf.data());
    reader.read(buf.data(), 8);
    out.d = deserialize_arithmetic<double>(buf.data());
    size_t remaining = b.size() - 12;
    out.s = std::string(remaining, 0);
    reader.read(reinterpret_cast<uint8_t*>(out.s.data()), remaining);
    return out;
}

void serde_custom() {
    std::cout << "running serde_custom\n";
    CustomStruct s;
    s.d = 0.5;
    s.u = 500;
    s.s = "abcd";
    auto b = Bytes::serialize(s, CustomCodec());
    CustomStruct out = b.deserialize<CustomStruct>(CustomCodec());
    assert(s.d == out.d);
    assert(s.u == out.u);
    assert(s.s == out.s);

    CustomStruct s2 = s;

    b = Bytes::serialize(std::move(s2), CustomCodec());
    out = b.deserialize<CustomStruct>(CustomCodec());
    assert(s.d == out.d);
    assert(s.u == out.u);
    assert(s.s == out.s);

    b = Bytes::serialize(std::make_shared<CustomStruct>(s), CustomCodec());
    out = b.deserialize<CustomStruct>(CustomCodec());
    assert(s.d == out.d);
    assert(s.u == out.u);
    assert(s.s == out.s);
}

int main(int argc, char** argv) {
    reader_writer();
    reader_seek_tell();
    serde_basic();
    serde_iter();
    serde_advanced();
    serde_shared();

    reader_writer_append();
    reader_writer_append_bounded();
    serde_custom();
}
