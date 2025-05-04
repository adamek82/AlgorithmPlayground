# Algorithm Playground

A personal collection of **algorithms & data‑structures exercises** written in modern C++20.  
Each exercise lives in its own header/implementation pair and is verified by a small,
self‑contained test harness so you can build and run everything out‑of‑the‑box with CMake.

---

## Assumptions & Goals

* **Learning‑by‑doing** – every directory showcases a single classical problem or data‑structure.
* **Clean APIs, tight bounds** – each solution targets the asymptotic complexity quoted in the
  exercise description.
* **Zero external dependencies** – everything builds with a stock C++20 compiler.
* **Editor‑friendly** – workspace files are provided for VS Code + the CMake Tools extension.

---

## Project Layout

```
.
├── CMakeLists.txt            # root build script
├── include/                  # public headers (one per exercise)
│   ├── ClosestPairSolver.h
│   ├── inMemoryDb.h
│   └── …
├── src/                      # implementations + test runner
│   ├── ClosestPairSolver.cpp
│   ├── inMemoryDb.cpp
│   └── AlgorithmPlayground.cpp
├── bin/                      # CMake runtime output (git‑ignored)
└── .vscode/                  # launch / build / intellisense settings
```

`AlgorithmPlayground.cpp` is the single **executable entry‑point**; it pulls in each
exercise’s unit tests so that a *single* run gives a green (or red!) report.

---

## Implemented Exercises

| # | Topic | Files | Key Points |
|---|-------|-------|------------|
| 1 | **Closest Pair of Points** (Geometry) | `ClosestPairSolver.*` | Divide‑&‑conquer, \(O(n log n)\) time, \(O(n)\) extra space |
| 2 | **In‑Memory Transactional KV‑Store** | `inMemoryDb.*` | Hash‑table CRUD with nested `BEGIN / ROLLBACK / COMMIT`; \(O(1)\) avg time per op |

More exercises will be added over time – feel free to open an issue or PR with suggestions!

---

## Building & Running

```bash
# configure out‑of‑source
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
# compile
cmake --build build
# run the bundled tests
build/AlgorithmPlayground      # or bin/Release/AlgorithmPlayground on MSVC
```

On Windows with VS Code, simply press **F5** (launch configuration is bundled).

---

## Contributing

1. Fork / clone the project.
2. Drop your new header & cpp into `include/` and `src/`.
3. Add tests to `AlgorithmPlayground.cpp`.
4. Run `clang-format` (or `std::format` 😉) and open a PR. Happy hacking!