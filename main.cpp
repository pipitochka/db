#include <iostream>
#include "db.h"

int main()
{
    // db q;
    std::string query1 = {R"(create table users ({key, autoincrement} id : int32, {unique} login: string[32], password_hash: bytes[8], is_admin: bool = false))"};
    std::string query2 = {R"(insert (,"vasya", 0xdeadbeefdeadbeef) to users)"};
    std::string query3 = {R"(insert (login = "vasy", password_hash = 0xdeadbeefdeadbeef) to users)"};
    std::string query4 = {R"(insert (,"admin", 0x0000000000000000, true) to users)"};
    std::string query5 = {R"(insert (
is_admin = true,
         login = "admin",
         password_hash = 0x0000000000000000
     ) to users)"};
    std::string query6 = {R"(select id, login from users where is_admin || id < 100)"};
    std::string query7 = {R"(update users set is_admin = true where login = "vasya")"};
    std::string query8 = {R"(update users set login = login + "_deleted", is_admin = false where password_hash < 0x00000000ffffffff)"};
    std::string query9 = {R"(delete users where |login| % 2 = 0)"};
    std::string query10 = {R"(create table users ({key, autoincrement} id : int32, {unique} login: string[32] = "zhopa", password_hash: bytes[8], is_admin: bool))"};
    std::string query11 = {R"(1 + 2 * 3 + 6 * 12 * 3 - 12 / 3 - 7)"};
    std::string query12 = {R"(3 * (1 + 2))"};
    std::string query13 = {R"(5 + (1 + 2) * (3 + 4))"};
    std::string query14 = {R"(1 + (2 * 3))"};
    std::string query15 = {R"(1 > 3)"};
    std::string query16 = {R"(select id, login from users where is_admin || id < 10)"};
    std::string query17 = {R"(update users set is_admin = true where login == "vasya")"};
    std::string query18 = {R"(update users set login = login + "_deleted", is_admin = false where password_hash < 0x00000000ffffffff)"};
    std::string query19 = {R"(delete users where login == "vasya")"};
    std::string query20= {R"(insert (,"vasy", 0xdeadbeefdeadbeef) to users)"};
    std::string query21 = {R"(delete users where |login| % 2 == 0)"};

    std::string query22 = {R"(create table abacaba (name: string[32], age : int32))"};
    std::string query23 = {R"(insert (name = "vasya", age = 12) to abacaba)"};
    std::string query24 = {R"(insert (name = "vasya", age = 15) to abacaba)"};
    std::string query25 = {R"(insert (name = "admin", age = 17) to abacaba)"};

    std::string query26 = {R"(users join abacaba on name == login)"};
    std::string query27 = {R"(select name, age, password_hash from users join abacaba on name == login where true)"};
    // std::vector<Token> data;
    // makeListOfTokens(query16, data);
    // Node* ast = MakeAST(data, 0, data.size());

    db q;
    q.execute(query1);
    q.execute(query3);
    q.execute(query5);
    q.execute(query16);
    // std::vector<Restriction> restrictions;
    // Statement statement;
    //  std::vector<Token> data;
    //  makeListOfTokens(query15, data);
    //  Node* ast = MakeAST(data, 0, data.size());
    // std::variant<int32_t, bool, std::string, bytes> qq = CalculateValue(restrictions, statement, ast);
    //  deleteTree(ast);
    // db q;
    // q.execute(query1);
    // q.execute(query2);
    // q.execute(query4);
    //
    // q.execute(query22);
    // q.execute(query23);
    // q.execute(query24);
    // q.execute(query25);
    // Table* qq = q.execute(query27);
    // Table* table = q.execute(query16);
    // std::string qq{"afasfaa"};
    // table->rename(qq);
    // q.addTable(table);
    // q.execute(query17);
    // q.execute(query18);
    // q.execute(query21);
    return 0;
}
