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

struct CountParametersMetric final: public IMetric {
protected:
    MetricResult::ValueType CalculateImpl(const function::Function& f) const override {
        auto def_pos = f.ast.find("def " + f.name);
        if (def_pos == std::string::npos) return 0;
        
        auto paren_pos = f.ast.find('(', def_pos);
        if (paren_pos == std::string::npos) return 0;
        
        auto end_paren_pos = f.ast.find(')', paren_pos);
        if (end_paren_pos == std::string::npos) return 0;
        
        std::string params_str = f.ast.substr(paren_pos + 1, end_paren_pos - paren_pos - 1);
        return CountParameters(params_str);
    }

    std::string Name() const override {
        return "parameter_count";
    }

private:
    int CountParameters(const std::string& params_str) const {
        auto params = params_str 
            | rv::split(',')
            | rv::transform([this](auto&& chunk) {
                std::string param(chunk.begin(), chunk.end());
                return ProcessParameter(param);
            })
            | rv::filter([](const std::string& param) {
                return !param.empty() && param != "self" && param != "cls";
            });
        
        return static_cast<int>(rs::distance(params));
    }
    
    std::string ProcessParameter(const std::string& param) const {
        auto clean_param = param 
            | rv::take_while([](char c) { return c != '='; })
            | rs::to<std::string>();
        
        return Trim(clean_param);
    }
    
    std::string Trim(const std::string& str) const {
        auto start = str | rv::drop_while([](char c) { return std::isspace(c); });
        auto end = str | rv::reverse | rv::drop_while([](char c) { return std::isspace(c); });
        return {start.begin(), end.begin().base()};
    }
};

}; // namespace analyser::metric::metric_impl
