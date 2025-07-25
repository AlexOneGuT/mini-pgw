#include "SessionManager.hpp"
#include <chrono>
#include <iostream>

int main() {
  const int N = 1000000;
  SessionManager m(60);
  auto start = std::chrono::high_resolution_clock::now();
  for(int i = 0; i < N; ++i) m.create("IMSI" + std::to_string(i));
  auto mid = std::chrono::high_resolution_clock::now();
  for(int i = 0; i < N; ++i) m.exists("IMSI" + std::to_string(i));
  auto end = std::chrono::high_resolution_clock::now();
  std::cout << "Create " << N << ": "
            << std::chrono::duration_cast<std::chrono::milliseconds>(mid - start).count()
            << " ms\n";
  std::cout << "Exists " << N << ": "
            << std::chrono::duration_cast<std::chrono::milliseconds>(end - mid).count()
            << " ms\n";
}
