#include "Huffman.h"

#include <array>
#include <memory>
#include <optional>
#include <queue>
#include <utility>
#include <vector>

const inline size_t CHAR_MX = 256;

using namespace Arch;
using Freq = std::array<size_t, CHAR_MX>;
using Codes = std::array<std::optional<std::vector<bool>>, CHAR_MX>;

enum Error {
    OK = 0,
    WRONG_ARGUMENTS,
    MALFORMED_HEADER,
    MALFORMED_DATA,
};

class ReadCodingTableException: std::exception {};
class DecodeInputException: std::exception {};

struct Node {
    std::shared_ptr<Node> left = nullptr, right = nullptr;
    unsigned char c = 0;
};

template <typename T>
std::ostream* Serialize(const T &x, std::ostream *out) {
    out->write(reinterpret_cast<const char*>(&x), sizeof(x));
    return out;
}

template <typename T>
std::istream* Deserialize(T &x, std::istream *in) {
    in->read(reinterpret_cast<char*>(&x), sizeof(x));
    return in;
}

template <typename Buffer>
class BinaryWriter {
private:
    std::ostream *out;
    Buffer buffer = 0;
    int buffer_pos = 0;

public:
    explicit BinaryWriter(std::ostream *_out): out(_out) {}

    void flush() {
        if (buffer_pos > 0) {
            Serialize(buffer, out);
            buffer = 0;
            buffer_pos = 0;
        }
    }

    void write(bool bit) {
        if (bit) {
            buffer |= (1ll << buffer_pos);
        }
        ++buffer_pos;
        if (buffer_pos == 8 * sizeof(buffer)) {
            flush();
        }
    }
};

template <typename Buffer>
class BinaryReader {
private:
    std::istream *in;
    Buffer buffer = 0;
    int buffer_pos = 0;

public:
    explicit BinaryReader(std::istream *_in): in(_in) {}

    bool read() {
        if (buffer_pos == 0) {
            Deserialize(buffer, in);
        }
        bool bit = (buffer >> buffer_pos) & 1;
        ++buffer_pos;
        if (buffer_pos == 8 * sizeof(buffer)) {
            buffer_pos = 0;
        }
        return bit;
    }
};

size_t calculate_frequencies(std::istream *in, Freq &freq) {
    size_t sz = 0;
    unsigned char c;
    while (!Deserialize(c, in)->fail()) {
        ++freq[c];
        ++sz;
    }
    return sz;
}

std::shared_ptr<Node> build_trie_by_freq(const Freq &freq) {
    std::priority_queue<std::pair<long long, std::shared_ptr<Node>>> q;
    for (size_t i = 0; i != CHAR_MX; ++i) {
        if (freq[i] == 0) {
            continue;
        }
        auto leaf = std::make_shared<Node>();
        leaf->c = i;
        q.emplace(-freq[i], leaf);
    }
    if (q.empty()) {
        return nullptr;
    }
    while (q.size() > 1) {
        auto [left_prior, left_ptr] = q.top();
        q.pop();
        auto [right_prior, right_ptr] = q.top();
        q.pop();
        auto root = std::make_shared<Node>();
        root->left = left_ptr;
        root->right = right_ptr;
        q.emplace(left_prior + right_prior, root);
    }
    return q.top().second;
}

void calculate_codes(const std::shared_ptr<Node>& root, std::vector<bool> &code, Codes &codes) {
    if (root == nullptr) {
        return;
    }
    if (root->left == nullptr && root->right == nullptr) {
        codes[root->c] = code;
        return;
    }
    code.push_back(false);
    calculate_codes(root->left, code, codes);
    code.back() = true;
    calculate_codes(root->right, code, codes);
    code.pop_back();
}

void write_coding_table(const Codes &codes, std::ostream *out) {
    size_t sz = 0;
    for (size_t i = 0; i != CHAR_MX; ++i) {
        if (codes[i].has_value()) {
            ++sz;
        }
    }
    Serialize(sz, out);
    for (size_t i = 0; i != CHAR_MX; ++i) {
        if (!codes[i].has_value()) {
            continue;
        }
        unsigned char c = i;
        const auto &code = codes[i].value();
        Serialize(c, out);
        Serialize(code.size(), out);
        BinaryWriter<uint8_t> writer(out);
        for (bool bit : code) {
            writer.write(bit);
        }
        writer.flush();
    }
}

void encode_input(std::istream *in, std::ostream *out, size_t sz, const Codes &codes) {
    Serialize(sz, out);
    BinaryWriter<uint64_t> writer(out);
    unsigned char c;
    while (!Deserialize(c, in)->fail()) {
        for (bool bit : codes[c].value()) {
            writer.write(bit);
        }
    }
    writer.flush();
}

int Arch::compress(std::istream *in, std::ostream *out) {
    if (in == nullptr || out == nullptr) {
        return Error::WRONG_ARGUMENTS;
    }

    Freq freq{};
    size_t sz = calculate_frequencies(in, freq);
    auto root = build_trie_by_freq(freq);
    Codes codes{};
    std::vector<bool> code;
    calculate_codes(root, code, codes);
    write_coding_table(codes, out);

    in->clear();
    in->seekg(0);
    encode_input(in, out, sz, codes);

    return Error::OK;
}

void read_coding_table(std::istream *in, Codes &codes) {
    size_t sz;
    Deserialize(sz, in);
    if (in->fail() || sz > CHAR_MX) {
        throw ReadCodingTableException();
    }
    for (size_t i = 0; i != sz; ++i) {
        unsigned char c;
        Deserialize(c, in);
        size_t code_len;
        Deserialize(code_len, in);
        if (in->fail() || code_len > CHAR_MX) {
            throw ReadCodingTableException();
        }
        BinaryReader<uint8_t> reader(in);
        codes[c].emplace();
        for (size_t j = 0; j != code_len; ++j) {
            codes[c].value().push_back(reader.read());
            if (in->fail()) {
                throw ReadCodingTableException();
            }
        }
    }
}

std::shared_ptr<Node> build_trie_by_codes(const Codes &codes) {
    if (codes.empty()) {
        return nullptr;
    }
    auto root = std::make_shared<Node>();
    for (size_t i = 0; i != CHAR_MX; ++i) {
        if (!codes[i].has_value()) {
            continue;
        }
        auto cur = root;
        for (bool bit : codes[i].value()) {
            if (!bit) {
                if (cur->left == nullptr) {
                    cur->left = std::make_shared<Node>();
                }
                cur = cur->left;
            } else {
                if (cur->right == nullptr) {
                    cur->right = std::make_shared<Node>();
                }
                cur = cur->right;
            }
        }
        cur->c = i;
    }
    return root;
}

void decode_input(std::istream *in, std::ostream *out, const std::shared_ptr<Node>& root) {
    size_t sz;
    Deserialize(sz, in);
    if (in->fail()) {
        throw DecodeInputException();
    }
    BinaryReader<uint64_t> reader(in);
    for (size_t i = 0; i != sz; ++i) {
        auto cur = root;
        while (!(cur->left == nullptr && cur->right == nullptr)) {
            bool bit = reader.read();
            if (in->fail()) {
                throw DecodeInputException();
            }
            if (!bit) {
                cur = cur->left;
            } else {
                cur = cur->right;
            }
        }
        Serialize(cur->c, out);
    }
    in->get();
    if (!in->fail()) {
        throw DecodeInputException();
    }
}

int Arch::decompress(std::istream *in, std::ostream *out) {
    if (in == nullptr || out == nullptr) {
        return Error::WRONG_ARGUMENTS;
    }

    try {
        Codes codes{};
        read_coding_table(in, codes);
        auto root = build_trie_by_codes(codes);
        decode_input(in, out, root);
    } catch(const ReadCodingTableException&) {
        return Error::MALFORMED_HEADER;
    } catch(const DecodeInputException&) {
        return Error::MALFORMED_DATA;
    }

    return Error::OK;
}
