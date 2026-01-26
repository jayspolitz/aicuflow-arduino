#ifndef ALL_APPS_H
#define ALL_APPS_H

// import all apps / pages here at once
// #include "library/apps/all.h"
#include "library/apps/boot.cpp"    // page: boot screen & setup
#include "library/apps/core.cpp"    // page: core utils for page / menu setup

#include "library/apps/measurement.cpp"  // page: measurement helpers

#include "library/apps/about.cpp"   // page: about (message)
#include "library/apps/random.cpp"  // page: random colors screen test
#include "library/apps/wifiscan.cpp"  // page: wifi scan
#include "library/apps/btscan.cpp"  // page: bluethooth scan
#include "library/apps/colortest.cpp"  // page: gradient color test page
#include "library/apps/colorwheeltest.cpp"  // page: color test page
#include "library/apps/snakegame.cpp"  // page: snake game
#include "library/apps/gol.cpp"  // page: conways game of life
#include "library/apps/mandelbrot.cpp"  // page: mandelbrot set render
#include "library/apps/wireframe3d.cpp"  // page: 3d rotation renderer
// add more pages here
// #include "library/apps/_expand.cpp" // page: custom page

#endif // ALL_APPS_H
