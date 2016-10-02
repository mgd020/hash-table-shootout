all: build/robin_hood build/stl_map build/glib_hash_table build/stl_unordered_map build/boost_unordered_map build/google_sparse_hash_map build/google_dense_hash_map build/qt_qhash build/python_dict build/ruby_hash

build/glib_hash_table: src/glib_hash_table.c Makefile src/template.c
	gcc -ggdb -O2 -lm `pkg-config --cflags --libs glib-2.0` src/glib_hash_table.c -o build/glib_hash_table

build/stl_unordered_map: src/stl_unordered_map.cc Makefile src/template.c
	g++ -O2 -lm src/stl_unordered_map.cc -o build/stl_unordered_map -std=c++0x

build/stl_map: src/stl_map.cc Makefile src/template.c
	g++ -O2 -lm src/stl_map.cc -o build/stl_map -std=c++0x

build/boost_unordered_map: src/boost_unordered_map.cc Makefile src/template.c
	g++ -O2 -lm src/boost_unordered_map.cc -o build/boost_unordered_map

vendor/sparsehash/src/sparsehash/internal/sparseconfig.h:
	cd vendor/sparsehash && \
	./configure && \
	make

google_sparsehash: vendor/sparsehash/src/sparsehash/internal/sparseconfig.h

build/google_sparse_hash_map: google_sparsehash src/google_sparse_hash_map.cc Makefile src/template.c
	g++ -O2 -lm -I vendor/sparsehash/src src/google_sparse_hash_map.cc -o build/google_sparse_hash_map

build/google_dense_hash_map: google_sparsehash src/google_dense_hash_map.cc Makefile src/template.c
	g++ -O2 -lm -I vendor/sparsehash/src src/google_dense_hash_map.cc -o build/google_dense_hash_map

build/qt_qhash: src/qt_qhash.cc Makefile src/template.c
	g++ -O2 -lm `pkg-config --cflags --libs QtCore` src/qt_qhash.cc -o build/qt_qhash

build/python_dict: src/python_dict.c Makefile src/template.c
	gcc -O2 -lm -I/usr/include/python2.7 -lpython2.7 src/python_dict.c -o build/python_dict

build/ruby_hash: src/ruby_hash.c Makefile src/template.c
	gcc -O2 -lm -I/usr/include/ruby-1.9.0 -I /usr/include/ruby-1.9.0/x86_64-linux -lruby1.9 src/ruby_hash.c -o build/ruby_hash

build/robin_hood: src/robin_hood.cc Makefile src/template.c
	g++ -O2 -lm src/robin_hood.cc -o build/robin_hood -std=c++0x

build/custom: src/custom.cc Makefile src/template.c
	g++ -O2 -lm -std=c++11 src/custom.cc -o build/custom

bench:
	python -u bench.py
	cat build/*.csv | python make_chart_data.py | python make_html.py > build/bench.html

.PHONY: clean
clean:
	rm build/*
