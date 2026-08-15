#include <sqlite3.h>
#include <string>
#include "db.hpp"
#include <iostream>
#include <limits>
#include "tableGen.hpp"

static std::string cents_to_dollars(int cents) {
    std::ostringstream ss;
    ss << "$" << (cents / 100) << "." 
       << std::setw(2) << std::setfill('0') << (cents % 100);
    return ss.str();
}

bool init_database(sqlite3* db){
    const char* sql = R"sql(
        PRAGMA foreign_keys = ON;

        CREATE TABLE IF NOT EXISTS authors(
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            author_name TEXT NOT NULL

        );
        
        CREATE TABLE IF NOT EXISTS categories (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            cat_name TEXT UNIQUE NOT NULL
            
        ); 

        
        CREATE TABLE IF NOT EXISTS books (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            book_name TEXT NOT NULL,
            author_id INTEGER NOT NULL,
            isbn TEXT NOT NULL,
            category_id INTEGER NOT NULL,
            price INTEGER NOT NULL,
            link TEXT NOT NULL,

            FOREIGN KEY (category_id) REFERENCES categories(id) ON DELETE CASCADE,
            FOREIGN KEY (author_id) REFERENCES authors(id) ON DELETE CASCADE
        );




    )sql";

    char* err_msg = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &err_msg) != SQLITE_OK) {
        std::cerr << "[DB Error] Schema initialization failed: " << err_msg << "\n";
        sqlite3_free(err_msg);
        return false;
    }
    return true;
}

int get_or_create_id(sqlite3*db, const std::string& tableName, const std::string& colName, const std::string &search){
    std::string selectSql = "SELECT id FROM "+tableName + " WHERE " + colName + " = ? COLLATE NOCASE;";
    sqlite3_stmt* stmt = nullptr;
    int id = -1;

    if(sqlite3_prepare_v2(db, selectSql.c_str(), -1, &stmt, nullptr) == SQLITE_OK){
        sqlite3_bind_text(stmt,1, search.c_str(),-1,SQLITE_TRANSIENT);
        if(sqlite3_step(stmt)==SQLITE_ROW){
            id = sqlite3_column_int(stmt,0);
        }
    }
    sqlite3_finalize(stmt);

    if(id!=-1) return id;

    std::string insertSql = "INSERT INTO "+tableName + " (" + colName + ") VALUES (?);";
    if(sqlite3_prepare_v2(db, insertSql.c_str(),-1,&stmt, nullptr)==SQLITE_OK){
        sqlite3_bind_text(stmt, 1, search.c_str(), -1, SQLITE_TRANSIENT);
        if(sqlite3_step(stmt) == SQLITE_DONE){
            id = static_cast<int>(sqlite3_last_insert_rowid(db));
        }
    }
    sqlite3_finalize(stmt);

    return id;
}

bool insert_book(sqlite3* db, const std::string &isbn, const std::string &title, const std::string &author, int price, const std::string &link, const std::string &category){
    int author_id = get_or_create_id(db, "authors", "author_name", author);
    int category_id = get_or_create_id(db, "categories", "cat_name", category);

    if(author_id == -1 || category_id == -1){
        std::cerr<<"Smth went wrong with the author or category ids \n";
        return false;
    }

    std::string checkSql = "SELECT book_name, author_id FROM books WHERE isbn = ?;";
    sqlite3_stmt* checkStmt = nullptr;

    if(sqlite3_prepare_v2(db, checkSql.c_str(), -1, &checkStmt, nullptr) == SQLITE_OK){
        sqlite3_bind_text(checkStmt, 1, isbn.c_str(), -1, SQLITE_TRANSIENT);

        if(sqlite3_step(checkStmt) == SQLITE_ROW){
            const unsigned char* textAddr = sqlite3_column_text(checkStmt, 0);
            std::string existingTitle = textAddr ? reinterpret_cast<const char*>(textAddr) : "";
            int existingAuthorId = sqlite3_column_int(checkStmt, 1);

            if(existingTitle != title || existingAuthorId != author_id) {
                std::cerr<<"[Rejected] ISBN "<<isbn<<" is already tied to ->" << existingTitle <<". cant have same isbn but diff title\n";
                sqlite3_finalize(checkStmt);
                return false;
            }
        }
    }

    sqlite3_finalize(checkStmt);
    const char* insertSql = R"sql(
        INSERT INTO books (book_name, author_id, category_id, isbn, price, link) 
        VALUES (?,?,?,?,?,?);
    )sql";

    sqlite3_stmt* stmt = nullptr;
    if(sqlite3_prepare_v2(db, insertSql,-1, &stmt, nullptr) != SQLITE_OK){
        std::cerr<<"failed to insert statement: "<<sqlite3_errmsg(db)<<"\n";
        return false;   
    }

    sqlite3_bind_text(stmt, 1, title.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt,2, author_id);
    sqlite3_bind_int(stmt, 3, category_id);
    sqlite3_bind_text(stmt, 4, isbn.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 5, price);
    sqlite3_bind_text(stmt, 6, link.c_str(), -1, SQLITE_TRANSIENT);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    if(!success){
        std::cerr<<"Insert failed: "<<sqlite3_errmsg(db)<<"\n";
    }

    sqlite3_finalize(stmt);
    return success;
}

bool delete_book_by_id(sqlite3* db, int id) {
    const char* sql = "DELETE FROM books WHERE id = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_int(stmt, 1, id);
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);

    if (success && sqlite3_changes(db) > 0) {
        std::cout << "[SUCCESS] Removed listing #" << id << ".\n";
    } else {
        std::cout << "[NOTICE] No listing found with ID " << id << ".\n";
    }

    sqlite3_finalize(stmt);
    return success;
}

bool delete_isbn(sqlite3* db, const std::string& isbn) {
    std::string countSql = "SELECT COUNT(*) FROM books WHERE isbn = ?;";
    sqlite3_stmt* stmt;
    int count = 0;

    if (sqlite3_prepare_v2(db, countSql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, isbn.c_str(), -1, SQLITE_TRANSIENT);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }
    }
    sqlite3_finalize(stmt);

    if (count == 0) {
        std::cout << "[NOTICE] No records found for ISBN " << isbn << ".\n";
        return false;
    }

    std::cout << "[WARNING] Found " << count << " listing(s) for ISBN " << isbn << ". Delete all? (y/N): ";
    char choice;
    std::cin >> choice;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    if (choice != 'y' && choice != 'Y') {
        std::cout << "[CANCELLED] Deletion aborted.\n";
        return false;
    }

    const char* deleteSql = "DELETE FROM books WHERE isbn = ?;";
    if (sqlite3_prepare_v2(db, deleteSql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, isbn.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        std::cout << "[SUCCESS] Deleted all " << count << " listing(s).\n";
        return true;
    }

    return false;
}

int delete_batch(sqlite3* db, del_type target, const std::string& value) {
    std::string sql;

    switch (target) {
        case del_type::Author:
            sql = "DELETE FROM authors WHERE author_name = ?;";;
            break;

        case del_type::ISBN:
            sql = "DELETE FROM books WHERE isbn = ?;";
            break;

        case del_type::Title:
            sql = "DELETE FROM books WHERE book_name = ?;";
            break;
    }

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "[DB Error] Failed to prepare batch delete: " << sqlite3_errmsg(db) << "\n";
        return 0;
    }

    sqlite3_bind_text(stmt, 1, value.c_str(), -1, SQLITE_TRANSIENT);

    int rowsDeleted = 0;
    if (sqlite3_step(stmt) == SQLITE_DONE) {
        rowsDeleted = sqlite3_changes(db);
    } else {
        std::cerr << "[DB Error] Deletion execution failed: " << sqlite3_errmsg(db) << "\n";
    }

    sqlite3_finalize(stmt);
    return rowsDeleted;
}

bool display_books(sqlite3* db, filter_type filter, const std::string& value) {
    std::string sql = R"sql(
        SELECT b.id, b.book_name, a.author_name, c.cat_name, b.isbn, b.price, b.link
        FROM books b
        JOIN authors a ON b.author_id = a.id
        JOIN categories c ON b.category_id = c.id
    )sql";

    switch (filter) {
        case filter_type::Author:
            sql += " WHERE a.author_name = ? COLLATE NOCASE";
            break;
        case filter_type::Category:
            sql += " WHERE c.cat_name = ? COLLATE NOCASE";
            break;
        case filter_type::ISBN:
            sql += " WHERE b.isbn = ?";
            break;
        case filter_type::None:
            break;
    }

    sql += " ORDER BY b.id ASC;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "[DB Error] Failed to prepare list query: " << sqlite3_errmsg(db) << "\n";
        return false;
    }

    if (filter != filter_type::None) {
        sqlite3_bind_text(stmt, 1, value.c_str(), -1, SQLITE_TRANSIENT);
    }

    simpleTable table;
    table.setHeaders({"ID", "Title", "Author", "Category", "ISBN", "Price", "Link"});

    int rowCount = 0;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);

        const unsigned char* t_title  = sqlite3_column_text(stmt, 1);
        const unsigned char* t_author = sqlite3_column_text(stmt, 2);
        const unsigned char* t_cat    = sqlite3_column_text(stmt, 3);
        const unsigned char* t_isbn   = sqlite3_column_text(stmt, 4);
        int priceCents                = sqlite3_column_int(stmt, 5);
        const unsigned char* t_link   = sqlite3_column_text(stmt, 6);

        std::string title    = t_title  ? reinterpret_cast<const char*>(t_title)  : "";
        std::string author   = t_author ? reinterpret_cast<const char*>(t_author) : "";
        std::string category = t_cat    ? reinterpret_cast<const char*>(t_cat)    : "";
        std::string isbn     = t_isbn   ? reinterpret_cast<const char*>(t_isbn)   : "";
        std::string price    = cents_to_dollars(priceCents);
        std::string link     = t_link   ? reinterpret_cast<const char*>(t_link)   : "";

        table.setRows({
            std::to_string(id),
            title,
            author,
            category,
            isbn,
            price,
            link
        });

        rowCount++;
    }

    sqlite3_finalize(stmt);

    if (rowCount == 0) {
        std::cout << "[NOTICE] No matching records found.\n";
    } else {
        table.printTable();
    }

    return true;
}

bool update_book_fields(sqlite3* db, int id, const book_update& updates) {
    if (!updates.isbn && !updates.title && !updates.author &&
        !updates.priceCents && !updates.link && !updates.category) {
        std::cout << "[NOTICE] No fields provided to update.\n";
        return true;
    }

    std::vector<std::string> clauses;

    int author_id = -1;
    if (updates.author) {
        author_id = get_or_create_id(db, "authors", "author_name", *updates.author);
        if (author_id == -1) return false;
        clauses.push_back("author_id = ?");
    }

    int cat_id = -1;
    if (updates.category) {
        cat_id = get_or_create_id(db, "categories", "cat_name", *updates.category);
        if (cat_id == -1) return false;
        clauses.push_back("category_id = ?");
    }

    if (updates.isbn)       clauses.push_back("isbn = ?");
    if (updates.title)      clauses.push_back("book_name = ?");
    if (updates.priceCents) clauses.push_back("price = ?");
    if (updates.link)       clauses.push_back("link = ?");


    std::string sql = "UPDATE books SET ";
    for (size_t i = 0; i < clauses.size(); ++i) {
        sql += clauses[i];
        if (i + 1 < clauses.size()) sql += ", ";
    }
    sql += " WHERE id = ?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "[DB Error] Failed to prepare UPDATE statement: " << sqlite3_errmsg(db) << "\n";
        return false;
    }

    int bindIdx = 1;

    if (updates.author) {
        sqlite3_bind_int(stmt, bindIdx++, author_id);
    }
    if (updates.category) {
        sqlite3_bind_int(stmt, bindIdx++, cat_id);
    }
    if (updates.isbn) {
        sqlite3_bind_text(stmt, bindIdx++, updates.isbn->c_str(), -1, SQLITE_TRANSIENT);
    }
    if (updates.title) {
        sqlite3_bind_text(stmt, bindIdx++, updates.title->c_str(), -1, SQLITE_TRANSIENT);
    }
    if (updates.priceCents) {
        sqlite3_bind_int(stmt, bindIdx++, *updates.priceCents);
    }
    if (updates.link) {
        sqlite3_bind_text(stmt, bindIdx++, updates.link->c_str(), -1, SQLITE_TRANSIENT);
    }

    sqlite3_bind_int(stmt, bindIdx, id);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "[DB Error] Failed to update record: " << sqlite3_errmsg(db) << "\n";
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);

    if (sqlite3_changes(db) == 0) {
        std::cerr << "[Error] No book found with ID " << id << ".\n";
        return false;
    }

    std::cout << "[SUCCESS] Book ID " << id << " updated successfully.\n";
    return true;
}