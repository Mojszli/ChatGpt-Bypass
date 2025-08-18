#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <string>

void writeHeaderFile(const std::vector<unsigned char>& data, const std::string& arrayName, const std::string& outputFile) {
    std::ofstream out(outputFile);
    if (!out) {
        std::cerr << "Failed to open output file: " << outputFile << std::endl;
        return;
    }

    out << "// Auto-generated header file\n";
    out << "unsigned char " << arrayName << "[] = {\n";

    for (size_t i = 0; i < data.size(); ++i) {
        out << "0x" << std::hex << std::setw(2) << std::setfill('0') << (int)data[i];
        if (i != data.size() - 1) out << ", ";
        if ((i + 1) % 12 == 0) out << "\n";
    }

    out << "\n};\n";
    out << "unsigned int " << arrayName << "_len = " << std::dec << data.size() << ";\n";

    out.close();
    std::cout << "Header file written to: " << outputFile << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <input.exe> <output.h>" << std::endl;
        return 1;
    }

    std::string inputFile = argv[1];
    std::string outputFile = argv[2];

    std::ifstream in(inputFile, std::ios::binary);
    if (!in) {
        std::cerr << "Failed to open input file: " << inputFile << std::endl;
        return 1;
    }

    std::vector<unsigned char> buffer((std::istreambuf_iterator<char>(in)), {});
    in.close();

    writeHeaderFile(buffer, "payload", outputFile);
    return 0;
}
