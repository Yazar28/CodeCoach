#include <cstdio>
#include <string>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <chrono>
#include <random>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <vector>
#include <algorithm>

#include "httplib.h"
#include "json.hpp"

using json = nlohmann::json;
using namespace std::chrono_literals;
namespace fs = std::filesystem;

#ifdef _WIN32
#include <windows.h>
static std::string short_path(const std::string& p) {
    char buf[MAX_PATH];
    DWORD n = GetShortPathNameA(p.c_str(), buf, MAX_PATH);
    if (n == 0 || n > MAX_PATH) return p;
    return std::string(buf, n);
}
#endif

struct Submission {
    std::string id;
    std::string status;
    json results = json::array();
    int timeMs = 0;
    int memoryKB = 256;
    std::string errorMsg;
};

static std::unordered_map<std::string, Submission> DB;
static std::mutex DBM;

static void set_cors(httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET,POST,OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
}

static std::string rand_id(const std::string& pfx = "sub-") {
    static std::mt19937_64 rng{ std::random_device{}() };
    static const char* K = "abcdefghijklmnopqrstuvwxyz0123456789";
    std::string s = pfx;
    for (int i = 0; i < 6; ++i) s += K[rng() % 36];
    return s;
}

static std::string read_file(const fs::path& p) {
    std::ifstream f(p, std::ios::binary);
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

static void write_file(const fs::path& p, const std::string& s) {
    std::ofstream f(p, std::ios::binary);
    f << s;
}

static std::string find_compiler() {
#ifdef _WIN32
    const char* CAND[] = {
        "C:\\\\Program Files\\\\LLVM\\\\bin\\\\clang++.exe",
        "C:\\\\msys64\\\\mingw64\\\\bin\\\\g++.exe",
        "clang++",
        "g++"
    };
    for (auto c : CAND) {
        std::string cmd = "\""; cmd += c; cmd += "\" --version >NUL 2>&1";
        if (std::system(cmd.c_str()) == 0) return c;
    }
    return "";
#else
    if (std::system("clang++ --version >/dev/null 2>&1") == 0) return "clang++";
    if (std::system("g++ --version >/dev/null 2>&1") == 0) return "g++";
    return "";
#endif
}

// ======================= TWO SUM HARNESS =======================
static std::string make_two_sum_harness() {
    return R"(#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>
#include <algorithm>
using namespace std;

#include "user.cpp"

vector<int> twoSum(vector<int>& nums, int target) {
    Solution sol;
    return sol.twoSum(nums, target);
}

string to_str(const vector<int>& v) {
    ostringstream ss; ss << "[";
    for(size_t i = 0; i < v.size(); ++i) { 
        if(i) ss << ","; 
        ss << v[i]; 
    }
    ss << "]"; 
    return ss.str();
}

int main() {
    // Caso 1
    vector<int> nums1 = {2,7,11,15};
    int target1 = 9;
    auto result1 = twoSum(nums1, target1);
    cout << to_str(result1) << endl;
    
    // Caso 2
    vector<int> nums2 = {3,2,4};
    int target2 = 6;
    auto result2 = twoSum(nums2, target2);
    cout << to_str(result2) << endl;
    
    return 0;
}
)";
}

// =================== REVERSE STRING HARNESS ====================
static std::string make_reverse_string_harness() {
    return R"(#include <iostream>
#include <vector>
#include <string>
#include <sstream>
using namespace std;

#include "user.cpp"

void reverseString(vector<char>& s) {
    Solution sol;
    sol.reverseString(s);
}

string to_str(const vector<char>& v) {
    string s;
    for(char c : v) s += c;
    return s;
}

int main() {
    // Caso 1: "hello" -> "olleh"
    vector<char> s1 = {'h','e','l','l','o'};
    reverseString(s1);
    cout << to_str(s1) << endl;
    
    // Caso 2: "Hannah" -> "hannaH"
    vector<char> s2 = {'H','a','n','n','a','h'};
    reverseString(s2);
    cout << to_str(s2) << endl;
    
    return 0;
}
)";
}

// ==================== BINARY SEARCH HARNESS ====================
static std::string make_binary_search_harness() {
    return R"(#include <iostream>
#include <vector>
using namespace std;

#include "user.cpp"

// Llama a Solution::search
int search_wrapper(const vector<int>& nums, int target) {
    Solution sol;
    vector<int> copy = nums;
    return sol.search(copy, target);
}

int main() {
    {
        vector<int> nums = {-1,0,3,5,9,12};
        int target = 9;
        int res = search_wrapper(nums, target);
        cout << res << "\n";   // Esperado: 4
    }
    {
        vector<int> nums = {-1,0,3,5,9,12};
        int target = 2;
        int res = search_wrapper(nums, target);
        cout << res << "\n";   // Esperado: -1
    }
    {
        vector<int> nums = {1};
        int target = 1;
        int res = search_wrapper(nums, target);
        cout << res << "\n";   // Esperado: 0
    }
    {
        vector<int> nums = {1};
        int target = 2;
        int res = search_wrapper(nums, target);
        cout << res << "\n";   // Esperado: -1
    }
    return 0;
}
)";
}

// ================== COUNT NEGATIVES HARNESS ====================
static std::string make_count_negatives_harness() {
    return R"(#include <iostream>
#include <vector>
using namespace std;

#include "user.cpp"

// Se asume que Solution tiene:
// int solve(vector<int>& nums);
int solve_wrapper(vector<int> nums) {
    Solution sol;
    return sol.solve(nums);
}

int main() {
    {
        vector<int> nums = {-1, 2, -5, 7};
        int res = solve_wrapper(nums);
        cout << res << "\n";   // Esperado: 2
    }
    {
        vector<int> nums = {-1, -2, -3};
        int res = solve_wrapper(nums);
        cout << res << "\n";   // Esperado: 3
    }
    {
        vector<int> nums = {3, 4, 1};
        int res = solve_wrapper(nums);
        cout << res << "\n";   // Esperado: 0
    }
    return 0;
}
)";
}

// ======================= PIPELINE GENÉRICO =====================
static void run_pipeline(const std::string& id,
    const std::string& userSource,
    const std::string& problemType) {

    auto tStart = std::chrono::steady_clock::now();
    std::string compiler = find_compiler();

    if (compiler.empty()) {
        std::lock_guard<std::mutex> lk(DBM);
        DB[id].status = "done";
        DB[id].errorMsg = "No se encontró compilador C++";
        DB[id].results = json::array();
        return;
    }

    fs::path tmp = fs::temp_directory_path() / rand_id("cc_eval_");
    fs::create_directories(tmp);

    fs::path userp = tmp / "user.cpp";
    fs::path mainp = tmp / "main.cpp";

    write_file(userp, userSource);

    // Elegir harness según el tipo de problema
    std::string harness;
    std::vector<std::string> expected_outputs;

    if (problemType == "two-sum") {
        harness = make_two_sum_harness();
        expected_outputs = { "[0,1]", "[1,2]" };
    }
    else if (problemType == "reverse-string") {
        harness = make_reverse_string_harness();
        expected_outputs = { "olleh", "hannaH" };
    }
    else if (problemType == "binary-search") {
        harness = make_binary_search_harness();
        expected_outputs = { "4", "-1", "0", "-1" };
    }
    else if (problemType == "count-negatives") {
        harness = make_count_negatives_harness();
        expected_outputs = { "2", "3", "0" };
    }
    else {
        // Fallback: usa two-sum si llega algo inesperado
        harness = make_two_sum_harness();
        expected_outputs = { "[0,1]", "[1,2]" };
    }

    write_file(mainp, harness);

#ifdef _WIN32
    std::string compS = short_path(compiler);
    std::string tmpS = short_path(tmp.string());
#else
    std::string compS = compiler;
    std::string tmpS = tmp.string();
#endif

    // Compilar
    std::ostringstream ccmd;
#ifdef _WIN32
    ccmd << "cmd /S /C \"cd /d \"" << tmpS
        << "\" && \"" << compS
        << "\" -std=c++17 -O2 -o a.exe main.cpp > compile.err 2>&1\"";
#else
    ccmd << "cd \"" << tmpS << "\" && \"" << compS
        << "\" -std=c++17 -O2 -o a.out main.cpp > compile.err 2>&1";
#endif

    int cexit = std::system(ccmd.str().c_str());
    std::string cerrtxt = read_file(tmp / "compile.err");

#ifdef _WIN32
    std::string exeName = "a.exe";
#else
    std::string exeName = "a.out";
#endif

    if (cexit != 0) {
        std::lock_guard<std::mutex> lk(DBM);
        DB[id].status = "done";
        DB[id].errorMsg = "Error de compilación:\n" + cerrtxt;
        return;
    }

    // Ejecutar
    std::ostringstream rcmd;
#ifdef _WIN32
    rcmd << "cmd /S /C \"cd /d \"" << tmpS << "\" && \"" << exeName
        << "\" > run.out 2>&1\"";
#else
    rcmd << "cd \"" << tmpS << "\" && \"" << exeName
        << "\" > run.out 2>&1";
#endif

    auto t0 = std::chrono::steady_clock::now();
    int rexit = std::system(rcmd.str().c_str());
    auto t1 = std::chrono::steady_clock::now();
    (void)rexit;

    int totalMs = (int)std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    std::string out = read_file(tmp / "run.out");

    // Limpiar \r de Windows
    out.erase(std::remove(out.begin(), out.end(), '\r'), out.end());

    std::vector<std::string> lines;
    std::istringstream ss(out);
    std::string line;
    while (std::getline(ss, line)) {
        if (!line.empty()) {
            lines.push_back(line);
        }
    }

    json results = json::array();
    int ncases = (int)expected_outputs.size();
    int perCaseMs = (ncases > 0) ? totalMs / ncases : totalMs;

    for (size_t i = 0; i < expected_outputs.size(); ++i) {
        std::string expected = expected_outputs[i];
        std::string obtained = (i < lines.size()) ? lines[i] : "";

        bool pass = (expected == obtained);

        results.push_back(json{
            {"case",  (int)i + 1},
            {"pass",  pass},
            {"stdout", obtained},
            {"timeMs", perCaseMs}
            });
    }

    std::lock_guard<std::mutex> lk(DBM);
    DB[id].status = "done";
    DB[id].results = results;
    DB[id].timeMs = totalMs;
}

// =========================== SERVER ============================
int main() {
    httplib::Server svr;

    svr.Options(R"(/.*)", [](const httplib::Request&, httplib::Response& res) {
        set_cors(res);
        res.status = 200;
        });

    // Crear submission
    svr.Post("/submissions", [](const httplib::Request& req, httplib::Response& res) {
        set_cors(res);

        json body;
        try {
            body = json::parse(req.body);
        }
        catch (...) {
            res.status = 400;
            res.set_content(R"({"error":"invalid json"})", "application/json");
            return;
        }

        std::string pid = body.value("problemId", "");
        std::string src = body.value("source", "");
        std::string lang = body.value("lang", "");

        if (pid.empty() || src.empty() || lang.empty()) {
            res.status = 400;
            res.set_content(R"({"error":"missing fields"})", "application/json");
            return;
        }

        auto id = rand_id();
        {
            std::lock_guard<std::mutex> lk(DBM);
            DB[id] = Submission{ id, "queued" };
        }

        std::thread([id, pid, src]() {
            {
                std::lock_guard<std::mutex> lk(DBM);
                DB[id].status = "running";
            }
            run_pipeline(id, src, pid);
            }).detach();

        json out = { {"submissionId", id} };
        res.set_content(out.dump(), "application/json");
        });

    // Consultar submission
    svr.Get(R"(/submissions/([A-Za-z0-9\-]+))", [](const httplib::Request& req, httplib::Response& res) {
        set_cors(res);
        auto id = req.matches[1].str();

        std::lock_guard<std::mutex> lk(DBM);
        auto it = DB.find(id);
        if (it == DB.end()) {
            res.status = 404;
            res.set_content(R"({"error":"not found"})", "application/json");
            return;
        }

        const auto& s = it->second;
        json out = {
            {"status", s.status},
            {"results", s.results},
            {"timeMs", s.timeMs},
            {"memoryKB", s.memoryKB}
        };
        if (!s.errorMsg.empty()) out["note"] = s.errorMsg;
        res.set_content(out.dump(), "application/json");
        });

    std::printf("[EV] Escuchando en http://0.0.0.0:8082\n");
    svr.listen("0.0.0.0", 8082);
    return 0;
}
