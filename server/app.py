from __future__ import annotations

import json
import os
import threading
from copy import deepcopy
from datetime import datetime, timezone
from pathlib import Path

from flask import Flask, Response, jsonify, request, send_from_directory


BASE_DIR = Path(__file__).resolve().parent.parent
PUBLIC_DIR = BASE_DIR / "public"
ESP32_API_KEY = os.environ.get("ESP32_API_KEY", "smartagr-demo-key")
PORT = int(os.environ.get("PORT", "3000"))

app = Flask(__name__, static_folder=str(PUBLIC_DIR), static_url_path="")

state_lock = threading.Lock()
greenhouse_state = {
    "temperature": 26.3,
    "brightness": 640,
    "fanOn": False,
    "shadeClosed": False,
    "cameraUrl": "",
    "updatedAt": datetime.now(timezone.utc).isoformat(),
}


def now_iso() -> str:
    return datetime.now(timezone.utc).isoformat()


def read_state() -> dict:
    with state_lock:
        return deepcopy(greenhouse_state)


def update_state(payload: dict) -> dict:
    with state_lock:
        if isinstance(payload.get("temperature"), (int, float)):
            greenhouse_state["temperature"] = round(float(payload["temperature"]), 1)

        if isinstance(payload.get("brightness"), (int, float)):
            greenhouse_state["brightness"] = int(payload["brightness"])

        if isinstance(payload.get("fanOn"), bool):
            greenhouse_state["fanOn"] = payload["fanOn"]

        if isinstance(payload.get("shadeClosed"), bool):
            greenhouse_state["shadeClosed"] = payload["shadeClosed"]

        if isinstance(payload.get("cameraUrl"), str):
            greenhouse_state["cameraUrl"] = payload["cameraUrl"].strip()

        greenhouse_state["updatedAt"] = now_iso()
        return deepcopy(greenhouse_state)


@app.get("/")
def index() -> Response:
    return send_from_directory(PUBLIC_DIR, "index.html")


@app.get("/api/state")
def get_state() -> Response:
    return jsonify(read_state())


@app.post("/api/state")
def post_state() -> Response:
    auth_header = request.headers.get("Authorization", "")
    token = auth_header.removeprefix("Bearer ").strip()
    if token != ESP32_API_KEY:
        return jsonify({"message": "Unauthorized device request."}), 401

    payload = request.get_json(silent=True) or {}
    return jsonify({"ok": True, "state": update_state(payload)})


@app.get("/api/stream")
def stream() -> Response:
    def event_stream():
        last_sent = ""
        while True:
            snapshot = read_state()
            encoded = json.dumps(snapshot, ensure_ascii=False)
            if encoded != last_sent:
                yield f"data: {encoded}\n\n"
                last_sent = encoded
            threading.Event().wait(2)

    headers = {
        "Cache-Control": "no-cache",
        "X-Accel-Buffering": "no",
    }
    return Response(event_stream(), mimetype="text/event-stream", headers=headers)


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=PORT, debug=False, threaded=True)
