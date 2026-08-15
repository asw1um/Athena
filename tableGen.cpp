


#include <iostream>
#include <iomanip>
#include <algorithm>
#include "tableGen.hpp"


std::vector<size_t> simpleTable::columnWidth() const{
    std::vector<size_t> widths(headers.size(), 0);
    for (size_t col = 0; col<headers.size(); ++col){
        widths[col] = headers[col].length();
    }

    for (const auto& i:rows){
        for(size_t col = 0; col<i.size() && col<widths.size(); ++col){
            widths[col] = std::max(widths[col], i[col].length());
        }
    }

    return widths;
}

void simpleTable::printBorder(const std::vector<size_t> &widths) const{
    std::cout<<"+";
    for (size_t i :widths){
        std::cout << std::string(i+(padding*2), '-')<<"+";
    }
    std::cout<<"\n";
}

void simpleTable::printRow(const std::vector<std::string>& row, const std::vector<size_t> &widths) const{
    std::cout<<"|";
    for(size_t col = 0; col<widths.size(); ++col){
        std::string txt = (col<row.size()) ? row[col]:"";
        std::cout << std::string(padding, ' ') << std::left <<std::setw(widths[col]) << txt << std::string(padding, ' ')<<"|";
    }
    std::cout<<"\n";

}


void simpleTable::setHeaders(const std::vector<std::string>& h){
    headers = h;
}

void simpleTable::setRows(const std::vector<std::string> &row){
    rows.push_back(row);
}

void simpleTable::printTable() const{
    if(headers.empty()) return;
    std::vector<size_t> widths = columnWidth();

    printBorder(widths);
    printRow(headers, widths);
    printBorder(widths);

    for (const auto&row: rows){
        printRow(row, widths);
    }

    printBorder(widths);
}

