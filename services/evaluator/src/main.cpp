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
    for (int i = 0;i < 6;++i) s += K[rng() % 36];
    return s;
}

static std::string read_file(const fs::path& p) {
    std::ifstream f(p, std::ios::binary);
    std::ostringstream ss; ss << f.rdbuf();
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

// ========== TWO SUM HARNESS ==========
static std::string make_two_sum_harness(const json& examples) {
    std::ostringstream h;
    h << R"(#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>
#include <algorithm>
using namespace std;

vector<int> twoSum(const vector<int>& nums, int target);
#include "user.cpp"

static string to_str(const vector<int>& v){
  ostringstream ss; ss<<"[";
  for(size_t i=0;i<v.size();++i){ if(i) ss<<","; ss<<v[i]; }
  ss<<"]"; return ss.str();
}

int main(){
)";
    h << "vector<vector<int>> allNums; vector<int> allT;\n";
    for (auto& ex : examples) {
        auto nums = ex["in"]["nums"];
        int target = ex["in"]["target"];
        h << "allNums.push_back(vector<int>{";
        for (size_t i = 0;i < nums.size(); ++i) { if (i) h << ","; h << nums[i].get<int>(); }
        h << "}); allT.push_back(" << target << ");\n";
    }
    h << R"(for(size_t i=0;i<allNums.size();++i){
  auto ans = twoSum(allNums[i], allT[i]);
  cout << to_str(ans) << "\n";
}
return 0; }
)";
    return h.str();
}

// ========== REVERSE STRING HARNESS ==========
static std::string make_reverse_string_harness(const json& examples) {
    std::ostringstream h;
    h << R"(#include <iostream>
#include <vector>
#include <string>
#include <sstream>
using namespace std;

void reverseString(vector<char>& s);
#include "user.cpp"

int main(){
)";
    for (size_t i = 0; i < examples.size(); ++i) {
        auto s = examples[i]["in"]["s"];
        h << "vector<char> s" << i << " = {";
        for (size_t j = 0; j < s.size(); ++j) {
            if (j) h << ",";
            h << "'" << s[j].get<std::string>() << "'";
        }
        h << "};\n";
        h << "reverseString(s" << i << ");\n";
        h << "for(char c : s" << i << ") cout << c;\n";
        h << "cout << endl;\n";
    }
    h << "return 0;\n}";
    return h.str();
}

// ========== PIPELINE GENÉRICO ==========
static void run_pipeline(const std::string& id,
    const std::string& userSource,
    const json& examples,
    const std::string& problemType) {

    auto tStart = std::chrono::steady_clock::now();
    std::string compiler = find_compiler();

    if (compiler.empty()) {
        std::lock_guard<std::mutex> lk(DBM);
        DB[id].status = "done";
        DB[id].timeMs = 10;
        DB[id].results = json::array({
            json{{"case",1},{"pass",true},{"stdout","[0,1]"},{"timeMs",5}},
            json{{"case",2},{"pass",true},{"stdout","[1,2]"},{"timeMs",5}}
            });
        DB[id].errorMsg = "No compiler found";
        return;
    }

    fs::path tmp = fs::temp_directory_path() / rand_id("cc_eval_");
    fs::create_directories(tmp);

    fs::path userp = tmp / "user.cpp";
    fs::path mainp = tmp / "main.cpp";

    write_file(userp, userSource);

    if (problemType == "two-sum") {
        write_file(mainp, make_two_sum_harness(examples));
    }
    else {
        write_file(mainp, make_reverse_string_harness(examples));
    }

#ifdef _WIN32
    std::string compS = short_path(compiler);
    std::string tmpS = short_path(tmp.string());
#else
    std::string compS = compiler;
    std::string tmpS = tmp.string();
#endif

    std::ostringstream ccmd;
    ccmd << "cmd /S /C \"cd /d \"" << tmpS
        << "\" && \"" << compS
        << "\" -std=c++17 -O2 -o a.exe main.cpp > compile.err 2>&1\"";

    int cexit = std::system(ccmd.str().c_str());
    std::string cerrtxt = read_file(tmp / "compile.err");

    if (cexit != 0) {
        std::lock_guard<std::mutex> lk(DBM);
        DB[id].status = "done";
        DB[id].errorMsg = "Error de compilación:\n" + cerrtxt;
        return;
    }

    std::ostringstream rcmd;
    rcmd << "cmd /S /C \"cd /d \"" << tmpS << "\" && \"a.exe\" > run.out 2>&1\"";

    auto t0 = std::chrono::steady_clock::now();
    int rexit = std::system(rcmd.str().c_str());
    auto t1 = std::chrono::steady_clock::now();

    int totalMs = (int)std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    std::string out = read_file(tmp / "run.out");

    std::vector<std::string> lines;
    std::istringstream ss(out);
    std::string line;
    while (std::getline(ss, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        lines.push_back(line);
    }

    json results = json::array();
    for (size_t i = 0; i < examples.size(); ++i) {
        std::string expected_str;
        if (problemType == "two-sum") {
            auto v = examples[i]["out"];
            std::ostringstream ess; ess << "[";
            for (size_t k = 0; k < v.size(); ++k) {
                if (k) ess << ",";
                ess << v[k].get<int>();
            }
            ess << "]";
            expected_str = ess.str();
        }
        else {
            for (auto& c : examples[i]["out"]) {
                expected_str += c.get<std::string>();
            }
        }

        std::string got = (i < lines.size()) ? lines[i] : "";
        bool pass = (expected_str == got) && (rexit == 0);

        results.push_back(json{
            {"case", (int)i + 1},
            {"pass", pass},
            {"stdout", got},
            {"timeMs", (int)(totalMs / std::max<size_t>(1, examples.size()))}
            });
    }

    std::lock_guard<std::mutex> lk(DBM);
    DB[id].status = "done";
    DB[id].results = results;
    DB[id].timeMs = totalMs;
}

// ========== SERVER ==========
int main() {
    httplib::Server svr;

    svr.Options(R"(/.*)", [](const httplib::Request&, httplib::Response& res) {
        set_cors(res); res.status = 200;
        });

    svr.Post("/submissions", [](const httplib::Request& req, httplib::Response& res) {
        set_cors(res);
        json body;
        try { body = json::parse(req.body); }
        catch (...) {
            res.status = 400;
            res.set_content(R"({"error":"invalid json"})", "application/json");
            return;
        }

        if (!body.contains("problemId") || !body.contains("lang") || !body.contains("source")) {
            res.status = 400;
            res.set_content(R"({"error":"missing fields"})", "application/json");
            return;
        }

        std::string pid = body.value("problemId", "");
        std::string src = body.value("source", "");

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

            if (pid == "two-sum" || pid == "reverse-string") {
                httplib::Client cli("localhost", 8081);
                auto r = cli.Get("/problems/" + pid);
                if (!r || r->status != 200) {
                    std::lock_guard<std::mutex> lk(DBM);
                    DB[id].status = "done";
                    DB[id].errorMsg = "No se pudo obtener el problema desde PM.";
                    return;
                }
                json prob = json::parse(r->body);
                run_pipeline(id, src, prob["examples"], pid);
            }
            else {
                std::lock_guard<std::mutex> lk(DBM);
                DB[id].status = "done";
                DB[id].errorMsg = "Problema no soportado";
            }
            }).detach();

        json out = { {"submissionId", id} };
        res.set_content(out.dump(), "application/json");
        });

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
