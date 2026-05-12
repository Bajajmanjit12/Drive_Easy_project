from flask import Flask, request, jsonify
from flask_cors import CORS

app = Flask(__name__)
CORS(app)

data_store = {
    "alcohol": 0,
    "drowsiness": 0,
    "crash": 0,
    "latitude": 0,
    "longitude": 0
}

@app.route('/update', methods=['POST'])
def update():
    global data_store
    data = request.json

    if data:
        data_store = data
        print("Received:", data)

    return jsonify({"status": "ok"})

@app.route('/data', methods=['GET'])
def get_data():
    return jsonify(data_store)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
