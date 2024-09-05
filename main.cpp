#include "image.h"
#include "sixel.h"
#include "pgm.h"


int main (int argc, char* argv[])
{
  auto image = load_pgm<float> (argv[1]);
  Sixel::display (image, 0, 255);

  return 0;
}
