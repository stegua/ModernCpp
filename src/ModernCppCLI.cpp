/**
* @fileoverview Copyright (c) 2017, by Stefano Gualandi, UniPv,
*               via Ferrata, 1, Pavia, Italy, 27100
*
* @author stefano.gualandi@gmail.com (Stefano Gualandi)
*
*/

#include <cstdio>

#include "parallel.h"
#include "constexpressions.h"
#include "inheritance.h"
#include "iomanip.h"

int main(int argc, char* argv[]) {

   //fprintf(stdout, "Test Barrier: %d", testBarrier());

   //testConstExpr(20);

   //INH::testInheritance(7);
   //INH::testSpeed1(70000000);
   //INH::testSpeed2(70000000);

   testReadDatetime();

   return EXIT_SUCCESS;
}