
#include <ae.h>
#include <agl/act.h>

namespace geometry {
  inline static void color(AglObject *o, char r, char g, char b)
    throw()
  {
    ::aglColor3b(o, r, g, b);
  }

  inline static void color(AglObject *o, unsigned char r, unsigned char g, unsigned char b)
    throw()
  {
    ::aglColor3ub(o, r, g, b);
  }

  inline static void color(AglObject *o, double r, double g, double b)
    throw()
  {
    ::aglColor3d(o, r, g, b);
  }

  inline static void color(AglObject *o, float r, float g, float b)
    throw()
  {
    ::aglColor3f(o, r, g, b);
  }

  inline static void color(AglObject *o, int r, int g, int b)
    throw()
  {
    ::aglColor3i(o, r, g, b);
  }

  inline static void color(AglObject *o, unsigned r, unsigned g, unsigned b)
    throw()
  {
    ::aglColor3ui(o, r, g, b);
  }

  inline static void color(AglObject *o, short r, short g, short b)
    throw()
  {
    ::aglColor3s(o, r, g, b);
  }

  inline static void color(AglObject *o, unsigned short r, unsigned short g, unsigned short b)
    throw()
  {
    ::aglColor3us(o, r, g, b);
  }

  inline static void color(AglObject *o, char r, char g, char b, char a)
    throw()
  {
    ::aglColor4b(o, r, g, b, a);
  }

  inline static void color(AglObject *o, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
    throw()
  {
    ::aglColor4ub(o, r, g, b, a);
  }

  inline static void color(AglObject *o, double r, double g, double b, double a)
    throw()
  {
    ::aglColor4d(o, r, g, b, a);
  }

  inline static void color(AglObject *o, float r, float g, float b, float a)
    throw()
  {
    ::aglColor4f(o, r, g, b, a);
  }

  inline static void color(AglObject *o, int r, int g, int b, int a)
    throw()
  {
    ::aglColor4i(o, r, g, b, a);
  }

  inline static void color(AglObject *o, unsigned r, unsigned g, unsigned b, unsigned a)
    throw()
  {
    ::aglColor4ui(o, r, g, b, a);
  }

  inline static void color(AglObject *o, short r, short g, short b, short a)
    throw()
  {
    ::aglColor4s(o, r, g, b, a);
  }

  inline static void color(AglObject *o, unsigned short r, unsigned short g, unsigned short b, unsigned short a)
    throw()
  {
    ::aglColor4us(o, r, g, b, a);
  }

  template <typename T = unsigned char>
  class Color : public Act
  {
  public:
    T red, green, blue;

    inline Color(T r = 0, T g = 0, T b = 0) throw()
      : red(r), green(g), blue(b) {}

    template <typename T2>
    inline Color(const Color<T2> &c) throw()
      : red(c.red), green(c.green), blue(c.blue)
    {}

    inline ~Color() throw() {}

    inline void set(T r, T g, T b) throw() {
      red = r;
      green = g;
      blue = b;
    }

    operator std::string() const throw() { return "Color"; }

    void operator ()(AglObject *o) const throw() {
      color(o, red, green, blue);
    }
  };

  template <typename T = unsigned char>
  class Colorx : public Color<T>
  {
  public:
    T alpha;

    inline Colorx(T r = 0, T g = 0, T b = 0, T a = 0) throw()
      : Color<T>(r, g, b), alpha(a) {}

    template <typename T2>
    inline Colorx(const Colorx<T2> &c) throw()
      : Color<T>(c), alpha(c.alpha)
    {}

    inline ~Colorx() throw() {}

    inline void set(T r, T g, T b, T a) throw() {
      Color<T>::set(r, g, b);
      alpha = a;
    }

    operator std::string() const throw() { return "Colorx"; }

    void operator ()(AglObject *o) const throw() {
      color(o, Color<T>::red, Color<T>::green, Color<T>::blue, alpha);
    }
  };  
}
