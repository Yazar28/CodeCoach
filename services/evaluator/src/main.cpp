#include <string>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <chrono>
#include <random>

#include "httplib.h"
#include "json.hpp"

using json = nlohmann::json;
using namespace std::chrono_literals;

struct Submission {
    std::string id;
    std::string status;  // "queued" | "running" | "done" | "error"
    json results;        // array de casos [{case, pass, stdout, timeMs}]
    int timeMs = 0;
    int memoryKB = 0;
};

static std::unordered_map<std::string, Submission> DB;
static std::mutex DBM;

static void set_cors(httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET,POST,OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
}

static std::string rand_id() {
    static std::mt19937_64 rng{ std::random_device{}() };
    static const char* k = "abcdefghijklmnopqrstuvwxyz0123456789";
    std::string s = "sub-";
    for (int i = 0; i < 6; ++i) s += k[rng() % 36];
    return s;
}

int main() {
    httplib::Server svr;

    // Preflight CORS
    svr.Options(R"(/.*)", [](const httplib::Request&, httplib::Response& res) {
        set_cors(res);
        res.status = 200;
        });

    // POST /submissions
    svr.Post("/submissions", [](const httplib::Request& req, httplib::Response& res) {
        set_cors(res);

        json body;
        try { body = json::parse(req.body); }
        catch (...) {
            res.status = 400; res.set_content(R"({"error":"invalid json"})", "application/json"); return;
        }

        // (MVP) validaciones mínimas
        if (!body.contains("problemId") || !body.contains("lang") || !body.contains("source")) {
            res.status = 400; res.set_content(R"({"error":"missing fields"})", "application/json"); return;
        }

        auto id = rand_id();
        {
            std::lock_guard<std::mutex> lk(DBM);
            DB[id] = Submission{ id, "queued", json::array(), 0, 0 };
        }

        // Simular ejecución asíncrona
        std::thread([id]() {
            {
                std::lock_guard<std::mutex> lk(DBM);
                if (DB.count(id)) DB[id].status = "running";
            }
            std::this_thread::sleep_for(400ms);

            // (MVP) resultados fijos compatibles con Two Sum
            json cases = json::array({
              json{{"case",1},{"pass",true},{"timeMs",5},{"stdout","[0,1]"}},
              json{{"case",2},{"pass",true},{"timeMs",7},{"stdout","[1,2]"}}
                });

            {
                std::lock_guard<std::mutex> lk(DBM);
                if (DB.count(id)) {
                    DB[id].status = "done";
                    DB[id].results = cases;
                    DB[id].timeMs = 12;
                    DB[id].memoryKB = 256;
                }
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
        if (it == DB.end()) {
            res.status = 404;
            res.set_content(R"({"error":"not found"})", "application/json");
            return;
        }

        const auto& sub = it->second;
        json out = {
          {"status",   sub.status},
          {"results",  sub.results},
          {"timeMs",   sub.timeMs},
          {"memoryKB", sub.memoryKB}
        };
        res.set_content(out.dump(), "application/json");
        });

    const char* host = "0.0.0.0";
    int port = 8082;
    printf("[EV] Escuchando en http://%s:%d\n", host, port);
    svr.listen(host, port);
}
