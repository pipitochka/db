
#ifndef DATABASE_H
#define DATABASE_H
#include <iostream>
#include <vector>
#include <sstream>
#include <string>


class bytes {
public:
    std::vector<unsigned char> _bytes;

    bytes(std::vector<unsigned char> &bytes) {
        _bytes = bytes;
    };

    int size() const {
        return _bytes.size();
    }

    bool operator<=> (const bytes &other) const { return true; }

    bool operator==(const bytes & get) const{};

    bool operator!=(const bytes & get) const{};
};

bytes convertStringToBytes(std::string& str) {
    std::vector<unsigned char> byte;
    try {
        for (int i = 0; i < str.size(); i++) {
            if ((str[i] <= '9' && str[i] >= '0') || (str[i] <= 'f' && str[i] >= 'a')) {
                byte.push_back(str[i]);
            }
            else {
                throw std::invalid_argument("Invalid character");
            }
        }
        return bytes(byte);
    }
    catch (std::invalid_argument& e) {
        std::cout << e.what() << std::endl;
    }
};

bytes bytesFromStringToBytes(std::variant<int32_t, bool, std::string, bytes> qwe) {
    return convertStringToBytes(std::get<std::string>(qwe));
}

struct Restriction {
    std::string name;
    std::pair<std::string, int> type;
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
        switch (restrictions[index].type.first[0]) {
            case 'i':
                return std::holds_alternative<int32_t>(data);
            case 's':
                return std::holds_alternative<std::string>(data) && (std::get<std::string>(data).size() <= restrictions[index].type.second);
            case 'b':
                switch (restrictions[index].type.first[1]) {
                    case 'o':
                        return std::holds_alternative<bool>(data);
                    case 'y':
                        return std::holds_alternative<bytes>(data);
                    default:
                        break;
                }
                break;
        }
        return false;
    }

    bool checkTable(Statement &statement) {
        try {
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
        catch (std::invalid_argument &e) {
            std::cout << e.what() << std::endl;
            return false;
        }
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

    void addStatement(std::vector<std::pair<int, std::variant<int32_t, bool, std::string, bytes>>> &statement) {
        try {
            Statement newStatement;
            for (size_t i = 0; i < restrictions.size(); i++) {
                if (i < statement.size() && statement[i].first) {
                    switch (restrictions[i].type.first[0]) {
                        case 'i':
                            newStatement.data.push_back(std::get<int>(statement[i].second));
                        break;
                        case 's':
                            newStatement.data.push_back(std::get<std::string>(statement[i].second));
                        break;
                        case 'b':
                            switch (restrictions[i].type.first[1]) {
                                case 'y':
                                    newStatement.data.push_back(bytesFromStringToBytes(statement[i].second));
                                break;
                                case 'o':
                                    if ((std::get<std::string>(statement[i].second) == "true")) {
                                        newStatement.data.push_back(true);
                                    }
                                    else {
                                        newStatement.data.push_back(false);
                                    }
                                break;
                                default:
                                    throw std::invalid_argument("Bad type col" + std::to_string(i));
                            }
                        break;
                        default: throw std::invalid_argument("Bad type col" + std::to_string(i));
                    }}
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
        catch (std::invalid_argument& e) {
            std::cout << e.what() << std::endl;
        }
    }

    void addStatement(std::vector<std::pair<std::string, std::variant<int32_t, bool, std::string, bytes>>> &statement) {
        try {
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
                    switch (restrictions[i].type.first[0]) {
                        case 'i':
                            newStatement.data[i] = (std::get<int>(statement[x].second));
                        break;
                        case 's':
                            newStatement.data[i] = (std::get<std::string>(statement[x].second));
                        break;
                        case 'b':
                            switch (restrictions[i].type.first[1]) {
                                case 'y':
                                    newStatement.data[i] = bytesFromStringToBytes(statement[x].second);
                                    break;
                                case 'o':
                                    if ((std::get<std::string>(statement[x].second) == "true")) {
                                        newStatement.data[i] = true;
                                    }
                                    else {
                                        newStatement.data[i] = false;
                                    }
                                    break;
                                default:
                                    throw std::invalid_argument("Bad type col" + std::to_string(i));
                            }
                        break;
                        default:
                            throw std::invalid_argument("Bad type col" + std::to_string(i));
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
        catch (std::invalid_argument& e) {
            std::cout << e.what() << std::endl;
        }
    }

    void deleteStatement(int i) {
        statements.erase(statements.begin() + i);
    }
};

class DataBase {
private:
    std::vector<Table> tables;
    Table* findTable(const std::string& name) {
        for (auto& table : tables) {
            if (table.getName() == name) {
                return &table;
            }
        }
        return nullptr;
    }

public:
    DataBase()= default;

    Table* CreateTable(const std::string &name, std::vector<Restriction> &restrictions) {
        try {
            Table* ptr = findTable(name);
            if (ptr == nullptr) {
                Table table = Table(name, restrictions);
                tables.push_back(table);
                return &tables[tables.size() - 1];
            }
            else {
                throw std::invalid_argument("Table already exists");
            }
        } catch (std::invalid_argument& e) {
            std::cout << e.what() << std::endl;;
        }
    }

    Table* Insert(std::string &name, std::vector<std::pair<int, std::variant<int32_t, bool, std::string, bytes>>> &statements) {
        try {
            Table* q = findTable(name);
            if (q == nullptr) {
                throw std::invalid_argument("No table");
            }
            else {
                q->addStatement(statements);
                return q;
            }
        }
        catch (std::invalid_argument& e) {
            std::cout << e.what() << std::endl;
        }
    }

    Table* Insert(std::string &name, std::vector<std::pair<std::string, std::variant<int32_t, bool, std::string, bytes>>> &statements) {
        try {
            Table* q = findTable(name);
            if (q == nullptr) {
                throw std::invalid_argument("No table");
            }
            else {
                q->addStatement(statements);
                return q;
            }
        }
        catch (std::invalid_argument& e) {
            std::cout << e.what() << std::endl;
        }
    }

    ~DataBase()= default;

    void addTable(Table& table) {
        tables.push_back(table);
    }
};




#endif //DATABASE_H
