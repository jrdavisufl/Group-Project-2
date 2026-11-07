GPS Route Optimization: Comparing Dijkstra's and A*

University of Florida - COP3530 Data Structures and Algorithms
Group 116

PROJECT OVERVIEW

This project compares Dijkstra's algorithm and A* for finding shortest paths in a Florida road network using OpenStreetMap data.

The application uses a C++ backend for graph representation, data processing, and shortest-path calculations. A web-based frontend allows users to enter locations, visualize routes, view directions, and compare algorithm performance.

The goal is to evaluate how the geographic distance heuristic used by A* affects execution time, memory usage, and search efficiency compared to Dijkstra's algorithm.

TECHNOLOGIES USED

C++23 - Graph representation, data processing, caching, and pathfinding algorithms

Python - Backend integration and address geocoding

Flask - Web server connecting the frontend to the C++ application

HTML, CSS, JavaScript - Frontend interface

Leaflet.js - Interactive map and route visualization

OpenStreetMap - Road-network data and map tiles

GeoPy / Nominatim - Converting addresses into geographic coordinates

CMake - Building and managing the C++ project

FEATURES

The application allows users to enter a starting location and destination, choose between Dijkstra's algorithm and A*, and display the calculated shortest route on an interactive map.

Users can also compare both algorithms using the same starting and destination points.

The application displays route distance, execution time, nodes processed, edges examined, peak heap size, and other performance statistics.

It also provides turn-by-turn directions using street names from the road-network dataset.

FRONTEND

The frontend was implemented using HTML, CSS, and JavaScript, with Leaflet.js used for interactive maps and route visualization.

A Flask server was added to receive requests from the frontend, convert addresses into geographic coordinates, and execute the compiled C++ routing program.

The C++ application was extended with a get_route_json command that returns route information in JSON format.

The data parser was also extended to provide geographic coordinates, route directions, distances, and performance statistics in a format that the frontend can process.

The frontend supports A*, Dijkstra, and a comparison mode that displays performance results for both algorithms.

The frontend integration reuses the existing cached Florida road-network data rather than downloading or generating a new graph for each route request.

app.py - Flask backend responsible for processing frontend requests and communicating with the C++ application.

requirements.txt - Python dependencies required to run the frontend.

frontend/templates/index.html - Main webpage containing the application interface.

frontend/static/style.css - Styling and layout of the frontend.

frontend/static/script.js - JavaScript responsible for sending routing requests, displaying results, and drawing routes on the map.

MODIFIED FILES

src/core.cpp - Added support for the get_route_json command.

src/data_parser.cpp - Added functionality for returning routing results as JSON.

include/data_parser.hpp - Added declarations for the JSON routing functionality.

ALGORITHM IMPLEMENTATION

A* and Dijkstra are used to calculate shortest paths through the same weighted, directed road network.

The current geographic routing implementation uses a shared A* search function for both algorithms.

A* uses the Haversine distance heuristic to estimate the remaining distance to the destination.

Dijkstra uses the same search function with its heuristic set to zero, causing the algorithm to prioritize nodes based only on accumulated path cost.

The repository also includes a separate standalone Dijkstra implementation with a custom binary min-heap. This implementation is used for independent testing and is not currently connected to the frontend.

ROAD-NETWORK DATA

The project uses Florida road-network data obtained from OpenStreetMap.

The raw data is processed into nodes and edges representing road intersections and connections.

The data is organized into geographic buckets and stored in binary cache files to support efficient access during routing.

The generated Florida road-network dataset is not included in the Git repository because of its size.

The dataset can be downloaded from Google Drive here: https://drive.google.com/drive/folders/1McgOJ67nShRdBM9NzkVraxsfj4eY3lgt. Place the contents in a ".data" folder at the project root.

The expected directory structure is:

.data/
    nodes.csv
    edges.csv
    cache/
        meta.bin
        names.bin
        additional bucket cache files

If the binary cache is already available, the application can use it directly.

If only nodes.csv and edges.csv are available, the cache must first be generated using the stage_data command.

INSTALLATION AND SETUP

Clone or download the GitHub repository.

Open the project in CLion.

Ensure that a C++23-compatible compiler and CMake are installed.

Reload the CMake project and build the app target.

Alternatively, run the following command from the project root:

cmake --build cmake-build-debug --target app

On Windows, the compiled application should be located at:

cmake-build-debug/app.exe

Download the Florida road-network data and place the .data folder in the project root.

If the cache has not already been generated, run:

.\cmake-build-debug\app.exe stage_data

This converts the CSV files into the cached representation used by the C++ application.

Install the required Python packages by running:

python -m pip install -r requirements.txt

Start the Flask server by running:

python app.py

Open the following address in a browser:

http://127.0.0.1:5000

The frontend should now be accessible.

Keep the Flask server running while using the application.

HOW TO USE THE APPLICATION

Enter a starting address in the starting location field.

Enter a destination address in the destination field.

Select A*, Dijkstra, or the comparison option.

Click the Run Algorithm button.

The application converts both addresses into geographic coordinates and identifies nearby road-network nodes.

The selected algorithm calculates the shortest path between those nodes.

The resulting route is displayed on the interactive map.

The application displays route directions, total distance, execution time, and search statistics.

When comparison mode is selected, the application evaluates both algorithms using the same starting and ending locations.

HOW THE FRONTEND COMMUNICATES WITH THE BACKEND

The frontend sends an HTTP request to the Flask server containing the starting address, destination address, and selected algorithm.

Flask uses GeoPy and Nominatim to convert the addresses into latitude and longitude coordinates.

The coordinates are passed to the compiled C++ program through the get_route_json command.

The C++ application loads the cached Florida road network and locates the closest graph nodes to the requested coordinates.

The routing algorithm searches for the shortest path between those nodes.

The C++ application returns the path coordinates, route directions, and performance statistics as JSON.

Flask forwards this information to the frontend.

JavaScript processes the returned data, and Leaflet displays the calculated route on the map.

The frontend is responsible for visualization and user interaction. The shortest-path calculations are performed by the C++ backend.

TESTING

The project includes tests for the standalone Dijkstra implementation and the original C++ application.

The standalone Dijkstra implementation can be tested using:

.\cmake-build-debug\app.exe dijkstra

The frontend integration can be tested independently using the get_route_json command.

Example:

.\cmake-build-debug\app.exe get_route_json 29.646 -82.340 29.650 -82.340 astar

This command should return JSON containing the route coordinates and performance results, provided the necessary road-network data is available.

Additional testing should verify that both algorithms return consistent shortest-path distances for the same inputs and that the frontend correctly displays their results.