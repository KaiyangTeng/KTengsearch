#include <iostream>
#include <vector>
#include <nlohmann/json.hpp>

int main() {
    std::vector<int> v = {0,1,2,3,4,5,6,7,8,9};

    nlohmann::json j = v;

    std::string s = j.dump();
    std::string pretty = j.dump(2);

    std::cout << s << "\n\n" << pretty << std::endl;

    std::vector<int> v2 = j.get<std::vector<int>>();
}

