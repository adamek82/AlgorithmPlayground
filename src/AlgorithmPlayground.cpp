#include "AlgorithmPlayground.h"
#include "ClosestPairSolver.h"
#include "inMemoryDb.h"
#include "BitonicTSPSolver.h"
#include "LexiPathEngine.h"

// A simple struct to bundle each ClosestPairSolver test
struct TestCase {
    std::string name;
    std::vector<Point> points;
    double expectedDist;
};

/* ───────────── test‑case definition ───────────── */
struct DbTestCase {
  vector<string>                     ops;        // command names
  vector<vector<string>>             args;       // args per command
  vector<optional<string>>           expected;   // output or nullopt

  DbTestCase(vector<string>  o,
             vector<vector<string>> a,
             vector<optional<string>> e)
      : ops(std::move(o))
      , args(std::move(a))
      , expected(std::move(e)) {}
};

// TestCase struct to group input and expected output
typedef std::vector<std::vector<double>> Arr;
struct BitonicTestCase {
    std::string name;
    Arr arr;
    double expected;
};

// All of our closest-pair tests live here
static void runClosestPairTests() {
    const double EPS = 1e-6;
    ClosestPairSolver solver;

    std::vector<TestCase> tests = {
        { "Minimal (2 pts)",
          { {0,0}, {1,1} },
          std::sqrt(2.0) },

        { "Three collinear",
          { {0,0}, {5,5}, {3,3} },
          std::sqrt(8.0) },

        { "Duplicates => zero",
          { {1,1}, {2,2}, {1,1}, {3,3} },
          0.0 },

        { "Six-sample set",
          { {2,3}, {12,30}, {40,50}, {5,1}, {12,10}, {3,4} },
          std::sqrt(2.0) },

        { "Medium (13 pts)",
          { {0,0}, {0,1}, {0,5}, {5,5},
            {100,100}, {100,101}, {105,100},
            {50,50}, {49,50}, {51,49}, {49,49},
            {60,60}, {61,61} },
          1.0 },

        { "Noisy Grid 4 x 5",
          { {0,0}, {0,1}, {0,2}, {0,3}, {0,4},
            {1,0}, {1,1}, {1,2}, {1,3}, {1,4},
            {2,0}, {2,1}, {2,2}, {2,3}, {2,4},
            {3,0}, {3,1}, {3,2}, {3,3}, {3,4},
            {100,0}, { -50, 50 } }, // two far-away noise points
          1.0 },

        { "Manual noisy 35 pts",
          {
            {0,0}, {5,5}, {5,6}, {10,10}, {11,10},
            {20,20}, {20,22}, {21,21}, {100,100}, {101,101},
            {102,100}, {50,49}, {50,50}, {49,50}, {49,49},
            {-5,-5}, {-4,-4}, {-6,-5}, {30,30}, {30,31},
            {30,32}, {31,30}, {31,31}, {45,45}, {44,44},
            {46,45}, {47,47}, {60,60}, {61,60}, {60,61},
            {1000,1000}, {999,1000}, {1000,999}, {123,456}, {124,456}
          },
          1.0 }
    };

    std::cout << std::fixed << std::setprecision(6);
    for (size_t i = 0; i < tests.size(); ++i) {
        const auto& tc = tests[i];
        PairDist res = solver.closestPair(tc.points);
        bool pass = std::fabs(res.dist - tc.expectedDist) < EPS;

        std::cout << "Test " << (i+1)
                  << ": " << tc.name << ": "
                  << (pass ? "PASS" : "FAIL")
                  << " (got " << res.dist
                  << ", exp " << tc.expectedDist << ")\n";
    }
}

static void runInMemoryDbTests() {
    /* ------------- all sessions from the prompt ------------- */
    vector<DbTestCase> tests = {
        /* Example 1 */
        {
            {"SET","GET","DELETE","GET"},
            {{"a","10"},{"a"},{"a"},{"a"}},
            {nullopt,"10",nullopt,"NULL"}
        },
        /* Example 2 */
        {
            {"SET","SET","COUNT","COUNT","DELETE","COUNT","SET","COUNT"},
            {{"a","10"},{"b","10"},{"10"},{"20"},{"a"},{"10"},{"b","30"},{"10"}},
            {nullopt,nullopt,"2","0",nullopt,"1",nullopt,"0"}
        },
        /* Example 3 (nested rollbacks) */
        {
            {"BEGIN","SET","GET","BEGIN","SET","GET","ROLLBACK","GET","ROLLBACK","GET"},
            {{},{"a","10"},{"a"},{},{"a","20"},{"a"},{},{"a"},{},{"a"}},
            {nullopt,nullopt,"10",nullopt,nullopt,"20",nullopt,"10",nullopt,"NULL"}
        },
        /* Example 4 (commit, then NO TRANSACTION) */
        {
            {"BEGIN","SET","BEGIN","SET","COMMIT","GET","ROLLBACK"},
            {{},{"a","30"},{},{"a","40"},{},{"a"},{}},
            {nullopt,nullopt,nullopt,nullopt,nullopt,"40","NO TRANSACTION"}
        },
        /* Example 5 (delete inside nested txns) */
        {
            {"SET","BEGIN","GET","SET","BEGIN","DELETE","GET","ROLLBACK","GET","COMMIT","GET"},
            {{"a","50"},{},{"a"},{"a","60"},{},{"a"},{"a"},{},{"a"},{},{"a"}},
            {nullopt,nullopt,"50",nullopt,nullopt,nullopt,"NULL",nullopt,"60",nullopt,"60"}
        },
        /* Example 6 (COUNT with rollback) */
        {
            {"SET","BEGIN","COUNT","BEGIN","DELETE","COUNT","ROLLBACK","COUNT"},
            {{"a","10"},{},{"10"},{},{"a"},{"10"},{},{"10"}},
            {nullopt,nullopt,"1",nullopt,nullopt,"0",nullopt,"1"}
        }
    };

    /* ---------------- execute the tests ---------------- */
    for (size_t tc = 0; tc < tests.size(); ++tc) {
        std::cout << "Running DB Test Case " << tc + 1 << ":\n";
        InMemoryDB db;

        for (size_t i = 0; i < tests[tc].ops.size(); ++i) {
            const string& op   = tests[tc].ops[i];
            const auto&   arg  = tests[tc].args[i];
            const auto&   exp  = tests[tc].expected[i];
            bool          pass = true;              // optimistic

            if (op == "SET") {
                db.set(arg[0], arg[1]);
                std::cout << "  SET(" << arg[0] << "," << arg[1] << ") -> null\n";
            }
            else if (op == "GET") {
                string out = db.get(arg[0]).value_or("NULL");
                pass = (exp && out == *exp);
                std::cout << "  GET(" << arg[0] << ") -> " << out
                          << (exp ? (pass ? " [PASS]" : " [FAIL]") : "") << '\n';
            }
            else if (op == "DELETE") {
                db.del(arg[0]);
                std::cout << "  DELETE(" << arg[0] << ") -> null\n";
            }
            else if (op == "COUNT") {
                string out = std::to_string(db.count(arg[0]));
                pass = (exp && out == *exp);
                std::cout << "  COUNT(" << arg[0] << ") -> " << out
                          << (exp ? (pass ? " [PASS]" : " [FAIL]") : "") << '\n';
            }
            else if (op == "BEGIN") {
                db.begin();
                std::cout << "  BEGIN() -> null\n";
            }
            else if (op == "ROLLBACK") {
                TxnStatus st = db.rollback();
                if (st == TxnStatus::NoTransaction) {
                    pass = (exp && *exp == "NO TRANSACTION");
                    std::cout << "  ROLLBACK() -> NO TRANSACTION"
                              << (exp ? (pass ? " [PASS]" : " [FAIL]") : "") << '\n';
                } else {
                    std::cout << "  ROLLBACK() -> null\n";
                }
            }
            else if (op == "COMMIT") {
                TxnStatus st = db.commit();
                if (st == TxnStatus::NoTransaction) {
                    pass = (exp && *exp == "NO TRANSACTION");
                    std::cout << "  COMMIT() -> NO TRANSACTION"
                              << (exp ? (pass ? " [PASS]" : " [FAIL]") : "") << '\n';
                } else {
                    std::cout << "  COMMIT() -> null\n";
                }
            }
            else {
                std::cerr << "  Unknown operation: " << op << '\n';
            }
        }
        std::cout << '\n';
    }
}

static void runBitonicTSPTests() {
    const double EPS = 1e-3;
    std::vector<BitonicTestCase> tests = {
        {"7-point sample",
         {{0,6}, {1,0}, {2,3}, {5,4}, {6,1}, {7,5}, {8,2}},
         25.584}
    };

    std::cout << std::fixed << std::setprecision(3);
    for (size_t i = 0; i < tests.size(); ++i) {
        auto &tc = tests[i];
        std::vector<Point> pts;
        pts.reserve(tc.arr.size());
        for (auto &p : tc.arr) pts.push_back({p[0], p[1]});

        BitonicTSP solver(int(pts.size()));
        double result = solver.solve(pts);
        bool pass = std::fabs(result - tc.expected) < EPS;

        std::cout << "Test " << (i+1) << ": " << tc.name
                  << ": " << (pass?"PASS":"FAIL")
                  << " (got " << result << ", exp "
                  << tc.expected << ")\n";
    }
}

// Run the lexicographic SSSP engine on an input blob that follows the "stdin" format:
//   N M S
//   M lines: u v w
//   Q
//   Q lines: ADD u v w | REM u v w | ASK t
// It returns the concatenated outputs (each on its own line) for ASK commands.
static std::string runLexiEngineFromString(const std::string& input) {
    std::istringstream in(input);
    std::ostringstream out;

    int N, M, S;
    if (!(in >> N >> M >> S)) {
        return ""; // malformed input
    }

    DynamicDirectedGraph graph(N);
    for (int i = 0; i < M; ++i) {
        int u, v, w;
        in >> u >> v >> w;
        graph.addEdge(u, v, w);
    }

    LexiSSSP engine(graph, S);
    engine.touch(); // ensure first ASK triggers recompute

    int Q; in >> Q;
    for (int i = 0; i < Q; ++i) {
        std::string op; in >> op;
        if (op == "ADD") {
            int u, v, w; in >> u >> v >> w;
            engine.addEdgeCmd(u, v, w);
        } else if (op == "REM") {
            int u, v, w; in >> u >> v >> w;
            engine.removeEdgeCmd(u, v, w);
        } else if (op == "ASK") {
            int t; in >> t;
            out << engine.ask(t) << '\n';
        } else {
            std::string rest; std::getline(in, rest); // skip unknown line
        }
    }
    return out.str();
}

static void runLexiPathTests() {
    struct Case {
        std::string name;
        std::string input;
        std::string expected;
    };

    // Example from our discussion (corrected Q=8 so the last ASK is processed)
    const std::string sample1 =
        "5 5 1\n"
        "1 2 3\n"
        "1 3 5\n"
        "2 4 4\n"
        "3 4 4\n"
        "4 5 6\n"
        "8\n"
        "ASK 5\n"
        "ADD 1 5 100\n"
        "ASK 5\n"
        "REM 4 5 6\n"
        "ASK 5\n"
        "ADD 3 5 7\n"
        "ASK 5\n"
        "ASK 4\n";

    // Explanation of expected:
    // ASK 5 -> 6       (1-2-4-5 or 1-3-4-5: sum=13, bottleneck=6)
    // ASK 5 -> 6       (+1->5(100) doesn't help, shortest is still 13)
    // ASK 5 -> 100     (after removing 4->5, only 1->5(100) remains)
    // ASK 5 -> 7       (adding 3->5(7) makes shortest 1->3->5 sum=12, bottleneck=7)
    // ASK 4 -> 4       (to node 4: dist=7, best bottleneck=4)
    const std::string expected1 =
        "6\n"
        "6\n"
        "100\n"
        "7\n"
        "4\n";

    // Second mini test: unreachable becomes reachable, then improved bottleneck
    const std::string sample2 =
        "4 1 1\n"
        "1 2 5\n"
        "6\n"
        "ASK 4\n"        // unreachable
        "ADD 2 4 10\n"   // path 1->2->4 (sum=15, bottleneck=10)
        "ASK 4\n"
        "ADD 1 3 7\n"    // add alternative via 3
        "ADD 3 4 7\n"    // 1->3->4 (sum=14, bottleneck=7) => better sum, better bottleneck
        "ASK 4\n";

    const std::string expected2 =
        "-1\n"
        "10\n"
        "7\n";

    std::vector<Case> tests = {
        {"Baseline with multiple updates", sample1, expected1},
        {"Reachability and bottleneck improvement", sample2, expected2}
    };

    for (std::size_t i = 0; i < tests.size(); ++i) {
        const auto& tc = tests[i];
        std::string got = runLexiEngineFromString(tc.input);
        bool pass = (got == tc.expected);
        std::cout << "LexiSSSP Test " << (i+1) << ": " << tc.name
                  << ": " << (pass ? "PASS" : "FAIL") << "\n";
        if (!pass) {
            std::cout << "  Expected:\n" << tc.expected
                      << "  Got:\n"      << got;
        }
    }
}

int main() {
    cout << "Running ClosestPairSolver Tests:" << endl;
    runClosestPairTests();
    cout << "Running InMemoryDb Tests:" << endl;
    runInMemoryDbTests();
    cout << "Running BitonicTSP Tests:" << endl;
    runBitonicTSPTests();
    cout << "Running LexiSSSP Tests:" << endl;
    runLexiPathTests();
    return 0;
}