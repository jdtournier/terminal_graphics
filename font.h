#ifndef __SIXEL_FONT_H__
#define __SIXEL_FONT_H__

#include <vector>
#include <stdexcept>

#include "image.h"

namespace Sixel {

  // **************************************************************************
  //                       public interface:
  // **************************************************************************

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








  // **************************************************************************
  //                   imlementation details below:
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
      default: throw std::runtime_error ("font size " + std::to_string (size) + " not supported");
    }
  }

}

#endif
