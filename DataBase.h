#ifndef DATABASE_H
#define DATABASE_H

#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include <variant>
#include "bytes.h"

enum types {int_32, byte, string, boolean};

struct Restriction {
    std::string name;
    std::pair<types, int> type;
    std::pair<bool, std::variant<int32_t, bool, std::string, bytes>> value;
    int atribute[3];
};

int getNumberOfRestrictions(std::vector<Restriction> &restrictions, std::string name) {
    for (int i = 0; i < restrictions.size(); i++) {
        if (restrictions[i].name == name) {
            return i;
        }
    }
    return -1;
};

struct Statement {
    std::vector<std::variant<int32_t, bool, std::string, bytes>> data;
};

class Table {
    std::vector<Statement> statements;
    std::vector<Restriction> restrictions;
    std::string name;

    bool checkUnique(const size_t index, const std::variant<int32_t, bool, std::string, bytes>& data) const {
        for (const auto & statement : statements) {
            if (statement.data[index] == data) {
                return false;
            }
        }
        return true;
    }

    bool checkType(int index, std::variant<int32_t, bool, std::string, bytes>& data) {
        switch (restrictions[index].type.first) {
            case int_32:
                return std::holds_alternative<int32_t>(data);
            case string:
                return std::holds_alternative<std::string>(data) && (std::get<std::string>(data).size() <= restrictions[index].type.second);
            case byte:
                return std::holds_alternative<bytes>(data) && (std::get<bytes>(data).size() <= restrictions[index].type.second);
            case boolean:
                return std::holds_alternative<bool>(data);
        }
        return false;
    }

    bool checkTable(Statement &statement) {
    for (size_t i = 0; i < restrictions.size(); i++) {
        if (!checkType(i, statement.data[i])) {
            throw std::invalid_argument("Bad type col" + std::to_string(i));
        }
        if (restrictions[i].atribute[0] || restrictions[i].atribute[2]) {
            if (!checkUnique(i, statement.data[i])) {
                throw std::invalid_argument("Non unique col" + std::to_string(i));
            }
        }
    }
    return true;

    }

    int getNumber(std::vector<std::pair<std::string, std::variant<int32_t, bool, std::string, bytes>>> &statement, std::string name) {
        for (int i = 0; i < statement.size(); i++) {
            if (statement[i].first == name) {
                return i;
            }
        }
        return -1;
    }

public:
    std::string getName() {
        return name;
    }

    Table(const std::string &_name, const std::vector<Restriction>& _restrictions) {
        restrictions = _restrictions;
        name = _name;
    }

    Table(const std::vector<Restriction>& _restrictions, const std::vector<Statement>& _statements) {
        restrictions = _restrictions;
        statements = _statements;
    }

    void addStatement(std::vector<std::pair<int, std::variant<int32_t, bool, std::string, bytes>>> &statement) {
    Statement newStatement;
    for (size_t i = 0; i < restrictions.size(); i++) {
        if (i < statement.size() && statement[i].first) {
            switch (restrictions[i].type.first) {
                case int_32:
                    newStatement.data.push_back(std::get<int>(statement[i].second));
                    break;
                case string:
                    newStatement.data.push_back(std::get<std::string>(statement[i].second));
                    break;
                case byte:
                    newStatement.data.push_back(std::get<bytes>(statement[i].second));
                    break;
                case boolean:
                    newStatement.data.push_back(std::get<bool>(statement[i].second));
                    break;
                default: throw std::invalid_argument("Bad type col" + std::to_string(i));
            }
        }
        else {
            if (restrictions[i].atribute[1]) {
                if (statements.size() > 0) {
                    newStatement.data.push_back(std::get<int>(statements[statements.size() - 1].data[i]) + 1);
                }
                else {
                    newStatement.data.push_back(int32_t(0));
                }
            }
            else if (restrictions[i].value.first) {
                newStatement.data.push_back(restrictions[i].value.second);
            }
            else {
                throw std::invalid_argument("No describtion of argument" + std::to_string(i));
            }
        }
    }
    if (checkTable(newStatement)) {
        statements.push_back(newStatement);
    }
    else {
        throw std::invalid_argument("Bad string");
    }
    }

    void addStatement(std::vector<std::pair<std::string, std::variant<int32_t, bool, std::string, bytes>>> &statement) {
        Statement newStatement;
        newStatement.data.resize(restrictions.size());
        for (size_t i = 0; i < restrictions.size(); i++) {
            int x;
            x = getNumber(statement, restrictions[i].name);
            if (x == -1) {
                if (restrictions[i].value.first) {
                    newStatement.data[i] = restrictions[i].value.second;
                }
                else if (restrictions[i].atribute[1]) {
                    if (!statements.empty()) {
                        newStatement.data[i] = std::get<int>((statements[statements.size() - 1].data[i])) + 1;
                    }
                    else {
                        newStatement.data[i] = 0;
                    }
                }
                else {throw std::invalid_argument("Not describe coloum" + std::to_string(i));}
            }
            else {
                switch (restrictions[i].type.first) {
                    case int_32:
                        newStatement.data[i] = (std::stoi(std::get<std::string>(statement[x].second)));
                        break;
                    case string:
                        newStatement.data[i] = (std::get<std::string>(statement[x].second));
                        break;
                    case byte:
                        newStatement.data[i] = (bytesFromStringToBytes(statement[x].second));
                        break;
                    case boolean:
                        if (std::get<std::string>(statement[x].second) == "true") {
                            newStatement.data[i] = true;
                        }
                        else if (std::get<std::string>(statement[x].second) == "false") {
                            newStatement.data[i] = false;
                        }
                        break;
                    default: throw std::invalid_argument("Bad type col" + std::to_string(i));
                }
            }
        }
        if (checkTable(newStatement)) {
            statements.push_back(newStatement);
        }
        else {
            throw std::invalid_argument("Bad string");
        }
    }

    void deleteStatement(int i) {
        statements.erase(statements.begin() + i);
    }

    std::vector<Statement>& getStatements() {
        return statements;
    }
    std::vector<Restriction>& getRestriction() {
        return restrictions;
    }

    void rename(std::string& newName) {
        name = newName;
    }
};

class DataBase {
private:
    std::vector<Table> tables;


public:
    DataBase()= default;

    Table* findTable(const std::string& name) {
        for (auto& table : tables) {
            if (table.getName() == name) {
                return &table;
            }
        }
        return nullptr;
    }

    Table* CreateTable(const std::string &name, std::vector<Restriction> &restrictions) {
        Table* ptr = findTable(name);
        if (ptr == nullptr) {
            Table table = Table(name, restrictions);
            tables.push_back(table);
            return &tables[tables.size() - 1];
        }
        else {
            throw std::invalid_argument("Table already exists");
        }
    }

    Table* CreateTable(Table* table) {
        Table* ptr = findTable(table->getName());
        if (ptr == nullptr) {
            tables.push_back(*table);
        }
        else {
            throw std::invalid_argument("Table already exists");
        }
    }

    Table* Insert(std::string &name, std::vector<std::pair<int, std::variant<int32_t, bool, std::string, bytes>>> &statements) {
        Table* q = findTable(name);
        if (q == nullptr) {
            throw std::invalid_argument("No table");
        }
        else {
            q->addStatement(statements);
            return q;
        }
    }

    Table* Insert(std::string &name, std::vector<std::pair<std::string, std::variant<int32_t, bool, std::string, bytes>>> &statements) {
        Table* q = findTable(name);
        if (q == nullptr) {
            throw std::invalid_argument("No table");
        }
        else {
            q->addStatement(statements);
            return q;
        }
    }

    ~DataBase()= default;

    void addTable(Table& table) {
        tables.push_back(table);
    }
};




#endif //DATABASE_H
