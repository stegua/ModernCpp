/**
* @fileoverview Copyright (c) 2017, by Stefano Gualandi, UniPv,
*               via Ferrata, 1, Pavia, Italy, 27100
*
* @author stefano.gualandi@gmail.com (Stefano Gualandi)
*
*/

#pragma once

#include <cstdio>
#include <vector>

using namespace std;

namespace INH { // Inheritance

class A {
 public:
   virtual void print() {
      fprintf(stdout, "Class A\n");
   }

   virtual double op(int value) {
      return pow(value, 1.01);
   }
};

class B : public A {
 public:
   void print() override {
      fprintf(stdout, "Class B\n");
   }

   double op(int value) override {
      return 0;
   }

};

bool testInheritance(size_t num) {
   vector<unique_ptr<A>> vec;
   //vector<A> vec;

   vec.emplace_back(make_unique<B>());
   for (size_t i = 0; i < num; i++)
      vec.emplace_back(make_unique<A>());

   for (auto& o : vec)
      o->print();

   return true;
}


class C {
 private:
   const int _type;

 public:
   explicit C(int t) : _type(t) {}

   void print() {
      if(_type)
         fprintf(stdout, "Class A\n");
      else
         fprintf(stdout, "Class B\n");
   }

   // TRY WITH:
   //INH::testSpeed1(70 000 000);
   //INH::testSpeed2(70 000 000);
   double op(int value) const {
      return _type * pow(value, 1.01);
      // THE IMPACT OF MISSED PREDICT BRANCHES:
      //if (_type == 0)
      //   return 0;
      //return pow(value, 1.01);
   }
};


bool testSpeed1(size_t num) {
   vector<C> vec1;
   vec1.reserve(num);

   for (size_t i = 0; i < num; i++)
      vec1.emplace_back(C(i%2));

   std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

   double c = 0;
   for (auto& o : vec1)
      c += o.op(2);

   std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
   auto elapsed = std::chrono::duration <double, std::milli>(end - start).count();
   fprintf(stdout, "elapsed: %f, value: %.4f\n", elapsed, c);

   return true;
}

bool testSpeed2(size_t num) {
   vector<unique_ptr<A>> vec1;
   vec1.reserve(num);

   for (size_t i = 0; i < num; i++)
      if (i%2)
         vec1.emplace_back(make_unique<B>());
      else
         vec1.emplace_back(make_unique<A>());

   std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

   double c = 0;
   for (auto& o : vec1)
      c += o->op(2);

   std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
   auto elapsed = std::chrono::duration <double, std::milli>(end - start).count();
   fprintf(stdout, "elapsed: %f, value: %.4f\n", elapsed, c);

   return true;
}

}
