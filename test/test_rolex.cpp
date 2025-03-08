// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/openmethod.hpp>
#include <boost/openmethod/compiler.hpp>

struct Role {
    virtual ~Role() {
    }
};

struct Employee : Role {
    virtual double pay();
};

struct Manager : Employee {
    virtual double pay();
};

struct Founder : Role {};

struct Expense {
    virtual ~Expense() {
    }
};

struct Public : Expense {};
struct Bus : Public {};
struct Metro : Public {};
struct Taxi : Expense {};
struct Jet : Expense {};

BOOST_OPENMETHOD_CLASSES(
    Role, Employee, Manager, Founder, Expense, Public, Bus, Metro, Taxi, Jet);

//static_assert(!virtual_ptr<Role>::IsSmartPtr);
BOOST_OPENMETHOD(pay, (virtual_ptr<Employee>), double);
BOOST_OPENMETHOD(
    approve, (virtual_ptr<const Role>, virtual_ptr<const Expense>, double),
    bool);

BOOST_OPENMETHOD_OVERRIDE(pay, (virtual_ptr<Employee>), double) {
    return 3000;
}

BOOST_OPENMETHOD_OVERRIDE(pay, (virtual_ptr<Manager> exec), double) {
    return next(exec) + 2000;
}

BOOST_OPENMETHOD_OVERRIDE(
    approve,
    (virtual_ptr<const Role> r, virtual_ptr<const Expense> e, double amount),
    bool) {
    return false;
}

BOOST_OPENMETHOD_OVERRIDE(
    approve,
    (virtual_ptr<const Employee> r, virtual_ptr<const Public> e, double amount),
    bool) {
    return true;
}

BOOST_OPENMETHOD_OVERRIDE(
    approve,
    (virtual_ptr<const Manager> r, virtual_ptr<const Taxi> e, double amount),
    bool) {
    return true;
}

BOOST_OPENMETHOD_OVERRIDE(
    approve,
    (virtual_ptr<const Founder> r, virtual_ptr<const Expense> e, double amount),
    bool) {
    return true;
}

int main() {
    boost::openmethod::initialize();
}

double call_pay(Employee& emp) {
    return pay(emp);
}

double Employee::pay() {
    return 3000;
}

double Manager::pay() {
    return Employee::pay() + 2000;
}

double call_pay_vfunc(Employee& emp) {
    return emp.pay();
}

bool call_approve(const Role& r, const Expense& e, double a) {
    return approve(r, e, a);
}
