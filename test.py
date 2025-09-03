from flask import Flask, request, jsonify
import subprocess
import json

app = Flask(__name__)

@app.route("/search")
def search():
    query = request.args.get("q", "")
    if not query:
        return jsonify([])

    result = subprocess.run(["./out", query], capture_output=True, text=True)
    try:
        data = json.loads(result.stdout)
        return jsonify(data)
    except Exception as e:
        return jsonify({"error": "invalid JSON from C++", "detail": result.stdout}), 500

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5001)

