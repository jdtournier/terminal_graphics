#ifndef __PLOT_H__
#define __PLOT_H__

#include <algorithm>

#include "sixel.h"
#include "image.h"


// **************************************************************************
//                       public interface:
// **************************************************************************

class Plot {
  public:
    Plot (int width, int height);
    ~Plot ();

    Plot& add_line (float x0, float y0, float x1, float y1, std::array<Sixel::ctype,3> colour = { 100, 100, 0 }, int stiple = 0);
    Plot& add_line (float x0, float y0, float x1, float y1, int colour_index, int stiple = 0);

    template <class VerticesType>
    Plot& add_line (const VerticesType& x, const VerticesType& y, std::array<Sixel::ctype,3> colour = { 100, 100, 0 }, int stiple = 0);

    Plot& set_xlim (int min, int max);
    Plot& set_ylim (int min, int max);
    Plot& set_xticks (int interval);
    Plot& set_yticks (int interval);

  private:
    Image<Sixel::ctype> canvas;
    Sixel::ColourMap cmap;
    int max_index;

    int get_cmap_index (std::array<Sixel::ctype,3> colour);

    template <class ImageType>
    static void line_x (ImageType& canvas, float x0, float y0, float x1, float y1, int colour_index, int stiple);

};









// **************************************************************************
//                   imlementation details below:
// **************************************************************************

template <class ImageType>
inline void Plot::line_x (ImageType& canvas, float x0, float y0, float x1, float y1, int colour_index, int stiple)
{
  if (x0 > x1) {
    std::swap (x0, x1);
    std::swap (y0, y1);
  }

  float x_range = x1 - x0;
  float y_range = y1 - y0;

  int xmax = std::min (static_cast<int>(x1+1.0f), static_cast<int>(canvas.width()));
  for (int x = std::max (std::round (x0), 0.0f); x < xmax; ++x) {
    if (stiple > 0)
      if ( (x%(2*stiple)) >= stiple)
        continue;
    int y = std::round (y0 + y_range*(x-x0)/x_range);
    if (y >= 0 && y < canvas.height())
      canvas(x,y) = colour_index;
  }
}


inline Plot::Plot (int width, int height) :
  canvas (width, height),
  {
    reset();
  }

inline Plot::~Plot ()
{
  show();
}

inline Plot::show()
{
  if (max_index)
    Sixel::imshow (canvas, cmap);
}

inline Plot::reset ()
{
  max_index = 0;
}



inline Plot& Plot::add_line (float x0, float y0, float x1, float y1, int colour_index, int stiple)
{
  max_index = std::max (max_index, colour_index);

  if (std::abs (x1-x0) < std::abs (y1-y0)) {
    struct TransposeCanvas {
      Image<Sixel::ctype>& canvas;
      int width () const { return canvas.height(); }
      int height () const { return canvas.width(); }
      decltype(canvas(0,0)) operator() (int x, int y) const { return canvas(y,x); }
    } transposed = { canvas };
    line_x (transposed, y0, x0, y1, x1, colour_index, stiple);
  }
  else
    line_x (canvas, x0, y0, x1, y1, colour_index, stiple);

  return *this;
}


template <class VerticesType>
inline Plot& Plot::add_line (const VerticesType& x, const VerticesType& y, int colour_index, int stiple)
{
  if (x.size() != y.size())
    throw std::runtime_error ("number of x & y vertices do not match");

  max_index = std::max (max_index, colour_index);

  for (int n = 0; n < x.size()-1; ++n)
    add_line (x[n], y[n], x[n+1], y[n+1], colour_index, stiple);

  return *this;
}


#endif
