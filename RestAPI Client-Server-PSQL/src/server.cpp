

#include <iostream>
#include <string>
#include <vector>
#include <chrono>

#include "httplib.h"      // put into third_party/httplib.h
#include "json.hpp"       // put into third_party/json.hpp

#include <pqxx/pqxx>      // libpqxx

using json = nlohmann::json;

struct DBConfig {
    std::string conninfo; // e.g. "host=127.0.0.1 port=5432 dbname=test user=postgres password=secret"
};

class PostgresDB {
public:
    PostgresDB(const DBConfig& cfg) : cfg_(cfg), conn_(cfg.conninfo) {
        if (!conn_.is_open()) {
            throw std::runtime_error("Cannot open database connection");
        }
    }

    // Insert: table name + object of column->value
    // Returns inserted primary key if serial, or rows affected
    json insert(const std::string& table, const json& values) {
        pqxx::work txn(conn_);
        // Build column list and quoted values (use conn_.quote to escape)
        std::vector<std::string> cols;
        std::vector<std::string> vals;
        for (auto it = values.begin(); it != values.end(); ++it) {
            cols.push_back(it.key());
            vals.push_back(conn_.quote(it.value().dump(0))); // quote JSON representation safe
        }
        // Create SQL; note: we use literal quoting - for production prefer prepared statements
        std::string col_part;
        std::string val_part;
        for (size_t i = 0; i < cols.size(); ++i) {
            if (i) { col_part += ", "; val_part += ", "; }
            col_part += cols[i];
            val_part += vals[i];
        }

        // Use RETURNING id if id column exists; user should ensure table has serial PK named id
        std::string sql = "INSERT INTO " + table + " (" + col_part + ") VALUES (" + val_part + ") RETURNING id;";
        try {
            pqxx::result r = txn.exec(sql);
            txn.commit();
            if (!r.empty()) {
                return json{{"success", true}, {"id", r[0][0].as<long long>()}};
            } else {
                return json{{"success", true}, {"rows", 1}};
            }
        } catch (const std::exception &e) {
            txn.abort();
            return json{{"success", false}, {"error", e.what()}};
        }
    }

    // Update: table, values (object), where (string, e.g. "id = 5")
    json update(const std::string& table, const json& values, const std::string& where_clause) {
        pqxx::work txn(conn_);
        std::string set_part;
        bool first = true;
        for (auto it = values.begin(); it != values.end(); ++it) {
            if (!first) set_part += ", ";
            first = false;
            set_part += it.key();
            set_part += " = ";
            set_part += conn_.quote(it.value().dump(0));
        }
        std::string sql = "UPDATE " + table + " SET " + set_part + " WHERE " + where_clause + ";";
        try {
            pqxx::result r = txn.exec(sql);
            long rows = r.affected_rows();
            txn.commit();
            return json{{"success", true}, {"rows_affected", rows}};
        } catch (const std::exception &e) {
            txn.abort();
            return json{{"success", false}, {"error", e.what()}};
        }
    }

    // Delete: table, where clause
    json remove(const std::string& table, const std::string& where_clause) {
        pqxx::work txn(conn_);
        std::string sql = "DELETE FROM " + table + " WHERE " + where_clause + ";";
        try {
            pqxx::result r = txn.exec(sql);
            long rows = r.affected_rows();
            txn.commit();
            return json{{"success", true}, {"rows_affected", rows}};
        } catch (const std::exception &e) {
            txn.abort();
            return json{{"success", false}, {"error", e.what()}};
        }
    }

private:
    DBConfig cfg_;
    pqxx::connection conn_;
};

int main() {
    // Example connection string - replace with your settings or env var
    DBConfig cfg;
    cfg.conninfo = "host=127.0.0.1 port=5432 dbname=test user=postgres password=postgres";

    PostgresDB db(cfg);

    httplib::Server svr;

    // Health check
    svr.Get("/health", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(R"({"status":"ok"})", "application/json");
    });

    // Insert endpoint
    svr.Post("/insert", [&db](const httplib::Request& req, httplib::Response& res) {
        try {
            auto j = json::parse(req.body);
            std::string table = j.at("table").get<std::string>();
            json values = j.at("values");
            json out = db.insert(table, values);
            res.set_content(out.dump(), "application/json");
        } catch (const std::exception &e) {
            json out = {{"success", false}, {"error", e.what()}};
            res.status = 400;
            res.set_content(out.dump(), "application/json");
        }
    });

    // Update endpoint
    svr.Post("/update", [&db](const httplib::Request& req, httplib::Response& res) {
        try {
            auto j = json::parse(req.body);
            std::string table = j.at("table").get<std::string>();
            json values = j.at("values");
            std::string where = j.at("where").get<std::string>(); // e.g. "id = 5"
            json out = db.update(table, values, where);
            res.set_content(out.dump(), "application/json");
        } catch (const std::exception &e) {
            json out = {{"success", false}, {"error", e.what()}};
            res.status = 400;
            res.set_content(out.dump(), "application/json");
        }
    });

    // Delete endpoint
    svr.Post("/delete", [&db](const httplib::Request& req, httplib::Response& res) {
        try {
            auto j = json::parse(req.body);
            std::string table = j.at("table").get<std::string>();
            std::string where = j.at("where").get<std::string>();
            json out = db.remove(table, where);
            res.set_content(out.dump(), "application/json");
        } catch (const std::exception &e) {
            json out = {{"success", false}, {"error", e.what()}};
            res.status = 400;
            res.set_content(out.dump(), "application/json");
        }
    });

    std::cout << "Server listening on http://0.0.0.0:8080\n";
    svr.listen("0.0.0.0", 8080);

    return 0;
}
