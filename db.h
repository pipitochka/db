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
        std::string name = tokens[2].value;
        Restriction r;
        std::vector<Restriction> restrictions;
        int i = 4;
        while (i < tokens.size() && tokens[i].value != ")") {
            makeBase(r);
            if (tokens[i].value == "{") {
                i++;
                bool flag = false;
                while (lower(tokens[i].value) != "}") {
                    flag = false;
                    if (tokens[i].value == "unique") {
                        r.atribute[0] = 1;
                        flag = true;
                    }
                    else if (lower(tokens[i].value) == "autoincrement") {
                        r.atribute[1] = 1;
                        flag = true;
                    }
                    else if (lower(tokens[i].value) == "key") {
                        r.atribute[2] = 1;
                        flag = true;
                    }
                    if (tokens[i+1].value == ",") {
                        i += 1;
                    }
                    i+=1;
                    if (flag != true) {
                        throw std::invalid_argument("Wrong syntaxis");
                    }
                }
                i++;
            }
            r.name = tokens[i].value;
            i+= 2;
            if (tokens[i].value == "bytes") {
                if (stoi(tokens[i+2].value) == -1) {
                    throw std::invalid_argument("Wrong syntaxis");
                }
                r.type = std::make_pair(byte, stoi(tokens[i+2].value));
                i += 4;
            }
            else if (tokens[i].value == "string") {
                if (stoi(tokens[i+2].value) == -1) {
                    throw std::invalid_argument("Wrong syntaxis");
                }
                r.type = std::make_pair(string, stoi(tokens[i+2].value));
                i += 4;
            }
            else if (tokens[i].value == "int32") {
                r.type = {int_32, 0};
                i += 1;
            }
            else if (tokens[i].value == "bool") {
                r.type = {boolean, 0};
                i += 1;
            }
            if (i < tokens.size() && tokens[i].value == "=") {
                if (i + 1 < tokens.size()) {
                    switch (r.type.first) {
                        case int_32:
                            r.value = {1, stoi(tokens[i+1].value)};
                            break;
                        case string:
                            r.value = {1, tokens[i+1].value};
                            break;
                        case boolean:
                            if (tokens[i+1].value == "true") {
                                r.value = {1, true};
                            }
                            else {
                                r.value = {1, false};
                            }
                            break;
                        case byte:
                            r.value = {1, convertStringToBytes(tokens[i+1].value)};
                            break;
                        default:
                            throw std::invalid_argument("Wrong syntaxis");
                    }
                    i += 2;
                    if (tokens[i + 1].value == ",") {i++;}
                }
                else {
                    throw std::invalid_argument("Wrong syntaxis");
                }
            }
            else {
                if (tokens[i].value == ")") {
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
        if (tokens[tokens.size() - 5].value == "=") {
            std::string name = tokens[tokens.size() - 1].value;
            std::vector<std::pair<std::string, std::variant<int32_t, bool, std::string, bytes>>> data;
            int i = 2;
            while (i < tokens.size() - 3) {
                data.push_back({tokens[i].value, tokens[i+2].value});
                i += 4;
            }
            database.Insert(name, data);
        }
        else {
            std::vector<std::pair<int, std::variant<int32_t, bool, std::string, bytes>>> data;
            std::string name = tokens[tokens.size() - 1].value;
            int i = 2;
            while (i < tokens.size() - 3) {
                if (tokens[i].value == ",") {
                    data.push_back({0, 0});
                    i++;
                }
                else {
                    data.push_back({1, tokens[i].value});
                    i+=2;
                }
            }
            database.Insert(name, data);
        }
    }

    Table* deleteExpression(Node* left, Table* table) {
        std::vector<Restriction> restrictions = table->getRestriction();
        std::vector<Statement> statements = table->getStatements();
        for (int i = statements.size() - 1; i>= 0; i--) {
            std::variant<int32_t, bool, std::string, bytes> q;
            q = CalculateValue(restrictions, statements[i], left);
            if (std::get<bool>(q)) {
                statements.erase(statements.begin() + i);
            }
        }
        table->changeStatement(statements);
        return table;
    }

    Table* select(Node* left, Node* right, Table* table) {
        std::vector<Restriction> restrictions = table->getRestriction();
        std::vector<Statement> statements = table->getStatements();
        std::vector<Restriction> newRestrictions;
        std::vector<Statement> newStatements;
        for (int j = 0; j < statements.size(); j++) {
            std::variant<int32_t, bool, std::string, bytes> q;
            q = CalculateValue(restrictions, statements[j], right);
            if (std::get<bool>(q)) {
                newStatements.push_back(statements[j]);
            }
        }
        for (int j = restrictions.size() - 1; j >= 0 ; j--) {
            bool found = false;
            for (auto& nname : left->lines) {
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

    Table* join(Table* table1, Table* table2, Node *right) {
        std::vector<Restriction> restrictions1 = table1->getRestriction();
        std::vector<Statement> statements1 = table1->getStatements();
        std::vector<Restriction> restrictions2 = table2->getRestriction();
        std::vector<Statement> statements2 = table2->getStatements();
        std::vector<Restriction> newRestrictions = restrictions1;
        newRestrictions.insert(newRestrictions.end(), restrictions2.begin(), restrictions2.end());
        Table *table = new Table(newRestrictions);
        for (int j = 0; j < statements1.size(); j++) {
            for (int k = 0; k < statements2.size(); k++) {
                Statement s = statements1[j];
                s.data.insert(s.data.end(), statements2[k].data.begin(), statements2[k].data.end());
                std::variant<int32_t, bool, std::string, bytes> q;
                q = CalculateValue(newRestrictions, s, right);
                if (std::get<bool>(q)) {
                    table->addStatement(s);
                }
            }
        }
        return table;
    }

    Table* update(Node* left, Node* right, Table* table) {
        std::vector<Restriction> restrictions = table->getRestriction();
        std::vector<Statement> statements = table->getStatements();
        for (int j = 0; j < statements.size(); j++) {
            std::variant<int32_t, bool, std::string, bytes> q;
            q = CalculateValue(restrictions, statements[j], right);
            if (std::get<bool>(q)) {
                for (int k = 0; k < restrictions.size(); k++) {
                    for (int l = 0; l < left->lines.size(); l++) {
                        if (left->lines[l] == restrictions[k].name) {
                            q = CalculateValue(restrictions, statements[j], left->qwe[l]);
                            statements[j].data[k] = q;
                        }
                    }
                }
            }
        }
        table->changeStatement(statements);
        return table;
    }

    Table* createIndex(std::vector<Token>& tokens) {}

    std::variant<Table*> Parse(Node* root) {
        if (root == nullptr) {
            throw std::invalid_argument("Invalid text");
        }
        else if (root->token.type == KWORD) {
            if (root->token.value == "select") {
                if (root->mid->token.type == IDENT) {
                    Table* table = database.findTable(root->mid->token.value);
                    return select(root->left, root->right, table);
                }
                else {
                    Table* table = std::get<Table*>(Parse(root->mid));
                    return select(root->left, root->right, table);
                }
            }
            else if (root->token.value == "update") {
                if (root->mid->token.type == IDENT) {
                    Table* table = database.findTable(root->mid->token.value);
                    return update(root->left, root->right, table);
                }
                else {
                    Table* table = std::get<Table*>(Parse(root->mid));
                    return update(root->left, root->right, table);
                }
            }
            else if (root->token.value == "delete") {
                if (root->mid->token.type == IDENT) {
                    Table* table = database.findTable(root->mid->token.value);
                    return deleteExpression(root->right, table);
                }
                else {
                    Table* table = std::get<Table*>(Parse(root->mid));
                    return deleteExpression(root->right, table);
                }
            }
            else if (root->token.value == "join") {
                Table* table1;
                Table* table2;
                if (root->left->token.type == IDENT) {
                    table1 = database.findTable(root->left->token.value);
                }
                else {
                    table1 = std::get<Table*>(Parse(root->left));;
                }
                if (root->mid->token.type == IDENT) {
                    table2 = database.findTable(root->mid->token.value);
                }
                else {
                    table2 = std::get<Table*>(Parse(root->mid));;
                }
                return join(table1, table2, root->right);
            }
        }
        else {
            throw std::invalid_argument("Invalid text");
        }
    }

public:
    db() = default;
    Table* execute(std::string& data) {
        std::vector<Token> tokens;
        makeListOfTokens(data, tokens);
            if (tokens[0].type == KWORD) {
                // ReSharper disable once CppDFAUnreachableCode
                if (tokens[0].value == "createtable") {
                    Table* q = createTable(tokens);
                    if (q == nullptr) {
                        throw std::invalid_argument("Table creation failed");
                    }
                    return q;
                }
                if (tokens[0].value == "insert") {
                    Table* q = insert(tokens);
                    if (q == nullptr) {
                        throw std::invalid_argument("Insert failed");
                    }
                    return q;
                }
                if (tokens[0].value == "delete") {
                    Node* q = MakeAST(tokens, 0, tokens.size());
                    Table* qq = std::get<Table*>(Parse(q));
                    if (q == nullptr) {
                        throw std::invalid_argument("Select failed");
                    }
                    return qq;
                }
                if (tokens[0].value == "update") {
                    Node* q = MakeAST(tokens, 0, tokens.size());
                    Table* qq = std::get<Table*>(Parse(q));
                    if (q == nullptr) {
                        throw std::invalid_argument("Select failed");
                    }
                    return qq;
                }
                if (tokens[0].value == "select") {
                    Node* q = MakeAST(tokens, 0, tokens.size());
                    Table* qq = std::get<Table*>(Parse(q));
                    if (q == nullptr) {
                        throw std::invalid_argument("Select failed");
                    }
                    return qq;
                }
                throw std::invalid_argument("Invalid syntax");
            }
            else {
                Node* q = MakeAST(tokens, 0, tokens.size());
                Table* qq = std::get<Table*>(Parse(q));
                if (q == nullptr) {
                    throw std::invalid_argument("Select failed");
                }
                return qq;
            }
        }

    void addTable(Table* table) {
        database.addTable(*table);
    }
};
#endif //DB_H
