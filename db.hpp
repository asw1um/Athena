#pragma once
#include <iomanip>
#include <sqlite3.h>
#include <string>
#include <optional>

enum class del_type {Author,ISBN, Title};

enum class filter_type { None, Author, Category, ISBN };

struct book_update {
    std::optional<std::string> isbn;
    std::optional<std::string> title;
    std::optional<std::string> author;
    std::optional<int> priceCents;
    std::optional<std::string> link;
    std::optional<std::string> category;
};


bool init_database(sqlite3* db);
int get_or_create_id(sqlite3*db, const std::string& tableName, const std::string& colName, const std::string &search);
bool insert_book(sqlite3* db, const std::string &isbn, const std::string &title, const std::string &author, int price, const std::string &link, const std::string &category);
bool delete_book_by_id(sqlite3* db, int id);
bool delete_isbn(sqlite3* db, const std::string& isbn) ;
int delete_batch(sqlite3* db, del_type type, const std::string & value);
bool display_books(sqlite3* db, filter_type filter = filter_type::None, const std::string& value = "");
bool update_book_fields(sqlite3* db, int id, const book_update& updates);
