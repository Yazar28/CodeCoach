#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>

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

// -------------------- CORS --------------------

static void set_cors(httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET,POST,OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
}

// -------------------- HINTS BASE (REGLAS) --------------------

// Two Sum
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
            {
                "¡Excelente! Has resuelto Two Sum correctamente.",
                "Podrías intentar optimizar el uso de memoria o comentar mejor tu solución."
            },
            {"hashmap", "two-pointers"},
            "O(n)"
        };
    }
    else {
        return {
            {
                "Piensa en usar una tabla (hash map) para almacenar valor→índice.",
                "Evita reutilizar el mismo elemento dos veces.",
                "Intenta reducir la complejidad a O(n) con búsquedas O(1)."
            },
            {"hashmap"},
            "O(n) esperada"
        };
    }
}

// Reverse String
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
            {
                "¡Bien hecho! Tu función para invertir la cadena pasa todos los casos.",
                "Como reto adicional, intenta implementar también una versión recursiva."
            },
            {"two-pointers", "in-place"},
            "O(n)"
        };
    }
    else {
        return {
            {
                "Usa dos punteros, uno al inicio y otro al final, e intercambia caracteres.",
                "Haz la inversión in-place, sin crear otro arreglo de apoyo.",
                "Cuidado con el caso cuando los punteros se cruzan o se encuentran."
            },
            {"two-pointers", "in-place"},
            "O(n) esperada"
        };
    }
}

// Binary Search
static AnalysisResult analyze_binary_search(const AnalysisRequest& req) {
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
            {
                "Excelente, tu implementación de binary search pasó todos los casos.",
                "Revisa si manejas correctamente casos borde como arreglos vacíos o target fuera del rango."
            },
            {"binary-search", "divide-and-conquer"},
            "O(log n)"
        };
    }
    else {
        return {
            {
                "Recuerda que en binary search reduces el intervalo a la mitad en cada paso.",
                "Verifica las condiciones de los punteros left y right para evitar bucles infinitos.",
                "Cuidado con errores off-by-one al actualizar mid, left y right."
            },
            {"binary-search", "divide-and-conquer"},
            "O(log n) esperada"
        };
    }
}

// -------------------- PROMPT PARA LA IA --------------------

static std::string build_llm_prompt(const AnalysisRequest& req) {
    std::ostringstream oss;

    oss << "Eres un asistente que ayuda a estudiantes a mejorar soluciones de algoritmos en C++.\n"
        "No debes dar la solución completa ni pegar código final listo para copiar.\n"
        "Solo da sugerencias, pistas, posibles errores y mejoras.\n"
        "Responde en español, de forma clara y breve.\n\n";

    oss << "ID del problema: " << req.problemId << "\n";

    // Resumen de resultados
    if (req.results.is_object() && req.results.contains("results")) {
        const auto& arr = req.results["results"];
        int total = (int)arr.size();
        int passed = 0;
        for (const auto& r : arr) {
            if (r.is_object() && r.contains("pass") && r["pass"].get<bool>()) {
                ++passed;
            }
        }
        oss << "Casos de prueba: " << passed << " de " << total << " pasaron.\n";
        oss << "Detalle de algunos casos:\n";
        int count = 0;
        for (const auto& r : arr) {
            if (count >= 4) break;
            if (!r.is_object()) continue;
            oss << "- Caso " << r.value("case", -1) << ": "
                << (r.value("pass", false) ? "PASS" : "FAIL");
            if (r.contains("stdout")) {
                oss << ", salida = " << r["stdout"].dump();
            }
            if (r.contains("expected")) {
                oss << ", esperado = " << r["expected"].dump();
            }
            oss << "\n";
            ++count;
        }
    }
    else {
        oss << "No se recibieron resultados detallados.\n";
    }

    // Código fuente (truncado)
    if (!req.source.empty()) {
        std::string src = req.source;
        if (src.size() > 1600) {
            src = src.substr(0, 1600);
            src += "\n// (código truncado)\n";
        }
        oss << "\nCódigo del estudiante (C++):\n";
        oss << "-------------------------\n";
        oss << src << "\n";
        oss << "-------------------------\n";
    }

    oss << "\nPor favor, da sugerencias, posibles errores y mejoras.\n"
        "No des una solución completa ni el código final.\n";

    return oss.str();
}

// -------------------- LLAMAR AL PROXY PYTHON --------------------

static std::string call_llm_via_proxy(const AnalysisRequest& req) {
    std::string prompt = build_llm_prompt(req);

    // Cliente HTTP hacia el proxy en Python (llm_proxy) en localhost:8090
    httplib::Client cli("localhost", 8090);

    json payload = {
        {"prompt", prompt},
        {"problemId", req.problemId}
    };

    auto res = cli.Post("/llm-feedback", payload.dump(), "application/json");
    if (!res) {
        return "No se pudo contactar al servicio LLM (llm_proxy en puerto 8090). "
            "Verifica que llm_proxy.py esté corriendo.";
    }
    if (res->status != 200) {
        return "Error desde el servicio LLM: HTTP " + std::to_string(res->status);
    }

    try {
        auto body = json::parse(res->body);
        if (body.contains("feedback")) {
            return body["feedback"].get<std::string>();
        }
        else {
            return "Respuesta del servicio LLM sin campo 'feedback'.";
        }
    }
    catch (...) {
        return "No se pudo parsear la respuesta del servicio LLM.";
    }
}

// -------------------- MAIN SERVER --------------------

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
        else if (areq.problemId == "binary-search") {
            ar = analyze_binary_search(areq);
        }
        else {
            ar = {
                {"Revisa la lógica y cubre casos borde."},
                {"unknown"},
                "O(?)"
            };
        }

        // Llamar al servicio LLM y agregar su feedback a las pistas
        std::string llm_text = call_llm_via_proxy(areq);

        // Encabezado fijo
        ar.hints.push_back("Sugerencia generada por IA:");

        // Dividir la respuesta de la IA en líneas y agregar cada una como hint aparte
        std::stringstream ss(llm_text);
        std::string line;

        while (std::getline(ss, line)) {
            // Quitar espacios en blanco al inicio y fin (trim sencillo)
            auto start = line.find_first_not_of(" \t\r\n");
            auto end = line.find_last_not_of(" \t\r\n");

            if (start == std::string::npos) continue; // línea vacía

            std::string cleaned = line.substr(start, end - start + 1);

            if (!cleaned.empty()) {
                ar.hints.push_back(cleaned);
            }
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
