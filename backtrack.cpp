#include <iostream>
#include <vector>
#include <cassert> 
using namespace std;

// edges = { {u, v}, ... }
vector<vector<int>> edges;
int n, m, k_mod;
vector<vector<int>> all_flows;

// idx = index aktualne spracovavanej hrany
// inf[v] = sucet hodnot hran vchadzajucich do vrchola v (inflow)
// outf[v] = sucet hodnot hran vychadzajucich z vrchola v (outflow)

void set_ZK(int idx, vector<int>& inf, vector<int>& outf, vector<int>& current_flow) {
    // ak sme priradili hodnoty vsetkym hranam
    if (idx == m) {
        // kontrola podmienky v kazdom vrchole: inf[v] % k == outf[v] % k
        for (int v = 1; v <= n; ++v) {
            if (inf[v] % k_mod != outf[v] % k_mod) return;
        }
        // ak sme tu, tok je platny ulozime ho
        all_flows.push_back(current_flow);
        return;
    }
    int u = edges[idx][0];
    int v = edges[idx][1];
    // skusame vsetky mozne nenulove hodnoty v Z_k: 1, 2, ..., k-1
    for (int val = 1; val < k_mod; ++val) {
        // tok ide z u do v, hodnota = val
        outf[u] = (outf[u] + val) % k_mod;
        inf[v] = (inf[v] + val) % k_mod;
        current_flow[idx] = val;
        set_ZK(idx + 1, inf, outf, current_flow);

        // backtracking: obnovime stav pred rekurziou
        outf[u] = (outf[u] - val + k_mod) % k_mod;
        inf[v] = (inf[v] - val + k_mod) % k_mod;
    }
}
//tests
bool isValidFlow(const vector<int>& flow, const vector<vector<int>>& edges, int n, int k) {
    vector<int> inf(n + 1, 0);
    vector<int> outf(n + 1, 0);

    for (size_t i = 0; i < edges.size(); ++i) {
        int u = edges[i][0];
        int v = edges[i][1];
        int val = flow[i];
        outf[u] = (outf[u] + val) % k;
        inf[v] = (inf[v] + val) % k;
    }

    for (int v = 1; v <= n; ++v) {
        if (inf[v] != outf[v]) return false;
    }
    return true;
}
void runTests() {
    //n=3, m=3, k=3
    vector<vector<int>> edges1 = { {1, 2}, {2, 3}, {3, 1} };
    assert(isValidFlow({ 1, 1, 1 }, edges1, 3, 3) == true);
    assert(isValidFlow({ 2, 2, 2 }, edges1, 3, 3) == true);
    assert(isValidFlow({ 1, 1, 2 }, edges1, 3, 3) == false);

    // n=4, m=4, k=3
    vector<vector<int>> edges2 = { {1, 2}, {2, 3}, {3, 4}, {4, 1} };
    assert(isValidFlow({ 1, 1, 1, 1 }, edges2, 4, 3) == true);
    assert(isValidFlow({ 2, 2, 2, 2 }, edges2, 4, 3) == true);

    //n=3, m=2, k=3
    edges = { {1, 2}, {2, 3} };
    n = 3;
    m = 2;
    k_mod = 3;

    vector<int> inf(n + 1, 0);
    vector<int> outf(n + 1, 0);
    vector<int> current_flow(m);

    all_flows.clear();
    set_ZK(0, inf, outf, current_flow);

    assert(all_flows.size() == 0);

    cout << "All tests passed\n";
}

int main() {
    runTests();
    cout << "n (vertices), m (edges), k (mod): ";
    cin >> n >> m >> k_mod;
    // nacitame m hran
    edges.resize(m);
    for (int i = 0; i < m; ++i) {
        int u, v;
        cin >> u >> v;
        edges[i] = { u, v };
    }
    // inicializacia pomocnych struktur
    vector<int> inf(n + 1, 0);
    vector<int> outf(n + 1, 0);
    vector<int> current_flow(m);
    // spustime backtracking
    all_flows.clear();
    set_ZK(0, inf, outf, current_flow);
    // vypiseme vysledok
    cout << "Found " << all_flows.size() << " Z_" << k_mod << "-flows:\n";
    for (size_t i = 0; i < all_flows.size(); ++i) {
        cout << "Flow " << i + 1 << ": ";
        for (int j = 0; j < m; ++j) {
            cout << all_flows[i][j] << " ";
        }
        cout << "\n";
    }
}