import csv
import math
import os
import sys

from pyrosm import OSM

PBF_PATH = "../.data/florida-latest.osm.pbf"

OUTPUT_DIR = "../.data"

NODES_FILE = os.path.join(OUTPUT_DIR, "nodes.csv")
EDGES_FILE = os.path.join(OUTPUT_DIR, "edges.csv")

NETWORK_TYPE = "driving"

def normalize_street_name(value) -> str:
    # Normalize OSM street names.

    if value is None:
        return "Unknown"

    values = (
        value
        if isinstance(value, (list, tuple, set))
        else [value]
    )

    names = []
    for item in values:

        if item is None:
            continue

        name = str(item).strip()

        if not name:
            continue

        if name.lower() in ("nan", "none", "null"):
            continue

        if name not in names:
            names.append(name)

    return ", ".join(names) if names else "Unknown"


def normalize_distance(data) -> float:
    raw_distance = data.get("length", data.get("distance", 0))

    try:
        distance = float(raw_distance)
    except (ValueError, TypeError):
        return 0.0

    if not math.isfinite(distance):
        return 0.0

    if distance <= 0:
        return 0.0

    return distance


def normalize_coordinate(value) -> float:
    try:
        coordinate = float(value)
    except (ValueError, TypeError):
        raise ValueError("Invalid coordinate")

    if not math.isfinite(coordinate):
        raise ValueError("Non-finite coordinate")

    return coordinate

def load_graph():
    if not os.path.exists(PBF_PATH):
        raise FileNotFoundError(f"OSM PBF file not found: {PBF_PATH}")

    osm = OSM(PBF_PATH)
    nodes, edges = osm.get_network(network_type=NETWORK_TYPE, nodes=True)

    if nodes is None or edges is None:
        raise RuntimeError(
            "Pyrosm failed to extract the road network"
        )

    if nodes.empty or edges.empty:
        raise RuntimeError("Extracted road network is empty")

    G = osm.to_graph(nodes, edges, graph_type="networkx")
    if G.number_of_nodes() == 0:
        raise RuntimeError("Graph contains no nodes")

    if G.number_of_edges() == 0:
        raise RuntimeError("Graph contains no edges")

    return G


def validate_graph(G):
    valid_nodes = set()

    for nodeID, data in G.nodes(data=True):
        lat = normalize_coordinate(data.get("y", data.get("lat")))
        lon = normalize_coordinate(data.get("x", data.get("lon")))

        if not (-90 <= lat <= 90):
            raise ValueError("Invalid latitude")

        if not (-180 <= lon <= 180):
            raise ValueError("Invalid longitude")

        valid_nodes.add(nodeID)

    for source, destination, data in G.edges(data=True):
        if source not in valid_nodes:
            continue

        if destination not in valid_nodes:
            continue

        distance = normalize_distance(data)

        if distance <= 0:
            continue

    return valid_nodes

def export_nodes(G, valid_nodes):
    # csv format:
    # id latitude longitude
    with open(NODES_FILE, "w", newline="", encoding="utf-8") as file:

        writer = csv.writer(file, delimiter="\t")

        writer.writerow([
            "id",
            "latitude",
            "longitude"
        ])

        for nodeID, data in G.nodes(data=True):
            if nodeID not in valid_nodes:
                continue

            lat = normalize_coordinate(data.get("y", data.get("lat")))
            lon = normalize_coordinate(data.get("x", data.get("lon")))

            writer.writerow([
                nodeID,
                lat,
                lon
            ])

def export_edges(G, valid_nodes):
    # csv format:
    # source destination distance streetName
    with open(EDGES_FILE, "w", newline="", encoding="utf-8") as file:
        writer = csv.writer(file, delimiter="\t")
        writer.writerow([
            "source",
            "destination",
            "distance",
            "streetName"
        ])

        for source, destination, data in G.edges(data=True):
            if source not in valid_nodes:
                continue
            
            if destination not in valid_nodes:
                continue

            distance = normalize_distance(data)
            if distance <= 0:
                continue

            streetName = normalize_street_name(data.get("name"))

            writer.writerow([
                source,
                destination,
                distance,
                streetName
            ])

def makeCSV():
    os.makedirs(OUTPUT_DIR, exist_ok=True)

    G = load_graph()
    valid_nodes = validate_graph(G)

    if not valid_nodes:
        raise RuntimeError("No valid graph nodes found")

    export_nodes(G, valid_nodes)
    export_edges(G, valid_nodes)

if __name__ == "__main__":
    try:
        makeCSV()
    except Exception as error:
        print(f"\nERROR: {error}")
        sys.exit(1)