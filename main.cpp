#include "sixel.h"
#include "pgm.h"
#include "plot.h"


int main (int argc, char* argv[])
{
  auto image = load_pgm<float> (argv[1]);
  Sixel::imshow (image, 0, 255);

  std::vector<float> x = { 0, 100, 200, 300, 600, 800 };
  std::vector<float> y = { 10, 50, 179, 420, -10, 100 };

  Plot (778,512)
    .add_line (200, 0, 100, 256, {100, 100, 0}, 10)
    .add_line (x, y, {0,100,100}, 3);

  return 0;
}
