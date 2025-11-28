C++ REST API Server with PostgreSQL

Description:
This project is a C++ REST API server connected to a PostgreSQL database.
Supports POST requests for insert, update, delete operations
Secured with API Key / JWT authentication
Returns JSON responses
Easily extensible for production use

 Features
REST endpoints:
/insert
/update
/delete
/health
Database: PostgreSQL
Authentication: API Key (Bearer) / JW
JSON handling: nlohmann/json
HTTP server & client: cpp-httplib
Secure database operations: uses quote() / prepared statements recommended for production


Prerequisites
```
C++22 compatible compiler (g++, clang)
CMake ≥ 3.10
PostgreSQL server + libpqxx development libraries
Optional: Docker (for testing)
```
Ubuntu example:
```
sudo apt update
sudo apt install -y build-essential cmake libpq-dev libpqxx-dev
```
Setup

Create a PostgreSQL database and user:
```
CREATE DATABASE test;
CREATE USER demo WITH PASSWORD 'demo';
GRANT ALL PRIVILEGES ON DATABASE test TO demo;
```

Apply the schema:
```
psql -h 127.0.0.1 -U demo -d test -f sql/schema.sql
```

Set your API Key / JWT secret in server.cpp:
```
const std::string API_KEY = "MY_SECRET_API_KEY";
```

Build & run:
```
mkdir build && cd build
cmake ..
make -j
./server

```
