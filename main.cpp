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

#include "analyse.hpp"
#include "cmd_options.hpp"
#include "file.hpp"
#include "function.hpp"
#include "metric.hpp"
#include "metric_accumulator.hpp"
#include "metric_accumulator_impl/accumulators.hpp"
#include "metric_impl/metrics.hpp"

// int main(int argc, char *argv[]) {
//     analyser::cmd::ProgramOptions options;
//     // распарсите входные параметры

//     // analyser::metric::MetricExtractor metric_extractor;
//     // зарегистрируйте метрики в metric_extractor

//     // запустите analyser::AnalyseFunctions
//     // выведете результаты анализа на консоль

//     // analyser::metric_accumulator::MetricsAccumulator accumulator;
//     // зарегистрируйте аккумуляторы метрик в accumulator

//     // запустите analyser::SplitByFiles
//     // запустите analyser::AccumulateFunctionAnalysis для каждого подмножества результатов метрик
//     // выведете результаты на консоль

//     // запустите analyser::SplitByClasses
//     // запустите analyser::AccumulateFunctionAnalysis для каждого подмножества результатов метрик
//     // выведете результаты на консоль

//     // запустите analyser::AccumulateFunctionAnalysis для всех результатов метрик
//     // выведете результаты на консоль

//     return 0;
// }

void PrintResults(const auto& analysis) {
    rs::for_each(analysis, [](const auto& item) {
        const auto& [func, metrics] = item;
        std::cout << "Function: " << func.name;
        if (func.class_name) {
            std::cout << " (Class: " << *func.class_name << ")";
        }
        std::cout << " [File: " << func.filename << "]\n";

        // Вложенный ranges::for_each для metrics
        rs::for_each(metrics, [](const auto& metric) {
            std::cout << "  " << metric.metric_name << ": "
                      << std::get<int>(metric.value) << "\n";
        });
        
        std::cout << std::endl;
    });
}

void PrintAccumulatedResults(const analyser::metric_accumulator::MetricsAccumulator& accumulator, 
                           const std::string& context = "Overall") {
    std::cout << "=== Aggregated Metrics for " << context << " ===\n";
    
    auto print_if_exists = [&]<typename T>(const std::string& metric_name, auto printer) {
        try {
            auto& acc = accumulator.GetFinalizedAccumulator<T>(metric_name);
            printer(acc);
        } catch (const std::exception& e) {
            if (std::string(e.what()).find("not found") == std::string::npos) {
                std::cerr << "Error: " << e.what() << std::endl;
            }
        }
    };
    
    print_if_exists.template operator()<analyser::metric_accumulator::metric_accumulator_impl::SumAverageAccumulator>(
        "cyclomatic_complexity", 
        [](const auto& acc) {
            auto result = acc.Get();
            std::cout << "Cyclomatic Complexity - Sum: " << result.sum 
                      << ", Average: " << result.average << "\n";
        });
    
    print_if_exists.template operator()<analyser::metric_accumulator::metric_accumulator_impl::SumAverageAccumulator>(
        "code_lines_count",
        [](const auto& acc) {
            auto result = acc.Get();
            std::cout << "Code Lines - Sum: " << result.sum 
                      << ", Average: " << result.average << "\n";
        });
    
    print_if_exists.template operator()<analyser::metric_accumulator::metric_accumulator_impl::AverageAccumulator>(
        "parameter_count",
        [](const auto& acc) {
            std::cout << "Parameter Count - Average: " << acc.Get() << "\n";
        });
    
    std::cout << std::endl;
}

int main(int argc, char *argv[]) {
    // if (argc < 2) {
    //     std::cerr << "Usage: " << argv[0] << " [options] <file1.py> [file2.py ...]\n";
    //     return 1;
    // }
    
    analyser::cmd::ProgramOptions options;
    options.Parse(argc, argv);
    
    // if (options.files.empty()) {
    //     std::cerr << "Error: No input files specified\n";
    //     return 1;
    // }
    
    // Регистрируем метрики
    analyser::metric::MetricExtractor metric_extractor;
    metric_extractor.RegisterMetric(
        std::make_unique<analyser::metric::metric_impl::CodeLinesCountMetric>()
    );
    metric_extractor.RegisterMetric(
        std::make_unique<analyser::metric::metric_impl::CyclomaticComplexityMetric>()
    );
    metric_extractor.RegisterMetric(
        std::make_unique<analyser::metric::metric_impl::CountParametersMetric>()
    );
    
    // Анализ функций
    auto analysis = analyser::AnalyseFunctions(options.GetFiles(), metric_extractor);
    
    // Вывод индивидуальных результатов
    // if (options.show_individual) {
        std::cout << "=== Individual Function Metrics ===\n";
        PrintResults(analysis);
    // }
    
    // Регистрируем аккумуляторы
    analyser::metric_accumulator::MetricsAccumulator accumulator;
    accumulator.RegisterAccumulator<analyser::metric_accumulator::metric_accumulator_impl::SumAverageAccumulator>(
        "cyclomatic_complexity",
        std::make_unique<analyser::metric_accumulator::metric_accumulator_impl::SumAverageAccumulator>()
    );
    accumulator.RegisterAccumulator<analyser::metric_accumulator::metric_accumulator_impl::SumAverageAccumulator>(
        "code_lines_count", 
        std::make_unique<analyser::metric_accumulator::metric_accumulator_impl::SumAverageAccumulator>()
    );
    accumulator.RegisterAccumulator<analyser::metric_accumulator::metric_accumulator_impl::AverageAccumulator>(
        "parameter_count",
        std::make_unique<analyser::metric_accumulator::metric_accumulator_impl::AverageAccumulator>()
    );
    
    // Агрегация по различным критериям
    auto process_groups = [&](const auto& groups, const std::string& type) {
        rs::for_each(groups, [&](const auto& group) {
            if (!group.empty()) {
                analyser::AccumulateFunctionAnalysis(group, accumulator);
                auto context = type + ": " + (
                    type == "File" ? group.front().first.filename :
                    type == "Class" ? *group.front().first.class_name : "All"
                );
                PrintAccumulatedResults(accumulator, context);
            }
        });
    };
        
    // if (options.aggregate_by_file) {
        auto by_files = analyser::SplitByFiles(analysis);
        process_groups(by_files, "File");
    // }
    
    // if (options.aggregate_by_class) {
        auto by_classes = analyser::SplitByClasses(analysis);
        process_groups(by_classes, "Class");
    // }
    
    // Общая агрегация
    analyser::AccumulateFunctionAnalysis(analysis, accumulator);
    PrintAccumulatedResults(accumulator, "All Functions");
    
    return 0;
}