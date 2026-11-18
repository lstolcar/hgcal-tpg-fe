// Compare towerSums_v3.py using the same packed unpacker inputs and v2 mappings.
// ROOT is read by the Python unpackers; this program processes their text output.
#include "TowerSums.h"
#include <limits>

namespace fs = std::filesystem;

namespace {
void requireFile(const fs::path& path) {
   if (!fs::is_regular_file(path))
      throw std::runtime_error("Missing input file: " + path.string());
}

unsigned long long parseNumber(const char* argument) {
   const std::string value(argument);
   if (value.empty() || value.find_first_not_of("0123456789") != std::string::npos)
      throw std::invalid_argument("Expected a nonnegative integer: " + value);
   return std::stoull(value);
}

int parseBits(const char* argument) {
   const auto value = parseNumber(argument);
   if (value > 63)
      throw std::invalid_argument("Bit parameters cannot exceed 63");
   return static_cast<int>(value);
}

void writeValues(const fs::path& path, const std::vector<uint64_t>& values, bool binary) {
   std::ofstream file(path);
   if (!file)
      throw std::runtime_error("Cannot write output: " + path.string());
   for (size_t i = 0; i < values.size(); ++i) {
      if (values[i] >= 0x100)
         throw std::overflow_error("Packed pTT exceeds 8 bits");
      if (binary)
         file << std::bitset<8>(values[i]);
      else
         file << std::hex << std::setw(2) << std::setfill('0') << values[i];
      if (i + 1 != values.size()) file << '\n';
   }
   if (!file)
      throw std::runtime_error("Failed writing output: " + path.string());
}
}

int main(int argc, char** argv) {
   if (argc == 2 && std::string(argv[1]) == "--help") {
      std::cout << "Usage: " << argv[0]
                << " UNPACKER_DIR MAP_DIR OUTPUT_DIR EVENT [MSB_CEE MSB_CEH TARGET_BITS]\n"
                << "Defaults: MSB_CEE=20 MSB_CEH=20 TARGET_BITS=18\n";
      return 0;
   }
   if (argc != 5 && argc != 8) {
      std::cerr << "Usage: " << argv[0]
                << " UNPACKER_DIR MAP_DIR OUTPUT_DIR EVENT [MSB_CEE MSB_CEH TARGET_BITS]\n";
      return 2;
   }
   try {
      const fs::path inputs = argv[1], maps = argv[2], output = argv[3];
      const auto event = parseNumber(argv[4]);
      const int msbE = argc == 8 ? parseBits(argv[5]) : 20;
      const int msbH = argc == 8 ? parseBits(argv[6]) : 20;
      const int bits = argc == 8 ? parseBits(argv[7]) : 18;
      if (bits < 5 || bits > 19 || msbE < bits || msbH < bits - 4)
         throw std::invalid_argument("Require 5 <= TARGET_BITS <= 19, MSB_CEE >= TARGET_BITS, MSB_CEH >= TARGET_BITS-4");
      fs::create_directories(output);
      TowerSums ts;
      size_t countE = 0, countH = 0;
      for (int board = 0; board < 14; ++board) {
         const std::string number = std::to_string(board);
         const auto eMap = maps / ("CE_E_" + number + "_v2.vh");
         const auto hMap = maps / ("CE_H_" + number + "_v2.vh");
         const auto eDimensions = ts.getParametersFromVhFile(eMap.string());
         const auto hDimensions = ts.getParametersFromVhFile(hMap.string());
         auto eArchitecture = ts.vhArchInputToArray_CE_E(eMap.string(), eDimensions.second, eDimensions.first);
         auto hArchitecture = ts.vhArchInputToArray_CE_H(hMap.string(), hDimensions.second, hDimensions.first, "oneSize");
         for (int sector = 0; sector < 3; ++sector) {
            const std::string name = "Sector_" + std::to_string(sector) + "_Board_" + number;
            std::vector<uint64_t> eInputs;
            if (eDimensions.first > 0) {
               const auto path = inputs / "CEE" / (name + ".txt");
               requireFile(path);
               eInputs = ts.readInputEnergies_CE_E(path.string());
            }
            std::vector<std::vector<uint64_t>> hInputs(6);
            if (hDimensions.first > 0) {
               const auto path = inputs / "CEH" / (name + ".txt");
               requireFile(path);
               // As in Python: missing input rows remain zero-filled.
               hInputs = ts.readInputEnergies_CE_H(path.string(), hDimensions.first);
            }
            const auto eValues = ts.emulateCE_E(eInputs, eArchitecture, msbE, bits);
            const auto hValues = ts.emulateCE_H(hInputs, hArchitecture, msbH, bits);
            writeValues(output / ("TowerSums_" + name + "_CE_E_Event_" + std::to_string(event) + ".txt"), eValues, false);
            writeValues(output / ("TowerSums_" + name + "_CE_H_Event_" + std::to_string(event) + ".txt"), hValues, true);
            countE += eValues.size();
            countH += hValues.size();
         }
      }
      std::cout << "Event " << event << "; MSB_CEE=" << msbE << "; MSB_CEH=" << msbH << "; targetBits=" << bits << '\n'
                << "CE-E: " << countE << " pTTs; CE-H: " << countH << " pTTs\n";
      return 0;
   } catch (const std::exception& error) {
      std::cerr << "Tower emulation failed: " << error.what() << '\n';
      return 1;
   }
}
