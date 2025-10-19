#pragma once
#include <unistd.h>

#include <algorithm>
#include <any>
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

#include "metric.hpp"

namespace rv = std::ranges::views;
namespace rs = std::ranges;

namespace analyser::metric_accumulator {

struct IAccumulator {
    virtual void Accumulate(const metric::MetricResult& metric_result) = 0;
    virtual void Finalize() = 0;
    virtual void Reset() = 0;
    virtual ~IAccumulator() = default;

    bool IsFinalized() const { return is_finalized; }
protected:
    bool is_finalized = false;
};

struct MetricsAccumulator {
    template <typename Accumulator>
    void RegisterAccumulator(const std::string& metric_name, std::unique_ptr<Accumulator> acc) {
        accumulators[metric_name] = std::move(acc);
    }

    template <typename Accumulator>
    const Accumulator& GetFinalizedAccumulator(const std::string& metric_name) const {
        auto it = accumulators.find(metric_name);
        if (it == accumulators.end()) {
            throw std::runtime_error("Accumulator for metric " + metric_name + " not found");
        }
        
        auto* accumulator = dynamic_cast<Accumulator*>(it->second.get());
        if (!accumulator) {
            throw std::runtime_error("Invalid accumulator type for metric " + metric_name);
        }
        
        if (!accumulator->IsFinalized()) {
            throw std::runtime_error("Accumulator for metric " + metric_name + " not finalized");
        }
        
        return *accumulator;
    }

    void AccumulateNextFunctionResults(const std::vector<metric::MetricResult>& metric_results) const;

    void ResetAccumulators();

    // Добавлен публичный метод для доступа к аккумуляторам
    auto GetAccumulatorsView() const {
        return accumulators | rv::values;
    }

private:
    std::unordered_map<std::string, std::shared_ptr<IAccumulator>> accumulators;
};

} // namespace analyser::metric_accumulator
