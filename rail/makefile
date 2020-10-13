
.phony: core example clean
.default: all

#change the following paths, if necessary
ROOT=d:/trunk
APP=d:/rail

all: core example

core:
	g++ -m32 -I${ROOT}/include -L${ROOT}/bin -g -o a.out entry.cpp -lae
	gcc -m32 -I${ROOT}/include -L${ROOT}/bin -g -shared -o f.dll f.c -lae -lopengl32

example: circle.ae
	mkdir -p ./example
	cp geom.txt ./example/
	cp circle.ae ./example/
	cd example && ${ROOT}/bin/ae -DROUND_1 circle.ae
	cd ${ROOT}/bin && ${APP}/a.out -G geom.txt -Ql -o ${APP}/example/rail.agl -p ${APP}/example/circle.txt && ${APP}/a.out -G geom.txt -c -o ${APP}/example/center.txt -p ${APP}/example/circle.txt

clean:
	rm -fr example a.out f.dll
