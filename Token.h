#ifndef TOKEN_H
#define TOKEN_H

std::string keywords[] = { "insert",  "select", "update", "join", "delete"};

enum tok_names {NUL, KWORD, IDENT, NUM, BYTES, STRING, OPERATOR, DELIM, BOOL};

struct Token {
    enum tok_names type;
    std::string value;
    int order;
};

std::string lower(std::string& s) {
    std::string result = s;
    for (size_t i = 0; i < s.length(); i++) {
        result[i] = std::tolower(result[i]);
    }
    return result;
}

int isByte(int i, std::string &data, std::vector<Token> &tokens) {
    if ((i + 1 < data.size()) && (data[i] == '0' && data[i + 1] == 'x')) {
        int j = i + 2;
        while ((j < data.size()) && ((data[j] >= '0' && data[j] <= '1') || (data[j] >= 'a' && data[j] <= 'f'))) {
            j++;
        }
        tokens.push_back(Token({BYTES, data.substr(i + 2, j - i - 2)}));
        return j - i;
    }
    return 0;
}

int isNubmer(int i, std::string &data, std::vector<Token> &tokens) {
    if (data[i] >= '0' && data[i] <= '9') {
        int j = i + 1;
        while ((j < data.size()) && (data[j] >= '0' && data[j] <= '9')) {
            j++;
        }
        tokens.push_back(Token({NUM, data.substr(i, j - i)}));
        return j - i;
    }
    else return 0;
}

int isString(int i, std::string &data, std::vector<Token> &tokens) {
    if (data[i] == '\"') {
        int j = i + 1;
        while ((j < data.size()) && (data[j] != '\"')) {j++;}
        tokens.push_back(Token({STRING, data.substr(i+1, j - i - 1)}));
        return j - i + 1;
    }
    return 0;
}

int isBinOperator(int i, std::string &data, std::vector<Token> &tokens) {
    if (data[i] == '+' || data[i] == '-') {
        tokens.push_back(Token({OPERATOR, data.substr(i, 1), 6}));
        return 1;
    }
    if (data[i] == '*' || data[i] == '/' || data[i] == '%') {
        tokens.push_back(Token({OPERATOR, data.substr(i, 1), 5}));
        return 1;
    }
    if ((i+1 < data.size()) && (data[i] == '<' || data[i] == '>') && data[i + 1] == '=') {
        tokens.push_back(Token({OPERATOR, data.substr(i, 2), 9}));
        return 2;
    }
    if ((i+1 < data.size()) && (data[i] == '!' ) && data[i + 1] == '=') {
        tokens.push_back(Token({OPERATOR, data.substr(i, 2), 10}));
        return 2;
    }
    if ((data[i] == '<' || data[i] == '>')) {
        tokens.push_back(Token({OPERATOR, data.substr(i, 1), 9}));
        return 1;
    }
    if ((data[i] == data[i+1] && data[i+1] == '&')) {
        tokens.push_back(Token({OPERATOR, data.substr(i, 2), 14}));
        return 2;
    }
    if ((data[i] == data[i+1] && data[i+1] == '|')) {
        tokens.push_back(Token({OPERATOR, data.substr(i, 2), 15}));
        return 2;
    }
    if ((data[i] == data[i+1] && data[i+1] == '^')) {
        tokens.push_back(Token({OPERATOR, data.substr(i, 2), 12}));
        return 2;
    }
    if (data[i] == '!') {
        tokens.push_back(Token({OPERATOR, data.substr(i, 1), 3}));
        return 1;
    }
    if (data[i] == '=' && data[i + 1] == '=') {
        tokens.push_back(Token({OPERATOR, data.substr(i, 2), 10}));
        return 2;
    }
    if (data[i] == '=' && data[i + 1] != '=') {
        tokens.push_back(Token({OPERATOR, data.substr(i, 1), 16}));
        return 1;
    }
    if (data[i] == '|') {
        tokens.push_back(Token({OPERATOR, data.substr(i, 1), 20}));
        return 1;
    }
    return 0;
}

int isIdentifier(int i, std::string &data, std::vector<Token> &tokens) {
    if ((data[i] <= 'Z' && data[i] >= 'A') ||  (data[i] <= 'z' && data[i] >= 'a') || (data[i] == '_')) {
        int j = i + 1;
        while ((j < data.size()) && (data[j] <= 'Z' && data[j] >= 'A') ||  (data[j] <= 'z' && data[j] >= 'a') || (data[j] <= '9' && data[j] >= '0') || data[j] == '_') {
            j++;
        }
        std::string token_value = data.substr(i, j - i);
        for (auto element : keywords) {
            if (element == lower(token_value)) {
                tokens.push_back(Token({KWORD, element}));
                return j - i;
            }
        }
        if (token_value == "true" || token_value == "false") {
            tokens.push_back(Token({BOOL, token_value}));
            return token_value.length();
        }
        if (lower(token_value) == "create") {
            j++;
            while ((j < data.size()) && (data[j] <= 'Z' && data[j] >= 'A') ||  (data[j] <= 'z' && data[j] >= 'a') || (data[j] <= '9' && data[j] >= '0') || data[j] == '_') {
                j++;
            }
            std::string token_value = data.substr(i, j - i);
            if (lower(token_value) == "create table") {
                tokens.push_back(Token({KWORD, "createtable"}));
            }
        }
        tokens.push_back(Token({IDENT, token_value}));
        return j - i;
    }
    else {return 0;}
}

int isDelimiter(int i, std::string &data, std::vector<Token> &tokens) {
    switch (data[i]) {
        case ',':
            tokens.push_back(Token({DELIM, data.substr(i, 1)}));
            return 1;
        case '(':
            tokens.push_back(Token({DELIM, data.substr(i, 1)}));
            return 1;
        case ')':
            tokens.push_back(Token({DELIM, data.substr(i, 1)}));
            return 1;
        case '{':
            tokens.push_back(Token({DELIM, data.substr(i, 1)}));
            return 1;
        case '}':
            tokens.push_back(Token({DELIM, data.substr(i, 1)}));
            return 1;
        case ':':
            tokens.push_back(Token({DELIM, data.substr(i, 1)}));
            return 1;
        case '[':
            tokens.push_back(Token({DELIM, data.substr(i, 1)}));
            return 1;
        case ']':
            tokens.push_back(Token({DELIM, data.substr(i, 1)}));
            return 1;
        default:
            return 0;
    }
}

void makeListOfTokens(std::string& s, std::vector<Token> &data) {
    int i = 0;
    int j = 0;
    while (i < s.size()) {
        j++;
        while (s[i] == ' ' || s[i] == '\n'){i++;}
        i+= isByte(i, s, data);
        i+= isNubmer(i, s, data);
        i+= isString(i, s, data);
        i+= isBinOperator(i, s, data);
        i+= isIdentifier(i, s, data);
        i+= isDelimiter(i, s, data);
        if (j > s.size() * 10) {
            throw std::runtime_error("Invalid syntax");
        }
    }
}

#endif //TOKEN_H
