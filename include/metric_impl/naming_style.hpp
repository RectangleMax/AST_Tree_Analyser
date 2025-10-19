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
#include <regex>

#include "metric.hpp"

namespace analyser::metric::metric_impl {

struct NamingStyleMetric final : IMetric {
protected:
    MetricResult::ValueType CalculateImpl(const function::Function& f) const override {
        // Простая проверка стиля именования функций
        // 0 - snake_case, 1 - camelCase, 2 - другие
        if (f.name.empty()) return 2;
        
        // Проверяем snake_case (только строчные буквы, цифры и подчеркивания)
        if (std::regex_match(f.name, std::regex("^[a-z][a-z0-9_]*$"))) {
            return 0;
        }
        
        // Проверяем camelCase
        if (std::regex_match(f.name, std::regex("^[a-z][a-zA-Z0-9]*$")) && 
            f.name.find('_') == std::string::npos) {
            return 1;
        }
        
        return 2;
    }

    std::string Name() const override {
        return "naming_style";
    }
};

} // namespace analyser::metric::metric_impl
