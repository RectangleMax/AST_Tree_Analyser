#pragma once
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

#include "metric.hpp"

namespace analyser::metric::metric_impl {

struct CyclomaticComplexityMetric final : IMetric {
protected:
    MetricResult::ValueType CalculateImpl(const function::Function& f) const override {
        const std::vector<std::string> patterns = {
            "if", "elif", "else", "while", "for", "try", 
            "except", "finally", "match", "case", "assert"
        };
        
        auto pattern_counts = patterns | rv::transform([&](const std::string& pattern) {
            return CountPattern(f.ast, pattern);
        });
        
        int base_complexity = 1 + CountTernaryOperators(f.ast);
        int patterns_complexity = rs::fold_left(pattern_counts, 0, std::plus{});
        
        return base_complexity + patterns_complexity;
    }

    std::string Name() const override {
        return "cyclomatic_complexity";
    }

private:
    int CountPattern(const std::string& ast, const std::string& pattern) const {
        std::string_view ast_view(ast);
        std::string_view pattern_view(pattern);
        
        auto range = ast_view 
            | rv::split(pattern_view) 
            | rv::drop(1)
            | rv::transform([](auto&&) { return 1; });
            
        return rs::fold_left(range, 0, std::plus{});
    }
    
    int CountTernaryOperators(const std::string& ast) const {
        std::string_view ast_view(ast);
        std::string_view search_pattern(" if ");
        
        auto range = ast_view
            | rv::split(search_pattern)
            | rv::transform([](auto&& chunk) {
                std::string_view chunk_view(chunk.begin(), chunk.end());
                return chunk_view.find("else") != std::string_view::npos ? 1 : 0;
            });
            
        return rs::fold_left(range, 0, std::plus{});
    }
};

} // namespace analyser::metric::metric_impl
