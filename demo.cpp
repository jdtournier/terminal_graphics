#include <random>
#include <cmath>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <fstream>

#include "terminal_graphics.h"



// Simple function to load an ascii-encoded PGM grayscale image.
// This returns an object of type TG::Image<unsigned char>

TG::Image<unsigned char> load_pgm (const std::string& pgm_filename)
{
  std::ifstream in (pgm_filename);
  if (!in)
    throw std::runtime_error ("failed to open input PGM file \"" + pgm_filename + "\"");

  // strip out all comments, and present stringstream of clean file contents:
  std::string line;
  std::stringstream in_clean;
  while (std::getline (in, line)) {
    in_clean << line.substr (0, line.find_first_of('#')) << "\n";
  }

  std::string magic;
  in_clean >> magic;
  if (magic != "P2")
    throw std::runtime_error ("input file \"" + pgm_filename + "\" is not in expected PGM format");

  int xdim, ydim, maxval;
  in_clean >> xdim >> ydim >> maxval;

  if (maxval >= 65536)
    throw std::runtime_error ("PGM file \"" + pgm_filename + "\" is badly formed: maxval exceeds 65536");
  if (maxval <= 0)
    throw std::runtime_error ("PGM file \"" + pgm_filename + "\" is badly formed: maxval lower than or equal to zero");

  if (maxval > std::numeric_limits<unsigned char>::max())
    throw std::runtime_error ("maximum intensity in PGM file \"" + pgm_filename + "\" exceeds range of data type used");

  TG::Image<unsigned char> im (xdim, ydim);

  for (int y = 0; y < im.height(); ++y) {
    for (int x = 0; x < im.width(); ++x) {
      int val;
      in_clean >> val;
      im(x,y) = static_cast<unsigned int> (val);
    }
  }

  return im;
}






int main (int argc, char* argv[])
{
  try {
    // demonstrate use of TG::imshow():

    const std::string image_filename = "brain.pgm";

    const auto image = load_pgm (image_filename);
    std::cout << "Showing image \"" << image_filename << "\", size: " << image.width() << " x " << image.height() << std::endl;
    TG::imshow (image, 0, 255);
    std::cout << std::endl;


    // demonstate use of TG::plot():

    std::vector<float> x (50);
    std::vector<float> y (50);

    for (std::size_t n = 0; n < x.size();++n) {
      y[n] = std::sin (0.2*n) + 0.3*std::cos (0.33*n);
      x[n] = 20.0+10.0*std::cos (0.41*n) + 5.0*std::sin(0.21*n);
    }


    std::cout << "Plotting arbitrary lines:" << std::endl;
    TG::plot (768, 256, 8)
      .add_line (y, 4, 10)
      .add_line (x, y, 3)
      .add_text ("sinusoids", (x.size()-1)/2.0, 1.2, 0.5, 0.0, 12, 6);


    // a plot of random numbers:

    std::random_device rd;
    std::mt19937 gen (rd());
    std::normal_distribution normal (5.0, 2.0);

    std::vector<float> noise (512);
    for (auto& x : noise)
      x = normal (gen);

    std::cout << "Plotting Normally distributed random variables:" << std::endl;
    TG::plot (1024, 256)
      .set_grid (50, 2)
      .add_line (noise,2);

  }
  catch (std::exception& e) {
    std::cerr << "error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}

