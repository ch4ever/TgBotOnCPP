#include <string>
using namespace std;


size_t WriteCallback(void*, size_t, size_t, string*);
string getCompletion(const string&, const string& model = "gpt-3.5-turbo");


bool IsModeInactive(const map<string, bool>& stat) {
    return std::all_of(stat.begin(), stat.end(), [](const auto& pair) {
        return !pair.second;
        });
}

void SwitchMode(map<string, bool>& stat) {
    if (IsModeInactive(stat)) {
        return;
    }
    else {
        for (auto& pair : stat) {
            pair.second = false;
        }
    }
}