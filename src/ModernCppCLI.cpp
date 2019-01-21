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
#include "auto_simd.h"
#include "yocto_logger.h"


//int main(int argc, char* argv[]) {
//
//   //fprintf(stdout, "Test Barrier: %d", testBarrier());
//
//   //testConstExpr(20);
//
//   //INH::testInheritance(7);
//   //INH::testSpeed1(70000000);
//   //INH::testSpeed2(70000000);
//
//   //testReadDatetime();
//
//   // test auto_simd
//   //child_t v = argc;
//   //if (argc > 0)
//   //   v = child_t(std::stoi(argv[1]));
//   //test_simd(v);
//
//   yocto::Logger logger;
//   logger.info(yocto::fmt("prova %d, %f, %s", 12, 1 / 3, "ciao"));
//   logger.error(yocto::fmt("prova %d, %f, %s", 12, 1 / 3, "ciao"));
//
//   yocto::Logger fileLogger("prova.log");
//   fileLogger.setVerbosityLevel(yocto::VerbosityLevel::WARN);
//   fileLogger.info(yocto::fmt("prova %d, %f, %s", 12, 1 / 3, "ciao"));
//   fileLogger.error(yocto::fmt("prova %d, %f, %s", 12, 1 / 3, "ciao"));
//
//   return EXIT_SUCCESS;
//}
//#include <thrust/host_vector.h>
//#include <thrust/device_vector.h>


#include <amp.h>
#include <amp_math.h>

#include <ppl.h>
using namespace concurrency;

#include <iostream>
#include <chrono>

int seed = 13;
std::random_device rd;  //Will be used to obtain a seed for the random number engine
std::uniform_real_distribution<> Uniform01(0, 1000);

vector<float> randomVec(size_t n) {
   vector<float> r(n, 0);
   std::mt19937 gen(seed); //Standard mersenne_twister_engine seeded with rd()
   seed++;
   for (float& a : r)
      a = Uniform01(gen);
   return r;
}

void MultiplyWithoutTiling() {
   int s = 512;
   int n = s * s;

   vector<float> aMatrix = randomVec(n);
   vector<float> bMatrix = randomVec(n);
   vector<float> productMatrix(n, 0);

   for (int i = 0; i < s; ++i)
      for (int j = 0; j < s; ++j)
         for (int h = 0; h < s; ++h)
            productMatrix[i*s + j] += aMatrix[i*s+h] * bMatrix[h*s+j];

}

void MultiplyCore() {
   int s = 512;
   int n = s * s;

   vector<float> aMatrix = randomVec(n);
   vector<float> bMatrix = randomVec(n);
   vector<float> productMatrix(n, 0);

   concurrency::parallel_for(0, (int)s, [&](int i) {
      for (int j = 0; j < s; ++j)
         for (int h = 0; h < s; ++h)
            productMatrix[i*s + j] += aMatrix[i*s + h] * bMatrix[h*s + j];
   });
}

void MultiplyGPU() {
   int s = 512;
   int n = s * s;

   vector<float> aMatrix = randomVec(n);
   vector<float> bMatrix = randomVec(n);
   vector<float> productMatrix(n, 0);

   concurrency::array_view<const float, 1> a(n, &aMatrix[0]);
   concurrency::array_view<const float, 1> b(n, &bMatrix[0]);
   concurrency::array_view<float, 1> product(n, &productMatrix[0]);

   for (int i = 0; i < s; ++i) {
      concurrency::parallel_for_each(product.extent, [=](concurrency::index<1> idx) restrict(amp) {
         for (int h = 0; h < s; ++h)
            product[i*s + idx] += a[i*s + h] * b[h*s + idx];
      });
      try {
         product.synchronize();
      } catch (const Concurrency::accelerator_view_removed& e) {
         printf("%s\n", e.what());
      }
   }
}

void MultiplyWithTiling() {
   // The tile size is 2.
   static const int TS = 16;

   // The raw data.
   //int aMatrix[] = { 1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 8 };
   //int bMatrix[] = { 1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 8 };
   //int productMatrix[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

   int s = 512;
   int n = s*s;

   vector<float> aMatrix = randomVec(n);
   vector<float> bMatrix = randomVec(n);
   vector<float> productMatrix(n, 0);

   // Create the array_view objects.
   concurrency::array_view<const float, 2> a(s, s, &aMatrix[0]);
   concurrency::array_view<const float, 2> b(s, s, &bMatrix[0]);
   concurrency::array_view<float, 2> product(s, s, &productMatrix[0]);

   // Call parallel_for_each by using 2x2 tiles.
   concurrency::parallel_for_each(product.extent.tile<TS, TS>(),
   [=](concurrency::tiled_index<TS, TS> t_idx) restrict(amp) {
      // Get the location of the thread relative to the tile (row, col)
      // and the entire array_view (rowGlobal, colGlobal).
      int row = t_idx.local[0];
      int col = t_idx.local[1];
      int rowGlobal = t_idx.global[0];
      int colGlobal = t_idx.global[1];
      float sum = 0;

      // Given a 4x4 matrix and a 2x2 tile size, this loop executes twice for each thread.
      // For the first tile and the first loop, it copies a into locA and e into locB.
      // For the first tile and the second loop, it copies b into locA and g into locB.
      for (int i = 0; i < s; i += TS) {
         tile_static float locA[TS][TS];
         tile_static float locB[TS][TS];
         locA[row][col] = a(rowGlobal, col + i);
         locB[row][col] = b(row + i, colGlobal);
         // The threads in the tile all wait here until locA and locB are filled.
         t_idx.barrier.wait();

         // Return the product for the thread. The sum is retained across
         // both iterations of the loop, in effect adding the two products
         // together, for example, a*e.
         for (int k = 0; k < TS; k++) {
            sum += locA[row][k] * locB[k][col];
         }

         // All threads must wait until the sums are calculated. If any threads
         // moved ahead, the values in locA and locB would change.
         t_idx.barrier.wait();
         // Now go on to the next iteration of the loop.
      }

      // After both iterations of the loop, copy the sum to the product variable by using the global location.
      product[t_idx.global] = sum;
   });

   // Copy the contents of product back to the productMatrix variable.
   product.synchronize();

}

void MyPricing() {

   // The raw data.
   int k = 3;  //we are in R2
   int s = 4;
   int n = k*s;

   // The tile size is 2.
   static const int TS = 2;
   static const int TK = 3;

   vector<float> aMatrix = { 1,2,3,4,
                             1,2,3,4,
                             1,2,3,4
                           };
   vector<float> bMatrix = { 0,0,0,0,
                             0,0,0,0,
                             1,1,1,1
                           };//randomVec(n);
   vector<float> pMatrix(s, 0);

   // Create the array_view objects.
   concurrency::array_view<float, 2> a(k, s, &aMatrix[0]);
   concurrency::array_view<float, 2> b(k, s, &bMatrix[0]);
   concurrency::array_view<float, 1> p(s, &pMatrix[0]);

   // Call parallel_for_each by using 2x2 tiles.
   concurrency::parallel_for_each(p.extent.tile<TS>(),
   [=](concurrency::tiled_index<TS> t_idx) restrict(amp) {
      int col = t_idx.local[0];
      int colGlobal = t_idx.global[0];
      tile_static float A[TK][TS];
      for (int h = 0; h < TK; ++h)
         A[h][col] = a(h, colGlobal);

      float cmin = 10000;
      for (int i = 0; i < s; i += TS) {
         tile_static float B[TK][TS];
         for (int h = 0; h < TK; ++h)
            B[h][col] = b(h, colGlobal);
         t_idx.barrier.wait();

         for (int j = 0; j < TS; ++j) {
            float t = 0;
            for (int h = 0; h < TK; ++h)
               t += concurrency::fast_math::pow(A[h][col] - B[h][j], 2);
            cmin = (t < cmin ? t : cmin);
         }

         t_idx.barrier.wait();
      }
      p[colGlobal] = cmin;
   });

// Copy the contents of product back to the productMatrix variable.
   p.synchronize();
   for (int i = 0; i < s; ++i) {
      fprintf(stdout, "a: %f\t", p[i]);
   }
   fprintf(stdout, "\n");

}

int main(void) {
   auto start = std::chrono::steady_clock::now();
   double elapsed;
   MyPricing();
//   for (int i = 0; i < 100; i++)
   //    MultiplyGPU();
   //MultiplyCore();
   //      //MultiplyWithoutTiling();
//      MultiplyWithTiling();

   auto end = std::chrono::steady_clock::now();
   elapsed = double(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()) / 1000;
   printf("Basic model up: %.4f\n", elapsed);

   //// H has storage for 4 integers
   //thrust::host_vector<int> H(4);

   //// initialize individual elements
   //H[0] = 14;
   //H[1] = 20;
   //H[2] = 38;
   //H[3] = 46;

   //// H.size() returns the size of vector H
   //std::cout << "H has size " << H.size() << std::endl;

   //// print contents of H
   //for (int i = 0; i < H.size(); i++)
   //   std::cout << "H[" << i << "] = " << H[i] << std::endl;

   //// resize H
   //H.resize(2);

   //std::cout << "H now has size " << H.size() << std::endl;

   //// Copy host_vector H to device_vector D
   //thrust::device_vector<int> D = H;

   //// elements of D can be modified
   //D[0] = 99;
   //D[1] = 88;

   //// print contents of D
   //for (int i = 0; i < D.size(); i++)
   //   std::cout << "D[" << i << "] = " << D[i] << std::endl;

   //// H and D are automatically deleted when the function returns
   //return 0;
}