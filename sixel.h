#ifndef __SIXEL_H__
#define __SIXEL_H__

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <array>
#include <limits>
#include <cmath>

#include "image.h"

namespace Sixel {

  // the data type to use to store intensities:
  using ctype = unsigned char;

  // the data structure used to hold a colourmap:
  using ColourMap = std::vector<std::array<ctype,3>>;

  // convenience function to generate ready-made grayscale colourmap:
  ColourMap gray (int number = 100);


  // display an indexed image to the terminal, according to the colourmap supplied.
  //
  // ImageType can be any object that implements the following methods:
  //     int width() const
  //     int height() const
  //     integer_type operator() (int x, int y) const
  template <class ImageType>
    void imshow (const ImageType& image, const ColourMap& cmap);


  // display a scalar image to the terminal, rescaled between (min, max),
  // according to the (optional) colourmap supplied.
  //
  // ImageType can be any object that implements the following methods:
  //     int width() const
  //     int height() const
  //     scalar_type operator() (int x, int y) const
  //         (where scalar_type can be any integer or floating-point type)
  template <class ImageType>
    void imshow (const ImageType& image, double min, double max, const ColourMap& cmap = gray());









  // functions in anonymous namespace will remain private to this file:
  namespace {

    // helper functions for colourmap handling:

    inline std::string colourmap_specifier (const ColourMap& colours)
    {
      int n = 0;
      std::stringstream specifier;
      for (const auto& c : colours)
        specifier << "#" << n++ << ";2;" << static_cast<int>(c[0]) << ";" << static_cast<int>(c[1]) << ";" << static_cast<int>(c[2]);
      return specifier.str();
    }




    inline void commit (ctype current, int repeats)
    {
      if (repeats <=3) {
        for (int n = 0; n < repeats; ++n)
          std::cout.put (63+current);
      }
      else
        std::cout << '!' << repeats << char(63+current);
    }


    template <class ImageType>
    inline void encode_row (const ImageType& im, int y0, int x_dim, int nsixels, const ctype intensity)
    {
      std::cout << "#" << static_cast<int>(intensity);

      int repeats = 0;
      ctype current = std::numeric_limits<ctype>::max();

      for (int x = 0; x < x_dim; ++x) {
        ctype c = 0;
        for (int y = 0; y < nsixels; ++y)
          if (im(x,y+y0) == intensity)
            c |= 1U<<y;
        if (c == current) {
          ++repeats;
          continue;
        }
        commit (current, repeats);
        current = c;
        repeats = 1;
      }
      if (current)
        commit (current, repeats);
    }


    template <class ImageType>
      inline void encode (const ImageType& im, int cmap_size, int y0)
      {
        const int nsixels = std::min (im.height()-y0, 6);

        bool first = true;
        for (ctype intensity = 0; intensity < cmap_size; ++intensity) {
          if (first) first = false;
          else std::cout.put('$');
          encode_row (im, y0, im.width(), nsixels, intensity);
        }
        std::cout.put('-');
      }

  }






  // public implementations:

  inline ColourMap gray (int number)
  {
    // lambda function to clamp incoming value between 0 & 100 and round to
    // nearest integer:
    auto clamp = [](float val, int number)
    {
      return ctype (std::round (std::min (std::max ((100.0/number)*val, 0.0), static_cast<double>(number))));
    };

    ColourMap cmap (number);
    for (int n = 0; n < number; ++n) {
      ctype c = clamp (n, number);
      cmap[n] = { c, c, c };
    }
    return cmap;
  }




  template <class ImageType>
    inline void imshow (const ImageType& image, const ColourMap& cmap)
    {
      std::cout << "\033Pq" << colourmap_specifier (cmap);
      for (int y = 0; y < image.height(); y += 6)
        encode (image, cmap.size(), y);
      std::cout << "\033\\";
    }



  template <class ImageType>
    inline void imshow (const ImageType& image, double min, double max, const ColourMap& cmap)
    {
      // adapter class to rescale intensities of original image from (min, max)
      // range to range of colourmap, and round to nearest integer index:
      struct Adapter {
        const ImageType& im;
        const double min, max;
        const int cmap_size;

        int width () const { return im.width(); }
        int height () const { return im.height(); }
        ctype operator() (int x, int y) const {
          double rescaled = cmap_size * (im(x,y) - min) / (max - min);
          return std::round (std::min (std::max (rescaled, 0.0), cmap_size-1.0));
        }
      } adapted = { image, min, max, static_cast<int>(cmap.size()) };

      imshow (adapted, cmap);
    }

}

#endif
