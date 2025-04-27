#include "AlgorithmPlayground.h"
#include "ClosestPairSolver.h"

// A simple struct to bundle each ClosestPairSolver test
struct TestCase {
    std::string name;
    std::vector<Point> points;
    double expectedDist;
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

int main() {
    cout << "Running ClosestPairSolver Tests:" << endl;
    runClosestPairTests();
    return 0;
}