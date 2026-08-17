#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <sqlite3.h>
#include <optional>
#include <cmath>
#include "db.hpp"

int price_to_cent(double dollars) {
    return static_cast<int>(dollars * 100.0 + 0.5);
}

void print_help() {
    std::cout << "Athena Commands:\n"
              << "  athena init\n"
              << "  athena add <isbn> <title> <author> <price_dollars> <link> <category>\n"
              << "  athena edit, -ed, update <id> [flags]\n"
              << "      -i, --isbn <isbn>\n"
              << "      -t, --title <title>\n"
              << "      -a, --author <author>\n"
              << "      -p, --price <price>\n"
              << "      -l, --link <link>\n"
              << "      -c, --category <category>\n"
              << "  athena -rm, delete -id <id>\n"
              << "  athena -rm, delete -is <isbn>\n"
              << "  athena -rm, delete -b <-a|-is|-t> <value>\n"
              << "  athena -ls, list, ls [-a <author> | -c <category> | -is <isbn>]\n";
}

int main(int argc, char* argv[]) {
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    if (argc < 2) {
        std::cerr << "[Error] No command provided.\n";
        print_help();
        return EXIT_FAILURE;
    }

    // Open database connection
    sqlite3* db = nullptr;
    if (sqlite3_open("library.db", &db) != SQLITE_OK) {
        std::cerr << "[DB Error] Failed to open database: " << sqlite3_errmsg(db) << "\n";
        std::cerr<< "Double check if database is initalized";
        return EXIT_FAILURE;
    }

    std::string command = argv[1];
    int exit_code = EXIT_SUCCESS;

    if (command == "init") {
        if (!init_database(db)) {
            exit_code = EXIT_FAILURE;
        } else {
            std::cout << "[SUCCESS] Database schema initialized.\n";
        }
    } 
    else if (command == "add") {
        if (argc < 8) {
            std::cerr << "Usage: ./library add <isbn> <title> <author> <price_dollars> <link> <category>\n";
            sqlite3_close(db);
            return EXIT_FAILURE;
        }

        std::string isbn = argv[2];
        std::string title = argv[3];
        std::string author = argv[4];
        double price_dollars = std::stod(argv[5]);
        std::string link = argv[6];
        std::string category = argv[7];

        int price_cents = price_to_cent(price_dollars);

        if (!insert_book(db, isbn, title, author, price_cents, link, category)) {
            exit_code = EXIT_FAILURE;
        } else {
            std::cout << "[SUCCESS] Added '" << title << "' to library.\n";
        }
    } 
    else if (command == "delete" || command == "-rm") {
        if (argc < 4) {
            std::cerr << "[Error] Invalid delete syntax.\n";
            print_help();
            sqlite3_close(db);
            return EXIT_FAILURE;
        }

        std::string subCmd = argv[2];

        if (subCmd == "-id") {
            int id = std::stoi(argv[3]);
            if (!delete_book_by_id(db, id)) {
                exit_code = EXIT_FAILURE;
            }
        } 
        else if (subCmd == "-is" || subCmd == "-isbn") {
            std::string isbn = argv[3];
            if (!delete_isbn(db, isbn)) {
                exit_code = EXIT_FAILURE;
            }
        } 
        else if (subCmd == "-b" || subCmd == "-batch") {
            if (argc < 5) {
                std::cerr << "Usage: ./library delete -b <-a|-is|-t> <value>\n";
                sqlite3_close(db);
                return EXIT_FAILURE;
            }

            std::string targetFlag = argv[3];
            std::string value = argv[4];
            del_type target;

            if (targetFlag == "-a" || targetFlag == "--author") {
                target = del_type::Author;
            } else if (targetFlag == "-is" || targetFlag == "-i" || targetFlag == "--isbn") {
                target = del_type::ISBN;
            } else if (targetFlag == "-t" || targetFlag == "--title") {
                target = del_type::Title;
            } else {
                std::cerr << "[Error] Unknown batch target flag: " << targetFlag << "\n";
                sqlite3_close(db);
                return EXIT_FAILURE;
            }

            int count = delete_batch(db, target, value);
            std::cout << "[SUCCESS] Batch deleted " << count << " record(s).\n";
        } 
        else {
            std::cerr << "[Error] Unknown delete flag: " << subCmd << "\n";
            print_help();
            exit_code = EXIT_FAILURE;
        }
    } 
    else if (command == "-h" || command == "-help"){
        print_help();
    }
    else if (command == "list" || command == "ls" || command == "-ls") {
            filter_type filter = filter_type::None;
            std::string filterValue = "";

            if (argc >= 4) {
                std::string flag = argv[2];
                filterValue = argv[3];

                if (flag == "-a" || flag == "--author") {
                    filter = filter_type::Author;
                } else if (flag == "-c" || flag == "--category") {
                    filter = filter_type::Category;
                } else if (flag == "-is" || flag == "--isbn") {
                    filter = filter_type::ISBN;
                } else {
                    std::cerr << "[Error] Unknown filter flag: " << flag << "\n";
                    std::cerr << "Usage: ./library -ls [-a <author> | -c <category> | -is <isbn>]\n";
                    sqlite3_close(db);
                    return EXIT_FAILURE;
                }
            }

            if (!display_books(db, filter, filterValue)) {
                exit_code = EXIT_FAILURE;
            }
        }
    else if (command == "edit" || command == "update" || command == "-ed") {
        if (argc < 4) {
            std::cerr << "Usage: library edit <id> [flags]\n";
            std::cerr << "Flags:\n";
            std::cerr << "  -i, --isbn <isbn>\n";
            std::cerr << "  -t, --title <title>\n";
            std::cerr << "  -a, --author <author>\n";
            std::cerr << "  -p, --price <price>\n";
            std::cerr << "  -l, --link <link>\n";
            std::cerr << "  -c, --category <category>\n";
            sqlite3_close(db);
            return EXIT_FAILURE;
        }

        int id = 0;
        try {
            id = std::stoi(argv[2]);
        } catch (...) {
            std::cerr << "[Error] Invalid book ID format: " << argv[2] << "\n";
            sqlite3_close(db);
            return EXIT_FAILURE;
        }

        book_update updates;

        for (int i = 3; i < argc; i += 2) {
            if (i + 1 >= argc) {
                std::cerr << "[Error] Missing value for flag: " << argv[i] << "\n";
                sqlite3_close(db);
                return EXIT_FAILURE;
            }

            std::string flag = argv[i];
            std::string val  = argv[i + 1];

            if (flag == "-i" || flag == "--isbn")         updates.isbn = val;
            else if (flag == "-t" || flag == "--title")    updates.title = val;
            else if (flag == "-a" || flag == "--author")   updates.author = val;
            else if (flag == "-c" || flag == "--category") updates.category = val;
            else if (flag == "-l" || flag == "--link")     updates.link = val;
            else if (flag == "-p" || flag == "--price") {
                try {
                    double p = std::stod(val);
                    updates.priceCents = static_cast<int>(std::round(p * 100));
                } catch (...) {
                    std::cerr << "[Error] Invalid price format: " << val << "\n";
                    sqlite3_close(db);
                    return EXIT_FAILURE;
                }
            } else {
                std::cerr << "[Error] Unknown edit flag: " << flag << "\n";
                sqlite3_close(db);
                return EXIT_FAILURE;
            }
        }

        if (!update_book_fields(db, id, updates)) {
            exit_code = EXIT_FAILURE;
        }
    }
    else {
        std::cerr << "[Error] Unknown command: " << command << "\n";
        print_help();
        exit_code = EXIT_FAILURE;
    }

    sqlite3_close(db);
    return exit_code;
}

