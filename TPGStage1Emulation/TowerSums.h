#ifndef TowerSums_h
#define TowerSums_h

#include <algorithm>
#include <bitset>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "Utilities.h"

using namespace std;

class TowerSums {
public:
   TowerSums();
   ~TowerSums();
   pair<int, int> getParametersFromVhFile(const string&);
   vector<vector<int>> vhArchInputToArray_CE_E(const string&, int, int);
   vector<vector<vector<int>>> vhArchInputToArray_CE_H(const string&, int, int, const string&);
   vector<uint64_t> readInputEnergies_CE_E(const string&);
   vector<vector<uint64_t>> readInputEnergies_CE_H(const string&, int);
   vector<uint64_t> unpackInteger3m(const vector<uint64_t>&);
   vector<vector<uint64_t>> unpackInteger4m(const vector<vector<uint64_t>>&);
   vector<uint64_t> packInteger4e4m(const vector<uint64_t>&);

   // Version 2 returns the raw weighted sum. Apply the final /16 in the caller.
   vector<uint64_t> summation(const vector<uint64_t>&, const vector<vector<int>>&, int, bool);
   vector<uint64_t> overflowChecker(const vector<uint64_t>&, int);
   vector<uint64_t> trimming(const vector<uint64_t>&, int, int);
   vector<uint64_t> trimmingWithRounding(const vector<uint64_t>&, int, int);

   // Packed unpacker inputs -> packed 4E4M pTTs, matching towerSums_v3.py.
   vector<uint64_t> emulateCE_E(const vector<uint64_t>&,
                              const vector<vector<int>>&, int msb, int targetBits);
   vector<uint64_t> emulateCE_H(const vector<vector<uint64_t>>&,
                              const vector<vector<vector<int>>>&, int msb, int targetBits);

   vector<string> generateInputShifts(int, int);
   vector<uint64_t> addVectors(const vector<uint64_t>&, const vector<uint64_t>&);
   void writeToFile(const vector<uint64_t>&, int, int, const string&);

private:
   Utilities* utilities;
};
#endif
