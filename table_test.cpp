#include <iostream>
#include <vector>
#include <string>

#include "tableGen.hpp" 

class TableTest {
public:
    TableTest() {
        std::cout << "Starting table test...\n\n";
    }

    ~TableTest() {
        std::cout << "\nStopping table test...\n";
    }

    void run() {
        std::vector<std::string> headers = {"Name", "Birthday", "Tags", "Address"};

        std::vector<std::vector<std::string>> rows = {
            {"Karl Kangaroo", "13. Sep 1988", "jumping", "Outback"},
            {"Austin Ape",    "24. Jul 2000", "climbing", "Jungle"},
            {"Bertha Bear",   "3. Feb 1976",  "sleeping", "Sherwood Forest"},
            {"Toni Tiger",    "31. Jan 1935", "hunting",  "Bengal"},
            {"Paul Penguin",  "6. Oct 1954",  "swimming", "Antarctica"},
            {"Gira Giraffe",  "10. Sep 1943", "eating",   "London Zoo"}
        };

        simpleTable table;
        table.setHeaders(headers);

        for (const auto& row : rows) {
            table.setRows(row);
        }

        table.printTable(); 
    }
};

int main() {
    TableTest test;
    test.run();
    return 0;
}