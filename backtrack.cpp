#include <iostream>
#include <vector>
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
int main() {
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