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

  // functions to generate ready-made colourmaps:
  ColourMap gray (int number = 100);
  ColourMap hot (int number = 100);
  ColourMap jet (int number = 100);


  // display an image to the terminal:
  // ImageType can be any object that implements the following methods:
  //     int width() const
  //     int height() const
  //     scalar_type operator() (int x, int y) const
  template <class ImageType>
    void display (const ImageType& image, double min, double max, const ColourMap& cmap = gray());









  // functions in anonymous namespace will not be accessible outside file:
  namespace {

    // helper functions for colourmap handling:
    inline ctype clamp_col (float val, int number)
    {
      return ctype (std::round (std::min (std::max ((100.0/number)*val, 0.0), double(number))));
    }

    inline std::string colourmap_specifier (const ColourMap& colours)
    {
      int n = 0;
      std::stringstream specifier;
      for (const auto& c : colours)
        specifier << "#" << n++ << ";2;" << int(c[0]) << ";" << int(c[1]) << ";" << int(c[2]);
      return specifier.str();
    }




    // helper functions for sixel render:
      inline ctype clamp (double v, int n)
      {
        return std::round (std::min (std::max (v, 0.0), n-1.0));
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


    inline void encode (const std::vector<ctype>& data, int x_dim, int nsixels, const ctype intensity)
    {
      std::cout << "#" << int(intensity);

      int repeats = 0;
      ctype current = std::numeric_limits<ctype>::max();

      for (int x = 0; x < x_dim; ++x) {
        ctype c = 0;
        for (int i = 0; i < nsixels; ++i)
          if (data[x+i*x_dim] == intensity)
            c |= 1U<<i;
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
      inline void encode (const ImageType& im, double min, double max, int cmap_size, int y0)
      {
        const int nsixels = std::min (im.height()-y0, 6);
        std::vector<ctype> data (nsixels*im.width());
        for (int y = 0; y < nsixels; ++y)
          for (int x = 0; x < im.width(); ++x)
            data[x+y*im.width()] = clamp (cmap_size*(im(x,y0+y)-min)/(max-min), cmap_size);

        bool first = true;
        for (ctype intensity = 0; intensity < cmap_size; ++intensity) {
          for (const auto& x : data) {
            if (x == intensity) {
              if (first) first = false;
              else std::cout.put('$');
              encode (data, im.width(), nsixels, intensity);
              break;
            }
          }
        }
        std::cout.put('-');
      }

  }

  inline ColourMap gray (int number)
  {
    ColourMap cmap (number);
    for (int n = 0; n < number; ++n) {
      ctype c = clamp_col (n, number);
      cmap[n] = { c, c, c };
    }
    return cmap;
  }


  inline ColourMap hot (int number)
  {
    ColourMap cmap (number);
    for (int n = 0; n < number; ++n) {
      cmap[n] = {
        clamp_col (3*n, number),
        clamp_col (3*n-number, number),
        clamp_col (3*n-2*number, number)
      };
    }
    return cmap;
  }


  inline ColourMap jet (int number)
  {
    ColourMap cmap (number);
    for (int n = 0; n < number; ++n) {
      cmap[n] = {
        clamp_col (1.5*number-std::abs(4*n-3*number), number),
        clamp_col (1.5*number-std::abs(4*n-2*number), number),
        clamp_col (1.5*number-std::abs(4*n-1*number), number)
      };
    }
    return cmap;
  }



  template <class ImageType>
    inline void display (const ImageType& image, double min, double max, const ColourMap& cmap)
    {
      std::cout << "\033Pq" << colourmap_specifier (cmap);
      for (int y = 0; y < image.height(); y += 6)
        encode (image, min, max, cmap.size(), y);
      std::cout << "\033\\";
    }



}

#endif
