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

class A {
 public:
   constexpr explicit A(int a) : _a(a) {}

   constexpr double myfunc(int b) const {
      // I cannot print, otherwise is not constexpr
      //fprintf(stdout, "a: %d, myfunc(%d)\n", _a, b);
      return _a*longtimefunc(b);
   }

   constexpr double longtimefunc(int b) const {
      // I cannot print, otherwise is not constexpr
      //fprintf(stdout, "a: %d, longtimefunc(%d)\n", _a, b);
      double c = b;
      for (int i = 0; i < 10000000; i++)
         c += c + sin(i*3.14) + cos(i*3.14); // useless computation to eat CPU cycles
      return c;
   }

 private:
   const int _a;
};


bool testConstExpr(int c) {
   vector<A> vec;

   // TODO: Check the assemlby code to understand if there iss any difference
   vec.push_back(A(0));
   for ( int i = 1; i < c; i++ )
      vec.push_back(A(1));

   for (A& a : vec)
      a.myfunc(3);

   return true;
}