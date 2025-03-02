// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/openmethod.hpp>
#include <boost/openmethod/compiler.hpp>

#include <iostream>
#include <memory>
#include <string>

using namespace boost::openmethod;
using std::cout;

struct Role {
    virtual ~Role() {
    }
};

struct Employee : Role {};

struct Manager : Employee {};

BOOST_OPENMETHOD_CLASSES(Role, Employee, Manager);

struct Payroll {
    int balance{10000};

    void pay(const Role& role) {
        pay_method::fn(this, role);
    }

  private:
    struct BOOST_OPENMETHOD_NAME(pay);
    using pay_method = method<
        BOOST_OPENMETHOD_NAME(pay)(Payroll*, virtual_<const Role&>), void>;

    void pay_employee(const Employee&) {
        balance -= 2000;
    }
    void pay_manager(const Manager& manager) {
        pay_method::next<&Payroll::pay_manager>(this, manager);
        balance -= 1000;
    }

  public:
    using pay_functions = Payroll::pay_method::override<
        &Payroll::pay_employee, &Payroll::pay_manager>;
};

BOOST_OPENMETHOD_REGISTER(Payroll::pay_functions);

#define BOOST_TEST_MODULE openmethod
#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_CASE(member_method) {
    initialize();

    Payroll pr;
    const Employee& alice = Employee();
    const Manager& bob = Manager();

    pr.pay(alice);
    BOOST_TEST(pr.balance == 8000);
    pr.pay(bob);
    BOOST_TEST(pr.balance == 5000);
}
