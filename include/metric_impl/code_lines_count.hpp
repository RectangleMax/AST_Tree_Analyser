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

struct CodeLinesCountMetric final : IMetric {
protected:
    MetricResult::ValueType CalculateImpl(const function::Function& f) const override {
        auto lines = GetFunctionLines(f);
        
        bool in_multiline_comment = false;
        
        auto valid_lines = lines | rv::transform([&](const std::string& line) {
            return ProcessLine(line, in_multiline_comment);
        }) | rv::filter([](const std::string& line) {
            return !line.empty() && line[0] != '#';
        });
        
        return static_cast<int>(rs::distance(valid_lines));
    }

    std::string Name() const override {
        return "code_lines_count";
    }

private:
    std::vector<std::string> GetFunctionLines(const function::Function& f) const {
        std::vector<std::string> lines;
        std::istringstream iss(f.ast);
        std::string line;
        
        while (std::getline(iss, line)) {
            lines.push_back(line);
        }
        
        return lines;
    }
    
    std::string ProcessLine(std::string line, bool& in_multiline_comment) const {
        line = Trim(line);
        
        if (in_multiline_comment) {
            auto end_pos = FindMultilineCommentEnd(line);
            if (end_pos != std::string::npos) {
                in_multiline_comment = false;
                line = line.substr(end_pos + 3);
            } else {
                return "";
            }
        }
        
        if (!in_multiline_comment) {
            auto start_pos = FindMultilineCommentStart(line);
            if (start_pos != std::string::npos) {
                in_multiline_comment = true;
                line = line.substr(0, start_pos);
            }
        }
        
        return line;
    }
    
    size_t FindMultilineCommentStart(const std::string& line) const {
        auto triple_single = line.find("'''");
        auto triple_double = line.find("\"\"\"");
        return std::min(triple_single, triple_double);
    }
    
    size_t FindMultilineCommentEnd(const std::string& line) const {
        auto triple_single = line.find("'''");
        auto triple_double = line.find("\"\"\"");
        return std::min(triple_single, triple_double);
    }
    
    std::string Trim(const std::string& str) const {
        auto start = str | rv::drop_while([](char c) { return std::isspace(c); });
        auto end = str | rv::reverse | rv::drop_while([](char c) { return std::isspace(c); });
        return {start.begin(), end.begin().base()};
    }
};


} // namespace analyser::metric::metric_impl
