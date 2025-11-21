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
        # >>> LO QUE LA UI ESPERA <<<
        "statement": (
            "Given an array of integers nums and an integer target, "
            "return indices of the two numbers such that they add up to target."
        ),
        # Ejemplos que se muestran al usuario
        "examples": [
            {
                "in": "nums = [2,7,11,15], target = 9",
                "out": "[0,1]",
            },
            {
                "in": "nums = [3,2,4], target = 6",
                "out": "[1,2]",
            },
        ],
        # Código base para el editor
        "starterCode": (
            "class Solution {\n"
            "public:\n"
            "    vector<int> twoSum(vector<int>& nums, int target) {\n"
            "        // tu código aquí\n"
            "    }\n"
            "};\n"
        ),
        # Tests que luego puede usar el Evaluator
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
        "tags": ["string", "two-pointers"],
        "statement": (
            "Write a function that reverses a string. The input string is given "
            "as an array of characters s. You must do this by modifying the input "
            "array in-place with O(1) extra memory."
        ),
        "examples": [
            {
                "in": 's = ["h","e","l","l","o"]',
                "out": '["o","l","l","e","h"]',
            },
            {
                "in": 's = ["H","a","n","n","a","h"]',
                "out": '["h","a","n","n","a","H"]',
            },
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
]

# Por ahora: limpiamos y sembramos siempre (útil en desarrollo)
problems_collection.delete_many({})
problems_collection.insert_many(sample_problems)
print("✅ 2 problemas insertados en MongoDB")


# =======================
#        ENDPOINTS
# =======================

@app.route("/problems", methods=["GET"])
def get_problems():
    problems = list(problems_collection.find({}, {"_id": 0}))
    return jsonify(problems)


@app.route("/problems/<problem_id>", methods=["GET"])
def get_problem(problem_id):
    problem = problems_collection.find_one({"id": problem_id}, {"_id": 0})
    if problem:
        return jsonify(problem)
    return jsonify({"error": "Problem not found"}), 404


@app.route("/problems", methods=["POST"])
def create_problem():
    data = request.json or {}
    result = problems_collection.insert_one(data)
    return jsonify({"id": data.get("id", str(result.inserted_id))}), 201


if __name__ == "__main__":
    print("🚀 Problem Manager Python iniciando en http://localhost:8081")
    app.run(host="0.0.0.0", port=8081, debug=True)
