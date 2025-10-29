#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

#include "httplib.h"      // third_party/httplib.h
#include "json.hpp"       // third_party/json.hpp
using json = nlohmann::json;

struct Problem {
    std::string id, title, difficulty;
    std::vector<std::string> tags;
    std::string statement;
    json examples; // [{ "in": {...}, "out": ... }]
};

static void set_cors(httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET,POST,OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
}

int main() {
    // ---- Base de datos en memoria (MVP) ----
    std::unordered_map<std::string, Problem> DB;

    DB["two-sum"] = Problem{
      "two-sum",
      "Two Sum",
      "easy",
      {"array","hashmap"},
      "Dado un arreglo nums y un entero target, retorna indices i y j tales que "
      "nums[i] + nums[j] = target. Asume una unica solucion y no reutilices el "
      "mismo elemento.",
      json::array({
        json{
          {"in",  json{{"nums", json::array({2,7,11,15})}, {"target", 9}}},
          {"out", json::array({0,1})}
        },
        json{
          {"in",  json{{"nums", json::array({3,2,4})}, {"target", 6}}},
          {"out", json::array({1,2})}
        }
      })
    };

    auto to_summary = [](const Problem& p) {
        return json{
          {"id", p.id},
          {"title", p.title},
          {"difficulty", p.difficulty},
          {"tags", p.tags}
        };
        };

    auto to_detail = [](const Problem& p) {
        return json{
          {"id", p.id},
          {"title", p.title},
          {"difficulty", p.difficulty},
          {"tags", p.tags},
          {"statement", p.statement},
          {"examples", p.examples}
        };
        };

    httplib::Server svr;

    // ---- Preflight CORS ----
    svr.Options(R"(.*)", [&](const httplib::Request&, httplib::Response& res) {
        set_cors(res);
        res.status = 200;
        });

    // ---- Endpoints REST ----
    svr.Get("/problems", [&](const httplib::Request&, httplib::Response& res) {
        json arr = json::array();
        for (auto& kv : DB) arr.push_back(to_summary(kv.second));
        set_cors(res);
        res.set_content(arr.dump(), "application/json");
        });

    // /problems/{id}
    svr.Get(R"(/problems/(.+))", [&](const httplib::Request& req, httplib::Response& res) {
        auto id = req.matches[1].str();
        auto it = DB.find(id);
        if (it == DB.end()) { res.status = 404; return; }
        set_cors(res);
        res.set_content(to_detail(it->second).dump(), "application/json");
        });

    // ---- Servir estáticos (build de React) ----
    // Copiaremos la build de la UI a ./public más adelante.
    svr.set_mount_point("/", "./public");

    std::cout << "[PM] Escuchando en http://localhost:8081\n";
    if (!svr.listen("0.0.0.0", 8081)) {
        std::cerr << "No se pudo abrir el puerto 8081\n";
        return 1;
    }
    return 0;
}
