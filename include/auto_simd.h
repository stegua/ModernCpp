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

#include <array>
#include <chrono>
#include <immintrin.h>

#include <random>

typedef int32_t   child_t;
typedef std::array<child_t, 1000>  child_vec;


int64_t contains(const child_vec& c_vec, child_t i) {
   for (auto p = 0; p < 1000; ++p)
      if (c_vec[p] == i)
         return 2 * (1000 - p);
   return -1;
}


int64_t contains2(const child_vec& c_vec, child_t i) {
   if (c_vec[0] == i)
      return 2 * 1000;
   int64_t r = 0;
   for (auto p = 0; p < 1000; p += 8) {
      r += p * int(c_vec[p] == i) + (p + 1)*int(c_vec[p + 1] == i) + (p + 2)*int(c_vec[p + 2] == i) + (p + 3)*int(c_vec[p + 3] == i)
           + (p + 4)*int(c_vec[p + 4] == i) + (p + 5)*int(c_vec[p + 5] == i) + (p + 6)*int(c_vec[p + 6] == i) + (p + 7)*int(c_vec[p + 7] == i);
   }
   if (r > 0)
      return 2 * (1000 - r);
   return -1;
}

int64_t contains3(const child_vec& c_vec, child_t i) {
   if (c_vec[0] == i)
      return 2 * 1000;
   int r = 0;
   for (int p = 0; p < 1000; ++p) {
      r += p * (c_vec[p] == i ? 1 : 0);
   }
   if (r > 0)
      return 2 * (1000 - r);
   return -1;
}

/*int64_t contains_simd(const child_vec& c_vec, child_t i) {
if (c_vec[0] == i)
return 2*1000;

child_vec t;
t.fill(i);
__m256i b = _mm256_loadu_si256((const __m256i*) (t.begin()+i));
for (size_t i = 0; i < 1000; i += 8) {
__m256i a = _mm256_loadu_si256((const __m256i*) (c_vec.begin()+i));
__m256i c = _mm256_cmpeq_epi32 (a, b);
}
return -1;
}*/

void test_simd(child_t v) {
   child_vec tmp;
   for (child_t i = 0; i < 1000; ++i)
      tmp[i] = i;

   std::mt19937 gen(13);
   std::shuffle(tmp.begin(), tmp.end(), gen);

   std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();
   std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
   auto elapsed = std::chrono::duration <double, std::milli>(end - start_time).count();

   int64_t c = 0;
   for (int j = 0; j < 10; ++j)
      for (int i = 0; i < 1000000; ++i)
         c += contains3(tmp, v*j);

   end = std::chrono::steady_clock::now();
   elapsed = std::chrono::duration <double, std::milli>(end - start_time).count();

   fprintf(stdout, "%I64d - %f\n", c, elapsed);

   ///////////////////////////
   start_time = std::chrono::steady_clock::now();
   c = 0;
   for (int j = 0; j < 10; ++j)
      for (int i = 0; i < 1000000; ++i)
         c += contains2(tmp, v*j);

   end = std::chrono::steady_clock::now();
   elapsed = std::chrono::duration <double, std::milli>(end - start_time).count();

   fprintf(stdout, "%I64d - %f\n", c, elapsed);

   ///////////////////////////
   start_time = std::chrono::steady_clock::now();
   c = 0;
   for (int j = 0; j < 10; ++j)
      for (int i = 0; i < 1000000; ++i)
         c += contains(tmp, v*j);

   end = std::chrono::steady_clock::now();
   elapsed = std::chrono::duration <double, std::milli>(end - start_time).count();

   fprintf(stdout, "%I64d - %f\n", c, elapsed);
}
