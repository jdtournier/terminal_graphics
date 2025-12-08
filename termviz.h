#pragma once

/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * (c) 2025 J-Donald Tournier (jdtournier@gmail.com)
 *
 * With inspiration from:
 * - for sixel protocol: https://vt100.net/shuford/terminal/all_about_sixels.txt
 */

#include <iostream>
#include <cassert>
#include <array>
#include <limits>
#include <cmath>
#include <vector>
#include <span>
#include <format>
#include <sstream>
#include <iomanip>
#include <variant>






//! The namespace within which all functionality is placed
namespace termviz {

  //! the data type to use to store intensities:
  using ctype = unsigned char;

  //! The data structure used to hold a colourmap
  /**
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

  //! convenience function to generate a ready-made grayscale colourmap
  ColourMap gray (int number = 101);

  //! convenience function to generate a ready-made hot colourmap
  ColourMap hot (int number = 101);

  //! convenience function to generate a ready-made jet colourmap
  ColourMap jet (int number = 101);



  //! VT100 code to set the cursor position to the top left of the screen
  /**
   * This can be used in conjuction with termviz::Clear to provide running updates.
   * Simply feed the string to `std::cout` to issue the instruction, for example:
   * ```
   *   std::cout << termviz::Clear;
   *   while (true) {
   *     std::cout << termviz::Home << "Current progress:\n";
   *     termviz::figure().render_line(...);
   *
   *     ...
   *     // perform computations, etc.
   *     ...
   *   }
   * ```
   * \sa termviz::Clear
   */
  constexpr inline std::string_view Home = "\033[H";

  //! VT100 code to clear the screen
   /** \sa termviz::Home
   */
  constexpr inline std::string_view Clear = "\033[2J";



  //! A simple class to hold a 2D image using datatype specified as `ValueType` template parameter
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




  //! Adapter class to rescale intensities of image to colourmap indices
  /**
   * Rescale intensities of image from (min, max) to the range of indices
   * in the specified colourmap, rounding to the nearest integer index, and
   * clamping the values to the [ min max ] range.
   */
  template <class ImageType>
    class Rescale {
      public:
        Rescale (const ImageType& image, double minval, double maxval, int cmap_size);

        int width () const;
        int height () const;
        ctype operator() (int x, int y) const;

      private:
        const ImageType& im;
        const double min, max;
        const int cmap_size;
    };



  //! Adapter class to magnify an image
  /**
   * This makes the image `factor` bigger than the original.
   */
  template <class ImageType>
    class magnify {
      public:
        magnify (const ImageType& image, int factor);

        int width () const;
        int height () const;
        decltype(std::declval<const ImageType>()(0,0)) operator() (int x, int y) const;

      private:
        const ImageType& im;
        const int factor;
    };





  //! Display an indexed image to the terminal, according to the colourmap supplied.
  /**
   * ImageType can be any object that implements the following methods:
   * - `int width() const`
   * - `int height() const`
   * - `integer_type operator() (int x, int y) const`
   *
   * Indexed images contain integer values that correspond to entries in the
   * associated ColourMap. Different image values can have completely different
   * colours, depending on the ColourMap used.
   *
   * The ColourMap must be specified via the `cmap` argument. See the
   * documentation for ColourMap for details.
   */
  template <class ImageType>
    void imshow (const ImageType& image, const ColourMap& cmap, const bool zero_is_transparent = false);


  //! Display a scalar image to the terminal, rescaled between (min, max)
  /**
   * ImageType can be any object that implements the following methods:
   * - `int width() const`
   * - `int height() const`
   * - `scalar_type operator() (int x, int y) const`
   *   (where scalar_type can be any integer or floating-point type)
   *
   * Note that as for most image formats, the x index rasters from left to
   * right, while the y index rasters from top to bottom.
   *
   * `min` & `max` specify how image values map to displayed intensities.
   * Values <= `min` will render as pure black, while values >= `max`
   * will render as pure white (assuming the default gray colourmap).
   *
   * A different colourmap can be specified via the `cmap` argument. See the
   * documentation for ColourMap for details on how to generate different
   * colourmaps if necessary.
   */
  template <class ImageType>
    void imshow (const ImageType& image, double min, double max, const ColourMap& cmap = gray(), const bool zero_is_transparent = false);




  //! Render a line segment
  /** This renders a straight line element from (x0,y0) to (x1,y1).
   *
   * See main class description for an explanation of the remaining
   * arguments.
   */
  template <typename ImageType>
    void render_line (ImageType& canvas, float x0, float y0, float x1, float y1,
        int colour_index, int stiple = 0, float stiple_frac = 0.5);


  //! Render text at the location specified
  /** This renders the text in `text` at the location (x,y). By default,
   * the text is centred on (x,y), but the location of the 'anchor' can be
   * set using the `anchor_x` & `anchor_y` parameters.
   *
   * See main class description for an explanation of the remaining
   * arguments.
   */
  template <typename ImageType>
    inline void render_text (ImageType& canvas, const std::string& text, float x, float y,
        float anchor_x = 0.5, float anchor_y = 0.5, int colour_index = 1);


  //! A class to hold the information about the font used for text rendering
  /**
   * This is should not need to be used directly outside of this file.
   */
  class Font {
    public:
      constexpr Font (int width, int height, const std::span<const unsigned char>& data);

      constexpr int width () const;
      constexpr int height () const;
      bool get (int offset, int x, int y) const;

      template <typename ImageType>
        void render (ImageType& canvas, char c, int x, int y, int colour_index) const;

      static constexpr const Font get_font (int size = 16);

    private:
      const int w, h;
      const std::span<const unsigned char> data;
  };





  // **************************************************************************
  //                   Plot implementation
  // **************************************************************************

  namespace {

    const ColourMap& get_default_cmap();

    constexpr auto inf = std::numeric_limits<float>::infinity();
    constexpr auto None = std::numeric_limits<float>::quiet_NaN();


    class Figure
    {
      public:
        Figure (int width, int height);
        ~Figure();

        Figure& line (float x0, float y0, float x1, float y1, int colour_index = 2, int stiple = 0, float stiple_frac = 0.5);

        template <typename VectorType>
          Figure& plot (const VectorType& y, int colour_index = 2, int stiple = 0, float stiple_frac = 0.5);

        template <typename VectorTypeX, typename VectorTypeY>
          Figure& plot (const VectorTypeX& x, const VectorTypeY& y, int colour_index = 2, int stiple = 0, float stiple_frac = 0.5);

        Figure& text (const std::string& text, float x, float y, float anchor_x = 0.5, float anchor_y = 0.5, int colour_index = 1);

        Figure& xlim (float x_min, float x_max);
        Figure& ylim (float y_min, float y_max);
        Figure& ticks (float x_spacing, float y_spacing);
        Figure& grid (bool show_xgrid, bool show_ygrid);
        Figure& colourmap (const ColourMap& cmap);
        Figure& transparent (bool is_transparent);

        void show ();

      private:
        std::array<int,2> m_canvas_size = { 768, 512 };
        ColourMap m_colourmap = get_default_cmap();
        bool m_zero_is_transparent = true;
        bool m_done = false;
        std::array<float,2> m_tick_spacing = { None, None };
        std::array<bool,2> m_grid = { true, true };
        std::array<float,2> m_xlim = { inf, -inf };
        std::array<float,2> m_ylim = { inf, -inf };

        struct Line {
          std::array<float,2> a, b;
          int colour_index, stiple;
          float stiple_frac;
        };

        struct YPlot {
          std::vector<float> y;
          int colour_index, stiple;
          float stiple_frac;
        };

        struct XYPlot {
          std::vector<float> x, y;
          int colour_index, stiple;
          float stiple_frac;
        };

        struct Text {
          std::string text;
          std::array<float,2> pos, anchor;
          int colour_index;
        };

        using Element = std::variant<Line,YPlot,XYPlot,Text>;

        std::vector<Element> m_elements;


        float compute_tick_spacing (const std::array<float,2>& lim, float init_spacing) const;
        void refine_lim (std::array<float,2>& lim, float spacing) const;

        struct Limits {
          std::array<float,2> x, y;
        };
        std::array<float,2> auto_xlim() const;
        std::array<float,2> auto_ylim() const;

        int get_colour_in_cmap (int colour_index) const;
    };


  }


  inline Figure figure (int width = 900, int height = 300) { return { width, height }; }















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

  namespace {
    inline ctype clamp (float val, int number) {
      return ctype (std::round (std::min (std::max ((100.0/number)*val, 0.0), static_cast<double>(number))));
    }
  }


  inline ColourMap gray (int number)
  {
    ColourMap cmap (number);
    for (int n = 0; n < number; ++n) {
      ctype c = clamp (n, number-1);
      cmap[n] = { c, c, c };
    }
    return cmap;
  }


  inline ColourMap hot (int number)
  {
    ColourMap cmap (number);
    for (int n = 0; n < number; ++n) {
      cmap[n] = {
        clamp (3*n, number-1),
        clamp (3*n-number, number-1),
        clamp (3*n-2*number, number-1)
      };
    }
    return cmap;
  }


  inline ColourMap jet (int number)
  {
    ColourMap cmap (number);
    for (int n = 0; n < number; ++n) {
      cmap[n] = {
        clamp (1.5*number-std::abs(4*n-3*number), number-1),
        clamp (1.5*number-std::abs(4*n-2*number), number-1),
        clamp (1.5*number-std::abs(4*n-1*number), number-1)
      };
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
  //                   Rescale implementation
  // **************************************************************************

  template <class ImageType>
    inline Rescale<ImageType>::Rescale (const ImageType& image, double minval, double maxval, int cmap_size) :
      im (image), min (minval), max (maxval), cmap_size (cmap_size) { }

  template <class ImageType>
    inline int Rescale<ImageType>::width () const { return im.width(); }

  template <class ImageType>
    inline int Rescale<ImageType>::height () const { return im.height(); }

  template <class ImageType>
    inline ctype Rescale<ImageType>::operator() (int x, int y) const {
      double rescaled = cmap_size * (im(x,y) - min) / (max - min);
      return std::round (std::min (std::max (rescaled, 0.0), cmap_size-1.0));
    }



  // **************************************************************************
  //                   magnify implementation
  // **************************************************************************

  template <class ImageType>
    inline magnify<ImageType>::magnify (const ImageType& image, int factor) :
      im (image), factor (factor) { }

  template <class ImageType>
    inline int magnify<ImageType>::width () const { return im.width() * factor; }

  template <class ImageType>
    inline int magnify<ImageType>::height () const { return im.height() * factor; }

  template <class ImageType>
    inline decltype(std::declval<const ImageType>()(0,0)) magnify<ImageType>::operator() (int x, int y) const {
      return im (x/factor, y/factor);
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
        if (!repeats) {
          ++repeats;
          current = c;
          continue;
        }
        if (c == current) {
          ++repeats;
          continue;
        }
        commit (out, current, repeats);
        current = c;
        repeats = 1;
      }
      commit (out, current, repeats);

      return out.str();
    }


    template <class ImageType>
      inline std::string encode (const ImageType& im, int cmap_start, int cmap_end, int y0)
      {
        std::string out;
        const int nsixels = std::min (im.height()-y0, 6);

        bool first = true;
        for (ctype intensity = cmap_start; intensity < cmap_end; ++intensity) {
          std::string row = encode_row (im, y0, im.width(), nsixels, intensity);
          if (row.size()) {
            if (first) first = false;
            else out += '$';
            out += std::format ("#{}{}", static_cast<int>(intensity), row);
          }
        }
        out += '-';
        return out;
      }

  }







  template <class ImageType>
    inline void imshow (const ImageType& image, const ColourMap& cmap, const bool zero_is_transparent)
    {
      std::string out = "\033P9;1q" + colourmap_specifier (cmap);
      for (int y = 0; y < image.height(); y += 6)
        out += encode (image, ( zero_is_transparent ? 1 : 0 ), cmap.size(), y);
      out += "\033\\\n";
      std::cout.write (out.data(), out.size());
      std::cout.flush();
    }



  template <class ImageType>
    inline void imshow (const ImageType& image, double min, double max, const ColourMap& cmap, const bool zero_is_transparent)
    {
      Rescale<ImageType> rescaled (image, min, max, cmap.size());
      imshow (rescaled, cmap, zero_is_transparent);
    }




  // **************************************************************************
  //                   line drawing implementation
  // **************************************************************************

  namespace {


    inline const ColourMap& get_default_cmap()
    {
      static  ColourMap default_cmap;

      if (!default_cmap.empty())
        return default_cmap;


      default_cmap = {
        {   0,   0,   0 },
        { 100, 100, 100 },
        { 100, 100,  20 },
        { 100,  20, 100 },
        {  20, 100, 100 },
        { 100,  20,  20 },
        {  20, 100,  20 },
        {  20,  20, 100 }
      };

      // disable deprecation warning for getenv() from Visual Studio
      // our usage should be safe given we only check whether variable is set
      // the value of the variable is never accessed
#ifdef _MSC_VER
#pragma warning(disable : 4996)
#endif
      if (std::getenv("WHITEBG") != nullptr)
        for (auto& x : default_cmap)
          for (auto& c : x)
            c = 100-c;

      return default_cmap;
    }



    template <class ImageType>
      inline void line_x (ImageType& canvas, float x0, float y0, float x1, float y1,
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

  }




  template <typename ImageType>
    inline void render_line (ImageType& canvas, float x0, float y0, float x1, float y1,
        int colour_index, int stiple, float stiple_frac)
    {
      struct CanvasView {
        ImageType& canvas;
        const bool transpose;
        int width () const { return transpose ? canvas.height() : canvas.width(); }
        int height () const { return transpose ? canvas.width() : canvas.height(); }
        ctype& operator() (int x, int y) { return transpose ? canvas(y,x) : canvas(x,y); }
      };

      bool transposed = std::abs (x1-x0) < std::abs (y1-y0);
      if (transposed) {
        std::swap (x0, y0);
        std::swap (x1, y1);
      }
      CanvasView view = { canvas, transposed };
      line_x (view, x0, y0, x1, y1, colour_index, stiple, stiple_frac);
    }




  template <typename ImageType>
    inline void render_text (ImageType& canvas, const std::string& text, float x, float y,
        float anchor_x, float anchor_y, int colour_index)
    {
      auto f = Font::get_font();
      const int text_width = f.width() * text.size();
      int posx = std::round (x - anchor_x * text_width);
      int posy = std::round (y - (1.0-anchor_y) * f.height());

      for (std::size_t n = 0; n < text.size(); ++n)
        f.render (canvas, text[n], posx+n*f.width(), posy, colour_index);
    }








  // **************************************************************************
  //                   Figure implementation
  // **************************************************************************

  namespace {

    template <typename VectorType>
      inline std::vector<float> make_vec (const VectorType& v)
      {
        std::vector<float> x (v.size());
        for (unsigned int n = 0; n < x.size(); n++)
          x[n] = v[n];
        return x;
      }

    inline Figure::Figure (int width, int height) :
      m_canvas_size { width, height } { }

    inline Figure::~Figure()
    {
      if (m_elements.size())
        show();
    }


    inline Figure& Figure::line (float x0, float y0, float x1, float y1, int colour_index, int stiple, float stiple_frac)
    {
      m_elements.push_back (Line { { x0, y0 }, { x1, y1 }, colour_index, stiple, stiple_frac });
      return *this;
    }


    template <typename VectorType>
      inline Figure& Figure::plot (const VectorType& y, int colour_index, int stiple, float stiple_frac)
      {
        m_elements.push_back (YPlot { make_vec(y), colour_index, stiple, stiple_frac });
        return *this;
      }

    template <typename VectorTypeX, typename VectorTypeY>
      inline Figure& Figure::plot (const VectorTypeX& x, const VectorTypeY& y, int colour_index, int stiple, float stiple_frac)
      {
        if (x.size() != y.size())
          throw std::runtime_error ("X & Y dimensions do not match");
        m_elements.push_back (XYPlot { make_vec(x), make_vec(y), colour_index, stiple, stiple_frac });
        return *this;
      }

    inline Figure& Figure::text (const std::string& text, float x, float y, float anchor_x, float anchor_y, int colour_index)
    {
      m_elements.push_back (Text { text, { x, y }, { anchor_x, anchor_y }, colour_index });
      return *this;
    }

    inline Figure& Figure::xlim (float x_min, float x_max)
    {
      m_xlim = { x_min, x_max };
      return *this;
    }

    inline Figure& Figure::ylim (float y_min, float y_max)
    {
      m_ylim = { y_min, y_max };
      return *this;
    }

    inline Figure& Figure::ticks (float x_spacing, float y_spacing)
    {
      m_tick_spacing = { x_spacing, y_spacing };
      return *this;
    }

    inline Figure& Figure::grid (bool show_xgrid, bool show_ygrid)
    {
      m_grid = { show_xgrid, show_ygrid };
      return *this;
    }

    inline Figure& Figure::colourmap (const ColourMap& cmap)
    {
      m_colourmap = cmap;
      return *this;
    }

    inline Figure& Figure::transparent (bool is_transparent)
    {
      m_zero_is_transparent = is_transparent;
      return *this;
    }

    inline float Figure::compute_tick_spacing (const std::array<float,2>& lim, float init_spacing) const
    {
      double tick = (lim[1]-lim[0])/init_spacing;
      double mult = std::pow(10.0, std::floor (std::log10 (tick)));
      double scaled = tick/mult;
      if (scaled < 2.0)
        return 2.0*mult;
      if (scaled < 5.0)
        return 5.0*mult;
      return 10.0*mult;
    }

    inline void Figure::refine_lim (std::array<float,2>& lim, float spacing) const
    {
      lim[0] = spacing * std::floor (lim[0] / spacing);
      lim[1] = spacing * std::ceil (lim[1] / spacing);
    }

    inline std::array<float,2> Figure::auto_xlim () const
    {
      std::array<float,2> x { inf, -inf };

      for (const auto& el : m_elements) {
        if (el.index() == 0) {
          const auto& p = std::get<Line> (el);
          x[0] = std::min (x[0], std::min (p.a[0], p.b[0]));
          x[1] = std::max (x[1], std::max (p.a[0], p.b[0]));
        }
        else if (el.index() == 1) {
          const auto& p = std::get<YPlot> (el);
          x[0] = std::min (x[0], 0.0f);
          x[1] = std::max (x[1], p.y.size()-1.0f);
        }
        else if (el.index() == 2) {
          const auto& p = std::get<XYPlot> (el);
          x[0] = std::min (x[0], *std::min_element (p.x.cbegin(), p.x.cend()));
          x[1] = std::max (x[1], *std::max_element (p.x.cbegin(), p.x.cend()));
        }
      }

      return x;
    }

    inline std::array<float,2> Figure::auto_ylim () const
    {
      std::array<float,2> y { inf, -inf };

      for (const auto& el : m_elements) {
        if (el.index() == 0) {
          const auto& p = std::get<Line> (el);
          y[0] = std::min (y[0], std::min (p.a[1], p.b[1]));
          y[1] = std::max (y[1], std::max (p.a[1], p.b[1]));
        }
        else if (el.index() == 1) {
          const auto& p = std::get<YPlot> (el);
          y[0] = std::min (y[0], *std::min_element (p.y.cbegin(), p.y.cend()));
          y[1] = std::max (y[1], *std::max_element (p.y.cbegin(), p.y.cend()));
        }
        else if (el.index() == 2) {
          const auto& p = std::get<XYPlot> (el);
          y[0] = std::min (y[0], *std::min_element (p.y.cbegin(), p.y.cend()));
          y[1] = std::max (y[1], *std::max_element (p.y.cbegin(), p.y.cend()));
        }
      }
      return y;
    }

    inline int Figure::get_colour_in_cmap (int colour_index) const
    {
      assert (colour_index >= 0);
      while (colour_index >= m_colourmap.size())
        colour_index -= (m_colourmap.size()-2);
      return colour_index;
    }


    inline void Figure::show ()
    {
      Image<ctype> canvas (m_canvas_size[0], m_canvas_size[1]);

      const bool xlim_manual = std::isfinite (m_xlim[0]) && std::isfinite (m_xlim[1]);
      const bool ylim_manual = std::isfinite (m_ylim[0]) && std::isfinite (m_ylim[1]);

      const bool show_xticks = !( m_tick_spacing[0] <= 0.0 );
      const bool show_yticks = !( m_tick_spacing[1] <= 0.0 );

      auto xlim = xlim_manual ? m_xlim : auto_xlim();
      auto ylim = ylim_manual ? m_ylim : auto_ylim();

      auto font = Font::get_font();
      int margin_left = 10*font.width();
      int margin_bottom = 2*font.height();
      int margin_right = 3*font.width();
      int margin_top = 1*font.height();

      auto xtick_spacing = compute_tick_spacing (xlim, std::max (8.0f, (canvas.width()-margin_left)/(5.0f*font.height())));
      auto ytick_spacing = compute_tick_spacing (ylim, std::max (8.0f, (canvas.height()-margin_bottom)/(5.0f*font.height())));

      if (!xlim_manual) refine_lim (xlim, xtick_spacing);
      if (!ylim_manual) refine_lim (ylim, ytick_spacing);

      if (std::isfinite (m_tick_spacing[0]) && m_tick_spacing[0] <= 0.0) xtick_spacing = m_tick_spacing[0];
      if (std::isfinite (m_tick_spacing[1]) && m_tick_spacing[1] <= 0.0) ytick_spacing = m_tick_spacing[1];


      struct CanvasMapper {
        const int width, height, margin_left, margin_bottom, margin_right, margin_top;
        const std::array<float,2> xlim, ylim;
        float mapx (float x) const { return (width-1) * (x-xlim[0])/(xlim[1]-xlim[0]); }
        float mapy (float y) const { return (height-1) * (1.0 - (y-ylim[0])/(ylim[1]-ylim[0])); }
        std::array<float,2> operator() (const std::array<float,2>& p) { return { mapx(p[0])+margin_left, mapy(p[1])+margin_top }; }
      };

      CanvasMapper map = {
        m_canvas_size[0]-margin_left-margin_right, m_canvas_size[1]-margin_bottom-margin_top,
        margin_left, margin_bottom, margin_right, margin_top, xlim, ylim };

      // render grid, tick, and tick values:
      for (int n = std::ceil (xlim[0]/xtick_spacing); n <= xlim[1]/xtick_spacing; n++) {
        const float x = n*xtick_spacing;
        const auto a = map ({ x, ylim[0] });
        const auto b = map ({ x, ylim[1] });
        if (m_grid[0])
          render_line (canvas, a[0], a[1], b[0], b[1], 1, 10, ( n == 0 ? 0.7 : 0.1 ));

          if (show_xticks) {
            std::stringstream legend;
            legend << std::setprecision (3) << x;
            render_text (canvas, legend.str(), a[0], a[1], 0.5, 1.5);
            render_line (canvas, a[0], a[1], a[0], a[1]-5, 1);
          }
        }

      for (int n = std::ceil (ylim[0]/ytick_spacing); n <= ylim[1]/ytick_spacing; n++) {
        const float y = n*ytick_spacing;
        const auto a = map ({ xlim[0], y });
        const auto b = map ({ xlim[1], y });
        if (m_grid[1])
          render_line (canvas, a[0], a[1], b[0], b[1], 1, 10, ( n == 0 ? 0.7 : 0.1 ));

        if (show_yticks) {
          std::stringstream legend;
          legend << std::setprecision (3) << y << " ";
          render_text (canvas, legend.str(), a[0], a[1], 1.0, 0.5);
          render_line (canvas, a[0], a[1], a[0]+5, a[1], 1);

        }
      }

      // Render elements:
      for (const auto& el : m_elements) {
        if (el.index() == 0) {
          const auto& p = std::get<Line>(el);
          const int colour_index = get_colour_in_cmap (p.colour_index);
          render_line (canvas, p.a[0], p.a[1], p.b[0], p.b[1], colour_index, p.stiple, p.stiple_frac);
        }
        else if (el.index() == 1) {
          const auto& p = std::get<YPlot>(el);
          const int colour_index = get_colour_in_cmap (p.colour_index);
          for (std::size_t n = 0; n < p.y.size()-1; ++n) {
            const auto a = map ({ static_cast<float>(n), p.y[n] });
            const auto b = map ({ static_cast<float>(n+1), p.y[n+1] });
            render_line (canvas, a[0], a[1], b[0], b[1], colour_index, p.stiple, p.stiple_frac);
          }
        }
        else if (el.index() == 2) {
          const auto& p = std::get<XYPlot>(el);
          const int colour_index = get_colour_in_cmap (p.colour_index);
          for (std::size_t n = 0; n < p.y.size()-1; ++n) {
            const auto a = map ({ p.x[n], p.y[n] });
            const auto b = map ({ p.x[n+1], p.y[n+1] });
            render_line (canvas, a[0], a[1], b[0], b[1], colour_index, p.stiple, p.stiple_frac);
          }
        }
        else if (el.index() == 3) {
          const auto& p = std::get<Text>(el);
          const int colour_index = get_colour_in_cmap (p.colour_index);
          const auto a = map ({ p.pos[0], p.pos[1] });
          render_text (canvas, p.text, a[0], a[1], p.anchor[0], p.anchor[1], colour_index);
        }
        else
          assert (false /* should not be here! */);
      }

      imshow (canvas, m_colourmap, m_zero_is_transparent);

      m_elements.clear();
    }

  }






  // **************************************************************************
  //                   Font imlementation
  // **************************************************************************


  inline constexpr Font::Font (int width, int height, const std::span<const unsigned char>& data) :
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

  template <typename ImageType>
    inline void Font::render (ImageType& canvas, char c, int x, int y, int colour_index) const
    {

      static constexpr char mapping [] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
        33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
        51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68,
        69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86,
        87, 88, 89, 90, 91, 92, 93, 94, 0
      };

      c = mapping[static_cast<int>(c)];
      for (int j = std::max (0,-y); j < h - std::max (0,y+h-canvas.height()); ++j)
        for (int i = std::max (0,-x); i < w - std::max (0,w+x-canvas.width()); ++i)
          if (get(c,i,j))
            canvas(x+i,y+j) = colour_index;

    }



  namespace {
    // Font definitions

    // This is a straight bit-wise raster of the Unifont glyphs in the ASCII
    // visible range.

    constexpr std::array<const unsigned char,3420> unifont8x16= {
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,8,8,8,8,8,8,0,8,8,0,0,0,0,34,34,34,34,0,
      0,0,0,0,0,0,0,0,0,0,0,0,0,36,36,36,63,18,18,63,9,9,9,0,0,0,0,0,0,8,62,73,9,14,56,
      72,73,62,8,0,0,0,0,0,0,70,41,41,22,8,8,52,74,74,49,0,0,0,0,0,0,28,34,34,20,12,74,
      81,33,49,78,0,0,0,0,12,8,8,4,0,0,0,0,0,0,0,0,0,0,0,0,0,16,8,8,4,4,4,4,4,4,8,8,16,
      0,0,0,0,2,4,4,8,8,8,8,8,8,4,4,2,0,0,0,0,0,0,0,8,73,42,28,42,73,8,0,0,0,0,0,0,0,0,
      0,8,8,8,127,8,8,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,12,8,8,4,0,0,0,0,0,0,0,0,0,30,0,
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,12,12,0,0,0,0,0,0,32,32,16,8,8,4,4,2,1,1,0,0,0,
      0,0,0,12,18,33,49,41,37,35,33,18,12,0,0,0,0,0,0,8,12,10,8,8,8,8,8,8,62,0,0,0,0,0,
      0,30,33,33,32,24,4,2,1,1,63,0,0,0,0,0,0,30,33,33,32,28,32,32,33,33,30,0,0,0,0,0,
      0,16,24,20,18,17,17,63,16,16,16,0,0,0,0,0,0,63,1,1,1,31,32,32,32,33,30,0,0,0,0,0,
      0,28,2,1,1,31,33,33,33,33,30,0,0,0,0,0,0,63,32,32,16,16,16,8,8,8,8,0,0,0,0,0,0,30,
      33,33,33,30,33,33,33,33,30,0,0,0,0,0,0,30,33,33,33,62,32,32,32,16,14,0,0,0,0,0,0,
      0,0,12,12,0,0,0,12,12,0,0,0,0,0,0,0,0,0,12,12,0,0,0,12,8,8,4,0,0,0,0,0,0,32,16,8,
      4,2,4,8,16,32,0,0,0,0,0,0,0,0,0,63,0,0,0,63,0,0,0,0,0,0,0,0,0,1,2,4,8,16,8,4,2,1,
      0,0,0,0,0,0,30,33,33,32,16,8,8,0,8,8,0,0,0,0,0,0,28,34,41,53,37,37,37,57,2,60,0,
      0,0,0,0,0,12,18,18,33,33,63,33,33,33,33,0,0,0,0,0,0,31,33,33,33,31,33,33,33,33,31,
      0,0,0,0,0,0,30,33,33,1,1,1,1,33,33,30,0,0,0,0,0,0,15,17,33,33,33,33,33,33,17,15,
      0,0,0,0,0,0,63,1,1,1,31,1,1,1,1,63,0,0,0,0,0,0,63,1,1,1,31,1,1,1,1,1,0,0,0,0,0,0,
      30,33,33,1,1,57,33,33,49,46,0,0,0,0,0,0,33,33,33,33,63,33,33,33,33,33,0,0,0,0,0,
      0,62,8,8,8,8,8,8,8,8,62,0,0,0,0,0,0,124,16,16,16,16,16,16,17,17,14,0,0,0,0,0,0,33,
      17,9,5,3,3,5,9,17,33,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,63,0,0,0,0,0,0,33,33,51,51,45,
      45,33,33,33,33,0,0,0,0,0,0,33,35,35,37,37,41,41,49,49,33,0,0,0,0,0,0,30,33,33,33,
      33,33,33,33,33,30,0,0,0,0,0,0,31,33,33,33,31,1,1,1,1,1,0,0,0,0,0,0,30,33,33,33,33,
      33,33,45,51,30,96,0,0,0,0,0,31,33,33,33,31,9,17,17,33,33,0,0,0,0,0,0,30,33,33,1,
      6,24,32,33,33,30,0,0,0,0,0,0,127,8,8,8,8,8,8,8,8,8,0,0,0,0,0,0,33,33,33,33,33,33,
      33,33,33,30,0,0,0,0,0,0,65,65,65,34,34,34,20,20,8,8,0,0,0,0,0,0,33,33,33,33,45,45,
      51,51,33,33,0,0,0,0,0,0,33,33,18,18,12,12,18,18,33,33,0,0,0,0,0,0,65,65,34,34,20,
      8,8,8,8,8,0,0,0,0,0,0,63,32,32,16,8,4,2,1,1,63,0,0,0,0,0,56,8,8,8,8,8,8,8,8,8,8,
      56,0,0,0,0,0,1,1,2,4,4,8,8,16,32,32,0,0,0,0,0,7,4,4,4,4,4,4,4,4,4,4,7,0,0,0,12,18,
      33,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,127,0,0,0,8,4,4,12,0,0,0,0,
      0,0,0,0,0,0,0,0,0,0,0,0,30,33,32,62,33,33,49,46,0,0,0,0,0,1,1,1,29,35,33,33,33,33,
      35,29,0,0,0,0,0,0,0,0,30,33,1,1,1,1,33,30,0,0,0,0,0,32,32,32,46,49,33,33,33,33,49,
      46,0,0,0,0,0,0,0,0,30,33,33,63,1,1,33,30,0,0,0,0,0,24,4,4,4,31,4,4,4,4,4,4,0,0,0,
      0,0,0,0,32,46,17,17,17,14,2,30,33,33,30,0,0,0,1,1,1,29,35,33,33,33,33,33,33,0,0,
      0,0,0,8,8,0,12,8,8,8,8,8,8,62,0,0,0,0,0,16,16,0,24,16,16,16,16,16,16,16,9,6,0,0,
      0,1,1,1,17,9,5,3,5,9,17,33,0,0,0,0,0,12,8,8,8,8,8,8,8,8,8,62,0,0,0,0,0,0,0,0,55,
      73,73,73,73,73,73,73,0,0,0,0,0,0,0,0,29,35,33,33,33,33,33,33,0,0,0,0,0,0,0,0,30,
      33,33,33,33,33,33,30,0,0,0,0,0,0,0,0,29,35,33,33,33,33,35,29,1,1,0,0,0,0,0,0,46,
      49,33,33,33,33,49,46,32,32,0,0,0,0,0,0,29,35,33,1,1,1,1,1,0,0,0,0,0,0,0,0,30,33,
      1,6,24,32,33,30,0,0,0,0,0,0,4,4,4,31,4,4,4,4,4,24,0,0,0,0,0,0,0,0,33,33,33,33,33,
      33,49,46,0,0,0,0,0,0,0,0,33,33,33,18,18,18,12,12,0,0,0,0,0,0,0,0,65,73,73,73,73,
      73,73,54,0,0,0,0,0,0,0,0,33,33,18,12,12,18,33,33,0,0,0,0,0,0,0,0,33,33,33,33,33,
      50,44,32,32,30,0,0,0,0,0,0,63,32,16,8,4,2,1,63,0,0,0,0,0,24,4,4,8,8,4,2,4,8,8,4,
      4,24,0,0,8,8,8,8,8,8,8,8,8,8,8,8,8,8,0,0,0,6,8,8,4,4,8,16,8,4,4,8,8,6,0,0,0,70,73,
    };
  }

  inline constexpr const Font Font::get_font (int size)
  {
    switch (size) {
      case 16: return { 8, 16, unifont8x16 };
      default: throw std::runtime_error (std::format ("font size {} not supported", size));
    }
  }




}


