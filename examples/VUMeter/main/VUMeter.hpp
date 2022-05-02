/*\
 *
 * LGFX VU Meter Gauge
 *
 * A demo inspired by https://github.com/armel/ICSMeter
 *
 * Copyright Apr. 2022 tobozo http://github.com/tobozo
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files ("M5Stack SD Updater"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
\*/

#pragma once

#include <LGFXMeter.h>


namespace LGFXMeter
{

  namespace VUMeter_Gauge
  {

    // lazy precision achieved with the help of the online protractor :-) https://www.ginifab.com/feeds/angle_measurement/

    const ruler_unit_t Units_0[] = {
    /*{ idx, angle,   label, size, distance,        fontFace, fontSize, textDatum }*/
      {   0,  0.0f, nullptr,    8,        0,         nullptr,     0.0f,  MC_DATUM },
      {   1,  2.0f,   "-20",   12,       20, &FreeSans12pt7b,     0.8f,  MC_DATUM },
      {   2, 13.0f,   "-10",   12,       17, &FreeSans12pt7b,     0.8f,  MC_DATUM },
      {   3, 22.0f,    "-7",   12,       15, &FreeSans12pt7b,     0.8f,  MC_DATUM },
      {   4, 26.0f, nullptr,    8,        0,         nullptr,     0.8f,  MC_DATUM },
      {   5, 30.0f,    "-5",   12,       15, &FreeSans12pt7b,     0.8f,  MC_DATUM },
      {   6, 35.0f, nullptr,    8,        0,         nullptr,     0.8f,  MC_DATUM },
      {   7, 41.0f,    "-3",   12,       15, &FreeSans12pt7b,     0.8f,  MC_DATUM },
      {   8, 47.0f,    "-2",   12,       15, &FreeSans12pt7b,     0.8f,  MC_DATUM },
      {   9, 54.0f,    "-1",   12,       15, &FreeSans12pt7b,     0.8f,  MC_DATUM },
      {  10, 58.0f, nullptr,    8,        0,         nullptr,     0.8f,  MC_DATUM },
      {  11, 61.0f,     "0",   12,       15, &FreeSans12pt7b,     0.8f,  MC_DATUM },
      {  12, 66.0f, nullptr,    8,        0,         nullptr,     0.8f,  MC_DATUM },
      {  13, 70.0f,    "+1",   12,       15, &FreeSans12pt7b,     0.8f,  MC_DATUM },
      {  14, 80.0f,    "+2",   12,       15, &FreeSans12pt7b,     0.8f,  MC_DATUM },
      {  15, 90.0f,    "+3",   12,       15, &FreeSans12pt7b,     0.8f,  MC_DATUM },
      {  16, 45.0f,    "VU",    0,      -20, &Orbitron_Light_24,  1.5f,  TC_DATUM }, // vanity ruler
    };



    const ruler_unit_t Units_1[] = {
    /*{ idx, angle,   label, size, distance,        fontFace, fontSize, textDatum }*/
      {   0,  0.0f, nullptr,    0,        0,         nullptr,     0.0f,  MC_DATUM },
      {   1,  2.0f,     "0",   -8,      -11,  &FreeSans9pt7b,     0.75f, MC_DATUM },
      {   2, 26.0f,    "50",   -8,      -11,  &FreeSans9pt7b,     0.75f, MC_DATUM },
      {  11, 61.0f,  "100%",   -8,      -11,  &FreeSans9pt7b,     0.75f,  MC_DATUM },
    };

    // black rulers/labels
    const ruler_t Ruler_0       = { 0.0f,  90.0f, 160, 1,    Units_0, sizeof(Units_0)  /sizeof(ruler_unit_t)  };
    const ruler_t Ruler_1       = { 0.0f,  61.0f, 156, 1,    Units_1, sizeof(Units_1)  /sizeof(ruler_unit_t)  };

    // red fill
    const ruler_t Ruler_0_Red = { 61.0f,  90.0f, 160, 6,   nullptr, 0 };

    // group all ruler items
    const ruler_item_t items[] =
    {
    /*{ ruler_t,  palette color index }*/
      { &Ruler_0_Red,   2 },
      { &Ruler_0,       1 },
      { &Ruler_1,       1 }
    };

    // define a color palette
    static constexpr gauge_palette_t palette =
    {
      0xffffffU, /*.transparent_color  */
      0x222222U, /*.fill_color         */
      0xff4444U, /*.warn_color         */
      0x00ff00U, /*.ok_color           */
      0xff2222U, /*.arrow_color        */
      0xaaaaaaU, /*.arrow_border_color */
      0x888888U, /*.arrow_shadow_color */
    };

    static constexpr gauge_t cfg =
    {
      .items       = items,
      .items_count = sizeof(items)/sizeof(ruler_item_t),
      .palette     = palette,
      .start       = -45.0f,
      .end         =  45.0f
    };

  }; // end namespace VUMeter_Gauge

  static constexpr gauge_t VUMeter = VUMeter_Gauge::cfg;

}; // end namespace LGFXMeter

