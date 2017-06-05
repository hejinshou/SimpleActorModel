#include "libtask.hpp"

void main() {
	LockPtr<std::vector<int>> a;
	auto i(*a);
	i->push_back(1);
}