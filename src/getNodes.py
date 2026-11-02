import csv
import sys
from geopy.geocoders import Nominatim
from pyrosm import OSM
import osmnx as ox

PBF_PATH = "../inputs/data/florida-latest.osm.pbf"

# Using a starting location and destination, this function retrieves the map bounds 
def getCoords(start: str, end: str) -> list:
    geolocator = Nominatim(user_agent="gpsOptimization")
    margin: float = 0.01

    startLoc = geolocator.geocode(start)
    endLoc = geolocator.geocode(end)

    north: float = max(startLoc.latitude, endLoc.latitude) + margin
    south: float = min(startLoc.latitude, endLoc.latitude) + margin
    east: float = max(startLoc.longitude, endLoc.longitude) + margin
    west: float = min(startLoc.longitude, endLoc.longitude) + margin

    return [startLoc, endLoc, west, south, east, north]

# Write the nodes and edges to a csv using the local .pbf file
def makeCSV(start: str = "", end: str = ""):
    G = None
    osm = OSM(PBF_PATH)

    if start != "" and end != "":
        coords = getCoords(start, end)
        
        # Pyrosm bounding box format: [minx, miny, maxx, maxy] -> [west, south, east, north]
        box = [coords[2], coords[3], coords[4], coords[5]]

        nodes, edges = osm.get_network(network_type="driving", bounding_box=box, nodes=True)
        G = osm.to_graph(nodes, edges, graph_type="networkx")

        startLoc = coords[0]
        endLoc = coords[1]
        
        startNode: int = ox.distance.nearest_nodes(
            G, X=startLoc.longitude, Y=startLoc.latitude
        )
        endNode: int = ox.distance.nearest_nodes(
            G, X=endLoc.longitude, Y=endLoc.latitude
        )
    else:
        nodes, edges = osm.get_network(network_type="driving", nodes=True)
        G = osm.to_graph(nodes, edges, graph_type="networkx")

    # Write nodes to CSV
    with open("../inputs/data/nodes.csv", "w", newline="") as file:
        writer = csv.writer(file, delimiter="\t")
        writer.writerow(["id", "latitude", "longitude"])

        for nodeID, data in G.nodes(data=True):
            lat = data.get("y", data.get("lat", 0))
            lon = data.get("x", data.get("lon", 0))
            writer.writerow([nodeID, lat, lon])

    # Write edges to CSV
    with open("../inputs/data/edges.csv", "w", newline="") as file:
        writer = csv.writer(file, delimiter="\t")
        writer.writerow(["source", "destination", "distance", "streetName"])

        for source, destination, data in G.edges(data=True):
            streetName = data.get("name", "Unknown")

            # Safely handle lists, Nones, or floats in street names
            if streetName is None:
                streetName = "Unknown"
            elif isinstance(streetName, list):
                streetName = ", ".join(str(x) for x in streetName if x is not None)
            else:
                streetName = str(streetName)

            distance = data.get("length", data.get("distance", 0))
            writer.writerow([source, destination, distance, streetName])

if __name__ == "__main__":
    if len(sys.argv) > 2:
        start = sys.argv[1]
        end = sys.argv[2]
        makeCSV(start, end)
    else:
        makeCSV()

# plotting graph test
# ox.plot_graph(
#     G,
#     node_size=8,
#     node_color="red",
#     edge_color="gray"
# )