#include "metric_accumulator_impl/sum_average_accumulator.hpp"

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

    void SumAverageAccumulator::Accumulate(const metric::MetricResult& metric_result) {
        if (is_finalized) return;
        
        std::visit([this](const auto& value) {
            if constexpr (std::is_same_v<std::decay_t<decltype(value)>, int>) {
                sum += value;
                count++;
            }
        }, metric_result.value);
    }

    void SumAverageAccumulator::Finalize() {
        if (count > 0) {
            average = static_cast<double>(sum) / count;
        }
        is_finalized = true;
    }

    void SumAverageAccumulator::Reset() {
        sum = 0;
        count = 0;
        average = 0;
        is_finalized = false;
    }

    SumAverageAccumulator::SumAverage SumAverageAccumulator::Get() const {
        return {sum, average};
    }
}  // namespace analyser::metric_accumulator::metric_accumulator_impl
