#ifndef __PGM_H__
#define __PGM_H__

#include <fstream>
#include <string>
#include <exception>
#include <cmath>

#include "image.h"

template <typename ValueType>
Image<ValueType> load_pgm (const std::string& pgm_filename)
{
  std::ifstream in (pgm_filename);
  if (!in)
    throw std::runtime_error ("failed to open input PGM file \"" + pgm_filename + "\"");

  std::string magic;
  in >> magic;
  if (magic != "P2")
    throw std::runtime_error ("iut file \"" + pgm_filename + "\" is not in expected PGM format");

  int xdim, ydim, maxval;
  in >> xdim >> ydim >> maxval;

  Image<ValueType> im (xdim, ydim);

  for (int y = 0; y < im.height(); ++y)
    for (int x = 0; x < im.width(); ++x)
      in >> im(x,y);

  return im;
}

#endif
