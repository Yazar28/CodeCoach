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

// --- helpers Windows (rutas cortas) ---
#ifdef _WIN32
#include <windows.h>
static std::string short_path(const std::string& p) {
    char buf[MAX_PATH];
    DWORD n = GetShortPathNameA(p.c_str(), buf, MAX_PATH);
    if (n == 0 || n > MAX_PATH) return p; // si falla, devuelve la original
    return std::string(buf, n);
}
#endif

// ====== Modelo simple en memoria ======
struct Submission {
    std::string id;
    std::string status;  // queued | running | done
    json        results = json::array();
    int         timeMs = 0;
    int         memoryKB = 256;
    std::string errorMsg; // "note" para la UI
};

static std::unordered_map<std::string, Submission> DB;
static std::mutex DBM;

// ====== Utilidades ======
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

// ====== Detección del compilador ======
static std::string find_compiler() {
#ifdef _WIN32
    // Preferimos rutas absolutas primero, luego 8.3 si aplica
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
    if (std::system("g++ --version >/dev/null 2>&1") == 0)     return "g++";
    return "";
#endif
}

// ====== Harness específico Two Sum (sin <bits/stdc++.h>) ======
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

// ====== Pipeline: escribir, compilar, ejecutar, parsear ======
static void run_two_sum_pipeline(const std::string& id,
    const std::string& userSource,
    const json& examples) {
    auto tStart = std::chrono::steady_clock::now();

    std::string compiler = find_compiler();
    std::printf("[EV] Compiler detected: %s\n", compiler.empty() ? "<none>" : compiler.c_str());

    if (compiler.empty()) {
        std::lock_guard<std::mutex> lk(DBM);
        auto& s = DB[id];
        s.status = "done";
        s.timeMs = 12;
        s.results = json::array({
          json{{"case",1},{"pass",true },{"stdout","[0,1]"},{"timeMs",5}},
          json{{"case",2},{"pass",true },{"stdout","[1,2]"},{"timeMs",7}},
            });
        s.errorMsg = "No hay compilador en PATH (clang++/g++); se devolvió MOCK.";
        return;
    }

    // Carpeta temporal
    fs::path tmp = fs::temp_directory_path() / rand_id("cc_eval_");
    fs::create_directories(tmp);
    std::printf("[EV] Temp dir: %s\n", tmp.string().c_str());

    // Archivos de trabajo
    fs::path userp = tmp / "user.cpp";
    fs::path mainp = tmp / "main.cpp";
    fs::path exep = tmp / "a.exe";
    fs::path cmderr = tmp / "compile.err";
    fs::path runout = tmp / "run.out";

    // Escribir primero
    write_file(userp, userSource);
    write_file(mainp, make_two_sum_harness(examples));

    // Verificación
    std::printf("[EV] Paths check: tmp=%s exists=%d\n", tmp.string().c_str(), (int)fs::exists(tmp));
    std::printf("[EV] main.cpp exists=%d size=%zu\n", (int)fs::exists(mainp), read_file(mainp).size());
    std::printf("[EV] user.cpp exists=%d size=%zu\n", (int)fs::exists(userp), read_file(userp).size());

        // --- COMPILACIÓN (cd al tmp y usa rutas relativas) ---
    #ifdef _WIN32
        std::string compS = short_path(compiler);
        std::string tmpS = short_path(tmp.string());
    #else
        std::string compS = compiler;
        std::string tmpS = tmp.string();
    #endif

        // Guardamos también el comando (para depurar)
        std::ostringstream ccmd;
        ccmd << "cmd /S /C \"cd /d \"" << tmpS
            << "\" && \"" << compS
            << "\" -std=c++17 -O2 -o a.exe main.cpp > compile.err 2>&1\"";

        write_file(tmp / "compile_cmd.txt", ccmd.str());
        std::printf("[EV] Compile cmd: %s\n", ccmd.str().c_str());

        int cexit = std::system(ccmd.str().c_str());
        std::string cerrtxt = read_file(tmp / "compile.err");
        std::printf("[EV] Compile exit: %d, compile.err bytes: %zu\n", cexit, cerrtxt.size());

        if (cexit != 0) {
            std::lock_guard<std::mutex> lk(DBM);
            auto& s = DB[id];
            s.status = "done";
            s.timeMs = 0;
            s.results = json::array();
            s.errorMsg = std::string("Error de compilación:\n") + cerrtxt;
            return;
        }

        // --- EJECUCIÓN (cd al tmp y ejecuta relativo) ---
        // Nota: reusamos tmpS; NO lo redeclares aquí.
        std::ostringstream rcmd;
        rcmd << "cmd /S /C \"cd /d \"" << tmpS << "\" && \"a.exe\" > run.out 2>&1\"";
        std::printf("[EV] Run cmd: %s\n", rcmd.str().c_str());

        auto t0 = std::chrono::steady_clock::now();
        int rexit = std::system(rcmd.str().c_str());
        auto t1 = std::chrono::steady_clock::now();

        int totalMs = (int)std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
        std::string out = read_file(tmp / "run.out");
        std::printf("[EV] Run exit: %d, timeMs: %d, run.out bytes: %zu\n", rexit, totalMs, out.size());

        // Parseo: una línea por caso
        std::vector<std::string> lines;
        {
            std::istringstream ss(out);
            std::string line;
            while (std::getline(ss, line)) {
                if (!line.empty() && line.back() == '\r') line.pop_back();
                lines.push_back(line);
            }
        }

        // Construir resultados contra los expected del PM
        json results = json::array();
        for (size_t i = 0;i < examples.size(); ++i) {
            // expected como "[a,b]"
            std::string expected;
            {
                auto v = examples[i]["out"];
                std::ostringstream ss; ss << "[";
                for (size_t k = 0;k < v.size(); ++k) { if (k) ss << ","; ss << v[k].get<int>(); }
                ss << "]"; expected = ss.str();
            }
            std::string got = (i < lines.size()) ? lines[i] : "";
            bool pass = (expected == got) && (rexit == 0);
            results.push_back(json{
              {"case",   (int)i + 1},
              {"pass",   pass},
              {"stdout", got},
              {"timeMs", (int)(totalMs / std::max<size_t>(1, examples.size()))}
                });
        }

        // Guardar estado final y cerrar la función
        {
            std::lock_guard<std::mutex> lk(DBM);
            auto& s = DB[id];
            s.status = "done";
            s.results = results;
            s.timeMs = totalMs;
        }

        // Limpieza opcional:
        // fs::remove_all(tmp);
    } // <--- AQUÍ cierras run_two_sum_pipeline


// ====== HTTP server ======
int main() {
    httplib::Server svr;

    // CORS preflight
    svr.Options(R"(/.*)", [](const httplib::Request&, httplib::Response& res) {
        set_cors(res); res.status = 200;
        });

    // POST /submissions
    svr.Post("/submissions", [](const httplib::Request& req, httplib::Response& res) {
        set_cors(res);

        json body;
        try { body = json::parse(req.body); }
        catch (...) { res.status = 400; res.set_content(R"({"error":"invalid json"})", "application/json"); return; }

        if (!body.contains("problemId") || !body.contains("lang") || !body.contains("source")) {
            res.status = 400; res.set_content(R"({"error":"missing fields"})", "application/json"); return;
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

            if (pid == "two-sum") {
                // Obtener problema/ejemplos desde PM
                httplib::Client cli("localhost", 8081);
                auto r = cli.Get("/problems/two-sum");
                if (!r || r->status != 200) {
                    std::lock_guard<std::mutex> lk(DBM);
                    auto& s = DB[id];
                    s.status = "done";
                    s.errorMsg = "No se pudo obtener el problema desde PM.";
                    return;
                }
                json prob = json::parse(r->body);
                run_two_sum_pipeline(id, src, prob["examples"]);
            }
            else {
                std::lock_guard<std::mutex> lk(DBM);
                auto& s = DB[id];
                s.status = "done";
                s.timeMs = 12;
                s.results = json::array({
                  json{{"case",1},{"pass",true },{"stdout","[0,1]"},{"timeMs",5}},
                  json{{"case",2},{"pass",true },{"stdout","[1,2]"},{"timeMs",7}},
                    });
                s.errorMsg = "Problema no soportado aún; MOCK.";
            }
            }).detach();

        json out = { {"submissionId", id} };
        res.set_content(out.dump(), "application/json");
        });

    // GET /submissions/:id
    svr.Get(R"(/submissions/([A-Za-z0-9\-]+))", [](const httplib::Request& req, httplib::Response& res) {
        set_cors(res);
        auto id = req.matches[1].str();

        std::lock_guard<std::mutex> lk(DBM);
        auto it = DB.find(id);
        if (it == DB.end()) { res.status = 404; res.set_content(R"({"error":"not found"})", "application/json"); return; }

        const auto& s = it->second;
        json out = {
          {"status",   s.status},
          {"results",  s.results},
          {"timeMs",   s.timeMs},
          {"memoryKB", s.memoryKB}
        };
        if (!s.errorMsg.empty()) out["note"] = s.errorMsg;
        res.set_content(out.dump(), "application/json");
        });

    const char* host = "0.0.0.0"; int port = 8082;
    std::printf("[EV] Escuchando en http://%s:%d\n", host, port);
    svr.listen(host, port);
    return 0;
}
