#ifndef __PLOT_H__
#define __PLOT_H__

#include <algorithm>
#include <iomanip>

#include "sixel.h"
#include "image.h"
#include "font.h"

namespace Sixel {

  // **************************************************************************
  //                       public interface:
  // **************************************************************************

  class Plot {
    public:
      Plot (int width, int height, int font_size = 6, bool show_on_destruct = false);
      ~Plot ();

      Plot& reset();
      Plot& show();

      Plot& set_colourmap (const Sixel::ColourMap& colourmap);

      Plot& add_line (float x0, float y0, float x1, float y1, int colour_index = 2, int stiple = 0, float stiple_frac = 0.5);

      template <class VerticesType>
        Plot& add_line (const VerticesType& y, int colour_index = 2, int stiple = 0, float stiple_frac = 0.5);

      template <class VerticesType>
        Plot& add_line (const VerticesType& x, const VerticesType& y, int colour_index = 2, int stiple = 0, float stiple_frac = 0.5);

      Plot& add_text (const std::string& text, float x, float y, float anchor_x = 0.5, float anchor_y = 0.5, int colour_index = 1);

      Plot& set_xlim (float min, float max);
      Plot& set_ylim (float min, float max);
      Plot& set_grid (float x_interval, float y_interval);

    private:
      const bool show_on_destruct;
      const Font font;
      Image<Sixel::ctype> canvas;
      Sixel::ColourMap cmap;
      std::array<float,2> xlim, ylim;
      float xgrid, ygrid;
      int margin_x, margin_y;

      template <class ImageType>
        static void line_x (ImageType& canvas, float x0, float y0, float x1, float y1, int colour_index, int stiple, float stiple_frac);

      float mapx (float x) const;
      float mapy (float y) const;
  };

  // convenience function to use for immediate rendering:
  inline Plot plot (int width, int height, int font_size = 6) { return Plot (width, height, font_size, true); }








  // **************************************************************************
  //                   imlementation details below:
  // **************************************************************************



  template <class ImageType>
    inline void Plot::line_x (ImageType& canvas, float x0, float y0, float x1, float y1, int colour_index, int stiple, float stiple_frac)
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
          if ( x%stiple >= stiple_frac*stiple)
            continue;
        int y = std::round (y0 + y_range*(x-x0)/x_range);
        if (y >= 0 && y < canvas.height())
          canvas(x,y) = colour_index;
      }
    }


  inline Plot::Plot (int width, int height, int font_size, bool show_on_destruct) :
    show_on_destruct (show_on_destruct),
    font (Font::get_font_for_size (font_size)),
    canvas (width, height),
    cmap ({
        {   0,   0,   0 },
        { 100, 100, 100 },
        { 100, 100,   0 },
        { 100,   0, 100 },
        {   0, 100, 100 },
        { 100,   0,   0 },
        {   0, 100,   0 },
        {   0,   0, 100 }
        })
  {
    margin_x = 5*font.width();
    margin_y = 2*font.height();
    reset();
  }

  inline Plot::~Plot ()
  {
    if (show_on_destruct)
      show();
  }

  inline Plot& Plot::reset ()
  {
    xlim = { NAN, NAN };
    ylim = { NAN, NAN };
    xgrid = ygrid = NAN;
    return *this;
  }

  inline Plot& Plot::show()
  {
    if (std::isfinite (xgrid)) {
      for (float x = xgrid*std::ceil (xlim[0]/xgrid); x < xlim[1]; x += xgrid) {
        add_line (x, ylim[0], x, ylim[1], 1, ( x == 0.0 ? 0 : 10 ), 0.1);
        if (margin_y) {
          std::stringstream legend;
          legend << std::setprecision (3) << x;
          add_text (legend.str(), x, ylim[0], 0.5, 1.5);
        }
      }
    }
    if (std::isfinite (ygrid)) {
      for (float y = ygrid*std::ceil (ylim[0]/ygrid); y < ylim[1]; y += ygrid) {
        add_line (xlim[0], y, xlim[1], y, 1, ( y == 0.0 ? 0 : 10 ), 0.1);
        if (margin_x) {
          std::stringstream legend;
          legend << std::setprecision (3) << y << " ";
          add_text (legend.str(), xlim[0], y, 1.0, 0.5);
        }
      }
    }

    Sixel::imshow (canvas, cmap);

    return *this;
  }

  inline Plot& Plot::set_colourmap (const Sixel::ColourMap& colourmap)
  {
    cmap = colourmap;
    return *this;
  }

  inline Plot& Plot::set_xlim (float min, float max)
  {
    if (std::isfinite (xlim[0]) || std::isfinite (xlim[1])) {
      std::cerr << "WARNING: xlim already set (maybe implicitly by previous calls - call set_xlim() before any draw calls" << std::endl;
    }
    else {
      xlim[0] = min;
      xlim[1] = max;
      if (!std::isfinite (xgrid))
        xgrid = (xlim[1] - xlim[0])/5.0;
    }
    return *this;
  }

  inline Plot& Plot::set_ylim (float min, float max)
  {
    if (std::isfinite (ylim[0]) || std::isfinite (ylim[1])) {
      std::cerr << "WARNING: ylim already set (maybe implicitly by previous calls - call set_ylim() before any draw calls" << std::endl;
    }
    else {
      ylim[0] = min;
      ylim[1] = max;
      if (!std::isfinite (ygrid))
        ygrid = (ylim[1] - ylim[0])/5.0;
    }
    return *this;
  }

  inline Plot& Plot::set_grid (float x_interval, float y_interval)
  {
    xgrid = x_interval;
    ygrid = y_interval;
    return *this;
  }

  inline Plot& Plot::add_line (float x0, float y0, float x1, float y1, int colour_index, int stiple, float stiple_frac)
  {
    if (!std::isfinite (xlim[0]) || !std::isfinite (xlim[1]))
      set_xlim (std::min (x0, x1), std::max (x0, x1));

    if (!std::isfinite (ylim[0]) || !std::isfinite (ylim[1]))
      set_ylim (std::min (y0, y1), std::max (y0, y1));

    x0 = mapx (x0);
    y0 = mapy (y0);
    x1 = mapx (x1);
    y1 = mapy (y1);

    if (std::abs (x1-x0) < std::abs (y1-y0)) {
      struct TransposeCanvas {
        Image<Sixel::ctype>& canvas;
        int margin_y;
        int width () const { return canvas.height(); }
        int height () const { return canvas.width(); }
        decltype(canvas(0,0)) operator() (int x, int y) { return canvas(y,x); }
      } transposed = { canvas, margin_y };
      line_x (transposed, y0, x0, y1, x1, colour_index, stiple, stiple_frac);
    }
    else
      line_x (canvas, x0, y0, x1, y1, colour_index, stiple, stiple_frac);

    return *this;
  }


  template <class VerticesType>
    inline Plot& Plot::add_line (const VerticesType& y, int colour_index, int stiple, float stiple_frac)
    {
      if (!std::isfinite (xlim[0]) || !std::isfinite (xlim[1]))
        set_xlim (0, y.size()-1);

      if (!std::isfinite (ylim[0]) || !std::isfinite (ylim[1]))
        set_ylim (*std::min_element (y.begin(), y.end()), *std::max_element (y.begin(), y.end()));

      for (int n = 0; n < y.size()-1; ++n)
        add_line (n, y[n], n+1, y[n+1], colour_index, stiple, stiple_frac);

      return *this;
    }


  template <class VerticesType>
    inline Plot& Plot::add_line (const VerticesType& x, const VerticesType& y, int colour_index, int stiple, float stiple_frac)
    {
      if (x.size() != y.size())
        throw std::runtime_error ("number of x & y vertices do not match");

      if (!std::isfinite (xlim[0]) || !std::isfinite (xlim[1]))
        set_xlim (*std::min_element (x.begin(), x.end()), *std::max_element (x.begin(), x.end()));

      if (!std::isfinite (ylim[0]) || !std::isfinite (ylim[1]))
        set_ylim (*std::min_element (y.begin(), y.end()), *std::max_element (y.begin(), y.end()));

      for (int n = 0; n < x.size()-1; ++n)
        add_line (x[n], y[n], x[n+1], y[n+1], colour_index, stiple, stiple_frac);

      return *this;
    }




  Plot& Plot::add_text (const std::string& text, float x, float y, float anchor_x, float anchor_y, int colour_index)
  {
    const int text_width = font.width() * text.size();
    int posx = std::round (mapx (x) - anchor_x * text_width);
    int posy = std::round (mapy (y) - (1.0-anchor_y) * font.height());

    for (int n = 0; n < text.size(); ++n) {
      font.render (canvas, text[n], posx+n*font.width(), posy, colour_index);
    }

    return *this;
  }




  inline float Plot::mapx (float x) const
  {
    return margin_x + (canvas.width()-margin_x) * (x-xlim[0])/(xlim[1]-xlim[0]);
  }


  inline float Plot::mapy (float y) const
  {
    return (canvas.height()-margin_y) * (1.0 - (y-ylim[0])/(ylim[1]-ylim[0]));
  }

}

#endif
