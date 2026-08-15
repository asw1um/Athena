#pragma once
#include <vector>
#include <string>

class simpleTable {
private:
    std::vector<std::string> headers;
    std::vector<std::vector<std::string>> rows;
    size_t padding = 1;

    std::vector<size_t> columnWidth() const;
    void printBorder(const std::vector<size_t>& widths) const;
    void printRow(const std::vector<std::string>& row, const std::vector<size_t> &widths) const;
    template <typename T>
    std::string formatCell(const T& val) const {
        if constexpr (std::is_same_v<T, std::string>) {
            return val;
        } else if constexpr (std::is_same_v<T, const char*>) {
            return std::string(val);
        } else if constexpr (std::is_floating_point_v<T>) {
            std::ostringstream ss;
            ss << std::fixed << std::setprecision(2) << val;
            return ss.str();
        } else {
            return std::to_string(val);
        }
    }

public:
    simpleTable() = default;
    void setHeaders(const std::vector<std::string>& h);
    void setRows(const std::vector<std::string>& row);
    void printTable() const;

    template <typename... Args>
    void addRow(const Args&... args) {
        std::vector<std::string> formattedRow = { formatCell(args)... };
        rows.push_back(formattedRow);
    }
};