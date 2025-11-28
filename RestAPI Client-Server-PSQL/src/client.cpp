
#include <iostream>
#include "httplib.h"
#include "json.hpp"

using json = nlohmann::json;

int main() {
    httplib::Client cli("http://localhost:8080");

    // Example: Insert
    json insert_req = {
        {"table", "items"},
        {"values", {{"name", "apple"}, {"qty", 42}}}
    };

    auto res = cli.Post("/insert", insert_req.dump(), "application/json");
    if (res && res->status == 200) {
        std::cout << "Insert response: " << res->body << "\n";
    } else {
        std::cerr << "Insert failed\n";
    }

    // Example: Update
    json upd_req = {
        {"table", "items"},
        {"values", {{"qty", 10}}},
        {"where", "name = 'apple'"}
    };
    res = cli.Post("/update", upd_req.dump(), "application/json");
    if (res && res->status == 200) {
        std::cout << "Update response: " << res->body << "\n";
    } else {
        std::cerr << "Update failed\n";
    }

    // Example: Delete
    json del_req = {
        {"table", "items"},
        {"where", "name = 'apple'"}
    };
    res = cli.Post("/delete", del_req.dump(), "application/json");
    if (res && res->status == 200) {
        std::cout << "Delete response: " << res->body << "\n";
    } else {
        std::cerr << "Delete failed\n";
    }

    return 0;
}
