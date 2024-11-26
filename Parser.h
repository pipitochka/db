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
        else if (tokens[i].getTokeName() == DELIM && tokens[i + 1].getTokenValue() == "(") {
            int t = CheckNice(tokens, i + 1);
            Node* a = MakeAST(tokens, i + 1, t - 1);
            a->parent = q;
            q->right = a;
            q = (q->right);
            i = t;
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
#endif //PARSER_H
