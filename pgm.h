#ifndef __PGM_H__
#define __PGM_H__

#include <fstream>
#include <string>
#include <exception>
#include <limits>

#include "image.h"


// Simple function to load an ascii-encoded PGM grayscale image.
// This returns an object of type Image<ValueType>

template <typename ValueType>
Image<ValueType> load_pgm (const std::string& pgm_filename)
{
  std::ifstream in (pgm_filename);
  if (!in)
    throw std::runtime_error ("failed to open input PGM file \"" + pgm_filename + "\"");

  std::string magic;
  in >> magic;
  if (magic != "P2")
    throw std::runtime_error ("input file \"" + pgm_filename + "\" is not in expected PGM format");

  int xdim, ydim, maxval;
  in >> xdim >> ydim >> maxval;

  if (maxval >= 65536)
    throw std::runtime_error ("PGM file \"" + pgm_filename + "\" is badly formed: maxval exceeds 65536");
  if (maxval <= 0)
    throw std::runtime_error ("PGM file \"" + pgm_filename + "\" is badly formed: maxval lower than or equal to zero");

  if (maxval > std::numeric_limits<ValueType>::max())
    throw std::runtime_error ("maximum intensity in PGM file \"" + pgm_filename + "\" exceeds range of data type used");

  Image<ValueType> im (xdim, ydim);

  for (int y = 0; y < im.height(); ++y) {
    for (int x = 0; x < im.width(); ++x) {
      // casting from double necessary to avoid issues when ValueType is 'unsigned char'
      // as extracting these types from stream converts individual characters,
      // not full ASCII-encoded integers.
      double val;
      in >> val;
      im(x,y) = static_cast<ValueType> (val);
    }
  }

  return im;
}

#endif
