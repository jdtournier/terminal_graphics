/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * (c) 2024 J-Donald Tournier (jdtournier@gmail.com)
 *
 * With inspiration from:
 * - for sixel protocol: https://vt100.net/shuford/terminal/all_about_sixels.txt
 */

#ifndef __TERMINAL_GRAPHICS_H__
#define __TERMINAL_GRAPHICS_H__


#include <iostream>
#include <string>
#include <sstream>
#include <format>
#include <vector>
#include <array>
#include <limits>
#include <cmath>
#include <iomanip>
#include <algorithm>
#include <stdexcept>


/**
 * \mainpage
 *
 * This header provides functionality for producing simple graphics on the
 * terminal.
 *
 * NOTE: the graphics are produced using the sixel protocol, which is not
 * supported by many terminals. At the time of writing, the following terminals
 * are known to have the necessary capabilities:
 *
 * - Linux: WezTerm, mlTerm, xterm
 * - macOS: iTerm2
 * - Windows:minTTY
 *
 * To use in your code, place this file alongside your own code, and \#include
 * the file where necessary:
 *
 *     #include "terminal_graphics.h"
 *
 * At this point, you can make use of the functions. All functions are enclosed
 * in the TG namespace
 *
 * The main functions of interest are:
 *
 * - TG::imshow()  display a scalar image.
 * - TG::plot()    display a simple line plot for the data supplied
 *
 * Refer to the example code in `demo.cpp` (reproduced below) to see how to use
 * this functionality:
 *
 * \include demo.cpp
 */



/**
 * The namespace within which all functionality is placed
 */
namespace TG {

  // the data type to use to store intensities:
  using ctype = unsigned char;

  /**
   * The data structure used to hold a colourmap
   *
   * A ColourMap is a way to associate an index with a colour. It can be
   * represented as a simple table with 3 columns per row, to represent the
   * red, green & blue components of each colour, one colour per row. When in
   * use, the colour to be used is retrieved by looking up the values in the
   * row at the specified index.
   *
   * Since this is a simple structure, we use existing C++ objects to store
   * this information, as a (dynamically-sized) vector of (fixed-sized) arrays
   * of 3 values. For convenience, we define the shorthand name 'ColourMap' for
   * this specific type.
   *
   * Note that the sixel protocol we rely on in this implementation expects
   * colourmap intensity values between 0 & 100.
   *
   * Defining a simple colourmap can be done by list initialisation, for
   * example:
   *
   *     const ColourMap my_cmap = {
   *                    { 0, 0, 100 },     // pure red
   *                    { 0, 100, 0 },     // pure green
   *                    { 100, 0, 0 }      // pure blue
   *                };
   *
   * More complex colourmaps can be generated if required, see implementation
   * of the gray() colourmap for inspiration.
   */
  using ColourMap = std::vector<std::array<ctype,3>>;

  //* convenience function to generate ready-made grayscale colourmap:
  ColourMap gray (int number = 100);


  /**
   * A simple class to hold a 2D image using datatype specified as `ValueType`
   * template parameter
   */
  template <typename ValueType>
    class Image {
      public:
        //! Instantiate an Image with the specified dimensions
        Image (int x_dim, int y_dim);

        //! query image dimensions
        int width () const;
        int height () const;

        //! query or set intensity at coordinates (x,y)
        ValueType& operator() (int x, int y);
        //! query intensity at coordinates (x,y)
        const ValueType& operator() (int x, int y) const;

        //! clear image, setting all intensities to 0
        void clear ();

      private:
        std::vector<ValueType> data;
        const int x_dim, y_dim;
    };



  /**
   * Display an indexed image to the terminal, according to the colourmap supplied.
   *
   * ImageType can be any object that implements the following methods:
   *     int width() const
   *     int height() const
   *     integer_type operator() (int x, int y) const
   *
   * Indexed images contain integer values that correspond to entries in the
   * associated ColourMap. Different image values can have completely different
   * colours, depending on the ColourMap used.
   *
   * The ColourMap must be specified via the cmap argument. See the
   * documentation for ColourMap for details.
   */
  template <class ImageType>
    void imshow (const ImageType& image, const ColourMap& cmap);


  /**
   * Display a scalar image to the terminal, rescaled between (min, max),
   * according to the (optional) colourmap supplied.
   *
   * ImageType can be any object that implements the following methods:
   *     int width() const
   *     int height() const
   *     scalar_type operator() (int x, int y) const
   *         (where scalar_type can be any integer or floating-point type)
   *
   * Note that as for most image formats, the x index rasters from left to
   * right, while the y index rasters from top to bottom.
   *
   * min & max specify how image values map to displayed intensities.
   * Values <= min will render as pure black, while values >= max
   * will render as pure white (assuming the default gray colourmap).
   *
   * A different colourmap can be specified via the cmap argument. See the
   * documentation for ColourMap for details on how to generate different
   * colourmaps if necessary.
   */
  template <class ImageType>
    void imshow (const ImageType& image, double min, double max, const ColourMap& cmap = gray());





  /**
   * A class to hold the information about the font used for text rendering
   *
   * This is should not need to be used directly outside of this file.
   */
  class Font {
    public:
      constexpr Font (int width, int height, const unsigned char * data);

      constexpr int width () const;
      constexpr int height () const;
      bool get (int offset, int x, int y) const;

      template <typename ValueType>
        void render (Image<ValueType>& canvas, char c, int x, int y, int colour_index) const;

      static constexpr const Font get_font_for_size (int size);

    private:
      const int w, h;
      unsigned char const * const data;
  };




  /**
   * A class to provide plotting capabilities
   *
   * This can be used stand-alone, but it is easier to use it via the
   * TH::plot() function, which essentially simply returns an (anonymous) TG::Plot
   * Object (refer to the documentation for TG::plot() for details).
   *
   * This class provides a canvas initialised to the desired size (in pixels),
   * with a default font size (6 pixels wide by default). The
   * `show_on_destruct` argument is `false` by default, but that be set to
   * `true` to ensure the class destructor calls show() when the class goes out
   * of scope (this is what the TG::plot() function relies on).
   *
   * By default, the colourmap is set to:
   *
   * | index |  red  | green | blue  | name     |
   * |:-----:|:-----:|:-----:|:-----:|----------|
   * |   0   |   0   |   0   |   0   | black    |
   * |   1   |  100  |  100  |  100  | white    |
   * |   2   |  100  |  100  |   0   | yellow   |
   * |   3   |  100  |   0   |  100  | magenta  |
   * |   4   |   0   |  100  |  100  | cyan     |
   * |   5   |  100  |   0   |   0   | red      |
   * |   6   |   0   |  100  |   0   | green    |
   * |   7   |   0   |   0   |  100  | blue     |
   *
   * Most methods return a reference to the current instance. This allows them
   * to be daisy-chained like this (again, this is what the TG::plot() syntax
   * relies on):
   *
   *     std::vector<float> data (10);
   *     ...
   *     Plot p (256, 128);
   *     p.set_xlim (0,10).set_ylim (-1,1).add_line(data).show();
   *
   * Many methods accept the following arguments:
   *
   * - `colour_index` specifies the colour to use (default: 2 - yellow)
   * - `stiple` specifies the length of dashes (in pixels) if a dashed line
   *   is desired. Set to zero (the default) for a full line.
   * - `stiple_frac` specific the fraction of the dash interval to be
   *   filled (default: 0.5).
   * */
  class Plot {
    public:
      Plot (int width, int height, int font_size = 6, bool show_on_destruct = false);
      ~Plot ();

      //! clear canvas and reset X & Y limits
      Plot& reset();
      //! display the plot to the terminal
      /** This is to be called after all rendering instructions have been
       * invoked and the plot is ready to be displayed.
       *
       * If the plot has been constructed with `show_on_destruct` set to
       * `true`, this will automatically be invoked by the destructor (this is
       * the default behaviour when using the TG::plot() function).
       */
      Plot& show();

      //! set the colourmap if the default is not appropriate
      Plot& set_colourmap (const ColourMap& colourmap);

      //! add a single line connection point (x0,y0) to (x1,y1).
      /** If the X and/or Y limits have not yet been set (using set_xlim() or
       * set_ylim(), this will automatically set them to 10% wider than the
       * maximum range of the coordinates.
       *
       * See main class description for an explanation of the remaining
       * arguments.
       */
      Plot& add_line (float x0, float y0, float x1, float y1,
          int colour_index = 2, int stiple = 0, float stiple_frac = 0.5);

      //! plot the data series `y` along the x-axis
      /** The input `y` can be any class that provides `.size()` and
       * `operator[]()` methods (e.g. `std::vector`). This will plot the data
       * points at locations (0,y[0]), (1,y[1]), ... , (n,y[n]).
       *
       * If the X and/or Y limits have not yet been set (using set_xlim() or
       * set_ylim(), this will automatically set the X limit to (0,n), and the
       * Y limit to 10% wider than the maximum range of the data in `y`.
       *
       * See main class description for an explanation of the remaining
       * arguments.
       */
      template <class VerticesType>
        Plot& add_line (const VerticesType& y,
            int colour_index = 2, int stiple = 0, float stiple_frac = 0.5);

      //! plot the data series `y` against the data series `x`
      /** The inputs `x` & `y` can be any classes that provides `.size()` and
       * `operator[]()` methods (e.g. `std::vector`). This will plot the data
       * points at locations (x[0],[0]), (x[1],y[1]), ... , (x[n],y[n]).
       *
       * If the X and/or Y limits have not yet been set (using set_xlim() or
       * set_ylim(), these will automatically set them to 10% wider
       * than the maximum range of the data in `x` & `y` respectively.
       *
       * See main class description for an explanation of the remaining
       * arguments.
       */
      template <class VerticesTypeX, class VerticesTypeY>
        Plot& add_line (const VerticesTypeX& x, const VerticesTypeY& y,
            int colour_index = 2, int stiple = 0, float stiple_frac = 0.5);

      //! add text at the location specified
      /** This renders the text in `text` at ithe location (x,y). By default,
       * the text is centred on (x,y), but the location of the 'anchor' can be
       * set using the `anchor_x` & `anchor_y` parameters.
       *
       * See main class description for an explanation of the remaining
       * arguments.
       */
      Plot& add_text (const std::string& text, float x, float y,
          float anchor_x = 0.5, float anchor_y = 0.5, int font_size = 6, int colour_index = 1);

      //! set the range along the x-axis
      /** Note that this can only be done once, and if required, should be
       * invoked before any rendering commands.
       */
      Plot& set_xlim (float min, float max, float expand_by = 0.0);
      //! set the range along the y-axis
      /** Note that this can only be done once, and if required, should be
       * invoked before any rendering commands.
       */
      Plot& set_ylim (float min, float max, float expand_by = 0.0);
      //! set the interval of the gridlines, centered around zero
      Plot& set_grid (float x_interval, float y_interval);

    private:
      const bool show_on_destruct;
      const Font font;
      Image<ctype> canvas;
      ColourMap cmap;
      std::array<float,2> xlim, ylim;
      float xgrid, ygrid;
      int margin_x, margin_y;

      template <class ImageType>
        static void line_x (ImageType& canvas, float x0, float y0, float x1, float y1,
            int colour_index, int stiple, float stiple_frac);

      float mapx (float x) const;
      float mapy (float y) const;
  };

  /** Convenience function to use for immediate rendering
   *
   * This returns a (temporary) Plot object, which methods can be called on,
   * and daisy-chain to achieve the desired series of commands. The
   * Plot::show() method will implicitly be called when the temporary object is
   * destroyed, in other words immediately after the closing semicolon.
   *
   * For example:
   *
   *     std::vector x (10), y(10);
   *     ...
   *     TG::plot (256,128)
   *       .set_ylim (-1,2)
   *       .add_line (x, y, 3)
   *       .set_grid (2, 0.5)
   *       .add_text ("my plot", 5, 2);
   *
   * See methods in TG::Plot for details.
   */
  inline Plot plot (int width, int height, int font_size = 6) { return Plot (width, height, font_size, true); }








  // **************************************************************************
  // **************************************************************************
  //
  //              Implementation details from this point on
  //
  // **************************************************************************
  // **************************************************************************





  // **************************************************************************
  //                   ColourMap implementation
  // **************************************************************************

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







  // **************************************************************************
  //                   Image class implementation
  // **************************************************************************



  template <typename ValueType>
    inline Image<ValueType>::Image (int x_dim, int y_dim) :
      data (x_dim*y_dim, 0),
      x_dim (x_dim),
      y_dim (y_dim) { }

  template <typename ValueType>
    inline int Image<ValueType>::width () const
    {
      return x_dim;
    }

  template <typename ValueType>
    inline int Image<ValueType>::height () const
    {
      return y_dim;
    }

  template <typename ValueType>
    inline ValueType& Image<ValueType>::operator() (int x, int y)
    {
      return data[x+x_dim*y];
    }

  template <typename ValueType>
    inline const ValueType& Image<ValueType>::operator() (int x, int y) const
    {
      return data[x+x_dim*y];
    }


  template <typename ValueType>
    inline void Image<ValueType>::clear ()
    {
      for (auto& x : data)
        x = 0;
    }






  // **************************************************************************
  //                   imshow implementation
  // **************************************************************************




  // functions in anonymous namespace will remain private to this file:
  namespace {

    // helper functions for colourmap handling:

    inline std::string colourmap_specifier (const ColourMap& colours)
    {
      int n = 0;
      std::string specifier;
      for (const auto& c : colours)
        specifier += std::format ("#{};2;{};{};{}",
            n++, static_cast<int>(c[0]), static_cast<int>(c[1]),static_cast<int>(c[2]));
      return specifier;
    }




    inline void commit (std::stringstream& out, ctype current, int repeats)
    {
      if (repeats <=3) {
        for (int n = 0; n < repeats; ++n)
          out.put (63+current);
      }
      else
        out << std::format ("!{}{}", repeats, char(63+current));
    }


    template <class ImageType>
    inline std::string encode_row (const ImageType& im, int y0, int x_dim, int nsixels, const ctype intensity)
    {
      int repeats = 0;
      std::stringstream out;
      ctype current = std::numeric_limits<ctype>::max();

      for (int x = 0; x < x_dim; ++x) {
        ctype c = 0;
        for (int y = 0; y < nsixels; ++y) {
          if (im(x,y+y0) == intensity)
            c |= 1U<<y;
        }
        if (c == current) {
          ++repeats;
          continue;
        }
        commit (out, current, repeats);
        current = c;
        repeats = 1;
      }
      if (current)
        commit (out, current, repeats);

      return out.str();
    }


    template <class ImageType>
      inline void encode (const ImageType& im, int cmap_size, int y0)
      {
        const int nsixels = std::min (im.height()-y0, 6);

        bool first = true;
        for (ctype intensity = 0; intensity < cmap_size; ++intensity) {
          std::string row = encode_row (im, y0, im.width(), nsixels, intensity);
          if (row.size()) {
            if (first) first = false;
            else std::cout.put('$');
            std::cout << std::format ("#{}{}", static_cast<int>(intensity), row);
          }
        }
        std::cout.put('-');
      }

  }







  template <class ImageType>
    inline void imshow (const ImageType& image, const ColourMap& cmap)
    {
      std::cout << "\033Pq" + colourmap_specifier (cmap);
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








  // **************************************************************************
  //                   Plot implementation
  // **************************************************************************


  constexpr float lim_expand_by_factor = 0.1f;

  template <class ImageType>
    inline void Plot::line_x (ImageType& canvas, float x0, float y0, float x1, float y1,
        int colour_index, int stiple, float stiple_frac)
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
    canvas.clear();
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

    imshow (canvas, cmap);

    return *this;
  }

  inline Plot& Plot::set_colourmap (const ColourMap& colourmap)
  {
    cmap = colourmap;
    return *this;
  }

  inline Plot& Plot::set_xlim (float min, float max, float expand_by)
  {
    if (std::isfinite (xlim[0]) || std::isfinite (xlim[1]))
      throw std::runtime_error ("xlim already set (maybe implicitly by previous calls)");

    const float delta = expand_by * (max - min);
    xlim[0] = min - delta;
    xlim[1] = max + delta;
    if (!std::isfinite (xgrid))
      xgrid = (xlim[1] - xlim[0])/5.0;

    return *this;
  }

  inline Plot& Plot::set_ylim (float min, float max, float expand_by)
  {
    if (std::isfinite (ylim[0]) || std::isfinite (ylim[1]))
      throw std::runtime_error ("ylim already set (maybe implicitly by previous calls)");

    const float delta = expand_by * (max - min);
    ylim[0] = min - delta;
    ylim[1] = max + delta;
    if (!std::isfinite (ygrid))
      ygrid = (ylim[1] - ylim[0])/5.0;

    return *this;
  }

  inline Plot& Plot::set_grid (float x_interval, float y_interval)
  {
    xgrid = x_interval;
    ygrid = y_interval;
    return *this;
  }

  inline Plot& Plot::add_line (float x0, float y0, float x1, float y1,
      int colour_index, int stiple, float stiple_frac)
  {
    if (!std::isfinite (xlim[0]) || !std::isfinite (xlim[1]))
      set_xlim (std::min (x0, x1), std::max (x0, x1), lim_expand_by_factor);

    if (!std::isfinite (ylim[0]) || !std::isfinite (ylim[1]))
      set_ylim (std::min (y0, y1), std::max (y0, y1), lim_expand_by_factor);

    x0 = mapx (x0);
    y0 = mapy (y0);
    x1 = mapx (x1);
    y1 = mapy (y1);

    struct CanvasView {
      Image<ctype>& canvas;
      const int x_offset, y_offset;
      const bool transpose;
      int width () const { return transpose ? canvas.height()-y_offset : canvas.width()-x_offset; }
      int height () const { return transpose ? canvas.width()-x_offset : canvas.height()-y_offset; }
      ctype& operator() (int x, int y) { return transpose ? canvas(y+x_offset, x) : canvas(x+x_offset,y); }
    };

    bool transposed = std::abs (x1-x0) < std::abs (y1-y0);
    if (transposed) {
      std::swap (x0, y0);
      std::swap (x1, y1);
    }
    CanvasView view = { canvas, margin_x, margin_y, transposed };
    line_x (view, x0, y0, x1, y1, colour_index, stiple, stiple_frac);

    return *this;
  }


  template <class VerticesType>
    inline Plot& Plot::add_line (const VerticesType& y,
        int colour_index, int stiple, float stiple_frac)
    {
      if (!std::isfinite (xlim[0]) || !std::isfinite (xlim[1]))
        set_xlim (0, y.size()-1, 0.0);

      if (!std::isfinite (ylim[0]) || !std::isfinite (ylim[1]))
        set_ylim (*std::min_element (y.begin(), y.end()), *std::max_element (y.begin(), y.end()), lim_expand_by_factor);

      for (std::size_t n = 0; n < y.size()-1; ++n)
        add_line (n, y[n], n+1, y[n+1], colour_index, stiple, stiple_frac);

      return *this;
    }


  template <class VerticesTypeX, class VerticesTypeY>
    inline Plot& Plot::add_line (const VerticesTypeX& x, const VerticesTypeY& y,
        int colour_index, int stiple, float stiple_frac)
    {
      if (x.size() != y.size())
        throw std::runtime_error ("number of x & y vertices do not match");

      if (!std::isfinite (xlim[0]) || !std::isfinite (xlim[1]))
        set_xlim (*std::min_element (x.begin(), x.end()), *std::max_element (x.begin(), x.end()), lim_expand_by_factor);

      if (!std::isfinite (ylim[0]) || !std::isfinite (ylim[1]))
        set_ylim (*std::min_element (y.begin(), y.end()), *std::max_element (y.begin(), y.end()), lim_expand_by_factor);

      for (std::size_t n = 0; n < x.size()-1; ++n)
        add_line (x[n], y[n], x[n+1], y[n+1], colour_index, stiple, stiple_frac);

      return *this;
    }




  Plot& Plot::add_text (const std::string& text, float x, float y,
      float anchor_x, float anchor_y, int font_size, int colour_index)
  {
    auto f = Font::get_font_for_size (font_size);
    const int text_width = f.width() * text.size();
    int posx = std::round (margin_x + mapx (x) - anchor_x * text_width);
    int posy = std::round (mapy (y) - (1.0-anchor_y) * f.height());

    for (std::size_t n = 0; n < text.size(); ++n) {
      f.render (canvas, text[n], posx+n*f.width(), posy, colour_index);
    }

    return *this;
  }




  inline float Plot::mapx (float x) const
  {
    return (canvas.width()-margin_x) * (x-xlim[0])/(xlim[1]-xlim[0]);
  }


  inline float Plot::mapy (float y) const
  {
    return (canvas.height()-margin_y) * (1.0 - (y-ylim[0])/(ylim[1]-ylim[0]));
  }









  // **************************************************************************
  //                   Font imlementation
  // **************************************************************************


  inline constexpr Font::Font (int width, int height, const unsigned char * data) :
    w (width), h (height), data (data) { }

  inline constexpr int Font::width() const { return w; }
  inline constexpr int Font::height() const { return h; }

  inline bool Font::get (int offset, int x, int y) const
  {
    offset = w*(y+h*offset) + x;
    int index = offset/8;
    int shift = offset-index*8;
    return data[index] & (1U<<shift);
  }

  template <typename ValueType>
    inline void Font::render (Image<ValueType>& canvas, char c, int x, int y, int colour_index) const
    {

      static const char mapping [] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
        33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
        51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68,
        69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86,
        87, 88, 89, 90, 91, 92, 93, 94, 0
      };
      if (c >= 128)
        c = 63;

      c = mapping[static_cast<int>(c)];
      for (int j = std::max (0,-y); j < h - std::max (0,y+h-canvas.height()); ++j)
        for (int i = std::max (0,-x); i < w - std::max (0,w+x-canvas.width()); ++i)
          if (get(c,i,j))
            canvas(x+i,y+j) = colour_index;

    }



  namespace {

    static inline constexpr const unsigned char Spleen6x12_data[] =
    { 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 65, 16, 4, 65, 0, 4, 0, 0,
      128, 162, 40, 0, 0, 0, 0, 0, 0,
      0, 160, 124, 138, 162, 124, 10, 0, 0,
      132, 87, 20, 14, 69, 81, 15, 1, 0,
      0, 36, 41, 8, 65, 73, 2, 0, 0,
      0, 35, 73, 140, 145, 70, 46, 0, 0,
      0, 65, 16, 0, 0, 0, 0, 0, 0,
      24, 33, 8, 130, 32, 8, 4, 6, 0,
      6, 2, 65, 16, 4, 65, 136, 1, 0,
      0, 0, 72, 204, 207, 72, 0, 0, 0,
      0, 0, 16, 196, 71, 16, 0, 0, 0,
      0, 0, 0, 0, 0, 16, 132, 0, 0,
      0, 0, 0, 192, 7, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 4, 0, 0,
      16, 132, 32, 4, 33, 8, 65, 0, 0,
      128, 19, 101, 213, 20, 69, 14, 0, 0,
      0, 97, 16, 4, 65, 16, 14, 0, 0,
      128, 19, 65, 144, 19, 4, 31, 0, 0,
      128, 19, 65, 12, 4, 69, 14, 0, 0,
      64, 16, 36, 73, 242, 33, 8, 0, 0,
      192, 23, 4, 15, 4, 65, 15, 0, 0,
      128, 19, 4, 79, 20, 69, 14, 0, 0,
      192, 23, 65, 8, 65, 16, 4, 0, 0,
      128, 19, 69, 78, 20, 69, 14, 0, 0,
      128, 19, 69, 145, 7, 65, 14, 0, 0,
      0, 0, 0, 4, 0, 0, 4, 0, 0,
      0, 0, 0, 4, 0, 16, 132, 0, 0,
      0, 132, 16, 130, 64, 32, 16, 0, 0,
      0, 0, 0, 31, 240, 1, 0, 0, 0,
      128, 64, 32, 16, 132, 16, 2, 0, 0,
      128, 19, 65, 8, 65, 0, 4, 0, 0,
      128, 19, 69, 93, 215, 5, 30, 0, 0,
      128, 19, 69, 209, 23, 69, 17, 0, 0,
      192, 19, 69, 79, 20, 69, 15, 0, 0,
      128, 23, 4, 65, 16, 4, 30, 0, 0,
      192, 19, 69, 81, 20, 69, 15, 0, 0,
      128, 23, 4, 79, 16, 4, 30, 0, 0,
      128, 23, 4, 79, 16, 4, 1, 0, 0,
      128, 23, 4, 93, 20, 69, 30, 0, 0,
      64, 20, 69, 95, 20, 69, 17, 0, 0,
      128, 67, 16, 4, 65, 16, 14, 0, 0,
      128, 67, 16, 4, 65, 16, 3, 0, 0,
      64, 20, 37, 71, 18, 69, 17, 0, 0,
      64, 16, 4, 65, 16, 4, 30, 0, 0,
      64, 180, 125, 85, 20, 69, 17, 0, 0,
      64, 52, 77, 85, 149, 101, 17, 0, 0,
      128, 19, 69, 81, 20, 69, 14, 0, 0,
      192, 19, 69, 209, 19, 4, 1, 0, 0,
      128, 19, 69, 81, 20, 69, 14, 6, 0,
      192, 19, 69, 209, 19, 69, 17, 0, 0,
      128, 23, 4, 14, 4, 65, 15, 0, 0,
      192, 71, 16, 4, 65, 16, 4, 0, 0,
      64, 20, 69, 81, 20, 69, 30, 0, 0,
      64, 20, 69, 81, 20, 57, 14, 0, 0,
      64, 20, 69, 81, 245, 109, 17, 0, 0,
      64, 20, 41, 132, 18, 69, 17, 0, 0,
      64, 20, 69, 145, 7, 65, 15, 0, 0,
      192, 7, 33, 132, 16, 4, 31, 0, 0,
      158, 32, 8, 130, 32, 8, 130, 7, 0,
      65, 32, 8, 4, 129, 32, 16, 4, 0,
      30, 4, 65, 16, 4, 65, 144, 7, 0,
      0, 161, 68, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 192, 7, 0,
      128, 64, 32, 0, 0, 0, 0, 0, 0,
      0, 0, 56, 144, 23, 69, 30, 0, 0,
      64, 16, 60, 81, 20, 69, 15, 0, 0,
      0, 0, 120, 65, 16, 4, 30, 0, 0,
      0, 4, 121, 81, 20, 69, 30, 0, 0,
      0, 0, 120, 81, 244, 5, 30, 0, 0,
      0, 39, 8, 143, 32, 8, 2, 0, 0,
      0, 0, 120, 81, 20, 69, 14, 244, 0,
      64, 16, 60, 81, 20, 69, 17, 0, 0,
      0, 1, 24, 4, 65, 16, 12, 0, 0,
      0, 2, 32, 8, 130, 32, 136, 1, 0,
      64, 16, 36, 197, 80, 36, 17, 0, 0,
      128, 32, 8, 130, 32, 8, 12, 0, 0,
      0, 0, 60, 85, 85, 69, 17, 0, 0,
      0, 0, 60, 81, 20, 69, 17, 0, 0,
      0, 0, 56, 81, 20, 69, 14, 0, 0,
      0, 0, 60, 81, 20, 69, 79, 16, 4,
      0, 0, 120, 81, 20, 69, 30, 4, 65,
      0, 0, 120, 81, 16, 4, 1, 0, 0,
      0, 0, 120, 129, 3, 65, 15, 0, 0,
      128, 32, 28, 130, 32, 8, 12, 0, 0,
      0, 0, 68, 81, 20, 69, 30, 0, 0,
      0, 0, 68, 81, 20, 41, 4, 0, 0,
      0, 0, 68, 81, 245, 109, 17, 0, 0,
      0, 0, 68, 145, 227, 68, 17, 0, 0,
      0, 0, 68, 81, 20, 69, 30, 4, 61,
      0, 0, 124, 16, 66, 8, 31, 0, 0,
      24, 65, 16, 195, 64, 16, 4, 6, 0,
      4, 65, 16, 4, 65, 16, 4, 1, 0,
      6, 130, 32, 48, 140, 32, 136, 1, 0,
      0, 0, 0, 128, 212, 0, 0, 0, 0,
    };


    static inline constexpr const unsigned char Spleen8x16_data [] =
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 24, 24, 24, 24, 24, 24, 24, 0, 24, 24, 0, 0, 0, 0,
      0, 102, 102, 102, 102, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 54, 54, 127, 54, 54, 54, 54, 127, 54, 54, 0, 0, 0, 0,
      0, 8, 126, 11, 11, 11, 62, 104, 104, 104, 104, 63, 8, 0, 0, 0,
      0, 0, 96, 102, 54, 48, 24, 24, 12, 108, 102, 6, 0, 0, 0, 0,
      0, 0, 28, 54, 54, 54, 28, 14, 91, 51, 51, 94, 0, 0, 0, 0,
      0, 24, 24, 24, 24, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 112, 24, 12, 12, 6, 6, 6, 6, 12, 12, 24, 112, 0, 0, 0,
      0, 14, 24, 48, 48, 96, 96, 96, 96, 48, 48, 24, 14, 0, 0, 0,
      0, 0, 0, 0, 102, 60, 24, 255, 24, 60, 102, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 24, 24, 126, 24, 24, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24, 24, 12, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 126, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24, 24, 0, 0, 0, 0,
      0, 96, 96, 48, 48, 24, 24, 12, 12, 6, 6, 3, 3, 0, 0, 0,
      0, 0, 62, 99, 99, 115, 123, 111, 103, 99, 99, 62, 0, 0, 0, 0,
      0, 0, 24, 28, 30, 26, 24, 24, 24, 24, 24, 126, 0, 0, 0, 0,
      0, 0, 62, 99, 96, 96, 48, 24, 12, 6, 99, 127, 0, 0, 0, 0,
      0, 0, 62, 99, 96, 96, 60, 96, 96, 96, 99, 62, 0, 0, 0, 0,
      0, 0, 3, 3, 51, 51, 51, 51, 127, 48, 48, 48, 0, 0, 0, 0,
      0, 0, 127, 99, 3, 3, 63, 96, 96, 96, 99, 62, 0, 0, 0, 0,
      0, 0, 62, 99, 3, 3, 63, 99, 99, 99, 99, 62, 0, 0, 0, 0,
      0, 0, 127, 99, 96, 96, 48, 24, 12, 12, 12, 12, 0, 0, 0, 0,
      0, 0, 62, 99, 99, 99, 62, 99, 99, 99, 99, 62, 0, 0, 0, 0,
      0, 0, 62, 99, 99, 99, 99, 126, 96, 96, 99, 62, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 24, 24, 0, 0, 0, 24, 24, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 24, 24, 0, 0, 0, 24, 24, 12, 0, 0, 0,
      0, 0, 96, 48, 24, 12, 6, 6, 12, 24, 48, 96, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 126, 0, 0, 126, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 6, 12, 24, 48, 96, 96, 48, 24, 12, 6, 0, 0, 0, 0,
      0, 0, 62, 99, 96, 48, 24, 12, 12, 0, 12, 12, 0, 0, 0, 0,
      0, 0, 0, 62, 67, 91, 91, 91, 91, 123, 3, 62, 0, 0, 0, 0,
      0, 0, 62, 99, 99, 99, 127, 99, 99, 99, 99, 99, 0, 0, 0, 0,
      0, 0, 63, 99, 99, 99, 63, 99, 99, 99, 99, 63, 0, 0, 0, 0,
      0, 0, 126, 3, 3, 3, 3, 3, 3, 3, 3, 126, 0, 0, 0, 0,
      0, 0, 63, 99, 99, 99, 99, 99, 99, 99, 99, 63, 0, 0, 0, 0,
      0, 0, 126, 3, 3, 3, 31, 3, 3, 3, 3, 126, 0, 0, 0, 0,
      0, 0, 126, 3, 3, 3, 31, 3, 3, 3, 3, 3, 0, 0, 0, 0,
      0, 0, 126, 3, 3, 3, 123, 99, 99, 99, 99, 126, 0, 0, 0, 0,
      0, 0, 99, 99, 99, 99, 127, 99, 99, 99, 99, 99, 0, 0, 0, 0,
      0, 0, 126, 24, 24, 24, 24, 24, 24, 24, 24, 126, 0, 0, 0, 0,
      0, 0, 126, 24, 24, 24, 24, 24, 24, 24, 24, 15, 0, 0, 0, 0,
      0, 0, 99, 99, 99, 51, 31, 51, 99, 99, 99, 99, 0, 0, 0, 0,
      0, 0, 3, 3, 3, 3, 3, 3, 3, 3, 3, 126, 0, 0, 0, 0,
      0, 0, 99, 119, 127, 107, 99, 99, 99, 99, 99, 99, 0, 0, 0, 0,
      0, 0, 99, 99, 103, 103, 107, 107, 115, 115, 99, 99, 0, 0, 0, 0,
      0, 0, 62, 99, 99, 99, 99, 99, 99, 99, 99, 62, 0, 0, 0, 0,
      0, 0, 63, 99, 99, 99, 63, 3, 3, 3, 3, 3, 0, 0, 0, 0,
      0, 0, 62, 99, 99, 99, 99, 99, 99, 107, 107, 62, 24, 48, 0, 0,
      0, 0, 63, 99, 99, 99, 63, 99, 99, 99, 99, 99, 0, 0, 0, 0,
      0, 0, 126, 3, 3, 3, 62, 96, 96, 96, 96, 63, 0, 0, 0, 0,
      0, 0, 255, 24, 24, 24, 24, 24, 24, 24, 24, 24, 0, 0, 0, 0,
      0, 0, 99, 99, 99, 99, 99, 99, 99, 99, 99, 126, 0, 0, 0, 0,
      0, 0, 99, 99, 99, 99, 99, 99, 99, 54, 28, 8, 0, 0, 0, 0,
      0, 0, 99, 99, 99, 99, 99, 99, 107, 127, 119, 99, 0, 0, 0, 0,
      0, 0, 99, 99, 99, 54, 28, 54, 99, 99, 99, 99, 0, 0, 0, 0,
      0, 0, 99, 99, 99, 99, 126, 96, 96, 96, 96, 63, 0, 0, 0, 0,
      0, 0, 127, 96, 96, 48, 24, 12, 6, 3, 3, 127, 0, 0, 0, 0,
      0, 124, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 124, 0, 0, 0,
      0, 3, 3, 6, 6, 12, 12, 24, 24, 48, 48, 96, 96, 0, 0, 0,
      0, 62, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 62, 0, 0, 0,
      0, 8, 28, 54, 99, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0,
      0, 12, 24, 48, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 62, 96, 126, 99, 99, 99, 126, 0, 0, 0, 0,
      0, 0, 3, 3, 3, 63, 99, 99, 99, 99, 99, 63, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 126, 3, 3, 3, 3, 3, 126, 0, 0, 0, 0,
      0, 0, 96, 96, 96, 126, 99, 99, 99, 99, 99, 126, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 126, 99, 99, 127, 3, 3, 126, 0, 0, 0, 0,
      0, 0, 120, 12, 12, 12, 62, 12, 12, 12, 12, 12, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 126, 99, 99, 99, 99, 99, 62, 96, 96, 63, 0,
      0, 0, 3, 3, 3, 63, 99, 99, 99, 99, 99, 99, 0, 0, 0, 0,
      0, 0, 24, 24, 0, 28, 24, 24, 24, 24, 24, 56, 0, 0, 0, 0,
      0, 0, 24, 24, 0, 24, 24, 24, 24, 24, 24, 24, 24, 24, 14, 0,
      0, 0, 3, 3, 3, 51, 27, 15, 15, 27, 51, 99, 0, 0, 0, 0,
      0, 0, 12, 12, 12, 12, 12, 12, 12, 12, 12, 56, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 55, 107, 107, 107, 107, 99, 99, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 63, 99, 99, 99, 99, 99, 99, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 62, 99, 99, 99, 99, 99, 62, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 63, 99, 99, 99, 99, 99, 63, 3, 3, 3, 0,
      0, 0, 0, 0, 0, 126, 99, 99, 99, 99, 99, 126, 96, 96, 96, 0,
      0, 0, 0, 0, 0, 126, 99, 3, 3, 3, 3, 3, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 126, 3, 3, 62, 96, 96, 63, 0, 0, 0, 0,
      0, 0, 12, 12, 12, 62, 12, 12, 12, 12, 12, 120, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 99, 99, 99, 99, 99, 99, 126, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 99, 99, 99, 99, 54, 28, 8, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 99, 99, 107, 107, 107, 107, 118, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 99, 54, 28, 28, 54, 99, 99, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 99, 99, 99, 99, 99, 99, 126, 96, 96, 63, 0,
      0, 0, 0, 0, 0, 127, 96, 48, 24, 12, 6, 127, 0, 0, 0, 0,
      0, 112, 24, 24, 24, 24, 14, 14, 24, 24, 24, 24, 112, 0, 0, 0,
      0, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 0, 0, 0,
      0, 14, 24, 24, 24, 24, 112, 112, 24, 24, 24, 24, 14, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 76, 126, 50, 0, 0, 0, 0, 0, 0, 0
    };




    static inline constexpr const unsigned char Spleen12x24_data [] =
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 96, 0, 6, 96, 0, 6, 96, 0, 6, 96, 0, 6, 96, 0, 6, 96, 0, 0, 0, 0, 6, 96, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 12, 195, 48, 12, 195, 48, 12, 195, 48, 12, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 12, 195, 48, 12, 227, 127, 12, 195, 48, 12, 195, 48, 12, 195, 48, 12, 227, 127, 12, 195, 48, 12, 3, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 96, 0, 6, 248, 199, 6, 102, 96, 6, 102, 96, 6, 108, 128, 31, 96, 3, 102, 96, 6, 102, 96, 6, 54, 254, 1, 6, 96, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 48, 28, 99, 27, 182, 193, 13, 192, 0, 6, 96, 0, 3, 176, 131, 109, 216, 198, 56, 12, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 240, 128, 25, 12, 195, 48, 12, 195, 48, 152, 1, 15, 204, 96, 88, 6, 103, 48, 6, 195, 120, 248, 12, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 96, 0, 6, 96, 0, 6, 96, 0, 6, 96, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 7, 24, 192, 0, 6, 48, 0, 3, 24, 128, 1, 24, 128, 1, 24, 128, 1, 24, 128, 1, 48, 0, 3, 96, 0, 12, 128, 1, 112, 0, 0, 0,
      0, 0, 0, 14, 128, 1, 48, 0, 6, 192, 0, 12, 128, 1, 24, 128, 1, 24, 128, 1, 24, 128, 1, 24, 192, 0, 12, 96, 0, 3, 24, 224, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 131, 25, 240, 0, 6, 254, 7, 6, 240, 128, 25, 12, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 96, 0, 6, 252, 3, 6, 96, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 96, 0, 6, 96, 0, 3, 24, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 252, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 96, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 12, 192, 0, 6, 96, 0, 3, 48, 128, 1, 24, 192, 0, 12, 96, 0, 6, 48, 0, 3, 24, 128, 1, 12, 192, 0, 6, 96, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 248, 193, 48, 6, 102, 96, 6, 103, 120, 198, 102, 102, 54, 230, 97, 14, 102, 96, 6, 198, 48, 248, 1, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 112, 128, 7, 108, 64, 6, 96, 0, 6, 96, 0, 6, 96, 0, 6, 96, 0, 6, 96, 0, 6, 252, 3, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 248, 193, 48, 6, 6, 96, 0, 6, 96, 0, 3, 24, 192, 0, 6, 48, 128, 1, 12, 96, 96, 254, 7, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 248, 193, 48, 6, 6, 96, 0, 6, 48, 240, 1, 48, 0, 6, 96, 0, 6, 96, 6, 198, 48, 248, 1, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 6, 96, 0, 6, 96, 24, 134, 97, 24, 134, 97, 24, 134, 97, 24, 254, 7, 24, 128, 1, 24, 128, 1, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 254, 103, 96, 6, 96, 0, 6, 96, 0, 254, 1, 48, 0, 6, 96, 0, 6, 96, 6, 198, 48, 248, 1, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 248, 195, 96, 6, 96, 0, 6, 96, 0, 254, 97, 48, 6, 102, 96, 6, 102, 96, 6, 198, 48, 248, 1, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 254, 103, 96, 0, 6, 96, 0, 6, 48, 128, 1, 12, 96, 0, 3, 48, 0, 3, 48, 0, 3, 48, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 248, 193, 48, 6, 102, 96, 6, 198, 48, 248, 193, 48, 6, 102, 96, 6, 102, 96, 6, 198, 48, 248, 1, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 248, 193, 48, 6, 102, 96, 6, 102, 96, 6, 198, 96, 248, 7, 96, 0, 6, 96, 0, 102, 48, 252, 1, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 96, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 6, 96, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 96, 0, 6, 0, 0, 0, 0, 0, 0, 96, 0, 6, 96, 0, 3, 24, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 6, 48, 128, 1, 12, 96, 0, 3, 24, 192, 0, 24, 0, 3, 96, 0, 12, 128, 1, 48, 0, 6, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 224, 127, 0, 0, 0, 0, 0, 0, 254, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 12, 128, 1, 48, 0, 6, 192, 0, 24, 0, 3, 96, 0, 3, 24, 192, 0, 6, 48, 128, 1, 12, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 248, 193, 48, 6, 6, 96, 0, 6, 48, 128, 1, 12, 96, 0, 6, 96, 0, 0, 0, 0, 6, 96, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 128, 31, 12, 99, 96, 6, 102, 110, 230, 102, 110, 230, 102, 110, 230, 102, 126, 6, 192, 0, 248, 3, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 248, 193, 48, 6, 102, 96, 6, 102, 96, 6, 230, 127, 6, 102, 96, 6, 102, 96, 6, 102, 96, 6, 6, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 254, 97, 48, 6, 102, 96, 6, 102, 96, 6, 227, 31, 6, 99, 96, 6, 102, 96, 6, 102, 48, 254, 1, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 248, 199, 0, 6, 96, 0, 6, 96, 0, 6, 96, 0, 6, 96, 0, 6, 96, 0, 6, 192, 0, 248, 7, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 254, 97, 48, 6, 102, 96, 6, 102, 96, 6, 102, 96, 6, 102, 96, 6, 102, 96, 6, 102, 48, 254, 1, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 248, 199, 0, 6, 96, 0, 6, 96, 0, 6, 224, 31, 6, 96, 0, 6, 96, 0, 6, 192, 0, 248, 7, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 248, 199, 0, 6, 96, 0, 6, 96, 0, 6, 224, 31, 6, 96, 0, 6, 96, 0, 6, 96, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 248, 199, 0, 6, 96, 0, 6, 96, 0, 6, 96, 124, 6, 102, 96, 6, 102, 96, 6, 198, 96, 248, 7, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 6, 102, 96, 6, 102, 96, 6, 102, 96, 6, 230, 127, 6, 102, 96, 6, 102, 96, 6, 102, 96, 6, 6, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 252, 3, 6, 96, 0, 6, 96, 0, 6, 96, 0, 6, 96, 0, 6, 96, 0, 6, 96, 0, 6, 252, 3, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 252, 3, 6, 96, 0, 6, 96, 0, 6, 96, 0, 6, 96, 0, 6, 96, 0, 6, 96, 0, 7, 62, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 6, 102, 96, 6, 102, 96, 6, 102, 48, 134, 225, 15, 134, 97, 48, 6, 102, 96, 6, 102, 96, 6, 6, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 6, 96, 0, 6, 96, 0, 6, 96, 0, 6, 96, 0, 6, 96, 0, 6, 96, 0, 6, 192, 0, 248, 7, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 6, 230, 112, 158, 103, 111, 102, 102, 96, 6, 102, 96, 6, 102, 96, 6, 102, 96, 6, 102, 96, 6, 6, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 6, 230, 96, 14, 230, 97, 30, 102, 99, 54, 102, 102, 102, 102, 108, 198, 102, 120, 134, 103, 112, 6, 7, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 248, 193, 48, 6, 102, 96, 6, 102, 96, 6, 102, 96, 6, 102, 96, 6, 102, 96, 6, 198, 48, 248, 1, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 254, 97, 48, 6, 102, 96, 6, 102, 96, 6, 227, 31, 6, 96, 0, 6, 96, 0, 6, 96, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 248, 193, 48, 6, 102, 96, 6, 102, 96, 6, 102, 96, 6, 102, 96, 102, 102, 102, 198, 198, 60, 248, 1, 24, 0, 3, 48, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 254, 97, 48, 6, 102, 96, 6, 102, 96, 6, 227, 31, 6, 99, 96, 6, 102, 96, 6, 102, 96, 6, 6, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 248, 199, 0, 6, 96, 0, 6, 96, 0, 12, 128, 31, 0, 3, 96, 0, 6, 96, 0, 6, 48, 254, 1, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 254, 7, 6, 96, 0, 6, 96, 0, 6, 96, 0, 6, 96, 0, 6, 96, 0, 6, 96, 0, 6, 96, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 6, 102, 96, 6, 102, 96, 6, 102, 96, 6, 102, 96, 6, 102, 96, 6, 102, 96, 6, 198, 96, 248, 7, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 6, 102, 96, 6, 102, 96, 6, 102, 96, 6, 102, 96, 6, 102, 96, 12, 195, 48, 152, 1, 15, 96, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 6, 102, 96, 6, 102, 96, 6, 102, 96, 6, 102, 96, 6, 102, 96, 102, 102, 111, 158, 231, 112, 6, 6, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 6, 102, 96, 6, 102, 96, 6, 102, 96, 12, 131, 31, 12, 99, 96, 6, 102, 96, 6, 102, 96, 6, 6, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 6, 102, 96, 6, 102, 96, 6, 102, 96, 12, 134, 127, 0, 6, 96, 0, 6, 96, 0, 6, 112, 254, 3, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 254, 7, 96, 0, 6, 96, 0, 3, 24, 192, 0, 6, 48, 128, 1, 12, 96, 0, 6, 96, 0, 254, 7, 0, 0, 0, 0, 0, 0, 0,
      0, 128, 127, 24, 128, 1, 24, 128, 1, 24, 128, 1, 24, 128, 1, 24, 128, 1, 24, 128, 1, 24, 128, 1, 24, 128, 1, 24, 128, 1, 24, 128, 1, 248, 7, 0,
      0, 0, 0, 3, 48, 0, 6, 96, 0, 12, 192, 0, 24, 128, 1, 48, 0, 3, 96, 0, 6, 192, 0, 12, 128, 1, 24, 0, 3, 48, 0, 6, 96, 0, 0, 0,
      0, 224, 31, 128, 1, 24, 128, 1, 24, 128, 1, 24, 128, 1, 24, 128, 1, 24, 128, 1, 24, 128, 1, 24, 128, 1, 24, 128, 1, 24, 128, 1, 24, 254, 1, 0,
      0, 0, 0, 32, 0, 7, 216, 192, 24, 6, 51, 96, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 254, 7, 0,
      0, 0, 0, 24, 0, 3, 96, 0, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 252, 1, 48, 0, 6, 96, 248, 199, 96, 6, 102, 96, 6, 198, 96, 248, 7, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 6, 96, 0, 6, 96, 0, 254, 97, 48, 6, 102, 96, 6, 102, 96, 6, 102, 96, 6, 102, 48, 254, 1, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 248, 199, 0, 6, 96, 0, 6, 96, 0, 6, 96, 0, 6, 192, 0, 248, 7, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 6, 96, 0, 6, 96, 248, 199, 96, 6, 102, 96, 6, 102, 96, 6, 102, 96, 6, 198, 96, 248, 7, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 248, 199, 96, 6, 102, 96, 6, 230, 127, 6, 96, 0, 6, 192, 0, 248, 7, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 224, 3, 7, 48, 0, 3, 48, 0, 3, 252, 1, 3, 48, 0, 3, 48, 0, 3, 48, 0, 3, 48, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 248, 199, 96, 6, 102, 96, 6, 102, 96, 6, 102, 96, 6, 198, 96, 248, 3, 48, 0, 6, 96, 0, 195, 31,
      0, 0, 0, 0, 0, 0, 6, 96, 0, 6, 96, 0, 254, 97, 48, 6, 102, 96, 6, 102, 96, 6, 102, 96, 6, 102, 96, 6, 6, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 96, 0, 6, 0, 0, 0, 120, 0, 6, 96, 0, 6, 96, 0, 6, 96, 0, 6, 96, 0, 6, 224, 1, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 96, 0, 6, 0, 0, 0, 96, 0, 6, 96, 0, 6, 96, 0, 6, 96, 0, 6, 96, 0, 6, 96, 0, 6, 96, 0, 6, 48, 224, 1,
      0, 0, 0, 0, 0, 0, 12, 192, 0, 12, 192, 0, 12, 195, 48, 140, 193, 12, 124, 192, 6, 204, 192, 24, 12, 195, 96, 12, 6, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 48, 0, 3, 48, 0, 3, 48, 0, 3, 48, 0, 3, 48, 0, 3, 48, 0, 3, 48, 0, 7, 224, 3, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 158, 97, 54, 102, 102, 102, 102, 102, 102, 102, 102, 102, 6, 102, 96, 6, 6, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 254, 97, 48, 6, 102, 96, 6, 102, 96, 6, 102, 96, 6, 102, 96, 6, 6, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 248, 193, 48, 6, 102, 96, 6, 102, 96, 6, 102, 96, 6, 198, 48, 248, 1, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 254, 97, 48, 6, 102, 96, 6, 102, 96, 6, 102, 96, 6, 102, 48, 254, 97, 0, 6, 96, 0, 6, 96, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 248, 199, 96, 6, 102, 96, 6, 102, 96, 6, 102, 96, 6, 198, 96, 248, 7, 96, 0, 6, 96, 0, 6, 96,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 248, 199, 96, 6, 96, 0, 6, 96, 0, 6, 96, 0, 6, 96, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 252, 103, 0, 6, 96, 0, 6, 192, 63, 0, 6, 96, 0, 6, 96, 254, 3, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 48, 0, 3, 48, 0, 3, 252, 1, 3, 48, 0, 3, 48, 0, 3, 48, 0, 3, 48, 0, 7, 224, 3, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 102, 96, 6, 102, 96, 6, 102, 96, 6, 102, 96, 6, 198, 96, 248, 7, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 102, 96, 6, 102, 96, 6, 102, 96, 12, 195, 48, 152, 1, 15, 96, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 102, 96, 6, 102, 102, 102, 102, 102, 102, 102, 102, 102, 198, 102, 152, 7, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 102, 96, 12, 131, 25, 240, 0, 15, 152, 193, 48, 12, 99, 96, 6, 6, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 102, 96, 6, 102, 96, 6, 102, 96, 6, 102, 96, 6, 198, 96, 248, 7, 96, 0, 6, 96, 0, 227, 31,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 254, 7, 96, 0, 3, 24, 192, 0, 6, 48, 128, 1, 12, 96, 0, 254, 7, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 120, 192, 0, 6, 96, 0, 6, 96, 0, 6, 96, 0, 6, 48, 192, 1, 28, 0, 3, 96, 0, 6, 96, 0, 6, 96, 0, 6, 96, 0, 12, 128, 7, 0,
      0, 0, 0, 0, 0, 6, 96, 0, 6, 96, 0, 6, 96, 0, 6, 96, 0, 6, 96, 0, 6, 96, 0, 6, 96, 0, 6, 96, 0, 6, 96, 0, 0, 0, 0, 0,
      0, 192, 3, 96, 0, 12, 192, 0, 12, 192, 0, 12, 192, 0, 12, 128, 1, 112, 0, 7, 24, 192, 0, 12, 192, 0, 12, 192, 0, 12, 192, 0, 6, 60, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 28, 102, 99, 99, 51, 28, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };

  }


  inline constexpr const Font Font::get_font_for_size (int size)
  {
    switch (size) {
      case 0: return { 0, 0, nullptr };
      case 6: return { 6, 12, Spleen6x12_data };
      case 8: return { 8, 16, Spleen8x16_data };
      case 12: return { 12, 24, Spleen12x24_data };
      default: throw std::runtime_error (std::format ("font size {} not supported", size));
    }
  }



}

#endif
