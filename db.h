#ifndef DB_H
#define DB_H
#include "DataBase.h"
#include "Parser.h"

std::string makeLower(std::string& s) {
    for (auto& c : s) {
        c = std::tolower(c);
    }
    return s;
}

std::variant<int32_t, bool, std::string, bytes> convertTo(std::string& s, std::string& a) {
    try {
        if (s == "int32") {
            return std::stoi(a);
        }
        else if (s == "bool") {
            if (a == "true") {
                return true;
            }
            else{ return false;}
        }
        else if (s == "string") {
            return a;
        }
        else if (s == "bytes") {
            return convertStringToBytes(s);
        }
        throw std::invalid_argument("Invalid type");
    }
    catch (std::invalid_argument& e) {
        std::cout << e.what() << std::endl;
    }

}

void makeBase(Restriction& r) {
    r.atribute[0] = 0;
    r.atribute[1] = 0;
    r.atribute[2] = 0;
    r.value = {0, 0};
    r.type.second = 0;
}


class db {
private:
    DataBase database;
    Table* createTable(std::vector<Token>& tokens) {
        std::string name = tokens[2].getTokenValue();
        Restriction r;
        std::vector<Restriction> restrictions;
        int i = 4;
        while (i < tokens.size() && tokens[i].getTokenValue() != ")") {
            makeBase(r);
            if (tokens[i].getTokenValue() == "{") {
                i++;
                bool flag = false;
                while (lower(tokens[i].getTokenValue()) != "}") {
                    flag = false;
                    if (tokens[i].getTokenValue() == "unique") {
                        r.atribute[0] = 1;
                        flag = true;
                    }
                    else if (lower(tokens[i].getTokenValue()) == "autoincrement") {
                        r.atribute[1] = 1;
                        flag = true;
                    }
                    else if (lower(tokens[i].getTokenValue()) == "key") {
                        r.atribute[2] = 1;
                        flag = true;
                    }
                    if (tokens[i+1].getTokenValue() == ",") {
                        i += 1;
                    }
                    i+=1;
                    if (flag != true) {
                        throw std::invalid_argument("Wrong syntaxis");
                    }
                }
                i++;
            }
            r.name = tokens[i].getTokenValue();
            i+= 2;
            if (tokens[i].getTokenValue() == "bytes") {
                if (stoi(tokens[i+2].getTokenValue()) == -1) {
                    throw std::invalid_argument("Wrong syntaxis");
                }
                r.type = std::make_pair(byte, stoi(tokens[i+2].getTokenValue()));
                i += 4;
            }
            else if (tokens[i].getTokenValue() == "string") {
                if (stoi(tokens[i+2].getTokenValue()) == -1) {
                    throw std::invalid_argument("Wrong syntaxis");
                }
                r.type = std::make_pair(string, stoi(tokens[i+2].getTokenValue()));
                i += 4;
            }
            else if (tokens[i].getTokenValue() == "int32") {
                r.type = {int_32, 0};
                i += 1;
            }
            else if (tokens[i].getTokenValue() == "bool") {
                r.type = {boolean, 0};
                i += 1;
            }
            if (i < tokens.size() && tokens[i].getTokenValue() == "=") {
                if (i + 1 < tokens.size()) {
                    switch (r.type.first) {
                        case int_32:
                            r.value = {1, stoi(tokens[i+1].getTokenValue())};
                            break;
                        case string:
                            r.value = {1, tokens[i+1].getTokenValue()};
                            break;
                        case boolean:
                            if (tokens[i+1].getTokenValue() == "true") {
                                r.value = {1, true};
                            }
                            else {
                                r.value = {1, false};
                            }
                            break;
                        case byte:
                            r.value = {1, convertStringToBytes(tokens[i+1].getTokenValue())};
                            break;
                        default:
                            throw std::invalid_argument("Wrong syntaxis");
                    }
                    i += 2;
                    if (tokens[i + 1].getTokenValue() == ",") {i++;}
                }
                else {
                    throw std::invalid_argument("Wrong syntaxis");
                }
            }
            else {
                if (tokens[i].getTokenValue() == ")") {
                    restrictions.push_back(r);
                    break;
                }
            }
            restrictions.push_back(r);
            i += 1;
        }
        database.CreateTable(name, restrictions);
    }

    Table* insert(std::vector<Token>& tokens) {
        if (tokens[tokens.size() - 5].getTokenValue() == "=") {
            std::string name = tokens[tokens.size() - 1].getTokenValue();
            std::vector<std::pair<std::string, std::variant<int32_t, bool, std::string, bytes>>> data;
            int i = 2;
            while (i < tokens.size() - 3) {
                data.push_back({tokens[i].getTokenValue(), tokens[i+2].getTokenValue()});
                i += 4;
            }
            database.Insert(name, data);
        }
        else {
            std::vector<std::pair<int, std::variant<int32_t, bool, std::string, bytes>>> data;
            std::string name = tokens[tokens.size() - 1].getTokenValue();
            int i = 2;
            while (i < tokens.size() - 3) {
                if (tokens[i].getTokenValue() == ",") {
                    data.push_back({0, 0});
                    i++;
                }
                else {
                    data.push_back({1, tokens[i].getTokenValue()});
                    i+=2;
                }
            }
            database.Insert(name, data);
        }
    }

    Table* deleteExpression(std::vector<Token>& tokens) {}

    Table* select(std::vector<Token>& tokens) {
        std::vector<std::string> names;
        int i = 1;
        while (i < tokens.size()) {
            if (tokens[i].getTokenValue() == "from") {
                break;
            }
            else if(tokens[i].getTokenValue() != ",") {
                names.push_back(tokens[i].getTokenValue());
            }
            i += 1;
        }
        std::string name = tokens[i + 1].getTokenValue();
        Table* table = database.findTable(name);
        Node* root = MakeAST(tokens, i+3, tokens.size());
        std::vector<Restriction> restrictions = table->getRestriction();
        std::vector<Statement> statements = table->getStatements();
        std::vector<Restriction> newRestrictions;
        std::vector<Statement> newStatements;
        for (int j = 0; j < statements.size(); j++) {
            std::variant<int32_t, bool, std::string, bytes> q;
            q = CalculateValue(restrictions, statements[j], root);
            if (std::get<bool>(q)) {
                newStatements.push_back(statements[j]);
            }
        }
        for (int j = restrictions.size() - 1; j >= 0 ; j--) {
            bool found = false;
            for (auto& nname : names) {
                if (nname == restrictions[j].name) {
                    if (!found) {
                        newRestrictions.push_back(restrictions[j]);
                        found = true;
                    }
                }
            }
            if (!found) {
                for (auto& re : newStatements ) {
                    re.data.erase(re.data.begin() + j);
                }
            }

        }
        std::reverse(newRestrictions.begin(), newRestrictions.end());
        Table* newTable = new Table(newRestrictions, newStatements);
        return newTable;
    }

    Table* join(std::vector<Token>& tokens) {}

    Table* update(std::vector<Token>& tokens) {}

    Table* createIndex(std::vector<Token>& tokens) {}
public:
    db() = default;
    Table* execute(std::string& data) {
        std::vector<Token> tokens;
        makeListOfTokens(data, tokens);
        try {
            if (tokens[0].getTokeName() == KWORD) {
                if (tokens[0].getTokenValue() == "createtable") {
                    Table* q = createTable(tokens);
                    if (q == nullptr) {
                        throw std::invalid_argument("Table creation failed");
                    }
                    return q;
                }
                if (tokens[0].getTokenValue() == "insert") {
                    Table* q = insert(tokens);
                    if (q == nullptr) {
                        throw std::invalid_argument("Insert failed");
                    }
                    return q;
                }
                if (tokens[0].getTokenValue() == "delete") {
                    Table* q = deleteExpression(tokens);
                    if (q == nullptr) {
                        throw std::invalid_argument("Delete failed");
                    }
                    return q;
                }
                if (tokens[0].getTokenValue() == "update") {
                    Table* q = update(tokens);
                    if (q == nullptr) {
                        throw std::invalid_argument("Udpade failed");
                    }
                    return q;
                }
                if (tokens[0].getTokenValue() == "select") {
                    Table* q = select(tokens);
                    if (q == nullptr) {
                        throw std::invalid_argument("Select failed");
                    }
                    return q;
                }
                if (tokens[0].getTokenValue() == "join") {
                    Table* q = join(tokens);
                    if (q == nullptr) {
                        throw std::invalid_argument("Join failed");
                    }
                    return q;
                }
                if (tokens[0].getTokenValue() == "create" && tokens[2].getTokenValue() == "index") {
                    Table* q = update(tokens);
                    if (q == nullptr) {
                        throw std::invalid_argument("Creation index failed");
                    }
                    return q;
                }
                throw std::invalid_argument("Invalid syntax");
            }
            else {
                throw std::invalid_argument("Invalid type of keyword argument");
            }
        }
        catch (std::invalid_argument& e) {
            std::cout << e.what() << std::endl;
        }
    }

    void addTable(Table* table) {
        database.addTable(*table);
    }
};
#endif //DB_H
