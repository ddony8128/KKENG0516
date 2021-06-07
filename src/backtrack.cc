/**
 * @file backtrack.cc
 *
 */

#include "backtrack.h"
#include "data_structure.h"
#include "stdlib.h"
#include <algorithm>
#include <vector>
#include <queue>
#include <stack>


#define MAXINT 100000000

using namespace std;

SearchStack *search_stack;
size_t len_query;
ActivatedBitArray *activated_bit_arr;
BoundaryCountArray *boundary_count_arr;
IncomingNumberArray *incoming_number_arr;
size_t activated_count;
SelectedArray *selected_arr;
AvoidVector *avoid_vector;
bool finish_flag;
vector<vector<Vertex>> DAG;
vector<vector<Vertex>> RDAG;

Backtrack::Backtrack() {}
Backtrack::~Backtrack() {}

void Backtrack::PrintAllMatches(const Graph &data, const Graph &query,
                                const CandidateSet &cs) {
                              
  search_stack = new SearchStack();
  len_query = query.GetNumVertices();
  activated_bit_arr = new ActivatedBitArray(len_query);
  boundary_count_arr = new BoundaryCountArray(len_query);
  incoming_number_arr = new IncomingNumberArray(len_query);
  activated_count = 0;
  selected_arr = new SelectedArray(len_query, data);
  avoid_vector = new AvoidVector();
  finish_flag = false; 

                                
  Vertex root;
  root = RootForDAG(query, cs);


  DAG.resize(query.GetNumVertices());
  RDAG.resize(query.GetNumVertices());
  BuildDAG(query, cs, root, DAG);
  BuildReverseDAG(DAG, RDAG);


  while (1) {
    

    if (activated_count == len_query) {
      selected_arr->print_result();
      finish_flag = backtrack_3();
      if (finish_flag) {
        break;
      } else {
        continue;
      }
    }

    Vertex temp_smallest_extendable;
    vector<Vertex>* temp_smallest_cs;
    int temp_smallest_cs_num = MAXINT;
    vector<Vertex> *temp_cs = new vector<Vertex>();
    
    vector<Vertex> extendable_vertex;
    for (int k = 0; k < len_query; k++) {
      if( ( !activated_bit_arr->isActivated(k) ) && boundary_count_arr->getBoundaryCount(k) == incoming_number_arr->getIncomingNumber(k)) {

        
        vector<Vertex> illuminated_cs;

        for (int i = 0; i < cs.GetCandidateSize(k); i++) {
          temp_cs->emplace_back(cs.GetCandidate(k, i));
        }
        
        for (int j = temp_cs->size()-1; j>= 0; j--) {
              if (selected_arr->isSelected( (*temp_cs)[j]) || avoid_vector->isAvoided((*temp_cs)[j])) {
                illuminated_cs.emplace_back((*temp_cs)[j]);
                temp_cs->erase(temp_cs->begin() + j-1);
              }
        }


        if (temp_cs->size() == 0) {
          finish_flag = backtrack_1(data, query, cs, illuminated_cs, k);
          if (finish_flag) {
            break;
          } else {
            continue;
          }
        }

        for (int i = 0; i<RDAG[k].size() ; i++){
          Vertex parent = RDAG[k][i];
          Vertex parent_cs = selected_arr->getSelectedOf(parent);
          for (int j = temp_cs->size()-1; j>=0; j--){
            if (!data.IsNeighbor(parent_cs, (*temp_cs)[j])){
              temp_cs->erase(temp_cs->begin() + j-1);
            }
          }
        }

        if (temp_cs->size() == 0) {
          finish_flag = backtrack_2(data, query, cs, illuminated_cs, k);
          if (finish_flag) {
            break;
          } else {
            continue;
          }
        }

        if (temp_cs->size() < temp_smallest_cs_num) {
          temp_smallest_extendable = k;
          temp_smallest_cs_num = temp_cs->size();
          delete temp_smallest_cs;
          temp_smallest_cs = temp_cs;
//          temp_smallest_cs.clean();
  //        for (int i = 0; i < cs.GetCandidateSize(k); i++) {
 //           temp_smallest_cs.push_back(cs.GetCandidate(k, i));
 //         }
        } else{
          delete temp_cs;
        }

      }
    }

    sort(temp_smallest_cs->begin(), temp_smallest_cs->end());
    vector<Vertex> next_cs;
    next_cs.assign(temp_cs->begin()+1, temp_cs->end());
    search_stack->push(SearchStackElement(temp_smallest_extendable, (*temp_smallest_cs)[0], next_cs));
    activated_bit_arr->activate(temp_smallest_extendable);
    boundary_count_arr->activate(DAG, temp_smallest_extendable);
    selected_arr->select(temp_smallest_extendable, (*temp_smallest_cs)[0]);

  }



}




Vertex Backtrack::RootForDAG(const Graph &query, const CandidateSet &cs) {
  Vertex rroot = 0;
  int compare = cs.GetCandidateSize(0)/query.GetDegree(0);
  for (int i = 1; i<query.GetNumVertices(); i++) {
    rroot = (cs.GetCandidateSize(rroot)/query.GetDegree(rroot) < compare)? i:rroot;
    compare = (cs.GetCandidateSize(rroot)/query.GetDegree(rroot) < compare)? cs.GetCandidateSize(rroot)/query.GetDegree(rroot):compare;

  }
  return rroot;
}



void Backtrack::BuildDAG(const Graph &query, const CandidateSet &cs, Vertex root, vector<vector<Vertex>> &DAG) {
  int visit[query.GetNumVertices()];
  queue<int> q;
  q.push(root);
  visit[root] = true;

  while(!q.empty()) {
    int x = q.front();
    q.pop();

    for ( int i = query.GetNeighborStartOffset(x); i <= query.GetNeighborEndOffset(x); i++ ) {
      int y = query.GetNeighbor(i);
      if (!visit[y]) {
        q.push(y);
        visit[y] = true;
        incoming_number_arr->count(y);
        DAG[x].emplace_back(y);
      }
    }

  }
}


void Backtrack::BuildReverseDAG(vector<vector<Vertex>> &DAG, vector<vector<Vertex>> &RDAG){
  for (int i = 0; i<DAG.size();i++){
    for (int j = 0; j<DAG[i].size();j++){
      // Vertex child = DAG[i][j];
      RDAG[j].emplace_back(i);
    }
  }
}


bool Backtrack::backtrack_1(const Graph &data, const Graph &query,
                                const CandidateSet &cs, vector<Vertex> &illuminated_cs, Vertex k) {
  
  SearchStackElement temp;
  bool is_in_illuminated_cs = false;

  while (!is_in_illuminated_cs) {
    if (search_stack->isEmpty()) {
      return true;
    }

    temp = search_stack->pop();  
    
    Vertex temp_selected_cs = temp.getSelectedCS();
    for (int i = 0; i < illuminated_cs.size(); i++) {
      if (temp_selected_cs == illuminated_cs[i]) {
        is_in_illuminated_cs = true;
        i = illuminated_cs.size();
      }
    }

    if (!is_in_illuminated_cs){
      inactivate(temp.getQueryNode());
    }

  }

  
  
  AvoidVectorElement avoid_vector_element(temp.getSelectedCS(), temp.getQueryNode(), k);
  avoid_vector->add(avoid_vector_element);

  while (true){

    while (!temp.isEmptyNextCS()) {
        Vertex temp_next = temp.popNextCS();
        if (!avoid_vector->isAvoided(temp_next)) {
          temp.setSelectedCS(temp_next);
          search_stack->push(temp);
          return false;
        } 
      }

    inactivate(temp.getQueryNode());
    
    if (search_stack->isEmpty()) {
      return true;
    } 
    
    temp = search_stack->pop();
    
  }
  
  return false;

}


bool Backtrack::backtrack_2(const Graph &data, const Graph &query,
                                const CandidateSet &cs, vector<Vertex> &illuminated_cs, Vertex k) {

  SearchStackElement temp;
  bool is_in_illuminated;

  bool con = true;
  while (con) {
    if (search_stack->isEmpty()) {
      return true;
    }
  
    temp = search_stack->pop();
    is_in_illuminated = false;

    Vertex temp_selected_cs = temp.getSelectedCS();
    for (int i = 0; i < illuminated_cs.size(); i++) {
      if (temp_selected_cs == illuminated_cs[i]) {
        is_in_illuminated = true;
        i = illuminated_cs.size();
      }
    }


    if (is_in_illuminated || query.IsNeighbor(temp.getQueryNode(), k)){
      con = false;
    } else{
      inactivate(temp.getQueryNode());
    } 
      
  } 


  while(true){

     while (!temp.isEmptyNextCS()) {
      Vertex temp_next = temp.popNextCS();
      if (!avoid_vector->isAvoided(temp_next)) {
        temp.setSelectedCS(temp_next);
        search_stack->push(temp);
        return false;
      } 
    }
      
    inactivate(temp.getQueryNode());

    if (search_stack->isEmpty()) {
      return true;
    }

    temp = search_stack->pop();
  }
  
  return false;

}


bool Backtrack::backtrack_3(){

  SearchStackElement temp;


  while (true) {
    if (search_stack->isEmpty()) {
      return true;
    }
  
    temp = search_stack->pop();

    while (!temp.isEmptyNextCS()) {
      Vertex temp_next = temp.popNextCS();
      if (!avoid_vector->isAvoided(temp_next)) {
        temp.setSelectedCS(temp_next);
        search_stack->push(temp);
        return false;
      } 
    }
    inactivate(temp.getQueryNode());

  }  
  return false;
  
}


void Backtrack::inactivate(Vertex denied){
  avoid_vector->remove(denied);
  activated_bit_arr->inactivate(denied);
  boundary_count_arr->inactivate(DAG, denied);
  selected_arr->unselect(denied);
}
