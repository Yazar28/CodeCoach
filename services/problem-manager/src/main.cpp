#include "httplib.h"
#include "json.hpp"
#include <iostream>

using json = nlohmann::json;

// CORS igual que en analyzer/evaluator
static void set_cors(httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET,POST,OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
}

int main() {
    httplib::Server svr;

    // OPTIONS genérico para CORS
    svr.Options(R"(/.*)", [](const httplib::Request&, httplib::Response& res) {
        set_cors(res);
        res.status = 200;
        });

    // Health check simple
    svr.Get("/health", [](const httplib::Request&, httplib::Response& res) {
        set_cors(res);
        res.set_content(R"({"ok":true,"service":"problem-manager-cpp"})", "application/json");
        });

    // Cliente hacia el microservicio Python (mongo_manager.py en 8081)
    httplib::Client mongo("localhost", 8081);

    // GET /problems  -> proxy a Python
    svr.Get("/problems", [&mongo](const httplib::Request&, httplib::Response& res) {
        set_cors(res);

        auto pres = mongo.Get("/problems");
        if (!pres) {
            res.status = 500;
            res.set_content(R"({"error":"mongo_manager no responde en :8081"})", "application/json");
            return;
        }

        res.status = pres->status;
        // Siempre devolvemos JSON
        res.set_content(pres->body, "application/json");
        });

    // GET /problems/<id>  -> proxy a Python
    svr.Get(R"(/problems/([A-Za-z0-9\-\_]+))", [&mongo](const httplib::Request& req, httplib::Response& res) {
        set_cors(res);
        std::string id = req.matches[1];

        std::string path = "/problems/" + id;
        auto pres = mongo.Get(path.c_str());
        if (!pres) {
            res.status = 500;
            res.set_content(R"({"error":"mongo_manager no responde en :8081"})", "application/json");
            return;
        }

        res.status = pres->status;
        res.set_content(pres->body, "application/json");
        });

    // POST /problems  -> crear problema (proxy a Python)
    svr.Post("/problems", [&mongo](const httplib::Request& req, httplib::Response& res) {
        set_cors(res);

        auto pres = mongo.Post("/problems", req.body, "application/json");
        if (!pres) {
            res.status = 500;
            res.set_content(R"({"error":"mongo_manager no responde en :8081"})", "application/json");
            return;
        }

        res.status = pres->status;
        res.set_content(pres->body, "application/json");
        });

    std::cout << "[PM-CPP] Escuchando en http://0.0.0.0:8084\n";
    if (!svr.listen("0.0.0.0", 8084)) {
        std::cerr << "No se pudo abrir el puerto 8084\n";
        return 1;
    }

    return 0;
}
