/*
 * CompileFail.cpp
 *
 *  Created on: Mar 8, 2020
 *      Author: cf
 *
 * Used for compilation failure testing.
 */

#include <type_traits>

template<typename T>
class Test {

	static_assert(std::is_nothrow_move_assignable_v<T> && std::is_nothrow_move_constructible_v<T>);

};

class NoThrowMove {
public:
	NoThrowMove() = default;
	NoThrowMove(NoThrowMove&&) = default;
	NoThrowMove& operator=(NoThrowMove&&) = default;
};

Test<NoThrowMove> shouldNotCompile;

int main() {
	return 0;
}
