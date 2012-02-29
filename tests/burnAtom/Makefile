all : burnAtom test
.S:
	gcc -s -nostdlib -m32 -o $@ $<
burnAtom: burnAtom.S
.c:
	gcc -m32 -static -o $@ $<
test: test.c
