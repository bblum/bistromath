all: bbvisualizer rotvisualizer bitshift rowtablegen coltablegen rot45gen rot315gen

bbvisualizer: bbvisualizer.c
	gcc bbvisualizer.c -o bbvisualizer

rotvisualizer: rotvisualizer.c
	gcc rotvisualizer.c -o rotvisualizer

bitshift: bitshift.c
	gcc bitshift.c -o bitshift

rowtablegen: rowtablegen.c
	gcc rowtablegen.c -o rowtablegen

coltablegen: coltablegen.c
	gcc coltablegen.c -o coltablegen

rot45gen: rot45gen.c
	gcc rot45gen.c -o rot45gen

rot315gen: rot315gen.c
	gcc rot315gen.c -o rot315gen

clean:
	rm -f bbvisualizer bitshift rowtablegen coltablegen rot45gen rot315gen
