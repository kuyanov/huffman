#include "gtest/gtest.h"

#include "Huffman.h"

using namespace Arch;

#include <string>
#include <sstream>
#include <vector>
#include <random>
#include <algorithm>

struct TestData {
    std::string input;
    std::string desc = "";
};

std::string make_zeroed_string(size_t size) {
    std::string string;
    for (size_t index = 0; index != size; ++index) {
        string += '\0';
    }

    return string;
}

std::vector<char> generate_charset(size_t size) {
    std::vector<char> charset(size);
    std::mt19937 rng;
    std::uniform_int_distribution<> dist(std::numeric_limits<char>::min(), std::numeric_limits<char>::max());
    std::generate_n(charset.begin(), size, [&]() { return char(dist(rng)); });
    return charset;
}

std::string generate_random_string(size_t size, size_t charset_size = 10) {
    std::string string;
    std::mt19937 rng;
    std::vector<char> charset = generate_charset(charset_size);
    std::uniform_int_distribution<> dist(0, charset.size() - 1);
    string.resize(size);
    std::generate_n(string.begin(), size, [&]() { return charset[dist(rng)]; });
    return string;
}

TEST(MainTests, CorrectCompressionDecompression) {
    std::vector<TestData> tests = {
            {"", "empty string"},
            {"a", "one letter"},
            {"aaaaaaaaaaaaaaaaaaaaaaa", "one letter many times"},
            {"abcdefg", "several letters"},
            {"~!@#$%^&*()_+~!@#$%^&*()_+", "symbols"},
            {make_zeroed_string(100), "string with zero ASCII codes"},
    };

    for (const auto &test: tests) {
        std::istringstream input(test.input);
        std::ostringstream output;
        int err = compress(&input, &output);
        if (err != 0) {
            FAIL() << "failed to compress in test: '" << test.input << "', test description: " << test.desc
                   << ", error: " << err;
        }

        std::string compressed_content = output.str();
        input = std::istringstream(compressed_content);
        output = std::ostringstream();
        err = decompress(&input, &output);
        if (err != 0) {
            FAIL() << "failed to decompress in test: '" << test.input << "', test description: " << test.desc
                   << ", error: " << err;
        }

        std::string decompressed_content = output.str();
        if (test.input != decompressed_content) {
            FAIL() << "failed test. content mismatch. input: '" << test.input << "', "
                   << "output: '" << decompressed_content << "', test description: " << test.desc;
        }
    }
}

TEST(MainTests, MalformedInputInDecompression) {
    const size_t SIZE = 10000;
    std::string data = generate_random_string(SIZE);

    std::istringstream input(data);
    std::ostringstream output;
    int err = compress(&input, &output);
    if (err != 0) {
        FAIL() << "failed compression";
    }

    std::string compressed_content = output.str();
    ASSERT_GT(compressed_content.size(), 200);

    // Try to break first bytes
    for (size_t index = 0; index != 8; ++index) {
        char ch = compressed_content[index];
        compressed_content[index] = ~compressed_content[index];
        std::istringstream in(compressed_content);
        std::ostringstream out;
        err = decompress(&in, &out);
        if (err == 0) {
            FAIL() << "decompress missed malformed header with " << index << "-th byte flipped.";
        }

        compressed_content[index] = ch;
    }

    {
        std::string broken_content = compressed_content;
        broken_content += "0";
        std::istringstream in(broken_content);
        std::ostringstream out;
        err = decompress(&in, &out);
        if (err == 0) {
            FAIL() << "decompress missed malformed content with added tail";
        }
    }

    {
        std::string broken_content = compressed_content;
        broken_content.pop_back();
        std::istringstream in(broken_content);
        std::ostringstream out;
        err = decompress(&in, &out);
        if (err == 0) {
            FAIL() << "decompress missed malformed content with added tail";
        }
    }
}

TEST(MainTests, CheckSizeOfCompressedContent) {
    const size_t SIZE = 100000;
    std::string data = generate_random_string(SIZE, 10);

    std::istringstream input(data);
    std::ostringstream output;
    int err = compress(&input, &output);
    if (err != 0) {
        FAIL() << "failed compression";
    }

    std::string compressed_content = output.str();
    input = std::istringstream(compressed_content);
    output = std::ostringstream();
    err = decompress(&input, &output);
    if (err != 0) {
        FAIL() << "failed to decompress";
    }

    std::string decompressed_content = output.str();
    if (data != decompressed_content) {
        FAIL() << "data and decompressed_content mismatch";
    }

    ASSERT_LT(compressed_content.size(), data.size() * 0.5);
}
