import osmnx as ox
import csv
from geopy.geocoders import Nominatim

# Using a starting location and destination, this function retrieves the map bounds 
def getCoords(start: str, end : str) -> list:
    geolocator = Nominatim(user_agent="gpsOptimization")
    margin: float = 0.01

    startLoc: str = geolocator.geocode(start)
    endLoc: str = geolocator.geocode(end)

    north: float = max(startLoc.latitude, endLoc.latitude) + margin
    south: float = min(startLoc.latitude, endLoc.latitude) + margin
    east: float = max(startLoc.longitude, endLoc.longitude) + margin
    west: float = min(startLoc.longitude, endLoc.longitude) + margin

    return [startLoc, endLoc, west, south, east, north]

def getRoads(start: str, end : str) -> list[ox.MultiDiGraph, int, int]:
    coords: list = getCoords(start, end)

    G = ox.graph_from_bbox(
        coords[2:],
        network_type="drive",
        simplify=True
    )

    startLoc: str = coords[0]
    endLoc: str = coords[1]

    startNode: int = ox.distance.nearest_nodes(
        G, X=startLoc.longitude, Y=endLoc.latitude
    )

    endNode: int = ox.distance.nearest_nodes(
        G, X=endLoc.longitude, Y=endLoc.latitude
    )

