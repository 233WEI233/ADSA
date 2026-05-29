#include <bits/stdc++.h>
using namespace std;

struct PathResult {
    double prob;
    vector<int> pathIds; // edge ids u*n+v, 0-indexed
};

class PathEngine {
public:
    struct Edge {
        int to;
        int id;
        double p;
    };

    int n;
    const vector<vector<double>>& M;
    vector<vector<Edge>> adj;
    vector<int> positiveNonSelfIds;

    explicit PathEngine(const vector<vector<double>>& matrix) : n((int)matrix.size()), M(matrix) {
        adj.assign(n, {});
        for (int u = 0; u < n; ++u) {
            for (int v = 0; v < n; ++v) {
                if (u == v) continue;          // self-loops do not advance the attack
                if (M[u][v] > 0.0) {
                    int id = u * n + v;
                    adj[u].push_back({v, id, M[u][v]});
                    positiveNonSelfIds.push_back(id);
                }
            }
            sort(adj[u].begin(), adj[u].end(), [](const Edge& a, const Edge& b) {
                return a.p > b.p;
            });
        }
    }

    PathResult maxPath(const vector<char>& removed) const {
        vector<double> best(n, 0.0);
        vector<int> parentNode(n, -1), parentEdge(n, -1);
        priority_queue<pair<double, int>> pq;

        best[0] = 1.0;
        pq.push({1.0, 0});

        while (!pq.empty()) {
            auto [curProb, u] = pq.top();
            pq.pop();

            if (curProb < best[u]) continue;
            if (u == n - 1) break;

            for (const Edge& e : adj[u]) {
                if (removed[e.id]) continue;
                double cand = curProb * e.p;
                if (cand > best[e.to]) {
                    best[e.to] = cand;
                    parentNode[e.to] = u;
                    parentEdge[e.to] = e.id;
                    pq.push({cand, e.to});
                }
            }
        }

        if (best[n - 1] == 0.0) {
            return {0.0, {}};
        }

        vector<int> path;
        int cur = n - 1;
        while (cur != 0) {
            int eid = parentEdge[cur];
            if (eid < 0) break;
            path.push_back(eid);
            cur = parentNode[cur];
        }
        reverse(path.begin(), path.end());

        return {best[n - 1], path};
    }
};

class Dinic {
public:
    struct E {
        int to, rev, cap, origCap, id;
    };

    int n;
    vector<vector<E>> g;
    vector<int> level, it;

    explicit Dinic(int n_) : n(n_), g(n_), level(n_), it(n_) {}

    void addEdge(int u, int v, int cap, int id) {
        E a{v, (int)g[v].size(), cap, cap, id};
        E b{u, (int)g[u].size(), 0, 0, -1};
        g[u].push_back(a);
        g[v].push_back(b);
    }

    bool bfs(int s, int t) {
        fill(level.begin(), level.end(), -1);
        queue<int> q;
        level[s] = 0;
        q.push(s);

        while (!q.empty()) {
            int u = q.front();
            q.pop();

            for (const E& e : g[u]) {
                if (e.cap > 0 && level[e.to] < 0) {
                    level[e.to] = level[u] + 1;
                    q.push(e.to);
                }
            }
        }

        return level[t] >= 0;
    }

    int dfs(int u, int t, int f) {
        if (u == t) return f;

        for (int& i = it[u]; i < (int)g[u].size(); ++i) {
            E& e = g[u][i];

            if (e.cap <= 0 || level[e.to] != level[u] + 1) continue;

            int ret = dfs(e.to, t, min(f, e.cap));
            if (ret > 0) {
                e.cap -= ret;
                g[e.to][e.rev].cap += ret;
                return ret;
            }
        }

        return 0;
    }

    int maxflow(int s, int t, int limit) {
        int flow = 0;

        while (flow < limit && bfs(s, t)) {
            fill(it.begin(), it.end(), 0);

            while (flow < limit) {
                int pushed = dfs(s, t, limit - flow);
                if (pushed == 0) break;
                flow += pushed;
            }
        }

        return flow;
    }

    vector<int> minCutIds(int s) const {
        vector<int> seen(n, 0);
        queue<int> q;

        seen[s] = 1;
        q.push(s);

        while (!q.empty()) {
            int u = q.front();
            q.pop();

            for (const E& e : g[u]) {
                if (e.cap > 0 && !seen[e.to]) {
                    seen[e.to] = 1;
                    q.push(e.to);
                }
            }
        }

        vector<int> cut;
        for (int u = 0; u < n; ++u) {
            if (!seen[u]) continue;

            for (const E& e : g[u]) {
                if (e.origCap > 0 && !seen[e.to]) {
                    cut.push_back(e.id);
                }
            }
        }

        return cut;
    }
};

static vector<int> padToThree(const vector<int>& ids,
                              int n,
                              const vector<int>& positiveNonSelfIds) {
    vector<int> out = ids;
    vector<char> used(n * n, 0);

    for (int id : out) {
        if (0 <= id && id < n * n) used[id] = 1;
    }

    auto addId = [&](int id) {
        if ((int)out.size() < 3 && !used[id]) {
            used[id] = 1;
            out.push_back(id);
        }
    };

    for (int id : positiveNonSelfIds) addId(id);

    for (int u = 0; u < n; ++u) {
        for (int v = 0; v < n; ++v) {
            if (u != v) addId(u * n + v);
        }
    }

    for (int u = 0; u < n; ++u) {
        addId(u * n + u);
    }

    return out;
}

static unsigned long long stateKey(vector<int> ids) {
    sort(ids.begin(), ids.end());

    const unsigned long long BASE = 10001ULL;
    unsigned long long key = (unsigned long long)ids.size();

    for (int id : ids) {
        key = key * BASE + (unsigned long long)(id + 1);
    }

    return key;
}

static pair<int, int> idToPair(int id, int n) {
    return {id / n + 1, id % n + 1};
}

static vector<pair<int, int>> idsToPairs(const vector<int>& ids, int n) {
    vector<pair<int, int>> ans;

    for (int id : ids) {
        ans.push_back(idToPair(id, n));
    }

    return ans;
}

double solve_part1(const vector<vector<double>>& M) {
    int n = (int)M.size();

    PathEngine engine(M);
    vector<char> removed(n * n, 0);

    return engine.maxPath(removed).prob;
}

double solve_part2(const vector<vector<double>>& M,
                   vector<pair<int, int>>& edges_out) {
    int n = (int)M.size();

    PathEngine engine(M);

    Dinic dinic(n);
    for (int u = 0; u < n; ++u) {
        for (int v = 0; v < n; ++v) {
            if (u != v && M[u][v] > 0.0) {
                dinic.addEdge(u, v, 1, u * n + v);
            }
        }
    }

    int flow = dinic.maxflow(0, n - 1, 4);

    if (flow <= 3) {
        vector<int> cut = dinic.minCutIds(0);
        cut = padToThree(cut, n, engine.positiveNonSelfIds);

        edges_out = idsToPairs(cut, n);
        return 0.0;
    }

    vector<char> removed(n * n, 0);
    vector<int> chosen;
    vector<int> bestIds;
    double bestProb = numeric_limits<double>::infinity();

    unordered_set<unsigned long long> visited;

    auto updateBest = [&](const vector<int>& ids, double prob) {
        vector<int> padded = padToThree(ids, n, engine.positiveNonSelfIds);

        if (bestIds.empty() || prob < bestProb) {
            bestProb = prob;
            bestIds = padded;
        }
    };

    function<void()> dfs = [&]() {
        if (bestProb == 0.0) return;

        unsigned long long key = stateKey(chosen);
        if (visited.find(key) != visited.end()) return;
        visited.insert(key);

        PathResult cur = engine.maxPath(removed);

        if (cur.prob == 0.0) {
            updateBest(chosen, 0.0);
            return;
        }

        if ((int)chosen.size() == 3) {
            updateBest(chosen, cur.prob);
            return;
        }

        vector<int> branchEdges = cur.pathIds;

        sort(branchEdges.begin(), branchEdges.end(), [&](int a, int b) {
            int au = a / n, av = a % n;
            int bu = b / n, bv = b % n;
            return M[au][av] > M[bu][bv];
        });

        for (int id : branchEdges) {
            if (removed[id]) continue;

            removed[id] = 1;
            chosen.push_back(id);

            dfs();

            chosen.pop_back();
            removed[id] = 0;

            if (bestProb == 0.0) return;
        }
    };

    dfs();

    if (bestIds.empty()) {
        bestIds = padToThree({}, n, engine.positiveNonSelfIds);

        vector<char> rem2(n * n, 0);
        for (int id : bestIds) {
            rem2[id] = 1;
        }

        bestProb = engine.maxPath(rem2).prob;
    }

    vector<char> finalRemoved(n * n, 0);
    for (int id : bestIds) {
        finalRemoved[id] = 1;
    }

    bestProb = engine.maxPath(finalRemoved).prob;

    edges_out = idsToPairs(bestIds, n);
    return bestProb;
}

int main() {
    ifstream fin("matrix.txt");

    int n;
    fin >> n;

    vector<vector<double>> M(n, vector<double>(n));

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            fin >> M[i][j];
        }
    }

    double part1 = solve_part1(M);

    vector<pair<int, int>> edges;
    double part2 = solve_part2(M, edges);

    cout << fixed << setprecision(6);

    cout << "PART1: " << part1 << '\n';

    cout << "PART2_EDGES:";
    for (const auto& e : edges) {
        cout << " (" << e.first << "," << e.second << ")";
    }
    cout << '\n';

    cout << "PART2_PROB: " << part2 << '\n';

    return 0;
}