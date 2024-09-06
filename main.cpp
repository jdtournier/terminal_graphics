#include "sixel.h"
#include "pgm.h"


int main (int argc, char* argv[])
{
  auto image = load_pgm<float> (argv[1]);
  Sixel::imshow (image, 0, 255);

  return 0;
}
