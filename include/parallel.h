/**
* @fileoverview Copyright (c) 2017, by Stefano Gualandi, UniPv,
*               via Ferrata, 1, Pavia, Italy, 27100
*
* @author stefano.gualandi@gmail.com (Stefano Gualandi)
*
*/

#pragma once

#include <future>

//#include <experimental/future> WHERE CAN I FIND THIS IMPLEMENTATION ??
//

/**
* @brief Test a simple barrier algorithm using C++11 threading support
* @return true if test succeed
*/
bool testBarrier() {
   // One future per thread/task
   std::vector<std::future<int>> sols;
   std::atomic<bool> signal = false;

   for (int timeout : {
            3, 2, 10, 13, 1
         }) {
      sols.emplace_back(std::async([&](int to) {
         for (int i = to; i >= 0; i--) {
            fprintf(stdout, "thread: X, timeout: %d, tick: %d\n", to, i);
            std::this_thread::sleep_for(std::chrono::seconds(1));
            if (signal)
               return i;
         }
         signal = true;
         return to;
      }, timeout));
   }

   // WRONG WAY, DO NOT WRITE THE FOLLOWING:
   //for (int i = 0; i < sols.size(); i++) {
   //   if (sols[i].get() > 2) {
   //      fprintf(stdout, "test: %d\n", sols[i].get()); // ERROR: ASKING THE SECOND TIME FOR A FUTURE!!!
   //   }
   //}

   // GOOD WAY (STD::FUTURE get() called only once)
   for (auto& result : sols)
      fprintf(stdout, "result: %d\n", result.get());

   return true;
}
