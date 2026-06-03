#include <iostream>
#include <vector>
#include <fstream>
#include <functional>
#include <string>
#include <cassert>
#include <filesystem>

using namespace std;

struct Edge {
    int u, v; // fixed orientation u -> v
};

struct IncidentEdge {
    int edgeIndex;
    bool isIn; // true = edge enters vertex, false = edge leaves vertex
};

class SATFlowEncoder {
private:
    int n, m, k;
    vector<Edge> edges;
    vector<vector<int>> clauses;

    int modNormalize(int x) const {
        x %= k;
        if (x < 0) x += k;
        return x;
    }

public:
    SATFlowEncoder(int n_, int m_, int k_, const vector<Edge>& edges_)
        : n(n_), m(m_), k(k_), edges(edges_) {
    }

    int var(int edgeIndex, int value) const {
        return edgeIndex * (k - 1) + value;
    }

    int variableCount() const {
        return m * (k - 1);
    }

    const vector<vector<int>>& getClauses() const {
        return clauses;
    }

    void addClause(const vector<int>& clause) {
        clauses.push_back(clause);
    }

    void addExactlyOneValuePerEdge() {
        for (int i = 0; i < m; ++i) {
            vector<int> atLeastOne;
            for (int val = 1; val <= k - 1; ++val) {
                atLeastOne.push_back(var(i, val));
            }
            addClause(atLeastOne);

            for (int a = 1; a <= k - 1; ++a) {
                for (int b = a + 1; b <= k - 1; ++b) {
                    addClause({ -var(i, a), -var(i, b) });
                }
            }
        }
    }

    void addVertexConservationConstraints() {
        vector<vector<IncidentEdge>> incident(n + 1);

        for (int i = 0; i < m; ++i) {
            int u = edges[i].u;
            int v = edges[i].v;

            incident[u].push_back({ i, false }); // edge leaves u
            incident[v].push_back({ i, true });  // edge enters v
        }

        for (int vertex = 1; vertex <= n; ++vertex) {
            const auto& inc = incident[vertex];
            int d = (int)inc.size();

            if (d == 0) continue;

            vector<int> chosenValues(d, 1);

            function<void(int)> dfs = [&](int pos) {
                if (pos == d) {
                    int inflow = 0;
                    int outflow = 0;

                    for (int j = 0; j < d; ++j) {
                        int val = chosenValues[j];
                        if (inc[j].isIn) inflow = (inflow + val) % k;
                        else outflow = (outflow + val) % k;
                    }

                    if (inflow != outflow) {
                        vector<int> clause;
                        for (int j = 0; j < d; ++j) {
                            clause.push_back(-var(inc[j].edgeIndex, chosenValues[j]));
                        }
                        addClause(clause);
                    }
                    return;
                }

                for (int val = 1; val <= k - 1; ++val) {
                    chosenValues[pos] = val;
                    dfs(pos + 1);
                }
                };

            dfs(0);
        }
    }

    void addRichConstraints(bool allowConfluent, bool allowContrafluent) {
        vector<vector<IncidentEdge>> incident(n + 1);

        for (int i = 0; i < m; ++i) {
            int u = edges[i].u;
            int v = edges[i].v;

            incident[u].push_back({ i, false }); // edge leaves u
            incident[v].push_back({ i, true });  // edge enters v
        }

        for (int vertex = 1; vertex <= n; ++vertex) {
            const auto& inc = incident[vertex];
            int d = (int)inc.size();

            for (int i = 0; i < d; ++i) {
                for (int j = 0; j < d; ++j) {
                    if (i == j) continue;

                    int e1 = inc[i].edgeIndex;
                    int e2 = inc[j].edgeIndex;

                    for (int a = 1; a <= k - 1; ++a) {
                        for (int b = 1; b <= k - 1; ++b) {
                            // e1 normalizujeme ako hranu do vrchola
                            int val1 = inc[i].isIn ? a : modNormalize(-a);

                            // e2 normalizujeme ako hranu z vrchola
                            int val2 = inc[j].isIn ? modNormalize(-b) : b;

                            bool forbid = false;

                            if (!allowConfluent && val1 == val2) {
                                forbid = true;
                            }

                            if (!allowContrafluent && val1 == modNormalize(-val2)) {
                                forbid = true;
                            }

                            if (forbid) {
                                addClause({ -var(e1, a), -var(e2, b) });
                            }
                        }
                    }
                }
            }
        }
    }

    void buildCNF(bool allowConfluent = true, bool allowContrafluent = true) {
        clauses.clear();
        addExactlyOneValuePerEdge();
        addVertexConservationConstraints();
        addRichConstraints(allowConfluent, allowContrafluent);
    }

    void writeDIMACS(const string& filename) const {
        ofstream fout(filename);
        if (!fout) {
            cerr << "Error: cannot open output file " << filename << "\n";
            return;
        }

        fout << "p cnf " << variableCount() << " " << clauses.size() << "\n";
        for (const auto& clause : clauses) {
            for (int lit : clause) {
                fout << lit << " ";
            }
            fout << "0\n";
        }
    }

    void printSummary() const {
        cout << "Variables: " << variableCount() << "\n";
        cout << "Clauses: " << clauses.size() << "\n";
    }
};

int modNormalizeBF(int x, int k) {
    x %= k;
    if (x < 0) x += k;
    return x;
}

bool richConditionHolds(const vector<Edge>& edges, int n, int k, const vector<int>& flow,
    bool allowConfluent, bool allowContrafluent) {
    vector<vector<IncidentEdge>> incident(n + 1);

    for (size_t i = 0; i < edges.size(); ++i) {
        int u = edges[i].u;
        int v = edges[i].v;
        incident[u].push_back({ (int)i, false });
        incident[v].push_back({ (int)i, true });
    }

    for (int vertex = 1; vertex <= n; ++vertex) {
        const auto& inc = incident[vertex];
        int d = (int)inc.size();

        for (int i = 0; i < d; ++i) {
            for (int j = 0; j < d; ++j) {
                if (i == j) continue;

                int e1 = inc[i].edgeIndex;
                int e2 = inc[j].edgeIndex;

                int a = flow[e1];
                int b = flow[e2];

                int val1 = inc[i].isIn ? a : modNormalizeBF(-a, k);
                int val2 = inc[j].isIn ? modNormalizeBF(-b, k) : b;

                if (!allowConfluent && val1 == val2) return false;
                if (!allowContrafluent && val1 == modNormalizeBF(-val2, k)) return false;
            }
        }
    }

    return true;
}

bool existsZkFlowBruteforce(int n, int m, int k, const vector<Edge>& edges,
    bool allowConfluent = true, bool allowContrafluent = true) {
    if (k < 2) return false;
    vector<int> flow(m, 1);

    function<bool(int)> dfs = [&](int idx) -> bool {
        if (idx == m) {
            vector<int> inf(n + 1, 0), outf(n + 1, 0);

            for (int i = 0; i < m; ++i) {
                int u = edges[i].u;
                int v = edges[i].v;
                int val = flow[i] % k;
                outf[u] = (outf[u] + val) % k;
                inf[v] = (inf[v] + val) % k;
            }

            for (int v = 1; v <= n; ++v) {
                if (inf[v] != outf[v]) return false;
            }

            if (!richConditionHolds(edges, n, k, flow, allowConfluent, allowContrafluent)) {
                return false;
            }

            return true;
        }

        for (int val = 1; val <= k - 1; ++val) {
            flow[idx] = val;
            if (dfs(idx + 1)) return true;
        }
        return false;
        };

    return dfs(0);
}

vector<Edge> getPetersenGraph() {
    return {
        {1,2}, {2,3}, {3,4}, {4,5}, {5,1},
        {6,8}, {8,10}, {10,7}, {7,9}, {9,6},
        {1,6}, {2,7}, {3,8}, {4,9}, {5,10}
    };
}

void runTests() {
    {
        int n = 3, m = 3, k = 3;
        vector<Edge> edges = { {1,2}, {2,3}, {3,1} };
        assert(existsZkFlowBruteforce(n, m, k, edges) == true);

        SATFlowEncoder enc(n, m, k, edges);
        enc.buildCNF();
        assert(enc.variableCount() == 6);
        assert(!enc.getClauses().empty());
    }

    {
        int n = 3, m = 2, k = 3;
        vector<Edge> edges = { {1,2}, {2,3} };
        assert(existsZkFlowBruteforce(n, m, k, edges) == false);

        SATFlowEncoder enc(n, m, k, edges);
        enc.buildCNF();
        assert(enc.variableCount() == 4);
        assert(!enc.getClauses().empty());
    }

    {
        int n = 4, m = 4, k = 3;
        vector<Edge> edges = { {1,2}, {2,3}, {3,4}, {4,1} };
        assert(existsZkFlowBruteforce(n, m, k, edges) == true);
    }

    {
        int n = 2, m = 1, k = 3;
        vector<Edge> edges = { {1,2} };
        assert(existsZkFlowBruteforce(n, m, k, edges) == false);
    }

    {
        int n = 2, m = 2, k = 3;
        vector<Edge> edges = { {1,2}, {2,1} };
        assert(existsZkFlowBruteforce(n, m, k, edges) == true);
    }

    {
        // Petersen graph test
        int n = 10, m = 15, k = 5;
        vector<Edge> edges = getPetersenGraph();

        SATFlowEncoder enc(n, m, k, edges);
        enc.buildCNF(false, false);
        assert(enc.variableCount() == 60);
        assert(!enc.getClauses().empty());
    }

    cout << "All tests passed.\n";
}

int main() {
    runTests();

    int n, m, k;
    cout << "Enter n (vertices), m (edges), k (mod): ";
    cin >> n >> m >> k;

    if (k < 2) {
        cerr << "Error: k must be at least 2.\n";
        return 1;
    }

    int confluentInput, contrafluentInput;
    cout << "Allow confluent pairs? (1 = yes, 0 = no): ";
    cin >> confluentInput;
    cout << "Allow contrafluent pairs? (1 = yes, 0 = no): ";
    cin >> contrafluentInput;

    bool allowConfluent = (confluentInput != 0);
    bool allowContrafluent = (contrafluentInput != 0);

    vector<Edge> edges(m);
    cout << "Enter " << m << " edges as pairs u v (fixed orientation u -> v):\n";
    for (int i = 0; i < m; ++i) {
        cin >> edges[i].u >> edges[i].v;
        if (edges[i].u < 1 || edges[i].u > n || edges[i].v < 1 || edges[i].v > n) {
            cerr << "Error: invalid vertex number in edge " << i + 1 << ".\n";
            return 1;
        }
    }

    SATFlowEncoder encoder(n, m, k, edges);
    encoder.buildCNF(allowConfluent, allowContrafluent);
    encoder.printSummary();

    cout << "Current working directory: "
        << filesystem::current_path() << "\n";

    encoder.writeDIMACS("flow.cnf");

    cout << "CNF written to flow.cnf\n";

    if (m <= 8) {
        cout << "Bruteforce existence check says: "
            << (existsZkFlowBruteforce(n, m, k, edges, allowConfluent, allowContrafluent) ? "EXISTS" : "DOES NOT EXIST")
            << "\n";
    }
    else {
        cout << "Bruteforce existence check skipped for larger graph.\n";
    }

    cout << "Then run external SAT solver, for example:\n";
    cout << "minisat.exe flow.cnf result.out\n";

    return 0;
}
