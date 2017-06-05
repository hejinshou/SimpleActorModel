#include "libtask.hpp"

void main() {
	LockPtr<std::vector<int>> a;
	(*a)->push_back(1);
}