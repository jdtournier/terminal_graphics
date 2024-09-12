#include <random>
#include <cmath>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <fstream>

#include "terminal_graphics.h"
#include "load_pgm.h"


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

