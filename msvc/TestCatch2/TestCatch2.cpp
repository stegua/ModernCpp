//#define CATCH_CONFIG_MAIN
//#define CATCH_CONFIG_FAST_COMPILE
//#define CATCH_CONFIG_DISABLE_STRINGIFICATION
//#include "catch.hpp"

#include <iostream>
#include <string>
#include <random>

#include <amp.h>
#include <amp_math.h>

#include <ppl.h>

using namespace concurrency;

//TEST_CASE("test") {
//   CHECK(std::string(R"("\)") == "\"\\");
//}

void test_add() {
   std::random_device rd;  //Will be used to obtain a seed for the random number engine
   std::mt19937 gen(13); //Standard mersenne_twister_engine seeded with rd()
   std::uniform_int_distribution<> Uniform0N(0, 2048);

   int N = 128 * 128 * 128;
   float* x = new float[N];
   float* y = new float[N];
   float* s = new float[N];

   for (int i = 0; i < N; ++i) {
      x[i] = Uniform0N(gen);
      y[i] = Uniform0N(gen);
   }

   auto start = std::chrono::steady_clock::now();
   double elapsed;

   concurrency::array<float> xv(N, x, x+N);
   concurrency::array<float> yv(N, y, y+N);
   concurrency::array_view<float> sv(N, s);
   parallel_for_each(sv.extent, [=, &xv, &yv](index<1> idx) restrict(amp) {
      for (int j = 0; j < 3000; ++j)
         sv[idx] += (j%2)*concurrency::fast_math::sqrt(concurrency::fast_math::pow(xv[idx] - yv[j], 2) + concurrency::fast_math::pow(xv[idx] + yv[j], 2));
   });

   float mmin = sv[0];
   for (int i = 1; i < N; ++i)
      mmin = std::min<>(mmin, sv[i]);

   auto end = std::chrono::steady_clock::now();
   elapsed = double(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()) / 1000;
   printf("GPU NVIDIA   runtime: %f - min: %f\n", elapsed, mmin);
}

void test_add_dummy() {
   std::random_device rd;  //Will be used to obtain a seed for the random number engine
   std::mt19937 gen(13); //Standard mersenne_twister_engine seeded with rd()
   std::uniform_int_distribution<> Uniform0N(0, 2048);

   int N = 128*128*128;
   float* x = new float[N];
   float* y = new float[N];
   float* s = new float[N];

   for (int i = 0; i < N; ++i) {
      x[i] = Uniform0N(gen);
      y[i] = Uniform0N(gen);
   }

   auto start = std::chrono::steady_clock::now();
   double elapsed;

   for (int i = 0; i < N; ++i) {
      for (int j = 0; j < 3000; ++j)
         s[i] += (j % 2)* sqrt(pow(x[i] - y[j], 2) + pow(x[i] + y[j], 2));
   }

   float mmin = s[0];
   for (int i = 1; i < N; ++i)
      mmin = std::min<>(mmin, s[i]);

   auto end = std::chrono::steady_clock::now();
   elapsed = double(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()) / 1000;
   printf("CPU 1 core   runtime: %f - min: %f\n", elapsed, mmin);
}

void test_add_core() {
   std::random_device rd;  //Will be used to obtain a seed for the random number engine
   std::mt19937 gen(13); //Standard mersenne_twister_engine seeded with rd()
   std::uniform_int_distribution<> Uniform0N(0, 2048);

   int N = 128 * 128 * 128;
   float* x = new float[N];
   float* y = new float[N];
   float* s = new float[N];

   for (int i = 0; i < N; ++i) {
      x[i] = Uniform0N(gen);
      y[i] = Uniform0N(gen);
   }

   auto start = std::chrono::steady_clock::now();
   double elapsed;
   concurrency::parallel_for(0, N,
   [&](int i) {
      for (int j = 0; j < 3000; ++j)
         s[i] += (j % 2)* sqrt(pow(x[i] - y[j], 2) + pow(x[i] + y[j], 2));
   });

   float mmin = s[0];
   for (int i = 1; i < N; ++i)
      mmin = std::min<>(mmin, s[i]);

   auto end = std::chrono::steady_clock::now();
   elapsed = double(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()) / 1000;
   printf("CPU 10 cores runtime: %f - min: %f\n", elapsed, mmin);
}

typedef std::pair<int, int>     Point2D;
typedef std::vector<Point2D>    Space2D;
typedef std::vector<double>     Marginal;

Space2D randomSpace2D(size_t n, int seed) {
   std::random_device rd;  //Will be used to obtain a seed for the random number engine
   std::mt19937 gen(seed); //Standard mersenne_twister_engine seeded with rd()
   std::uniform_int_distribution<> Uniform0N(0, n);

   Space2D r;
   r.reserve(n);
   for (size_t i = 0; i < n; ++i)
      r.emplace_back(Uniform0N(gen), Uniform0N(gen));
   return r;
}

Marginal randomMarginal(size_t n, int max_value, int seed) {
   std::random_device rd;  //Will be used to obtain a seed for the random number engine
   std::mt19937 gen(seed); //Standard mersenne_twister_engine seeded with rd()
   std::uniform_int_distribution<> Uniform1N(1, max_value);

   Marginal pi;
   pi.reserve(n);
   double tot = 0;
   for (size_t i = 0; i < n; ++i) {
      double val = Uniform1N(gen);
      tot += val;
      pi.emplace_back(val);
   }
   for (auto& v : pi)
      v = v / tot;

   return pi;
}


class Column {
 public:
   int     a;  // First point
   int     b;  // Second point
   double  c;  // Distance

   Column() : a(-1), b(-1), c(-1) {}
   Column(int _a, int _b, double _c) : a(_a), b(_b), c(_c) {}
};

typedef std::vector<Column>                Columns;

double distance(const Point2D& a, const Point2D& b) {
   return sqrt(pow((a.first - b.first), 2) + pow((a.second - b.second), 2));
}

//Columns solvePricingCore(
//   const Space2D& X,
//   const Marginal& DualsPI,
//   const Space2D& Y,
//   const Marginal& DualsMU) {
//   Columns cols(X.size());
//
//   concurrency::parallel_for(0, (int)X.size(),
//   [&](int i) {
//      int a = -1;
//      int b = -1;
//      double c = -1;
//      double viol = -1;
//      for (int j = 0, j_max = Y.size(); j < j_max; ++j) {
//         double d = distance(X[i], Y[j]);
//         double v1 = DualsPI[i] + DualsMU[j] - d;
//         if (v1 > viol) {
//            a = i;
//            b = j;
//            c = d;
//            viol = v1;
//         }
//      }
//      if (viol > 0.001)
//         cols[i] = Column(a, b, c);
//   });
//
//   return cols;
//}

Columns solvePricingGPU(
   const Space2D& X,
   const Marginal& DualsPI,
   const Space2D& Y,
   const Marginal& DualsMU) {
   Columns cols(X.size());
   for (int j = 0; j < X.size(); ++j)
      cols[j].a = j;

   concurrency::array<Point2D> xv(X.size(), X.begin(), X.end());
   concurrency::array<Point2D> yv(Y.size(), Y.begin(), Y.end());
   concurrency::array<float> pi(DualsPI.size(), DualsPI.begin(), DualsPI.end());
   concurrency::array<float> mu(DualsMU.size(), DualsMU.begin(), DualsMU.end());

   concurrency::array_view<Column> cv(X.size(), cols);
   const int N = (int)Y.size();

   auto start = std::chrono::steady_clock::now();
   double elapsed;


   parallel_for_each(cv.extent, [=, &xv, &yv, &pi, &mu](index<1> idx) restrict(amp) {
      int b = -1;
      double c = -1;
      double viol = -1;
      for (int j = 0; j < N; ++j) {
         float d = concurrency::fast_math::sqrt(concurrency::fast_math::pow(xv[idx].first - yv[j].first, 2) + concurrency::fast_math::pow(xv[idx].second - yv[j].second, 2));
         float v1 = pi[idx] + mu[j] - d;
         if (v1 > viol) {
            b = j;
            c = d;
            viol = v1;
         }
      }
      if (viol > 0.001) {
         cv[idx].b = b;
         cv[idx].c = c;
      }
   });

   auto end = std::chrono::steady_clock::now();
   elapsed = double(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()) / 1000;
   printf("AMP runtime: %f - first: %f\n", elapsed, cv[0].c);

   return cols;
}

void mytest() {
   int n = 128;
   int seed = 13;
   auto X = randomSpace2D(n, ++seed);
   auto Y = randomSpace2D(n, ++seed);

   auto PI = randomMarginal(n*n, 1024, ++seed);
   auto MU = randomMarginal(n*n, 50, ++seed);
   for (auto& mu : MU)
      mu = -mu;
   solvePricingGPU(X, PI, Y, MU);
}

void tile() {
   int sampledata[] = {
      2, 2, 9, 7, 1, 4,
      4, 4, 8, 8, 3, 4,
      1, 5, 1, 2, 5, 2,
      6, 8, 3, 2, 7, 2
   };

   int averagedata[] = {
      0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0,
   };

   array_view<int, 2> sample(4, 6, sampledata);

   array_view<int, 2> average(4, 6, averagedata);


   parallel_for_each(sample.extent.tile<4, 1>(),
   [=](tiled_index<4, 1> idx) restrict(amp) {
      tile_static int nums[4][1];
      nums[idx.local[1]][idx.local[0]] = sample[idx.global];
      idx.barrier.wait();
      int sum = nums[0][0] + nums[1][0] + nums[2][0] + nums[3][0];
      average[idx.global] = sum;
   });


   for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 6; j++) {
         std::cout << average(i, j) << " ";
      }
      std::cout << "\n";
   }
}

void default_properties() {
   accelerator default_acc;
   std::wcout << default_acc.device_path << "\n";
   std::wcout << default_acc.dedicated_memory << "\n";
   std::wcout << (default_acc.supports_cpu_shared_memory ?
                  "CPU shared memory: true" : "CPU shared memory: false") << "\n";
   std::wcout << (default_acc.supports_double_precision ?
                  "double precision: true" : "double precision: false") << "\n";
   std::wcout << (default_acc.supports_limited_double_precision ?
                  "limited double precision: true" : "limited double precision: false") << "\n";
}

int main() {
   test_add_dummy();
   test_add_core();
   test_add();
   //tile();
   //mytest();
}