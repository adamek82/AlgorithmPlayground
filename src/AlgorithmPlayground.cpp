#include "AlgorithmPlayground.h"
#include "ClosestPairSolver.h"
#include "inMemoryDb.h"

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

int main() {
    cout << "Running ClosestPairSolver Tests:" << endl;
    runClosestPairTests();
    cout << "Running InMemoryDb Tests:" << endl;
    runInMemoryDbTests();
    return 0;
}