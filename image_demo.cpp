#include "sixel/imshow.h"
#include "pgm.h"


int main (int argc, char* argv[])
{
  if (argc < 2)
    throw std::runtime_error ("expected filename of PGM file as first argument");

  auto image = load_pgm<float> (argv[1]);
  std::cout << "Showing image \"" << argv[1] << "\", size: " << image.width() << " x " << image.height() << std::endl;
  Sixel::imshow (image, 0, 255);
  std::cout << std::endl;

  return 0;
}
