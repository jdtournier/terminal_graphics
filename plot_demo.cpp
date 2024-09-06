#include <random>
#include <vector>

#include "plot.h"

int main (int argc, char* argv[])
{
  std::vector<float> x = { 0, 1, 6, 3, 8, 2 };
  std::vector<float> y = { 1, 5, 1.79, 12, -10, 1 };

  std::cout << "Plotting arbitrary lines:" << std::endl;
  Plot (768,256)
    .set_ylim (-10, 20)
    .set_grid (2,5)
    .add_line (0, 0, 10, 13, 2, 10)
    .add_line (x, y, 3, 3)
    .add_line (y, 4);


  std::random_device rd;
  std::mt19937 gen (rd());
  std::normal_distribution normal (5.0, 2.0);

  std::vector<float> noise (512);
  for (auto& x : noise)
    x = normal (gen);

  std::cout << "Plotting Normally distributed random variables:" << std::endl;
  Plot (1024, 256)
    .set_grid (50, 2)
    .add_line (noise,2);

  return 0;
}

