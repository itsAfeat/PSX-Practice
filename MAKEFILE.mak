# ----------------------------
# PlayStation 1 Psy-Q MAKEFILE
# ----------------------------
all:
	
	ccpsx -O2 -Xm -Xo$80010000 main.c -o main.cpe
	cpe2x /ce main.cpe
