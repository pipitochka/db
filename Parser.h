#ifndef PARSER_H
#define PARSER_H

#include "Token.h"

struct Node {
public:
    Token token;
    Node* left;
    Node* right;
    Node* parent;
};

int CheckNice(std::vector<Token> &data, int i) {
    int l = 1, r = 0;
    i++;
    while ((l != r) && (i < data.size())) {
        if (data[i].getTokenValue() == "(") {
            l++;
        }
        else if (data[i].getTokenValue() == ")") {
            r++;
        }
        i++;
    }
    return i;
}

Node* MakeAST(std::vector<Token>& tokens, int j, int t) {
    Node* q = new Node({tokens[j], nullptr, nullptr});
    if (tokens[j].getTokenValue() == "(") {
        int tt = CheckNice(tokens, j + 1);
        q = MakeAST(tokens, j + 1, tt);
        j = tt - 1;
    }
    for (int i = j + 1; i < t; i++) {
        if (tokens[i].getTokeName() == OPERATOR) {
            Node* b = q;
            while ((b->parent != nullptr) && (b->parent->token.getTokeName() == OPERATOR) && (b->parent->token.getValue() <= tokens[i].getValue())) {b = b->parent;}
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
        else if (tokens[i].getTokeName() == NUM || tokens[i].getTokeName() == BYTES || tokens[i].getTokeName() == STRING || tokens[i].getTokeName() == IDENT) {
            Node* a = new Node({tokens[i], nullptr, nullptr, q});
            q->right = a;
            q = (q->right);
        }
        else if (tokens[i].getTokeName() == DELIM && tokens[i].getTokenValue() == "(") {
            int tt = CheckNice(tokens, i + 1);
            Node* a = MakeAST(tokens, i + 1, tt - 1);
            a->parent = q;
            q->right = a;
            q = (q->right);
            i = tt - 1;
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
            if (root->token.getTokeName() == OPERATOR) {
                throw std::invalid_argument("Invalid text");
            }
            else if (root->token.getTokeName() == NUM) {
                return std::stoi(root->token.getTokenValue());
            }
            else if (root->token.getTokeName() == BYTES) {
                return convertStringToBytes(root->token.getTokenValue());
            }
            else if (root->token.getTokeName() == STRING) {
                return root->token.getTokenValue();
            }
            else if (root->token.getTokeName() == IDENT) {
                int i = getNumberOfRestrictions(restrictions, root->token.getTokenValue());
                return statement.data[i];
            }
        }
        else if (root->left != nullptr && root->right != nullptr) {
            if (root->token.getTokeName() != OPERATOR) {
                throw std::invalid_argument("Invalid text");
            }
            std::variant<int32_t, bool, std::string, bytes> left = CalculateValue(restrictions, statement, root->left);
            std::variant<int32_t, bool, std::string, bytes> right = CalculateValue(restrictions, statement, root->right);
            if (root->token.getTokenValue() == "+") {
                if (std::holds_alternative<int32_t>(left) && std::holds_alternative<int32_t>(right)) {
                    return std::get<int32_t>(left) + std::get<int32_t>(right);
                }
                else if (std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right)) {
                    return std::get<std::string>(left) + std::get<std::string>(right);
                }
            }
            else if (root->token.getTokenValue() == "-") {
                return std::get<int32_t>(left) - std::get<int32_t>(right);
            }
            else if (root->token.getTokenValue() == "*") {
                return std::get<int32_t>(left) * std::get<int32_t>(right);
            }
            else if (root->token.getTokenValue() == "/") {
                return std::get<int32_t>(left) / std::get<int32_t>(right);
            }
            else if (root->token.getTokenValue() == "%") {
                return std::get<int32_t>(left) % std::get<int32_t>(right);
            }
            else if (root->token.getTokenValue() == "<") {
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
            else if (root->token.getTokenValue() == ">") {
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
            else if (root->token.getTokenValue() == "<=") {
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
            else if (root->token.getTokenValue() == ">=") {
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
            else if (root->token.getTokenValue() == "=") {
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
            else if (root->token.getTokenValue() == "==") {
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
            else if (root->token.getTokenValue() == "!=") {
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
            else if (root->token.getTokenValue() == "&&") {
                if (std::holds_alternative<bool>(left) && std::holds_alternative<bool>(right)) {
                    return std::get<bool>(left) && std::get<bool>(right);
                }
                else {
                    throw std::invalid_argument("Invalid text");
                }

            }
            else if (root->token.getTokenValue() == "||") {
                if (std::holds_alternative<bool>(left) && std::holds_alternative<bool>(right)) {
                    return std::get<bool>(left) && std::get<bool>(right);
                }
                else {
                    throw std::invalid_argument("Invalid text");
                }

            }
            else if (root->token.getTokenValue() == "!") {
                if (std::holds_alternative<bool>(left) && std::holds_alternative<bool>(right)) {
                    return std::get<bool>(left) && std::get<bool>(right);
                }
                else {
                    throw std::invalid_argument("Invalid text");
                }

            }
            else if (root->token.getTokenValue() == "^^") {
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
