from flask import Flask, jsonify, request
from flask_cors import CORS
from pymongo import MongoClient

app = Flask(__name__)
CORS(app)

# Conexión a MongoDB
client = MongoClient("mongodb://localhost:27017/")
db = client["codecoach"]
problems_collection = db["problems"]

# =======================
#  Datos de ejemplo (seed)
# =======================
sample_problems = [
    {
        "id": "two-sum",
        "title": "Two Sum",
        "difficulty": "easy",
        "tags": ["array", "hash-table"],
        "statement": "Dado un arreglo de enteros nums y un entero target, devuelve los índices de los dos números tales que sumen target.",
        "examples": [
            {
                "in": "nums = [2,7,11,15], target = 9",
                "out": "[0,1]"
            }
        ],
        "starterCode": (
            "class Solution {\n"
            "public:\n"
            "    vector<int> twoSum(vector<int>& nums, int target) {\n"
            "        // tu código aquí\n"
            "        return {};\n"
            "    }\n"
            "};\n"
        ),
        "tests": [
            {
                "in": {"nums": [2, 7, 11, 15], "target": 9},
                "out": [0, 1],
            },
            {
                "in": {"nums": [3, 2, 4], "target": 6},
                "out": [1, 2],
            },
        ],
    },
    {
        "id": "reverse-string",
        "title": "Reverse String",
        "difficulty": "easy",
        "tags": ["two-pointers", "string"],
        "statement": "Escribe una función que invierta un arreglo de caracteres in-place.",
        "examples": [
            {
                "in": "s = ['h','e','l','l','o']",
                "out": "['o','l','l','e','h']",
            }
        ],
        "starterCode": (
            "class Solution {\n"
            "public:\n"
            "    void reverseString(vector<char>& s) {\n"
            "        // tu código aquí\n"
            "    }\n"
            "};\n"
        ),
        "tests": [
            {
                "in": {"s": ["h", "e", "l", "l", "o"]},
                "out": ["o", "l", "l", "e", "h"],
            },
            {
                "in": {"s": ["H", "a", "n", "n", "a", "h"]},
                "out": ["h", "a", "n", "n", "a", "H"],
            },
        ],
    },
    {
        "id": "binary-search",
        "title": "Binary Search",
        "difficulty": "easy",
        "tags": ["array", "binary-search"],
        "statement": "Dado un arreglo ordenado nums y un entero target, devuelve el índice de target o -1 si no está.",
        "examples": [
            {
                "in": "nums = [-1,0,3,5,9,12], target = 9",
                "out": "4",
            }
        ],
        "starterCode": (
            "class Solution {\n"
            "public:\n"
            "    int search(vector<int>& nums, int target) {\n"
            "        // tu código aquí\n"
            "        return -1;\n"
            "    }\n"
            "};\n"
        ),
        "tests": [
            {
                "in": {"nums": [-1, 0, 3, 5, 9, 12], "target": 9},
                "out": 4,
            },
            {
                "in": {"nums": [-1, 0, 3, 5, 9, 12], "target": 2},
                "out": -1,
            },
            {
                "in": {"nums": [1], "target": 1},
                "out": 0,
            },
            {
                "in": {"nums": [1], "target": 2},
                "out": -1,
            },
        ],
    },
]


def ensure_seed_data():
    """Inserta problemas de ejemplo si la colección está vacía."""
    if problems_collection.count_documents({}) == 0:
        problems_collection.insert_many(sample_problems)
        print("🌱 Seed de problemas insertado en MongoDB")


# =======================
#  Rutas del API
# =======================


@app.route("/problems", methods=["GET"])
def list_problems():
    """Devuelve lista de problemas (resumen)."""
    docs = problems_collection.find(
        {},
        {
            "_id": 0,
            "id": 1,
            "title": 1,
            "difficulty": 1,
            "tags": 1,
        },
    )
    return jsonify(list(docs))

@app.route("/problems/<problem_id>", methods=["GET"])
def get_problem(problem_id):
    """Devuelve el problema completo por id."""
    problem = problems_collection.find_one({"id": problem_id}, {"_id": 0})
    if problem:
        return jsonify(problem)
    return jsonify({"error": "Problem not found"}), 404

@app.route("/problems", methods=["POST"])
def create_problem():
    """Crea un problema nuevo a partir del JSON enviado por la UI."""
    data = request.json or {}
    if "id" not in data:
        return jsonify({"error": "field 'id' is required"}), 400

    # Evitar duplicados por id
    existing = problems_collection.find_one({"id": data["id"]})
    if existing:
        return jsonify({"error": f"Problem with id '{data['id']}' already exists"}), 409

    problems_collection.insert_one(data)
    return jsonify({"id": data["id"]}), 201

@app.route("/problems/<problem_id>", methods=["DELETE"])
def delete_problem(problem_id):
    """Elimina un problema por id."""
    result = problems_collection.delete_one({"id": problem_id})
    if result.deleted_count == 0:
        return jsonify({"error": "Problem not found"}), 404
    return jsonify({"status": "deleted", "id": problem_id}), 200

if __name__ == "__main__":
    ensure_seed_data()
    print("🚀 Problem Manager Python iniciando en http://localhost:8081")
    app.run(host="0.0.0.0", port=8081, debug=True)
