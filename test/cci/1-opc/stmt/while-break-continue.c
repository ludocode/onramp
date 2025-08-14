// The MIT License (MIT)
// Copyright (c) 2025 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// A bug in cci/2: break or continue as unbraced if statement caused else to
// not be recognized. break and continue must consume the trailing semicolon.
int main(void) {
	while (0) {
		if (1)
			break;
		else
			break;
		if (1)
			continue;
		else
			continue;
	}
}
