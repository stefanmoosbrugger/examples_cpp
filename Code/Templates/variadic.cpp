#include <tuple>
#include <vector>
#include "../show.h"

template <typename ...T>
struct A {
    static constexpr int count = sizeof...(T);
};

template <typename ...T>
struct B {
    using tuple = std::tuple<T...>;
    using tuple_of_vector = std::tuple<std::vector<T>...>;

    tuple _data;

    B(T... args) : _data(args...) {}
};

void pretty_print(std::ostream& s) {
    s <<"\n";
}

template <typename T, typename ...Ts>
void pretty_print(std::ostream& s, T first, Ts ...values) {
    s << " {" << first << "} ";
    pretty_print(s, values...);
}


int main() {
    SHOW((A<int, float>::count));

    using theB = B<int, char, float>;
    static_assert(std::is_same<std::tuple_element<0, theB::tuple_of_vector>::type, std::vector<int>>::value, "");
    static_assert(std::is_same<std::tuple_element<1, theB::tuple_of_vector>::type, std::vector<char>>::value, "");

    theB x(200,'A',3.14);
    SHOW((std::get<0>(x._data)));
    SHOW((std::get<1>(x._data)));
    SHOW((std::get<2>(x._data)));

    pretty_print(std::cout, 3.2, "hello", 42, "world");
}
