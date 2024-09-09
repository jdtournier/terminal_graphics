#include <fstream>
#include <iostream>
#include <string>

#include "sixel.h"
#include "pgm.h"
#include "font.h"

class BitVector {
  public:
    BitVector (int size) : data ((size+7)/8, 0) { }
    std::vector<unsigned char> data;
    void set (int offset, bool value) {
      int index = offset/8;
      int shift = offset-index*8;
      if (value)
        data[index] |= 1U<<shift;
      else
        data[index] &= ~(1U<<shift);
    }

    bool get (int offset) {
      int index = offset/8;
      int shift = offset-index*8;
      return data[index] & (1U<<shift);
    }

};


int main (int argc, char* argv[])
{

  auto font = load_pgm<int> (argv[1]);

  std::string mapping = " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
/*
  std::vector<int> map (128,0);
  for (int n = 0; n < mapping.size(); ++n)
    map[mapping[n]] = n;

  for (const auto x : map)
    std::cerr << x << ", ";
  std::cerr << "\n";
*/

  std::cerr << "read image \"" << argv[1] << "\" with size " << font.width() << " x " << font.height() << "\n";

  int w = font.width()/mapping.size();
  int h = font.height();

  std::cerr << "assuming font size = " << w << " x " << h << "\n";

  BitVector bits (w*h);
  std::cout << "const char " << argv[2] << "[] = { ";
  for (int n = 0; n < mapping.size(); ++n) {
    Image<int> f (w,h);
    for (int y = 0; y < h; ++y)
      for (int x = 0; x < w; ++x)
        bits.set (x+y*w, font(n*w+x,y));
    for (const auto x : bits.data)
      std::cout << int(x) << ", ";
    std::cout << "\n";
  }
  std::cout << "};\n";



  return 0;
}
