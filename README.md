# ASTRA — Adaptive Space Traffic & Risk Avoidance System

ASTRA is a desktop prototype that tracks satellites and space debris, warns when objects come too close,
shows how objects connect to each other, and recommends the lowest-cost route between regions of space.

It combines three small algorithm modules (written in C and C++) behind a Qt 6 graphical interface.

> **Scope note:** ASTRA is a demonstration built on synthetic data. Coordinates are abstract "demo units",
> the orbit drawing is illustrative, risk scores are a simple distance heuristic (not collision probabilities),
> and maneuver text is not an actionable command. Validate all algorithms before describing them as scientific results.

---

## Features

- **Collision screening** – checks every pair of objects and classifies it as SAFE, WARNING or CRITICAL
- **Space network graph** – connects objects within a chosen range; runs BFS, DFS and route finding
- **Safe routing** – Dijkstra's algorithm over risk- and fuel-weighted sectors
- **Interactive GUI** – add objects, change the link range, and watch every page update live
- **Console tools** – each module also runs without the GUI
- **Local time display** – top bar shows local time, UTC offset and UTC

## Architecture

```
          ┌──────────────── Qt 6 GUI (C++) ────────────────┐
          │ Overview | Orbit | Collision | Network |         │
          │ Maneuver | Analytics | Learn                     │
          └──────┬────────────────┬────────────────┬─────────┘
                 │                │                │
          collision.c        graph.c         routing.cpp
        (C: distances)  (C: BFS/DFS/route)  (C++: Dijkstra)
```

| Module | Language | Files | Purpose |
|---|---|---|---|
| Collision | C | `src/core/collision.c/.h` | Pairwise Euclidean distance and risk level |
| Graph | C | `src/core/graph.c/.h` | Adjacency-matrix graph, BFS, DFS, fewest-hop route, file loader |
| Routing | C++ | `src/core/routing.cpp/.hpp` | Dijkstra with cost = distance × (1 + risk) + fuel |
| GUI | C++ / Qt 6 | `src/gui/main.cpp` | Dashboard that calls all three modules |

## Project structure

```
ASTRA/
├── CMakeLists.txt
├── README.md
├── .gitignore
├── data/
│   └── space_network.txt        sample graph for the console demo
└── src/
    ├── core/                    collision, graph, routing (static library)
    ├── cli/                     console programs
    └── gui/main.cpp             Qt GUI
```

## GUI pages

| Page | What it does | Module |
|---|---|---|
| Mission Overview | Live counts of objects, alerts and links; orbit animation with pause/start | all |
| Orbit Monitor | Illustrative orbit view (satellites cyan, debris coral) | – |
| Collision Risk | Risk register, proximity alerts, **Add object…**, **Reset demo data** | collision |
| Space Network | Adjacency list, BFS, DFS, route between two objects, adjustable link range | graph |
| Maneuver Planner | Demo proposal text, plus **Find lowest-cost route** for sectors 0–4 | collision, routing |
| Analytics | Catalogue statistics and risk-score bars | collision |
| Learn ASTRA | Short explanation of each module | – |

## How it works

**Collision screening.** For every unique pair of objects the distance is computed. A pair at ≤ 20 units is a
WARNING and at ≤ 10 units is CRITICAL. Each object also gets a 0–100 score from its nearest neighbour
(100 at distance 0, falling to 0 at 40 units). Complexity is O(n²).

**Space network.** Objects are nodes; two objects are linked if they are within the link range (default 60 units),
with the distance as edge weight. BFS visits nodes level by level, DFS follows one chain deep before backing up,
and route finding returns the path with the fewest hops. An object out of range of all others is isolated and unreachable.

**Safe routing.** Five demo sectors are connected by links that each have a distance, a risk factor and a fuel cost.
The cost of a leg is `distance × (1 + risk) + fuel`. Dijkstra's algorithm finds the cheapest total path, so a longer but
safer route can beat a short risky one. The demo result is `0 → 2 → 3 → 4` with total cost 26. The sectors are fixed
demo data and are not tied to the tracked objects.

## Requirements

- CMake 3.21 or newer
- A C11 / C++17 compiler (GCC, Clang or MSVC)
- Qt 6.3 or newer — only for the GUI

## Build

```bash
cmake -S . -B build
cmake --build build
```

If Qt 6 is not found, CMake prints a warning and still builds the console programs.
To point CMake at your Qt installation:

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x.x/<kit>
```

**Qt Creator:** File → Open File or Project → select `CMakeLists.txt` → choose a Qt 6 kit → Configure Project →
select the `ASTRA_GUI` target → Run (Ctrl+R).

### Build console tools without CMake

```bash
gcc -Isrc/core src/cli/collision_cli.c src/core/collision.c -lm -o astra_collision_cli
gcc -Isrc/core src/cli/graph_demo.c src/core/graph.c -o astra_graph_demo
g++ -std=c++17 -Isrc/core src/cli/routing_demo.cpp src/core/routing.cpp -o astra_routing_demo
```

## Run

```bash
./build/ASTRA_GUI              # GUI (build\Debug\ASTRA_GUI.exe on Windows with MSVC)
./build/astra_collision_cli    # enter objects and coordinates interactively
./build/astra_graph_demo       # BFS / DFS / route demo, reads data/space_network.txt
./build/astra_routing_demo     # Dijkstra demo, prints path 0->2->3->4
```

### Console collision example

```
Enter number of space objects (1-50): 3
Enter name of object 1: A   X: 0   Y: 0
Enter name of object 2: B   X: 5   Y: 5
Enter name of object 3: C   X: 15  Y: 0
```
Output: 3 alerts (1 critical, 2 warning); most dangerous pair A–B at distance 7.07.

## Network file format

`data/space_network.txt` uses a simple text format:

```
# comment
NODES 4
EDGE 0 1 10.5      # undirected link: from to weight
EDGE 0 2 15.0
```

## Limitations

- Synthetic data and abstract coordinates; no real orbital mechanics
- Risk score is a distance heuristic, not a collision probability
- Maneuver proposals are placeholders (no delta-v or fuel calculation)
- Routing sectors are fixed demo data
- Adjacency matrix limits the graph to 100 nodes; the GUI catalogue is limited to 50 objects

## Future work

- Real orbit propagation (TLE data, SGP4)
- Collision probability with position uncertainty
- Priority queue to rank the most urgent encounters
- Generate routing sectors from live object positions
- Machine-learning risk estimation
- Editable sectors and links in the GUI

## Troubleshooting

- **Qt 6 not found:** install Qt 6 and set `CMAKE_PREFIX_PATH`, or select a Qt 6 kit in Qt Creator.
- **No compiler kit:** install a compatible compiler through the Qt Maintenance Tool.
- **Case-sensitive systems (Linux/macOS):** file names are lowercase (`graph.c`, `graph.h`); keep them that way.
- Read the **first** compiler error before changing any code.
