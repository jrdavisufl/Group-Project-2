"""Local UI bridge for the team's existing C++ GPS route optimizer.

Run this from the repository root with: python app.py
Set GPS_ROUTER_EXE to the path of your built C++ program when needed.
"""
from functools import lru_cache
import json
import logging
import os
from pathlib import Path
import subprocess
from threading import Lock

from flask import Flask, jsonify, render_template, request
from geopy.exc import GeocoderServiceError, GeocoderTimedOut, GeocoderUnavailable
from geopy.extra.rate_limiter import RateLimiter
from geopy.geocoders import Nominatim

ROOT = Path(__file__).resolve().parent
app = Flask(__name__, template_folder="frontend/templates", static_folder="frontend/static")
app.config["MAX_CONTENT_LENGTH"] = 16 * 1024

# A distinct, identifying User-Agent is required by the public Nominatim service.
# The GitHub repository URL identifies this educational application.
geolocator = Nominatim(
    user_agent="UF-COP3530-GPS-Route-Optimizer/1.0 (https://github.com/jrdavisufl/Group-Project-2)",
    timeout=12,
)
rate_limited_geocode = RateLimiter(
    geolocator.geocode,
    min_delay_seconds=1,
    max_retries=1,
    error_wait_seconds=5,
    swallow_exceptions=False,
)
route_lock = Lock()


@lru_cache(maxsize=256)
def geocode_address(address):
    location = rate_limited_geocode(address, exactly_one=True)
    if location is None:
        raise ValueError(f"Address not found: {address}")
    return location.latitude, location.longitude


def router_executable():
    """Locate the project executable that accepts the get_route_json CLI flag."""
    configured = os.environ.get("GPS_ROUTER_EXE", "").strip()
    if configured:
        candidate = Path(configured).expanduser()
        if not candidate.is_absolute():
            candidate = ROOT / candidate
        if candidate.is_file():
            return candidate.resolve()
        raise FileNotFoundError(f"GPS_ROUTER_EXE does not point to a file: {candidate}")

    # These are common target names, not assertions about your team's CMakeLists.
    names = (
        "Group-Project-2.exe", "Group-Project-2", "gps_route.exe", "gps_route",
        "GPSRouteOptimization.exe", "GPSRouteOptimization", "route_optimizer.exe",
        "route_optimizer", "main.exe", "main",
    )
    for build_folder in ("cmake-build-debug", "cmake-build-release", "build", "build/Debug"):
        for name in names:
            candidate = ROOT / build_folder / name
            if candidate.is_file():
                return candidate.resolve()

    raise FileNotFoundError(
        "Could not find your C++ executable. Set GPS_ROUTER_EXE to its path "
        "(e.g. $env:GPS_ROUTER_EXE='cmake-build-debug\\YOUR_TARGET.exe')."
    )


def compute_route(executable, start_coords, end_coords, algorithm):
    command = [
        str(executable),
        "get_route_json",
        str(start_coords[0]), str(start_coords[1]),
        str(end_coords[0]), str(end_coords[1]),
        algorithm,
    ]
    completed = subprocess.run(
        command, cwd=ROOT, capture_output=True, text=True,
        check=False, timeout=120,
    )
    if completed.returncode != 0:
        logging.error("C++ routing error (%s): %s", algorithm, completed.stderr.strip())
        raise RuntimeError(completed.stderr.strip() or "C++ routing failed")
    try:
        result = json.loads(completed.stdout)
    except json.JSONDecodeError as exc:
        logging.error("Expected pure JSON, got: %r", completed.stdout[:1000])
        raise RuntimeError("The C++ executable did not return valid JSON") from exc
    return result


@app.get("/")
def index():
    return render_template("index.html")


@app.post("/api/route")
def get_route():
    payload = request.get_json(silent=True)
    if not isinstance(payload, dict):
        return jsonify(error="Expected a JSON request"), 400

    start = payload.get("start")
    end = payload.get("end")
    algorithm = payload.get("algorithm", "compare")
    if not isinstance(start, str) or not isinstance(end, str):
        return jsonify(error="Enter valid starting and destination addresses"), 400
    start, end = start.strip(), end.strip()
    if not start or not end or len(start) > 250 or len(end) > 250:
        return jsonify(error="Enter two addresses (up to 250 characters each)"), 400
    if algorithm not in ("astar", "dijkstra", "compare"):
        return jsonify(error="Invalid algorithm selection"), 400

    try:
        executable = router_executable()
        cache_dir = ROOT / ".data" / "cache"
        if not (cache_dir / "meta.bin").is_file() or not (cache_dir / "names.bin").is_file():
            raise FileNotFoundError("Road cache missing. Run the C++ 'stage_data' command first.")

        # Do not re-extract the Florida PBF for every click. Use the existing cache.
        with route_lock:
            start_coords = geocode_address(start)
            end_coords = geocode_address(end)
            algorithms = ("astar", "dijkstra") if algorithm == "compare" else (algorithm,)
            results = {
                name: compute_route(executable, start_coords, end_coords, name)
                for name in algorithms
            }

        return jsonify({
            "start": start,
            "end": end,
            "results": results,
        })
    except ValueError as exc:
        return jsonify(error=str(exc)), 422
    except FileNotFoundError as exc:
        return jsonify(error=str(exc)), 500
    except subprocess.TimeoutExpired:
        return jsonify(error="Routing timed out"), 504
    except (GeocoderServiceError, GeocoderTimedOut, GeocoderUnavailable):
        app.logger.exception("Geocoding unavailable")
        return jsonify(error="Address lookup is unavailable right now"), 503
    except (OSError, RuntimeError):
        app.logger.exception("Routing failed")
        return jsonify(error="Routing failed. Check the Flask terminal for details."), 500


if __name__ == "__main__":
    app.run(host="127.0.0.1", port=5000, debug=True, use_reloader=False)
