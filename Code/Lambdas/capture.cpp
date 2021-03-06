#include <iostream>

struct A {

    int a;
    int *p;

    A(int _a) :a(_a), p(&a) {}

    void operator()() {
        auto f = [=]() {int a=0; a++; std::cout << a << std::endl;};
        f();
    }

    void alternate() { // !! WARNING: only this is copied by value
        auto f = [=]() {a++; std::cout << a << std::endl;};
        f();
    }

    void alternate2() { // This is the same as alternate
        auto f = [=]() {this->a++; std::cout << this->a << std::endl;};
        f();
    }

    void safe_version() {
        int a = 5;
        auto x = [=] () {
            std::cout << this->a << std::endl;
            std::cout << a << std::endl;
        };
        a = 3;
        x();
    }

    void out() const {
        std::cout << a << "\n";
    }
};


int main() {

    A a(10);

    a();

    a.out();

    a.alternate();
    a.alternate2();

    a.out();

    a.safe_version();
    a.out();
    return 0;
}
