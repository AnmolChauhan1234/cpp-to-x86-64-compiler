.PHONY: clean setup build run docker-build docker-run docker-exec docker-clean setup-debug

clean:
	rm -rf build

setup: clean
	mkdir -p build
	cd build && cmake ..

setup-debug: clean
	mkdir -p debug
	cd debug && cmake -DCMAKE_BUILD_TYPE=Debug ..
	cmake --build debug/ 

build: 
	cmake --build build/

run: build
	./build/mycompiler input.txt
	./out

docker-run:
	docker-compose up -d

docker-exec: docker-run
	docker exec -it mycompiler_dev /bin/bash

docker-clean:
	docker-compose down -v --rmi all --remove-orphans
	
