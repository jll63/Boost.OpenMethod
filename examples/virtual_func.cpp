#include <iostream>
#include <memory>

struct Animal {
    virtual ~Animal() = default;
    virtual void poke(std::ostream&) = 0;
};

struct Cat : Animal {
    void poke(std::ostream& os) override {
        os << "hiss";
    }
};

struct Dog : Animal {
    void poke(std::ostream& os) override {
        os << "bark";
    }
};

int main() {
    std::unique_ptr<Animal> a(new Cat);
    std::unique_ptr<Animal> b(new Dog);

    a->poke(std::cout); // prints "hiss"
    std::cout << "\n";

    a->poke(std::cout); // prints "bark"
    std::cout << "\n";

    return 0;
}
