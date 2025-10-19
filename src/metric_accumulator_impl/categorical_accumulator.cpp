#include "metric_accumulator_impl/categorical_accumulator.hpp"

#include <unistd.h>

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <ranges>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

namespace analyser::metric_accumulator::metric_accumulator_impl {

    void CategoricalAccumulator::Accumulate(const metric::MetricResult& metric_result) {
        if (is_finalized) return;
        
        std::string category = std::visit([](const auto& value) -> std::string {
            if constexpr (std::is_same_v<std::decay_t<decltype(value)>, int>) {
                return std::to_string(value);
            } else if constexpr (std::is_same_v<std::decay_t<decltype(value)>, std::string>) {
                return value;
            }
            return "";
        }, metric_result.value);
        
        if (!category.empty()) {
            categories_freq[category]++;
        }
    }

    void CategoricalAccumulator::Finalize() {
        is_finalized = true;
    }

    void CategoricalAccumulator::Reset() {
        categories_freq.clear();
        is_finalized = false;
    }

    const std::unordered_map<std::string, int>& CategoricalAccumulator::Get() const {
        return categories_freq;
    }

}  // namespace analyser::metric_accumulator::metric_accumulator_impl
