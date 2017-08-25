/**
* @fileoverview Copyright (c) 2017, by Stefano Gualandi, UniPv,
*               via Ferrata, 1, Pavia, Italy, 27100
*
* @author stefano.gualandi@gmail.com (Stefano Gualandi)
*
*/

#pragma once

#include <iomanip>
#include <iostream>
#include <sstream>
#include <locale>

bool testReadDatetime() {
   // SEE: http://en.cppreference.com/w/cpp/io/manip/get_time

   std::tm t = {};
   std::istringstream ss("2017-07-13 10:22:44.123");
   //ss.imbue(std::locale("en_US.utf-8"));
   ss >> std::get_time(&t, "%Y-%m-%d %H:%M:%S");

   auto tp = std::chrono::system_clock::from_time_t(std::mktime(&t));

   if (ss.fail()) {
      std::cout << "Parse failed\n";
   } else {
      std::cout << std::put_time(&t, "%c") << '\n';
   }

   return true;
}