#include "neo.h"

int main() {
	var i = 1;
	autostr s = mkstring("hello world %d", i);
	puts(s);
	autostr t = "don't free this";
}
