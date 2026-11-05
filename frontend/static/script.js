"use strict";

const map = L.map("map", { zoomControl: false }).setView([29.647, -82.336], 13);
L.control.zoom({ position: "topright" }).addTo(map);
L.tileLayer("https://tile.openstreetmap.org/{z}/{x}/{y}.png", {
  maxZoom: 19,
  attribution: '&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors'
}).addTo(map);

const form = document.getElementById("route-form");
const button = document.getElementById("run-button");
const buttonText = document.getElementById("button-text");
const status = document.getElementById("status");
const metricsBody = document.getElementById("metrics-body");
const resultsCaption = document.getElementById("result-caption");
const directionsTitle = document.getElementById("directions-title");
const directionsList = document.getElementById("directions-list");
const colors = { astar: "#2563eb", dijkstra: "#ec8d38" };
let routeLayers = L.layerGroup().addTo(map);

function setStatus(message, kind = "") {
  status.textContent = message;
  status.className = "status" + (kind ? " " + kind : "");
}

function formatNumber(value, decimals = 0) {
  return Number.isFinite(Number(value)) ? Number(value).toLocaleString(undefined, {
    minimumFractionDigits: decimals,
    maximumFractionDigits: decimals
  }) : "—";
}

function drawResults(results) {
  routeLayers.clearLayers();
  const bounds = L.latLngBounds([]);

  // Draw Dijkstra first so a coincident A* route remains visible on top.
  for (const name of ["dijkstra", "astar"]) {
    const result = results[name];
    if (!result || !Array.isArray(result.coordinates) || !result.coordinates.length) continue;
    const color = colors[name];
    const line = L.polyline(result.coordinates, {
      color, weight: name === "astar" ? 5 : 8,
      opacity: name === "astar" ? 0.95 : 0.65
    }).addTo(routeLayers);
    line.bindPopup(name === "astar" ? "A* shortest path" : "Dijkstra shortest path");
    bounds.extend(line.getBounds());
  }

  const primary = results.astar || results.dijkstra;
  if (primary && primary.coordinates && primary.coordinates.length) {
    const coordinates = primary.coordinates;
    L.marker(coordinates[0]).addTo(routeLayers).bindPopup("Start");
    L.marker(coordinates[coordinates.length - 1]).addTo(routeLayers).bindPopup("Destination");
    if (bounds.isValid()) map.fitBounds(bounds, { padding: [30, 30], maxZoom: 16 });
  }

  const metrics = [
    ["Shortest distance", r => formatNumber(r.distanceMeters / 1000, 3) + " km"],
    ["Search time", r => formatNumber(r.runtimeSeconds * 1000, 3) + " ms"],
    ["Nodes processed", r => formatNumber(r.nodesProcessed)],
    ["Edges examined", r => formatNumber(r.edgesExamined)],
    ["Peak heap size", r => formatNumber(r.peakHeapSize)],
    ["Path nodes", r => formatNumber(r.pathNodes)],
    ["Stale heap entries", r => formatNumber(r.staleEntriesSkipped)],
    ["Node snapping time", r => formatNumber(r.snapSeconds, 3) + " s"]
  ];
  metricsBody.replaceChildren();
  for (const [label, getter] of metrics) {
    const row = document.createElement("tr");
    const first = document.createElement("td");
    first.textContent = label;
    row.appendChild(first);
    for (const name of ["astar", "dijkstra"]) {
      const cell = document.createElement("td");
      cell.textContent = results[name] ? getter(results[name]) : "—";
      row.appendChild(cell);
    }
    metricsBody.appendChild(row);
  }

  resultsCaption.textContent = Object.keys(results).length === 2
    ? "Two algorithms · same road graph"
    : "One algorithm";
  directionsTitle.textContent = results.astar ? "Directions (A*)" : "Directions (Dijkstra)";
  directionsList.replaceChildren();
  for (const step of primary.steps || []) {
    const li = document.createElement("li");
    li.textContent = `Follow ${step.street} for ${formatNumber(step.distanceMeters, 1)} m`;
    directionsList.appendChild(li);
  }
  const arrival = document.createElement("li");
  arrival.textContent = "Arrive at destination.";
  directionsList.appendChild(arrival);
}

form.addEventListener("submit", async event => {
  event.preventDefault();
  if (button.disabled) return;
  button.disabled = true;
  buttonText.textContent = "Calculating…";
  setStatus("Looking up addresses and running the algorithms…");
  try {
    const response = await fetch("/api/route", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({
        start: document.getElementById("start").value,
        end: document.getElementById("end").value,
        algorithm: document.getElementById("algorithm").value
      })
    });
    const body = await response.json();
    if (!response.ok) throw new Error(body.error || "Route request failed");
    drawResults(body.results);
    setStatus("Route calculated successfully.", "success");
  } catch (error) {
    setStatus(error.message, "error");
  } finally {
    button.disabled = false;
    buttonText.textContent = "Run algorithm";
  }
});
