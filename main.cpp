#include <bits/stdc++.h>
using namespace std;
// Stores the best path probability and the actual edge IDs on that path.
// Edge ID is encoded as id = u * n + v, where u and v are 0-indexed.
struct PathResult {
    double prob;
    vector<int> pathIds;
};

/*
    PathEngine builds an adjacency list from the matrix and provides maxPath().

*/
class PathEngine {
public:
    struct Edge {
        int to;       // destination node, 0-indexed
        int id;       // encoded edge id = from * n + to
        double p;     // success probability of this edge
    };

    int n;
    const vector<vector<double>>& M;
    vector<vector<Edge>> adj;
    vector<int> positiveNonSelfIds; // existing non-self edges, useful for padding Part 2 output to 3 edges

    explicit PathEngine(const vector<vector<double>>& matrix)
        : n((int)matrix.size()), M(matrix) {

        adj.assign(n, {});

        for (int u = 0; u < n; ++u) {
            for (int v = 0; v < n; ++v) {
                // Self-loops M[i][i] = 1 do not advance the attack, so ignore them.
                if (u == v) continue;

                // M[u][v] == 0 means there is no usable edge.
                if (M[u][v] > 0.0) {
                    int id = u * n + v;
                    adj[u].push_back({v, id, M[u][v]});
                    positiveNonSelfIds.push_back(id);
                }
            }

            // Sorting is not required for correctness, but exploring high-probability
            // edges first can make the priority queue settle good paths earlier.
            sort(adj[u].begin(), adj[u].end(), [](const Edge& a, const Edge& b) {
                return a.p > b.p;
            });
        }
    }

    // Return the maximum success probability from node 0 to node n-1, after
    // treating removed[id] == true edges as deleted.
    PathResult maxPath(const vector<char>& removed) const {
        vector<double> best(n, 0.0);
        vector<int> parentNode(n, -1);
        vector<int> parentEdge(n, -1);

        priority_queue<pair<double, int>> pq; // max-heap: {probability, node}

        best[0] = 1.0;
        pq.push({1.0, 0});

        while (!pq.empty()) {
            auto [curProb, u] = pq.top();
            pq.pop();

            // Skip stale queue entries.
            if (curProb < best[u]) continue;

            // Once target is popped, it already has the maximum possible probability.
            if (u == n - 1) break;

            for (const Edge& e : adj[u]) {
                if (removed[e.id]) continue;

                double candidate = curProb * e.p;

                if (candidate > best[e.to]) {
                    best[e.to] = candidate;
                    parentNode[e.to] = u;
                    parentEdge[e.to] = e.id;
                    pq.push({candidate, e.to});
                }
            }
        }

        // Target is unreachable after removals.
        if (best[n - 1] == 0.0) {
            return {0.0, {}};
        }

        // Reconstruct the actual best path by following parent pointers backward.
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

/*
    Dinic max-flow implementation.
    I give every existing non-self edge capacity 1. Then the min s-t cut size
    equals the minimum number of edges needed to disconnect source and target.
*/
class Dinic {
public:
    struct E {
        int to;
        int rev;
        int cap;
        int origCap;
        int id;
    };

    int n;
    vector<vector<E>> g;
    vector<int> level;
    vector<int> it;

    explicit Dinic(int n_) : n(n_), g(n_), level(n_), it(n_) {}

    void addEdge(int u, int v, int cap, int id) {
        E forward{v, (int)g[v].size(), cap, cap, id};
        E backward{u, (int)g[u].size(), 0, 0, -1};
        g[u].push_back(forward);
        g[v].push_back(backward);
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

            int pushed = dfs(e.to, t, min(f, e.cap));
            if (pushed > 0) {
                e.cap -= pushed;
                g[e.to][e.rev].cap += pushed;
                return pushed;
            }
        }

        return 0;
    }

    // The limit is 4 because for this assignment we only care whether the
    // min-cut is <= 3 or > 3. No need to compute huge flow values.
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

    // After maxflow(), nodes reachable from s in the residual graph define
    // the source side of a minimum cut. Edges crossing from reachable to
    // unreachable are the cut edges.
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

// The output must contain exactly 3 edges. If a solution uses fewer than 3
// edges, add harmless extra edges. Prefer real positive non-self edges first.
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

    // Prefer actual edges in the original graph.
    for (int id : positiveNonSelfIds) addId(id);

    // Fallback: any non-self pair. This should rarely be needed, but it keeps
    // the function safe even when the graph has fewer than 3 positive edges.
    for (int u = 0; u < n; ++u) {
        for (int v = 0; v < n; ++v) {
            if (u != v) addId(u * n + v);
        }
    }

    // Final fallback for extremely small graphs.
    for (int u = 0; u < n; ++u) {
        addId(u * n + u);
    }

    return out;
}

// Create a compact key for the current set of removed edges.
// chosen.size() is at most 3, and n <= 100, so edge IDs are at most 9999.
static unsigned long long stateKey(vector<int> ids) {
    sort(ids.begin(), ids.end());

    const unsigned long long BASE = 10001ULL;
    unsigned long long key = (unsigned long long)ids.size();

    for (int id : ids) {
        key = key * BASE + (unsigned long long)(id + 1);
    }

    return key;
}

// Convert encoded 0-indexed edge id to assignment-style 1-indexed pair.
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

// Required by the assignment.
double solve_part1(const vector<vector<double>>& M) {
    int n = (int)M.size();
    PathEngine engine(M);

    // No edges are removed in Part 1.
    vector<char> removed(n * n, 0);
    return engine.maxPath(removed).prob;
}

// Required by the assignment.
double solve_part2(const vector<vector<double>>& M,
                   vector<pair<int, int>>& edges_out) {
    int n = (int)M.size();
    PathEngine engine(M);

    // Step 1: Fast check whether 3 edge removals can completely disconnect
    // source from target. If yes, probability 0 is optimal.
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

    /*
        Step 2: Recursive search for the best 3 edges to remove.
    */
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
        // 0 is the smallest possible probability, so we cannot improve further.
        if (bestProb == 0.0) return;

        unsigned long long key = stateKey(chosen);
        if (visited.count(key)) return;
        visited.insert(key);

        PathResult cur = engine.maxPath(removed);

        // If already disconnected before using all 3 removals, pad to exactly 3.
        if (cur.prob == 0.0) {
            updateBest(chosen, 0.0);
            return;
        }

        // We have chosen exactly 3 edges. Evaluate this complete solution.
        if ((int)chosen.size() == 3) {
            updateBest(chosen, cur.prob);
            return;
        }

        // Branch only on edges in the current optimal path.
        vector<int> branchEdges = cur.pathIds;

        // Optional heuristic: try higher-probability edges first.
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

    // Safety fallback. Normally dfs() will always set bestIds.
    if (bestIds.empty()) {
        bestIds = padToThree({}, n, engine.positiveNonSelfIds);
    }

    // Recompute the probability for the final 3 printed edges, so PART2_PROB
    // always matches PART2_EDGES exactly.
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
