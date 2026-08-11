# Format
We give descriptions of the format of multi-boundary islands.

## Multi Boundary Island Format
$N$ \
$R_1$ ... $R_k$ \
$M$ \
$a_1$ $b_1$ $c_1$ \
...               \
$a_N$ $b_N$ $c_N$

---
$N$ denotes the number of vertices of degree 2 or 3.
$k$ is the number of rings.
$R_i$ ($1 \leq i \leq k$) denotes the number of edges that are incident to the degree-one vertices in the $i$-th ring.
The index of these edges are $\{a_i, a_i + 1, \ldots, a_i + R_i - 1\}$ where $a_i = \sum_{1 \leq j < i} R_j$.
These indices are ordered in clockwise order of rotatinos.
$M$ denotes the number of vertices of degree 2.
We add a dummy edge to each vertex of degree 2 to make its degree 3.
The index of these edges are $\{a_{k+1}, a_{k+1} + 1, \ldots, a_{k+1} + M - 1\}$ where $a_{k+1} = \sum_{1 \leq j < k + 1} R_j$.
For the $i$-th vertex, $a_i,b_i,c_i$ is the indices of the three incident edges.
If the degree of this vertex is 3, they are listed in the cyclic order around the vertex in the planar embedding.
