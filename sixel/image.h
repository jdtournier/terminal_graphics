#ifndef __IMAGE_H__
#define __IMAGE_H__

#include <vector>

namespace Sixel {

  // **************************************************************************
  //                       public interface:
  // **************************************************************************

  // A simple class to hold a 2D image using datatype
  // specified in template parameter
  template <typename ValueType>
    class Image {
      public:
        // Instantiate an Image with the specified dimensions:
        Image (int x_dim, int y_dim);

        // query image dimensions:
        int width () const;
        int height () const;

        // read / write intensity at coordinates (x,y):
        ValueType& operator() (int x, int y);
        const ValueType& operator() (int x, int y) const;

        // clear image, setting all intensities to 0:
        void clear ();

      private:
        std::vector<ValueType> data;
        const int x_dim, y_dim;
    };








  // **************************************************************************
  //                   imlementation details below:
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

}

#endif

