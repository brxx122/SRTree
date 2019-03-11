# SR-Tree Demo

The GiST Indexing Project is built by University of California, Berkeley. The home page can be found at http://gist.cs.berkeley.edu/. 
We implement a simple demo using the existing libgist extensions $sr\_point\_ext​$ for SR-Tree.
This demo supports the standard  insertion, deletion, query and update of keys.  We design several experiments to analyze its performance.



## Organization

```
SRTree
├── data            # data
│   ├── data.txt      # data to build index
│   ├── query.txt     # query to select and delete
│   ├── update.txt    # new data to update existing index
│   └── createData.py # script to generate random data
├── dump            # page dump archive
│   ├── insert.dump
│   ├── delete.dump
│   └── update.dump
├── log             # log
│   ├── insert.log
│   ├── select.log
│   ├── delete.log
│   └── update.log
├── SRTree.cc       # main
├── Makefile
└── README.md
```



## Environment

- System: Ubuntu 16.04
- Development Environment:
  - Python 2.7
  - GNU C++ (g++) 5.4.0
- Requirements
  - libgist 2.0
  - GNU make


## Installation Guides
Download libgist and use the default *src/Makefile.inc* configuration ($AMDB = false$).
Check g++ paths in  *src/Makefile.paths.Linux.x86_64*.
For more details, please refer to *amdb section* in Gist Home Page.

```shell
# make libgist and the pre-packaged extensions
$ cd libgist/src
$ gmake cmdline

# test the installation
$ cd ../tests
$ ./runtests
```


## Compile

```shell
$ export LIBGISTHOME=the/absolute/path/of/libgist
$ cd SRTree
$ make
$ ./SRTree
```



## Experiment Design
There are 1 experiment for insertion, 10 queries, 9 for deletion and 4 for update mechanisms.

### Insert
Generate 10,000 (key, data) tuples randomly for insertion.

- key: double array of length 2, which indicates 2-dimensional points. eg. {40.312, 50.452}
  - range in [-100, 100]
- data: int. e.g. 74
  - range in [-1000, 1000]


#### Input File Format
The default name of data file is "data.txt".
The integer in first line indicates the total number of data.
Each line is composed of key and data, which divided by "\t".


#### Output Dump Page
The default name of output file is "insert.dump".
It records the content of the whole index tree after insertion.



### Select
Design 10 queries for 5 spatial query operators.

1. **equality with point (p)**: Make the query key existent in index file, so the query will *hit*.
2. **equality with point (p)**: Make the query key non-existent in index file, so the query will *miss*.
3. **overlap with point (p)**: Make the query key existent in index file. Since only the equal points can overlap each other, the query will *hit*.
4. **overlap with point (p)**: Make the query key non-existent in index file, so the query will *miss*.
5. **overlap with rectangle (r)**: This query will select all points in the rectangle.
6. **key contains index with point (p)**: Make the query key existent in index file. Since only the equal points can contain each other, the query will *hit*.
7. **key contains index with rectangle (r)**: This query will select all points in the rectangle.
8. **index contains key with point (p)**: Make the query key non-existent in index file. Since only the equal points can contain each other, the query will *miss*.
9. **index contains key with rectangle (r)**: Since points can never contain a rectangle, this query will always *miss*.
10. **nearest-neighbor with point (p)**: This query will select points in a ascending order of the distance between the query key and index key. The retrieved number depends on the third parameter  $k$ defined in $gist::fetch\_init(cursor, query, k, io)$, which limits the maximum number of tuples to retrieve. In the experiment $k$ is set to 20 for this query.


#### Input File Format
The default name of data file is "query.txt"
Each line is composed of operators (= & < > ~), key type (p r) and key data.
All of them are divided by a single space.



### Delete
Select 9 of the above queries for deletion, except the nearest-neighbor operators which may delete all index.
5 of them (1, 3, 5, 6, 7) will actually delete entries in index file and the remaining 4 queries will do nothing.
Among the deletion, the 1st, 3rd and 6th will only delete a single entry due to the retrieve while the 5th and 7th will delete several entries.


#### Output Dump Page
The default name of output file is "delete.dump".
It records the content of the whole index tree after each deletion, which is divided by the line of "====".



### Update
Design 4 queries for update.

1. Make the query key existent in index file and give a new value for it.
2. Make the query key non-existent in index file, so the query will *miss*.
3. Make the query key existent in the origin index file but non-existent after deletion, so the query will *miss*.
4. Make the query key existent in index file and use the same value.

In order to update data, a query is first executed to check if the entry exists. If the query key does not exists or more than one items are retrieved, an error will be thrown out.
The single retrieved entry will be first deleted, and then a new entry with the same key and a new value will be inserted into the index file.


#### Input File Format
The default name of data file is "update.txt"
The integer in first line indicates the total number of data.
Each line is composed of key and data, which divided by "\t".


#### Output Dump Page
The default name of output file is "insert.dump".
It records the content of the whole index tree after each update, which is divided by the line of "====".
