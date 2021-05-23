/**
 * @file backtrack.cc
 *
 */

#include "backtrack.h"
#include "stdlib.h"
#include <queue>


Backtrack::Backtrack() {}
Backtrack::~Backtrack() {}

void Backtrack::PrintAllMatches(const Graph &data, const Graph &query,
                                const CandidateSet &cs) {
  std::cout << "t " << query.GetNumVertices() << "\n";

  // implement your code here.
  Vertex root;
  root = RootForDAG(query, cs);

  std::vector<std::vector<Vertex>> DAG;
  DAG.resize(query.GetNumVertices());
  BuildDAG(query, cs, root, DAG);
/*
  while (fin >> type) {
    if (type == 'c') {
      Vertex id;
      size_t candidate_set_size;

      fin >> id >> candidate_set_size;

      cs_[id].resize(candidate_set_size);

      for (size_t i = 0; i < candidate_set_size; ++i) {
        Vertex data_vertex;
        fin >> data_vertex;
        cs_[id][i] = data_vertex;
      }
    }
  }
  */
}

Vertex Backtrack::RootForDAG(const Graph &query, const CandidateSet &cs) {
  Vertex rroot = 0;
  int compare = cs.GetCandidateSize(0)/query.GetDegree(0);
  for (rroot = 1; rroot<query.GetNumVertices(); rroot++) {
    compare = (cs.GetCandidateSize(rroot)/query.GetDegree(rroot) < compare)? cs.GetCandidateSize(rroot)/query.GetDegree(rroot):compare;
  }
  return rroot;
}

void Backtrack::BuildDAG(const Graph &query, const CandidateSet &cs, Vertex root, const DAG &DAG) {
  int visit[query.GetNumVertices()];
  std::queue<int> q;
  q.push(root);
  visit[root] = true;

  while(!q.empty()) {
    int x = q.front();
    q.pop();

    for ( int i = query.GetNeighborStartOffset(x); i <= query.GetNeighborEndOffset(x); i++ ) {
      int y = query.GetNeighbor(i);
      if (!visit) {
        q.push(y);
        visit[y] = true;
        DAG[x].push_back(y);
      }
    }
  }
}
