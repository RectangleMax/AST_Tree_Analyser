#include <unistd.h>

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <print>
#include <ranges>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

#include "file.hpp"
#include "function.hpp"
#include "metric.hpp"
#include "metric_accumulator.hpp"

namespace analyser {

namespace rv = std::ranges::views;
namespace rs = std::ranges;

auto AnalyseFunctions(const std::vector<std::string>& files,
                      const analyser::metric::MetricExtractor& metric_extractor) {

    using ResultType = std::vector<std::pair<function::Function, metric::MetricResults>>;
    
    function::FunctionExtractor function_extractor;
    
    auto process_file = [&](const std::string& filename) -> ResultType {
        try {
            file::File file(filename);
            auto functions = function_extractor.Get(file);
            
            return functions 
                | rv::transform([&](const function::Function& func) {
                    auto metrics = metric_extractor.Get(func);
                    return std::make_pair(func, std::move(metrics));
                })
                | rs::to<std::vector>();
                
        } catch (const std::exception& e) {
            std::cerr << "Error processing file " << filename << ": " << e.what() << std::endl;
            return {};
        }
    };
    
    return files
        | rv::transform(process_file)
        | rv::join
        | rs::to<std::vector>();
}

auto SplitByClasses(const auto& analysis) {
    return analysis
        | rv::filter([](const auto& item) {
            const auto& [func, metrics] = item;
            return func.class_name.has_value();
        })
        | rv::chunk_by([](const auto& a, const auto& b) {
            return a.first.class_name == b.first.class_name;
        })
        | rv::transform([](auto&& group) {
            return group | rs::to<std::vector>();
        })
        | rs::to<std::vector>();
}

auto SplitByFiles(const auto& analysis) {
    return analysis
        | rv::chunk_by([](const auto& a, const auto& b) {
            return a.first.filename == b.first.filename;
        })
        | rv::transform([](auto&& group) {
            return group | rs::to<std::vector>();
        })
        | rs::to<std::vector>();
}

void AccumulateFunctionAnalysis(
    const auto& analysis, analyser::metric_accumulator::MetricsAccumulator& accumulator) {
    
    accumulator.ResetAccumulators();
    
    // Агрегируем все метрики
    auto all_metrics = analysis
        | rv::transform([](const auto& item) { return item.second; })
        | rv::join;
    
    rs::for_each(all_metrics, [&](const auto& metric_result) {
        accumulator.AccumulateNextFunctionResults({metric_result});
    });
    
    // Финаллизируем все аккумуляторы через публичный интерфейс
    auto accumulators_range = accumulator.GetAccumulatorsView();
    rs::for_each(accumulators_range, [](auto& acc) { 
        acc->Finalize(); 
    });
}

} // namespace analyser
