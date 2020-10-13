
#include <agl/core.h>
#include <agl/cube.h>
#include <agl/line.h>
#include "color.h"
#include <vector>

namespace geometry {
  template <typename T = double, typename C = unsigned char>
  class Rail {
  public:
    struct Crosstie {
      T wd;
      T ht;
      T dp;
      T gap;
      Color<C> color;

      inline Crosstie(T _wd = 1.5, T _ht = 0.05, T _dp = 0.1, T _gap = 0.5)
        throw() : wd(_wd), ht(_ht), dp(_dp), gap(_gap) {}

      inline Crosstie(const Crosstie &ct)
        throw() : wd(ct.wd), ht(ct.ht), dp(ct.dp), gap(ct.gap) {}

      inline ~Crosstie() throw() {}
    };

    Crosstie crosstie;
    T gap;
    Color<C> color;

    inline Rail(T _gap = 1.2) throw() : crosstie(), gap(_gap) {}

    inline Rail(const Crosstie &_crosstie, T _gap = 1.2)
      throw() : crosstie(_crosstie), gap(_gap) {}

    inline ~Rail() throw() {}

    template <typename T2, typename T3, size_t P>
    bool operator()(const std::vector<Point<T2> > &ps, Outter<T3, P> &outter)
      const throw()
    {
      if (ps.size() <= 1) return false;

      std::vector<Point<T> > ps2;

      outter(color);
      outter.line_strip();
      for (size_t i = 0; i < ps.size(); ++ i) {
        if (i + 1 < ps.size()) {
          Point<T> p = ps[i] - ps[i + 1];
          p.cross(Point<T>(0, 0, 1));
          p *= gap / p.length();
          ps2.push_back(p + ps[i]);
          if (i + 2 == ps.size()) ps2.push_back(p + ps[i + 1]);
        }
        outter.vertex(ps[i]);
      }
      outter.end();

      outter.line_strip();
      outter.vertex(ps2.front());
      for (size_t i = 1; i + 1 < ps2.size(); ++ i) {
        Point<T> p1 = ps2[i - 1], a1 = ps[i] - ps[i - 1];
        Point<T> p2 = ps2[i], a2 = ps[i + 1] - ps[i];
        if (std::isnormal(cross(a1, a2).length())) {
          Point<T> p(p1.x, p1.y); // X-Y planar intersection
          Line<T>::intersect(p, Point<T>(a1.x, a1.y), Point<T>(p2.x, p2.y), Point<T>(a2.x, a2.y));
          Line<T>::intersect(p1, a1, p, Point<T>(0, 0, 1));
          Line<T>::intersect(p2, a2, p, Point<T>(0, 0, 1));
          ps2[i] = (p1 + p2) / 2;
        }
        outter.vertex(ps2[i]);
      }
      outter.vertex(ps2.back());
      outter.end();

      outter(crosstie.color);
      T d = crosstie.gap * 0.5;
      for (size_t i = 0; i + 1 < ps.size(); ++ i) {
        Point<T> p1 = ps[i], a1 = ps[i + 1] - p1;
        Point<T> p2 = ps2[i], a2 = ps2[i + 1] - p2;
        Point<T> perp = cross(Point<T>(0, 0, 1), a1), up = cross(a1, perp);
        T l1 = a1.length(), l2 = a2.length();
        T z = d / crosstie.gap;
        for (a1.norm(); d < l1; d += crosstie.gap, ++ z) {
          Point<T> p3 = p1 + a1 * (crosstie.gap * z), p4(p3.x, p3.y);
          Line<T>::intersect(p4, Point<T>(perp.x, perp.y), Point<T>(p2.x, p2.y), Point<T>(a2.x, a2.y));
          Line<T>::intersect(p4, Point<T>(0, 0, 1), p2, a2);
          if (std::isgreater(p4.distance(p2), l2)) continue;
          outter.push();
          outter.look_at((p3 + p4) / 2, p4 - p3, up);
          outter.scale(crosstie.dp, crosstie.ht, crosstie.wd);
          Cube<>()(outter);
          outter.pop();
        }
        d -= l1;
      }

      return true;
    }

    template <typename T2, typename T3>
    bool operator()(const std::vector<Point<T2> >&ps, std::vector<Point<T3> >&ctr)
      const throw()
    {
      if (ps.size() <= 1) return false;
      ctr.clear();

      for (size_t i = 0; i < ps.size(); ++ i) {
        Point<T> p = ps[i] - ps[i + 1];
        p.cross(Point<T>(0, 0, 1));
        p *= gap / p.length();
        ctr.push_back(p + ps[i]);
        if (i + 2 == ps.size()) ctr.push_back(p + ps[i + 1]);
      }

      for (size_t i = 1; i < ctr.size(); ++ i) {
        Point<T> p1 = ctr[i - 1], a1 = ps[i] - ps[i - 1];
        Point<T> p2 = ctr[i], a2 = ps[i + 1] - ps[i];
        if (std::isnormal(cross(a1, a2).length())) {
          Point<T> p(p1.x, p1.y); // X-Y planar intersection
          Line<T>::intersect(p, Point<T>(a1.x, a1.y), Point<T>(p2.x, p2.y), Point<T>(a2.x, a2.y));
          Line<T>::intersect(p1, a1, p, Point<T>(0, 0, 1));
          Line<T>::intersect(p2, a2, p, Point<T>(0, 0, 1));
          ctr[i] = (p1 + p2) / 2;
        }
      }

      for (size_t i = 0; i < ctr.size(); ++ i) {
        ctr[i] = (ctr[i] + ps[i]) / 2;
      }

      return true;
    }
  };
}
