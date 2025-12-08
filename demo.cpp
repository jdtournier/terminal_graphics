#include <random>
#include <cmath>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <format>

#include "load_pgm.h"
#include "termviz.h"

int main ()
{
  try {
    // demonstrate use of termviz::imshow():

    const std::string image_filename = "brain.pgm";

    const auto image = load_pgm (image_filename);
    std::cout << std::format ("Showing image \"{}\", size: {} x {}\n", image_filename, image.width(), image.height());
    termviz::imshow (image, 0, 255);

    std::cout << "Same image using hot colourmap, magnified by a factor of 2, with transparency:\n";
    termviz::imshow (termviz::magnify (image, 2), 0, 255, termviz::hot(), true);


    // demonstrate use of termviz::figure():

    std::vector<float> y (50);

    for (unsigned int x = 0; x < y.size(); ++x)
      y[x] = exp (-0.1*x) - 1.5*exp (-0.4*x);

    std::cout << "A simple one-line plot:\n";
    termviz::figure().plot (y);




    std::vector<float> x (50);
    for (std::size_t n = 0; n < x.size();++n) {
      y[n] = std::sin (0.2*n) + 0.3*std::cos (0.33*n);
      x[n] = 20.0+10.0*std::cos (0.41*n) + 5.0*std::sin(0.21*n);
    }

    std::cout << "Plotting arbitrary lines, without transparency:\n";
    termviz::figure (768, 256)
      .transparent(false)
      .plot (y, 4, 10)
      .plot (x, y, 3)
      .text ("sinusoids", (x.size()-1)/2.0, 1.1, 0.5, 0.0, 6)
      .grid(false,false);




    // a plot of random numbers:

    // set up random number generator for Normal distribution:
    std::random_device rd;
    std::mt19937 gen (rd());
    std::normal_distribution normal (5.0, 2.0);

    // generate vector of Normally-distributed random numbers:
    std::vector<float> noise (256);
    for (auto& x : noise)
      x = normal (gen);

    std::cout << "Plotting Normally distributed random variables:\n";
    termviz::figure (1024, 256)
      .ylim (-1, 11)
      .grid (false, true)
      .plot (noise,2);

  }
  catch (std::exception& e) {
    std::cerr << "error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}

