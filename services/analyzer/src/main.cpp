#include <iostream>
#include <string>
#include <unordered_map>

#include "../third_party/httplib.h"
#include "../third_party/json.hpp"
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

// Análisis predefinido por problema
static AnalysisResult analyze_two_sum(const AnalysisRequest& req) {
    bool all_passed = true;
    if (req.results.contains("results")) {
        for (const auto& r : req.results["results"]) {
            if (r.contains("pass") && !r["pass"]) {
                all_passed = false;
                break;
            }
        }
    }

    if (all_passed) {
        return AnalysisResult{
            {"¡Excelente! Has resuelto Two Sum correctamente.",
             "Podrías intentar optimizar el uso de memoria."},
            {"hashmap", "two-pointers"},
            "O(n)"
        };
    }
    else {
        return AnalysisResult{
            {"Piensa en usar un mapa para almacenar los números y sus índices.",
             "Recuerda que necesitas encontrar dos números que sumen al target.",
             "Considera qué estructura de datos te permite búsquedas rápidas."},
            {"hashmap"},
            "O(n) esperada"
        };
    }
}

static AnalysisResult analyze_reverse_string(const AnalysisRequest& req) {
    bool all_passed = true;
    if (req.results.contains("results")) {
        for (const auto& r : req.results["results"]) {
            if (r.contains("pass") && !r["pass"]) {
                all_passed = false;
                break;
            }
        }
    }

    if (all_passed) {
        return AnalysisResult{
            {"¡Bien hecho! Reverse String resuelto correctamente.",
             "¿Podrías hacerlo recursivamente?"},
            {"two-pointers", "in-place"},
            "O(n)"
        };
    }
    else {
        return AnalysisResult{
            {"Intenta usar el enfoque de dos punteros: uno al inicio y otro al final.",
             "Recuerda que debes modificar el array in-place, sin crear uno nuevo.",
             "Intercambia los caracteres mientras los punteros se acercan."},
            {"two-pointers", "in-place"},
            "O(n) esperada"
        };
    }
}

int main() {
    httplib::Server svr;

    // CORS preflight
    svr.Options(R"(/.*)", [](const httplib::Request&, httplib::Response& res) {
        set_cors(res);
        res.status = 200;
        });

    // POST /analysis
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

        if (!body.contains("source") || !body.contains("results")) {
            res.status = 400;
            res.set_content(R"({"error":"missing source or results"})", "application/json");
            return;
        }

        AnalysisRequest analysis_req;
        analysis_req.source = body.value("source", "");
        analysis_req.results = body.value("results", json::object());
        analysis_req.problemId = body.value("problemId", "");

        AnalysisResult result;

        // Seleccionar análisis según el problema
        if (analysis_req.problemId == "two-sum") {
            result = analyze_two_sum(analysis_req);
        }
        else if (analysis_req.problemId == "reverse-string") {
            result = analyze_reverse_string(analysis_req);
        }
        else {
            // Análisis genérico para problemas desconocidos
            result = AnalysisResult{
                {"Revisa la lógica de tu solución.", "Considera casos edge."},
                {"unknown"},
                "O(?)"
            };
        }

        json response = {
            {"hints", result.hints},
            {"probablePatterns", result.probablePatterns},
            {"complexityEstimate", result.complexityEstimate}
        };

        res.set_content(response.dump(), "application/json");
        });

    std::cout << "[ANA] Analyzer escuchando en http://localhost:8083\n";
    if (!svr.listen("0.0.0.0", 8083)) {
        std::cerr << "No se pudo abrir el puerto 8083\n";
        return 1;
    }
    return 0;
}
