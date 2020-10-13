
#include <unistd.h>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include "rail.h"

int main(int argc, char *argv[]) {
  geometry::Rail<> rail;
  const char *oname = NULL, *profname = NULL, *ns = NULL;
  const char *geomname = NULL;
  bool lib = false, ctr = false, quiet = false;
  int opt;
  while ((opt = getopt(argc, argv, "cG:ln:o:p:Q")) != -1) {
    switch (opt) {
    case 'c':
      ctr = true;
      break;
    case 'G':
      geomname = optarg;
      break;
    case 'l':
      lib = true;
      break;
    case 'n':
      ns = optarg;
      break;
    case 'o':
      oname = optarg;
      break;
    case 'p':
      profname = optarg;
      break;
    case 'Q':
      quiet = true;
      break;
    default:
    err:
      std::cerr << "wrong format" << std::endl;
      ::exit(EXIT_FAILURE);
    }
  }

  if (geomname != NULL) {
    std::ifstream in(geomname);
    while (!in.eof()) {
      std::string line;
      std::getline(in, line);
      if (line.empty()) break;
      size_t eq = line.find_first_of("=");
      if (line.substr(0, eq) == "rail.gap")
        rail.gap = ::atof(line.substr(eq + 1, line.length()).c_str());
      else if (line.substr(0, eq) == "rail.crosstie.gap")
        rail.crosstie.gap = ::atof(line.substr(eq + 1, line.length()).c_str());
      else if (line.substr(0, eq) == "rail.crosstie.wd")
        rail.crosstie.wd = ::atof(line.substr(eq + 1, line.length()).c_str());
      else if (line.substr(0, eq) == "rail.crosstie.ht")
        rail.crosstie.ht = ::atof(line.substr(eq + 1, line.length()).c_str());
      else if (line.substr(0, eq) == "rail.crosstie.dp")
        rail.crosstie.dp = ::atof(line.substr(eq + 1, line.length()).c_str());
    }
    in.close();
  }

  if (!lib && !ctr || lib && ctr || !profname || !oname) goto err;
  std::ifstream in(profname);
  if (!in) {
    std::cerr << "can't open " << profname << std::endl;
    ::exit(EXIT_FAILURE);
  }

  std::vector<geometry::Point<> > ps;
  while (!in.eof()) {
    const std::string sep = " \t,;";
    std::string line;
    std::getline(in, line);
    if (line.empty()) break;
    size_t from = 0, to = line.find_first_of(sep);
    geometry::Point<> p;
    p.x = ::atof(line.substr(from, to).c_str());
    from = line.find_first_not_of(sep, to);
    to = line.find_first_of(sep, from);
    p.y = ::atof(line.substr(from, to).c_str());
    from = line.find_first_not_of(sep, to);
    to = line.find_first_of(sep, from);
    p.z = ::atof(line.substr(from, to).c_str());
    ps.push_back(p);
  }
  in.close();

  if (lib) { // rail's OpenGL object
    struct AglLibrary *al = ::aguMakeLibrary(ns);
    struct AglObject *obj = ::aguInsertObject(al, "rail", true);
    geometry::AGLOutter<> outter(obj);
    if (rail(ps, outter)) {
      if (quiet || !::aguCheckIntegrity(1, al)) {
        if (::aguSaveLibrary(al, oname))
          std::cerr << "fail to save `" << oname << '\'' << std::endl;
      }
    }
    ::aguKillLibrary(al);
  }
  else { // rail's center line profile
    std::vector<geometry::Point<> > ctr;
    if (rail(ps, ctr)) {
      std::ofstream out(oname);
      out << ctr.size() << '\n';
      for (size_t i = 0; i < ctr.size(); ++ i) {
        if (i > 0) out.put('\n');
        out << ctr[i].x << ',' << ctr[i].y << ',' << ctr[i].z;
      }
      out.close();
    }
  }

  return 0;
}
