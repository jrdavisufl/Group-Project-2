import osmnx as ox
import csv
import sys
from geopy.geocoders import Nominatim

# Using a starting location and destination, this function retrieves the map bounds 
def getCoords(start: str, end: str) -> list:
    geolocator = Nominatim(user_agent="gpsOptimization")
    margin: float = 0.01

    startLoc: str = geolocator.geocode(start)
    endLoc: str = geolocator.geocode(end)

    north: float = max(startLoc.latitude, endLoc.latitude) + margin
    south: float = min(startLoc.latitude, endLoc.latitude) + margin
    east: float = max(startLoc.longitude, endLoc.longitude) + margin
    west: float = min(startLoc.longitude, endLoc.longitude) + margin

    return [startLoc, endLoc, west, south, east, north]

#Write the nodes and edges between the starting and ending points to a csv
def makeCSV(start: str, end : str) :
    coords: list = getCoords(start, end)

    bounds: tuple = tuple(coords[2:])

    G = ox.graph_from_bbox(
        bounds,
        network_type="drive",
        simplify=True
    )

    startLoc: str = coords[0]
    endLoc: str = coords[1]

    startNode: int = ox.distance.nearest_nodes(
        G, X=startLoc.longitude, Y=startLoc.latitude
    )

    endNode: int = ox.distance.nearest_nodes(
        G, X=endLoc.longitude, Y=endLoc.latitude
    )

    # Write nodes in a csv
    with open("data/nodes.csv", "w", newline="") as file:
        writer = csv.writer(file, delimiter="\t")
        writer.writerow(["id", "latitude", "longitude"])

        for nodeID, data in G.nodes(data=True):
            writer.writerow([nodeID, data["y"], data["x"]])

    # Write edges in a csv
    with open("data/edges.csv", "w", newline="") as file:
        writer = csv.writer(file, delimiter="\t")
        writer.writerow(["source", "destination", "distance", "streetName"])

        for source, destination, data in G.edges(data=True):
            streetName = data.get("name", "Unknown")

            if isinstance(streetName, list):
                streetName = ", ".join(streetName)

            writer.writerow([source, destination, data.get("length", 0), streetName])


if __name__ == "__main__":
    start = sys.argv[1]
    end = sys.argv[2]

    makeCSV(start, end)

# plottting graph test
# ox.plot_graph(
#     G,
#     node_size=8,
#     node_color="red",
#     edge_color="gray"
# )