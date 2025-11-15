#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include "httplib.h"
#include "json.hpp"

using json = nlohmann::json;

struct AnalysisRequest {
    std::string source;
    json results;
    std::string problemId;
};

struct AnalysisResult {
    std::vector<std::string> hints;
    std::vector<std::string> probablePatterns;
    std::string complexityEstimate;
};

static void set_cors(httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET,POST,OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
}

static AnalysisResult analyze_two_sum(const AnalysisRequest& req) {
    bool all_passed = true;
    if (req.results.is_object() && req.results.contains("results")) {
        for (const auto& r : req.results["results"]) {
            if (r.is_object() && r.contains("pass") && !r["pass"].get<bool>()) {
                all_passed = false;
                break;
            }
        }
    }

    if (all_passed) {
        return {
            {"¡Excelente! Has resuelto Two Sum correctamente.",
             "Podrías intentar optimizar el uso de memoria."},
            {"hashmap", "two-pointers"},
            "O(n)"
        };
    }
    else {
        return {
            {"Piensa en usar un mapa para almacenar valor→índice.",
             "Evita reutilizar el mismo elemento dos veces.",
             "Busca reducir a O(n) con búsqueda O(1)."},
            {"hashmap"},
            "O(n) esperada"
        };
    }
}

static AnalysisResult analyze_reverse_string(const AnalysisRequest& req) {
    bool all_passed = true;
    if (req.results.is_object() && req.results.contains("results")) {
        for (const auto& r : req.results["results"]) {
            if (r.is_object() && r.contains("pass") && !r["pass"].get<bool>()) {
                all_passed = false;
                break;
            }
        }
    }

    if (all_passed) {
        return {
            {"¡Bien hecho! Pasaste los casos.",
             "¿Puedes implementar también una versión recursiva?"},
            {"two-pointers", "in-place"},
            "O(n)"
        };
    }
    else {
        return {
            {"Usa dos punteros (inicio/fin) e intercambia en cada paso.",
             "Hazlo in-place sin crear otro arreglo.",
             "Cuidado con el límite cuando los punteros se cruzan."},
            {"two-pointers", "in-place"},
            "O(n) esperada"
        };
    }
}

int main() {
    httplib::Server svr;

    svr.Options(R"(/.*)", [](const httplib::Request&, httplib::Response& res) {
        set_cors(res);
        res.status = 200;
        });

    svr.Get("/health", [](const httplib::Request&, httplib::Response& res) {
        set_cors(res);
        res.set_content(R"({"ok":true})", "application/json");
        });

    svr.Post("/analysis", [](const httplib::Request& req, httplib::Response& res) {
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

        AnalysisRequest areq;
        areq.source = body.value("source", "");
        areq.results = body.value("results", json::object());
        areq.problemId = body.value("problemId", "");

        AnalysisResult ar;
        if (areq.problemId == "two-sum") {
            ar = analyze_two_sum(areq);
        }
        else if (areq.problemId == "reverse-string") {
            ar = analyze_reverse_string(areq);
        }
        else {
            ar = {
                {"Revisa la lógica y cubre casos borde."},
                {"unknown"},
                "O(?)"
            };
        }

        json out = {
            {"hints", ar.hints},
            {"probablePatterns", ar.probablePatterns},
            {"complexityEstimate", ar.complexityEstimate}
        };

        res.set_content(out.dump(), "application/json");
        });

    std::cout << "[ANA] Analyzer escuchando en http://localhost:8083\n";
    if (!svr.listen("0.0.0.0", 8083)) {
        std::cerr << "No se pudo abrir el puerto 8083\n";
        return 1;
    }

    return 0;
}
