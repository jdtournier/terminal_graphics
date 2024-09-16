#include <random>
#include <cmath>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <format>

#include "terminal_graphics.h"
#include "load_pgm.h"


int main (int argc, char* argv[])
{
  try {
    // demonstrate use of TG::imshow():

    const std::string image_filename = "brain.pgm";

    const auto image = load_pgm (image_filename);
    std::cout << std::format ("Showing image \"{}\", size: {} x {}\n", image_filename, image.width(), image.height());
    TG::imshow (image, 0, 255);


    // demonstate use of TG::plot():

    std::vector<float> x (50);
    std::vector<float> y (50);



    for (int x : std::views::iota (0, static_cast<int>(y.size())))
      y[x] = exp (-0.1*x) - 1.5*exp (-0.4*x);

    std::cout << "A simple one-line plot:\n";
    TG::plot().add_line (y);




    for (std::size_t n = 0; n < x.size();++n) {
      y[n] = std::sin (0.2*n) + 0.3*std::cos (0.33*n);
      x[n] = 20.0+10.0*std::cos (0.41*n) + 5.0*std::sin(0.21*n);
    }

    std::cout << "Plotting arbitrary lines:\n";
    TG::plot (768, 256)
      .add_line (y, 4, 10)
      .add_line (x, y, 3)
      .add_text ("sinusoids", (x.size()-1)/2.0, 1.2, 0.5, 0.0, 6);




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
    TG::plot (768, 256)
      .set_ylim (-1, 11)
      .set_grid (50, 2)
      .add_line (noise,2);

  }
  catch (std::exception& e) {
    std::cerr << "error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}

