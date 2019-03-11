#include<stdio.h>
#include<iostream>
#include<fstream>
#include<vector>
#include<string>
#include<ctime>

#include "gist.h"
#include "gist_extensions.h"
#include "gist_defs.h"
#include "gist_cursor.h"
#include "gist_cursorext.h"
#include "gist_btree.h"
#include "gist_rtree.h"
#include "gist_rtpred_point.h"
#include "gist_rstartree.h"
#include "gist_nptree.h"
#include "gist_rrtree.h"
#include "gist_sptree.h"
#include "gist_sstree.h"
#include "gist_srtree.h"
#include "gist_rtreecext.h"

using namespace std;

const char INDEXFILENAME[] = "srtree-index";
static int k = 10000;
static int io = 100;

static rc_t ReadDataFile(const char * filename, vector<string> &keys, vector<string> &datas){
  ifstream fin(filename);
  int number;
  string x, y;
  string data;
  fin >> number;
  for(int i = 0; i < number; i++){
    fin >> x >> y >> data;
    keys.push_back(x + " " + y);
    datas.push_back(data);
  }
  fin.close();
  return 0;
}

static rc_t ReadQueryFile(const char * filename, vector<string> &queries){
  ifstream fin(filename);
  string s;
  while(getline(fin, s)){
    queries.push_back(s);
  }
  fin.close();
  return 0;
}

static void printDatum(void* key, int klen, void* data, int dlen, gist_ext_t* ext){
    vec_t keyv(key, klen);
    vec_t datav(data, dlen);
    ext->printPred(cout, keyv, 1);
    cout << ": ";
    ext->printData(cout, datav);
}

static rc_t runQuery(gist& index, const gist_query_t* query, int k, int io, int& numRetrieved) {
  /* param:
      gist&  index,
      const gist_query_t* query, // in: query to run
      int k, // in: max. number of tuples to retrieve
      int io, // in: max. number of page accesses
      int& numRetrieved) // out: number of retrieved tuples
  */
    gist_cursor_t cursor;
    if (index.fetch_init(cursor, query, k, io) != RCOK) {
    	cout << "can't initialize cursor" << endl;
    	return(eERROR);
    }
    gist_ext_t* ext = index.extension();

    bool eof = false;
    char key[gist_p::max_tup_sz];
    smsize_t klen;
    char data[gist_p::max_tup_sz];
    smsize_t dlen;
    int cnt = 0;
    for (;;) {
    	klen = gist_p::max_tup_sz;
    	dlen = gist_p::max_tup_sz;
    	if (index.fetch(cursor, (void *) key, klen, (void *) data, dlen, eof) != RCOK) {
    	    cerr << "Can't fetch from cursor" << endl;
    	    return(eERROR);
    	}
    	if (eof) break;
    	// print key and data
      printDatum(key, klen, data, dlen, ext);
      cout << endl;
    	cnt++;
    }
    numRetrieved = cnt;
    return(RCOK);
}

static rc_t Insert(gist& index, const char* kstr, const char* dstr){
  gist_ext_t* ext = index.extension();

  char key[gist_p::max_tup_sz];
  int klen;
  char data[gist_p::max_tup_sz];
  int dlen;
  if (ext->parsePred(kstr, key, klen) != RCOK) {
    cout << "can't parse key" << endl;
    return 1;
  }
  if (ext->parseData(dstr, data, dlen) != RCOK) {
    cout << "can't parse data" << endl;
    return 1;
  }

	// printDatum(key, klen, data, dlen, ext);
	// cout << endl;

  if (index.insert(key, klen, data, dlen) != RCOK) {
    cout << "can't insert" << endl;
    return 1;
  }
}

static rc_t Select(gist& index, const char* qstr, int k, int io, int& numRetrieved){
  gist_ext_t* ext = index.extension();
  gist_query_t* query;
  if (ext->parseQuery(qstr, query) != RCOK) {
      cerr << "Error parsing qualification" << endl;
      return 1;
  }

  if (runQuery(index, query, k, io, numRetrieved) != RCOK) {
      return 1;
  }
  cout << "[INFO] Retrieved " << numRetrieved << " items" << endl;
}

static rc_t Remove(gist& index, const char* qstr){
  gist_ext_t* ext = index.extension();
  gist_query_t* query;
  if (ext->parseQuery(qstr, query) != RCOK) {
      cout << "Error parsing qualification" << endl;
      return 1;
  }
  if (index.remove(query) != RCOK) {
    cerr << "Can't remove " << qstr << endl;
    return 1;
  }
}

static rc_t Update(gist& index, const char* kstr, const char* newdstr){
    gist_ext_t* ext = index.extension();

    string prefix = "= p ";
    string q(kstr);
    q = prefix + q;
    const char* qstr = q.c_str();

    int numRetrieved = 2;
    Select(index, qstr, k, io, numRetrieved);
    if (numRetrieved > 1){
      cerr << "more than 1 item is selected!" << endl;
      return 1;
    }
    else if(numRetrieved == 0){
      cerr << "key does not exist!" << endl;
      return 1;
    }
    else{
      Remove(index, qstr);
      Insert(index, kstr, newdstr);
    }
}


int main(){
  cout << "Test libgist for SR-Tree" << endl;
  gist index;

  ifstream indexfile(INDEXFILENAME);
  if(!indexfile.good()){
    cout << "Create a new index file" << endl;
    rc_t status = index.create(INDEXFILENAME, &sr_point_ext);
    if (status != RCOK) {
      cerr << "Error creating " << INDEXFILENAME << endl;
      return 1;
    }

    // insert
    cout << "\n\n**************** Insert Data ****************" << endl;
    vector<string> keys, datas;
    ReadDataFile("data/data.txt", keys, datas);
    clock_t start, end;
    start = clock();
    for(int i = 0; i < keys.size(); i++){
      Insert(index, keys[i].c_str(), datas[i].c_str());
    }
    end = clock();
    cout << "[INFO] Insert Use time: " << ((double)(end - start)) / CLOCKS_PER_SEC << endl;
    ofstream fout("dump/insert.dump");
    rc_t content = index.dump(fout, 0);
    fout.close();
  }
  else{
    cout << "Open an existing index file" << endl;
    rc_t status = index.open(INDEXFILENAME);
    if (status != RCOK) {
      cerr << "Error Opening "<< INDEXFILENAME << endl;
      return 1;
    }
  }

  // select queries

  vector<string> queries;
  ReadQueryFile("data/query.txt", queries);

  cout << "\n\n**************** Select Query ****************" << endl;
  int numRetrieved = 0;
  for(int i = 0; i < queries.size(); i++){
    cout << "[INFO] The query is " << queries[i] << endl;
    clock_t start, end;
    start = clock();
    if(queries[i].compare(0, 1, "~") == 0){
      Select(index, queries[i].c_str(), 20, io, numRetrieved);
    }
    else{
      Select(index, queries[i].c_str(), k, io, numRetrieved);
    }
    end = clock();
    cout << "[INFO] Select Use time: " << ((double)(end - start)) / CLOCKS_PER_SEC << endl;
    cout << "==========================================" << endl;
  }


  // delete: "delete query"
  cout << "\n\n**************** Delete Data ****************" << endl;
  ofstream fout("dump/delete.dump");
  for(int i = 0; i < queries.size(); i++){
    if(queries[i].compare(0, 1, "~") == 0){
      continue;
    }
    cout << "[INFO] The query is " << queries[i] << endl;
    clock_t start, end;
    start = clock();
    Remove(index, queries[i].c_str());
    end = clock();
    cout << "[INFO] Delete Use time: " << ((double)(end - start)) / CLOCKS_PER_SEC << endl;
    cout << "==========================================" << endl;

    fout << "[INFO] The query is " << queries[i] << endl;
    rc_t content = index.dump(fout, 0);
    fout << "==========================================" << endl;
    fout << "==========================================" << endl;
  }
  fout.close();

  // update
  cout << "\n\n****************Update Query****************" << endl;
  vector<string> qkeys, newdatas;
  ReadDataFile("data/update.txt", qkeys, newdatas);
  clock_t start, end;
  fout.open("dump/update.dump");
  for(int i = 0; i < qkeys.size(); i++){
    cout << "[INFO] The (key, data) is (" << qkeys[i] << ":" << newdatas[i] << ")" << endl;
    start = clock();
    Update(index, qkeys[i].c_str(), newdatas[i].c_str());
    end = clock();
    cout << "[INFO] Update Use time: " << ((double)(end - start)) / CLOCKS_PER_SEC << endl;
    cout << "==========================================" << endl;

    fout << "[INFO] The (key, data) is (" << qkeys[i] << ":" << newdatas[i] << ")" << endl;
    rc_t content = index.dump(fout, 0);
    fout << "==========================================" << endl;
    fout << "==========================================" << endl;
  }
  fout.close();

  index.close();
  cout << "****************Finish****************" << endl;

  return 0;
}
