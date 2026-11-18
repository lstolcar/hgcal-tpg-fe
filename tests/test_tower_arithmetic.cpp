#include "TowerSums.h"
#include <stdexcept>

namespace {
void equal(const std::vector<uint64_t>& actual, const std::vector<uint64_t>& expected) {
   if (actual != expected) throw std::runtime_error("Arithmetic regression failed");
}
template<class Call> void rejects(Call call) {
   try { call(); }
   catch (const std::invalid_argument&) { return; }
   throw std::runtime_error("Expected invalid_argument");
}
}

int main() {
   TowerSums ts;
   equal(ts.trimmingWithRounding({0, 1, 2, 3, 1048574, 1048575, 1048576, 16642998272ULL}, 18, 20),
         {0, 0, 1, 1, 262143, 262143, 262143, 262143});
   equal(ts.trimmingWithRounding({0, 1, 262144, 524287, 524288}, 19, 19),
         {0, 1, 262144, 524287, 524287});
   equal(ts.trimming({(uint64_t{1} << 20) + 64}, 14, 20), {1});
   equal(ts.trimming({uint64_t{1} << 34}, 19, 35), {262144});
   equal(ts.summation({8}, {{1}}, 2, false), {8});
   equal(ts.emulateCE_E({8}, {{1}}, 19, 19), {1});
   equal(ts.emulateCE_E({1}, {{16}}, 20, 19), {1});
   equal(ts.emulateCE_E({255}, {{16}}, 20, 18), {239});
   equal(ts.emulateCE_E({}, std::vector<std::vector<int>>(3), 20, 18), {0, 0, 0});

   std::vector<std::vector<uint64_t>> h(6, std::vector<uint64_t>(1, 0));
   std::vector<std::vector<std::vector<int>>> architecture(1, std::vector<std::vector<int>>(1, std::vector<int>(6, 0)));
   h[0][0] = h[1][0] = 8;
   architecture[0][0][0] = architecture[0][0][1] = 1;
   // Two word sums of 8 must carry into the final sum: (8+8)>>4 == 1.
   equal(ts.emulateCE_H(h, architecture, 15, 19), {1});
   h[0][0] = 511;
   h[1][0] = 0;
   architecture[0][0][0] = 16;
   architecture[0][0][1] = 0;
   // Preserve the reference handling of the 111111111 padding word.
   equal(ts.emulateCE_H(h, architecture, 20, 18), {0});
   equal(ts.emulateCE_H(std::vector<std::vector<uint64_t>>(6),
                       std::vector<std::vector<std::vector<int>>>(3), 20, 18), {0, 0, 0});
   equal(ts.overflowChecker({uint64_t{1} << 35}, 35), {(uint64_t{1} << 35) - 1});
   rejects([&] { ts.emulateCE_E({256}, {{1}}, 20, 18); });
   rejects([&] { ts.emulateCE_E({1}, {{1, 2}}, 20, 18); });
   rejects([&] { ts.emulateCE_E({1}, {{17}}, 20, 18); });
   rejects([&] { ts.trimming({1}, 19, 18); });
   rejects([&] { ts.emulateCE_H({}, {}, 20, 18); });
   std::cout << "Arithmetic regression cases passed\n";
}
