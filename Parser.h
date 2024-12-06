#ifndef PARSER_H
#define PARSER_H

#include "Token.h"

struct Node {
public:
    Token token;
    Node* left;
    Node* right;
    Node* parent;
    Node* mid;
    std::vector<std::string> lines;
};

int CheckNice(std::vector<Token> &data, int i) {
    int l = 1, r = 0;
    i++;
    while ((l != r) && (i < data.size())) {
        if (data[i].value == "(") {
            l++;
        }
        else if (data[i].value == ")") {
            r++;
        }
        i++;
    }
    return i;
}

Node* MakeAST(std::vector<Token>& tokens, int j, int t) {
    Node* q = nullptr;
    for (int i = j; i < t; i++) {
        if (tokens[i].type == OPERATOR) {
            Node* b = q;
            while ((b->parent != nullptr) && (b->parent->token.type == OPERATOR) && (b->parent->token.order <= tokens[i].order)) {b = b->parent;}
            if (b->parent == nullptr) {
                Node* a = new Node({tokens[i], b, nullptr, nullptr});
                b->parent = a;
                q = a;
            }
            else {
                Node* a = new Node({tokens[i], b, nullptr, b->parent});
                b->parent->right = a;
                b->parent = a;
                q = a;
            }

            }
        else if (tokens[i].type == NUM || tokens[i].type == BYTES || tokens[i].type == STRING || tokens[i].type == IDENT) {
            Node* a = new Node({tokens[i], nullptr, nullptr, nullptr});
            if (q != nullptr) {
                q->right = a;
                a->parent = q;
                q = (q->right);
            }
            else {
                q = a;
            }
        }
        else if (tokens[i].type == DELIM && tokens[i].value == "(") {
            int tt = CheckNice(tokens, i + 1);
            Node* a = MakeAST(tokens, i + 1, tt - 1);
            a->parent = q;
            q->right = a;
            q = (q->right);
            i = tt - 1;
        }
        else if (tokens[i].type == KWORD) {
            if (tokens[i].value == "select") {
                Node* a = new Node({tokens[i], nullptr, nullptr, nullptr, nullptr});
                Node* b = new Node({Token({NUL}), nullptr, nullptr, nullptr, nullptr});
                Node* c = new Node({Token({NUL}), nullptr, nullptr, nullptr, nullptr});
                Node* d = new Node({Token({NUL}), nullptr, nullptr, nullptr, nullptr});
                int k = i + 1;
                while (tokens[k].value != "from") {
                    if (tokens[k].value != ",") {b->lines.push_back(tokens[k].value);}
                    k += 1;
                }
                k++;
                int kk = t;
                if (tokens[k].type == IDENT) {
                    c->token.value = tokens[k].value;
                    c->token.type = IDENT;
                }
                else {
                    c = MakeAST(tokens, k, kk);
                }
                while (tokens[kk].value != "where") {
                    kk--;
                }
                d = MakeAST(tokens, kk + 1, t);
                d->parent = a;
                c->parent = a;
                b->parent = a;
                a->left = b;
                a->mid = c;
                a->right = d;
                if (q != nullptr) {
                    q->right = a;
                    a->parent = q;
                }
                else {
                    q = a;
                }
                i = t;
            }
        }
    }

    while (q->parent != nullptr) {
        q = q->parent;
    }
    return q;
}

void deleteTree(Node* root) {
    if (root->left != nullptr) {
        deleteTree(root->left);
    }
    if (root->right != nullptr) {
        deleteTree(root->right);
    }
    delete root;
}

std::variant<int32_t, bool, std::string, bytes> CalculateValue(std::vector<Restriction>& restrictions, Statement& statement, Node* root) {
    try {
        if (root->left == nullptr && root->right == nullptr) {
            if (root->token.type == OPERATOR) {
                throw std::invalid_argument("Invalid text");
            }
            else if (root->token.type == NUM) {
                return std::stoi(root->token.value);
            }
            else if (root->token.type == BYTES) {
                return convertStringToBytes(root->token.value);
            }
            else if (root->token.type == STRING) {
                return root->token.value;
            }
            else if (root->token.type == IDENT) {
                int i = getNumberOfRestrictions(restrictions, root->token.value);
                return statement.data[i];
            }
        }
        else if (root->left != nullptr && root->right != nullptr) {
            if (root->token.type != OPERATOR) {
                throw std::invalid_argument("Invalid text");
            }
            std::variant<int32_t, bool, std::string, bytes> left = CalculateValue(restrictions, statement, root->left);
            std::variant<int32_t, bool, std::string, bytes> right = CalculateValue(restrictions, statement, root->right);
            if (root->token.value == "+") {
                if (std::holds_alternative<int32_t>(left) && std::holds_alternative<int32_t>(right)) {
                    return std::get<int32_t>(left) + std::get<int32_t>(right);
                }
                else if (std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right)) {
                    return std::get<std::string>(left) + std::get<std::string>(right);
                }
            }
            else if (root->token.value == "-") {
                return std::get<int32_t>(left) - std::get<int32_t>(right);
            }
            else if (root->token.value == "*") {
                return std::get<int32_t>(left) * std::get<int32_t>(right);
            }
            else if (root->token.value == "/") {
                return std::get<int32_t>(left) / std::get<int32_t>(right);
            }
            else if (root->token.value == "%") {
                return std::get<int32_t>(left) % std::get<int32_t>(right);
            }
            else if (root->token.value == "<") {
                if (std::holds_alternative<int32_t>(left) && std::holds_alternative<int32_t>(right)) {
                    return std::get<int32_t>(left) < std::get<int32_t>(right);
                }
                else if (std::holds_alternative<bytes>(left) && std::holds_alternative<bytes>(right)) {
                    return std::get<bytes>(left) < std::get<bytes>(right);
                }
                else if (std::holds_alternative<bool>(left) && std::holds_alternative<bool>(right)) {
                    return std::get<bool>(left) < std::get<bool>(right);
                }
                else if (std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right)) {
                    return std::get<std::string>(left) < std::get<std::string>(right);
                }
                else {
                    throw std::invalid_argument("Invalid text");
                }
            }
            else if (root->token.value == ">") {
                if (std::holds_alternative<int32_t>(left) && std::holds_alternative<int32_t>(right)) {
                    return std::get<int32_t>(left) > std::get<int32_t>(right);
                }
                else if (std::holds_alternative<bytes>(left) && std::holds_alternative<bytes>(right)) {
                    return std::get<bytes>(left) > std::get<bytes>(right);
                }
                else if (std::holds_alternative<bool>(left) && std::holds_alternative<bool>(right)) {
                    return std::get<bool>(left) > std::get<bool>(right);
                }
                else if (std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right)) {
                    return std::get<std::string>(left) > std::get<std::string>(right);
                }
                else {
                    throw std::invalid_argument("Invalid text");
                }

            }
            else if (root->token.value == "<=") {
                if (std::holds_alternative<int32_t>(left) && std::holds_alternative<int32_t>(right)) {
                    return std::get<int32_t>(left) <= std::get<int32_t>(right);
                }
                else if (std::holds_alternative<bytes>(left) && std::holds_alternative<bytes>(right)) {
                    return std::get<bytes>(left) <= std::get<bytes>(right);
                }
                else if (std::holds_alternative<bool>(left) && std::holds_alternative<bool>(right)) {
                    return std::get<bool>(left) <= std::get<bool>(right);
                }
                else if (std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right)) {
                    return std::get<std::string>(left) <= std::get<std::string>(right);
                }
                else {
                    throw std::invalid_argument("Invalid text");
                }
            }
            else if (root->token.value == ">=") {
                if (std::holds_alternative<int32_t>(left) && std::holds_alternative<int32_t>(right)) {
                    return std::get<int32_t>(left) >= std::get<int32_t>(right);
                }
                else if (std::holds_alternative<bytes>(left) && std::holds_alternative<bytes>(right)) {
                    return std::get<bytes>(left) >= std::get<bytes>(right);
                }
                else if (std::holds_alternative<bool>(left) && std::holds_alternative<bool>(right)) {
                    return std::get<bool>(left) >= std::get<bool>(right);
                }
                else if (std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right)) {
                    return std::get<std::string>(left) >= std::get<std::string>(right);
                }
                else {
                    throw std::invalid_argument("Invalid text");
                }
            }
            else if (root->token.value == "=") {
                if (std::holds_alternative<int32_t>(left) && std::holds_alternative<int32_t>(right)) {
                    return std::get<int32_t>(left) = std::get<int32_t>(right);
                }
                else if (std::holds_alternative<bytes>(left) && std::holds_alternative<bytes>(right)) {
                    return std::get<bytes>(left) = std::get<bytes>(right);
                }
                else if (std::holds_alternative<bool>(left) && std::holds_alternative<bool>(right)) {
                    return std::get<bool>(left) = std::get<bool>(right);
                }
                else if (std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right)) {
                    return std::get<std::string>(left) = std::get<std::string>(right);
                }
                else {
                    throw std::invalid_argument("Invalid text");
                }
            }
            else if (root->token.value == "==") {
                if (std::holds_alternative<int32_t>(left) && std::holds_alternative<int32_t>(right)) {
                    return std::get<int32_t>(left) == std::get<int32_t>(right);
                }
                else if (std::holds_alternative<bytes>(left) && std::holds_alternative<bytes>(right)) {
                    return std::get<bytes>(left) == std::get<bytes>(right);
                }
                else if (std::holds_alternative<bool>(left) && std::holds_alternative<bool>(right)) {
                    return std::get<bool>(left) == std::get<bool>(right);
                }
                else if (std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right)) {
                    return std::get<std::string>(left) == std::get<std::string>(right);
                }
                else {
                    throw std::invalid_argument("Invalid text");
                }
            }
            else if (root->token.value == "!=") {
                if (std::holds_alternative<int32_t>(left) && std::holds_alternative<int32_t>(right)) {
                    return std::get<int32_t>(left) != std::get<int32_t>(right);
                }
                else if (std::holds_alternative<bytes>(left) && std::holds_alternative<bytes>(right)) {
                    return std::get<bytes>(left) != std::get<bytes>(right);
                }
                else if (std::holds_alternative<bool>(left) && std::holds_alternative<bool>(right)) {
                    return std::get<bool>(left) != std::get<bool>(right);
                }
                else if (std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right)) {
                    return std::get<std::string>(left) != std::get<std::string>(right);
                }
                else {
                    throw std::invalid_argument("Invalid text");
                }
            }
            else if (root->token.value == "&&") {
                if (std::holds_alternative<bool>(left) && std::holds_alternative<bool>(right)) {
                    return std::get<bool>(left) && std::get<bool>(right);
                }
                else {
                    throw std::invalid_argument("Invalid text");
                }

            }
            else if (root->token.value == "||") {
                if (std::holds_alternative<bool>(left) && std::holds_alternative<bool>(right)) {
                    return std::get<bool>(left) && std::get<bool>(right);
                }
                else {
                    throw std::invalid_argument("Invalid text");
                }

            }
            else if (root->token.value == "!") {
                if (std::holds_alternative<bool>(left) && std::holds_alternative<bool>(right)) {
                    return std::get<bool>(left) && std::get<bool>(right);
                }
                else {
                    throw std::invalid_argument("Invalid text");
                }

            }
            else if (root->token.value == "^^") {
                if (std::holds_alternative<bool>(left) && std::holds_alternative<bool>(right)) {
                    return std::get<bool>(left) && std::get<bool>(right);
                }
                else {
                    throw std::invalid_argument("Invalid text");
                }

            }
        }
    }
    catch(std::invalid_argument& e) {
        std::cout << e.what() << std::endl;
    }
}



#endif //PARSER_H
