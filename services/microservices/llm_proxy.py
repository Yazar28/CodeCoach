from flask import Flask, request, jsonify
from openai import OpenAI, OpenAIError
from dotenv import load_dotenv
import os

load_dotenv()

app = Flask(__name__)

# Cliente para OpenRouter (usa API tipo OpenAI)
client = OpenAI(
    api_key=os.environ.get("OPENROUTER_API_KEY"),
    base_url="https://openrouter.ai/api/v1",
)

INSTRUCTION = (
    "Eres un asistente que ayuda a estudiantes a mejorar soluciones de algoritmos en C++.\n"
    "NO debes dar una solución completa ni código listo para copiar.\n"
    "Solo da sugerencias, pistas, posibles errores y mejoras.\n"
    "Habla en español, en un tono claro y breve.\n"
    "FORMATO DE SALIDA OBLIGATORIO:\n"
    "- Complejidad: ...\n"
    "- Estilo: ...\n"
    "- Optimizaciones: ...\n"
    "Cada sugerencia debe ir en una línea distinta, empezando con '- '.\n"
    "No uses negritas ni otro markdown, solo texto plano."
)

@app.post("/llm-feedback")
def llm_feedback():
    data = request.get_json(force=True)
    prompt = data.get("prompt", "")
    problem_id = data.get("problemId", "unknown")

    if not prompt:
        return jsonify({"error": "prompt vacío"}), 400

    # Construimos TODO como un único mensaje de usuario
    full_prompt = (
        INSTRUCTION
        + "\n\nID del problema: "
        + problem_id
        + "\n\nContexto de la ejecución y del código del estudiante:\n"
        + prompt
        + "\n\nRecuerda: no des una solución completa ni el código final; "
          "solo pistas, posibles errores y mejoras."
    )

    try:
        resp = client.chat.completions.create(
            # 👇 Modelo free de xAI (Grok) que viste en la lista
            model="x-ai/grok-4.1-fast:free",
            messages=[
                {
                    "role": "user",
                    "content": full_prompt,
                }
            ],
            max_tokens=350,
            temperature=0.3,
        )

        text = resp.choices[0].message.content
        return jsonify({"feedback": text})

    except OpenAIError as e:
        print("Error al llamar a OpenRouter:", repr(e))
        msg = str(e)

        # Rate limit / cuota
        if "429" in msg or "rate" in msg.lower():
            return jsonify({
                "feedback": (
                    "La IA está temporalmente saturada (límite de uso alcanzado en el modelo gratuito). "
                    "Tu solución es válida y el sistema funciona, "
                    "pero en este momento el modelo no puede responder. "
                    "Intenta ejecutar de nuevo en unos segundos."
                )
            }), 200

        return jsonify({"error": msg}), 500

    except Exception as e:
        print("Error inesperado en llm_proxy:", repr(e))
        return jsonify({"error": "error inesperado en llm_proxy"}), 500


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=8090)
