#include "lib/Huffman.h"

#include <gflags/gflags.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <istream>

static bool validate_input_path(const char *flagname, const std::string &path) {
    if (path.empty()) {
        std::cout << "Invalid value for --" << flagname << ": can't be empty\n";
        return false;
    }

    if (!std::filesystem::exists(path)) {
        std::cout << "Invalid value for --" << flagname << ": " << path << ": No such file\n";
        return false;
    }

    return true;
}

static bool validate_output_path(const char *flagname, const std::string &path) {
    if (path.empty()) {
        std::cout << "Invalid value for --" << flagname << ": can't be empty\n";
        return false;
    }

    return true;
}

DEFINE_bool(compress, false, "option for compression");
DEFINE_bool(decompress, false, "option for decompression");
DEFINE_string(input, "", "Path to the input file");
DEFINE_string(output, "", "Path to the output file");

DEFINE_validator(input, &validate_input_path);
DEFINE_validator(output, &validate_output_path);

const char* usage_message = "archiving utility.\n"
                            "Usage example:\n"
                            "\tHuffman -compress -input File.txt -output CompressedFile\n"
                            "\tHuffman -decompress -input CompressedFile -output DecompressedFile.txt";

int main(int argc, char *argv[]) {
    gflags::SetUsageMessage(usage_message);
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    if (!(FLAGS_compress ^ FLAGS_decompress)) {
        std::cout << "Invalid options. Either --compress or --decompress should be used\n";
        return 1;
    }

    std::string &input = FLAGS_input;
    std::string &output =FLAGS_output;

    const size_t BUFFER_SIZE = (1U << 20U); // 1 MiB
    std::vector<char> input_buffer(BUFFER_SIZE);
    std::vector<char> output_buffer(BUFFER_SIZE);
    std::ifstream in(input, std::ios_base::binary);
    std::ofstream out(output, std::ios_base::binary | std::ios_base::trunc);
    in.rdbuf()->pubsetbuf(input_buffer.data(), input_buffer.size());
    out.rdbuf()->pubsetbuf(output_buffer.data(), output_buffer.size());
    int err;
    if (FLAGS_compress) {
        err = Arch::compress(&in, &out);
    } else {
        err = Arch::decompress(&in, &out);
    }

    if (err != 0) {
        std::cerr << "command has failed. error: " << err << "\n";
        return 1;
    }
}
