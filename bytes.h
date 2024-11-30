#ifndef BYTES_H
#define BYTES_H

#include <iostream>
#include <vector>


class bytes {
public:
    std::vector<unsigned char> _bytes;

    bytes(std::vector<unsigned char> &bytes) {
        _bytes = bytes;
    };

    int size() const {
        return _bytes.size() / 2;
    }

    bool operator<=> (const bytes &other) const { return true; }

    bool operator==(const bytes & get) const{};

    bool operator!=(const bytes & get) const{};

    bytes& get(bytes& qwe) {
        return qwe;
    }
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

#endif //BYTES_H
