# all: build/robin_hood build/stl_map build/glib_hash_table build/stl_unordered_map build/boost_unordered_map build/google_sparse_hash_map build/google_dense_hash_map build/qt_qhash build/python_dict build/ruby_hash

all: build/robin_hood build/stl_map build/stl_unordered_map build/google_sparse_hash_map build/google_dense_hash_map build/python_dict build/custom build/sparsepp

# build/glib_hash_table: src/glib_hash_table.c src/template.c
# 	gcc -ggdb -O2 -lm `pkg-config --cflags --libs glib-2.0` src/glib_hash_table.c -o build/glib_hash_table

build/stl_unordered_map: src/stl_unordered_map.cc src/template.c
	g++ -O2 -lm src/stl_unordered_map.cc -o build/stl_unordered_map -std=c++0x

build/stl_map: src/stl_map.cc src/template.c
	g++ -O2 -lm src/stl_map.cc -o build/stl_map -std=c++0x

# build/boost_unordered_map: src/boost_unordered_map.cc src/template.c
# 	g++ -O2 -lm src/boost_unordered_map.cc -o build/boost_unordered_map

vendor/sparsehash/src/sparsehash/internal/sparseconfig.h:
	cd vendor/sparsehash && \
	./configure && \
	make

build/google_sparse_hash_map: vendor/sparsehash/src/sparsehash/internal/sparseconfig.h src/google_sparse_hash_map.cc src/template.c
	g++ -O2 -lm -I vendor/sparsehash/src src/google_sparse_hash_map.cc -o build/google_sparse_hash_map

build/google_dense_hash_map: vendor/sparsehash/src/sparsehash/internal/sparseconfig.h src/google_dense_hash_map.cc src/template.c
	g++ -O2 -lm -I vendor/sparsehash/src src/google_dense_hash_map.cc -o build/google_dense_hash_map

build/sparsepp: src/sparsepp.cc src/template.c
	g++ -O2 -lm -I vendor/sparsepp src/sparsepp.cc -o build/sparsepp

# build/qt_qhash: src/qt_qhash.cc src/template.c
# 	g++ -O2 -lm `pkg-config --cflags --libs QtCore` src/qt_qhash.cc -o build/qt_qhash

build/python_dict: src/python_dict.c src/template.c
	gcc -O2 -lm -Ienv/include/python2.7 -lpython2.7 src/python_dict.c -o build/python_dict

build/ruby_hash: src/ruby_hash.c src/template.c
	gcc -O2 -lm -framework Ruby src/ruby_hash.c -o build/ruby_hash

build/robin_hood: src/robin_hood.cc src/template.c
	g++ -O2 -lm src/robin_hood.cc -o build/robin_hood -std=c++0x

build/custom: src/my_robin_hood.cc src/template.cpp
	g++ -O2 -lm -std=c++11 -Ivendor/benchmark/include -Lvendor/benchmark/src -lbenchmark src/my_robin_hood.cc -o build/custom

bench:
	python -u bench.py
	cat build/*.csv | python make_chart_data.py | python make_html.py > build/bench.html

.PHONY: clean
clean:
	rm build/*
