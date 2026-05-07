#pragma once

#include <map>
#include <string>
#include <vector>

namespace aurora::sim {

struct SimWaveform {
    std::string name;
    std::vector<double> x;   // time or frequency axis
    std::vector<double> y;   // voltage, current, or other quantity
};

struct SimResult {
    bool success{false};
    std::string errorMessage;
    std::string rawOutput;
    std::vector<SimWaveform> waveforms;
    std::map<std::string, double> dcOperatingPoint;  // node/branch → value
};

}  // namespace aurora::sim
