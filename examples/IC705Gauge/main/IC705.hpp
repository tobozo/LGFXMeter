/*\
 *
 * LGFX ICS Meter Gauge
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

  namespace IC705_Gauge
  {

    // lazy precision achieved with the help of the online protractor :-) https://www.ginifab.com/feeds/angle_measurement/


    // just a local shorthand for 1/14th linear ruler
    const float _14th = 90.0 / 14.0;

    // [ S..3..5..7..8..+10..+30..+60 ]
    const ruler_unit_t Units_S[] = {
    /*{ idx,       angle,   label, size, distance,            fontFace, fontSize, textDatum }*/
      {   0,        0.0f,     "S",   10,       12, &FreeSansBold18pt7b,    1.0f,  MC_DATUM },
      {   1,        0.5f, nullptr,   10,        0,             nullptr,    0.0f,  MC_DATUM },
      {   2,  1.0f*_14th, nullptr,    5,        0,             nullptr,    0.0f,  MC_DATUM },
      {   3,  2.0f*_14th,     "3",   10,       12, &FreeSansBold12pt7b,    0.95f, MC_DATUM },
      {   4,  3.0f*_14th, nullptr,    5,        0,             nullptr,    0.0f,  MC_DATUM },
      {   5,  4.0f*_14th,     "5",   10,       12, &FreeSansBold12pt7b,    0.95f, MC_DATUM },
      {   6,  5.0f*_14th, nullptr,    5,        0,             nullptr,    0.0f,  MC_DATUM },
      {   7,  6.0f*_14th,     "7",   10,       12, &FreeSansBold12pt7b,    0.95f, MC_DATUM },
      {   8,  7.0f*_14th, nullptr,    5,        0,             nullptr,    0.0f,  MC_DATUM },
      {   9,  8.0f*_14th,     "9",   10,       12, &FreeSansBold12pt7b,    0.95f, MC_DATUM },
      {  10,  9.0f*_14th,   "+10",   10,       13,  &FreeSansBold9pt7b,    0.8f,  MC_DATUM },
      {  11, 10.0f*_14th, nullptr,    0,        0,             nullptr,    0.0f,  MC_DATUM },
      {  12, 11.0f*_14th,   "+30",   10,       13,  &FreeSansBold9pt7b,    0.8f,  MC_DATUM },
      {  13, 12.0f*_14th, nullptr,    0,        0,             nullptr,    0.0f,  MC_DATUM },
      {  14, 13.0f*_14th, nullptr,    0,        0,             nullptr,    0.0f,  MC_DATUM },
      {  15, 14.0f*_14th,   "+60",   10,       13,  &FreeSansBold9pt7b,    0.8f,  MC_DATUM },
    };

    // in between +10+20+60 red rulers for Units_S
    const ruler_unit_t Units_SRED[] = {
    /*{ idx,       angle,   label, size, distance,            fontFace, fontSize, textDatum }*/
      {   0,  2.0f*_14th, nullptr,   2,        0,              nullptr,    0.0f,  MC_DATUM },
      {   0,  4.0f*_14th, nullptr,   2,        0,              nullptr,    0.0f,  MC_DATUM },
      {   0,  5.0f*_14th, nullptr,   2,        0,              nullptr,    0.0f,  MC_DATUM },
    };

    // [ PWR..1..3..5......10 ]
    const ruler_unit_t Units_PWR[] = {
    /*{ idx,       angle,   label, size, distance,            fontFace, fontSize, textDatum }*/
      {  0,        -5.0f,   "PWR",    0,      -10,      &FreeSans12pt7b,   0.5f,   BR_DATUM },
      {  1,         0.0f, nullptr,   -8,        0,              nullptr,   0.0f,   MC_DATUM },
      {  2,        13.0f, nullptr,   -6,        0,              nullptr,   0.0f,   MC_DATUM },
      {  3,        21.0f,     "1",   -8,      -10,      &FreeSans12pt7b,   0.5f,   MC_DATUM },
      {  4,        30.0f, nullptr,   -6,        0,              nullptr,   0.0f,   MC_DATUM },
      {  5,        38.0f,     "3",   -8,      -10,      &FreeSans12pt7b,   0.5f,   MC_DATUM },
      {  6,        45.0f, nullptr,   -6,        0,              nullptr,   0.0f,   MC_DATUM },
      {  7,        52.0f,     "5",   -8,      -10,      &FreeSans12pt7b,   0.5f,   MC_DATUM },
      {  8,        59.0f, nullptr,   -6,        0,              nullptr,   0.0f,   MC_DATUM },
      {  9,        64.5f, nullptr,   -6,        0,              nullptr,   0.0f,   MC_DATUM },
      { 10,        69.5f, nullptr,   -6,        0,              nullptr,   0.0f,   MC_DATUM },
      { 11,        74.3f, nullptr,   -6,        0,              nullptr,   0.0f,   MC_DATUM },
      { 12,        78.5f,    "10",   -8,      -10,      &FreeSans12pt7b,   0.5f,   MC_DATUM },
    };

    // [ SWR..1..1.5..2...3...4..5.10.∞ ]
    const char* copyright = "ICSMeter V0.0.2 by F4HWN";
    const ruler_unit_t Units_SWR[] = {
    /*{ idx,       angle,   label, size, distance,            fontFace, fontSize, textDatum }*/
      {  0,       -5.0f,    "SWR",    0,       10,      &FreeSans12pt7b,    0.5f,  TC_DATUM },
      {  1,        0.25f,     "1",    8,       10,      &FreeSans12pt7b,    0.5f,  MC_DATUM },
      {  2,       10.0f,    "1.5",    8,       10,      &FreeSans12pt7b,    0.5f,  MC_DATUM },
      {  3,       19.0f,      "2",    8,       10,      &FreeSans12pt7b,    0.5f,  MC_DATUM },
      {  4,       29.0f,  nullptr,    8,       10,      &FreeSans12pt7b,    0.0f,  MC_DATUM },
      {  5,       35.0f,      "3",    8,       10,      &FreeSans12pt7b,    0.5f,  MC_DATUM },
      {  6,       42.0f,  nullptr,    8,       10,      &FreeSans12pt7b,    0.0f,  MC_DATUM },
      {  7,       46.0f,      "4",    8,       10,      &FreeSans12pt7b,    0.5f,  MC_DATUM },
      {  8,       53.0f,      "5",    8,       10,      &FreeSans12pt7b,    0.5f,  MC_DATUM },
      {  9,       66.0f,     "10",    8,       10,      &FreeSans12pt7b,    0.5f,  MC_DATUM },
      { 10,       79.5f,      "∞",    8,       10,      &FreeSans12pt7b,    0.5f,  MC_DATUM },
      { 11,       45.0f, "IC-705",    0,       -1,  &FreeSansBold18pt7b,    0.65f, TC_DATUM }, // main title ruler
      { 11,       45.0f,  "F4HWN",    0,      -25,   &Orbitron_Light_24,    0.50f, TC_DATUM }, // vanity ruler
      { 12,       45.0f,  copyright,  0,      -40,      &Roboto_Thin_24,    0.50f, TC_DATUM }, // copyright ruler
    };

    // black rulers/labels
    const ruler_t Ruler_S       = {   0.0f,  90.0f, 180, 1,    Units_S, sizeof(Units_S)  /sizeof(ruler_unit_t)  };
    const ruler_t Ruler_PWR     = {   0.0f,  90.0f, 176, 1,  Units_PWR, sizeof(Units_PWR)/sizeof(ruler_unit_t)  };
    const ruler_t Ruler_SWR     = {   0.0f,  90.0f, 130, 1,  Units_SWR, sizeof(Units_SWR)/sizeof(ruler_unit_t)  };

    // red fill + rulers for Ruler_S
    const ruler_t Ruler_0_Red   = {  51.42f, 90.0f, 180, 4, Units_SRED, sizeof(Units_SRED)/sizeof(ruler_unit_t) };
    // green fill for Ruler_SWR
    const ruler_t Ruler_2_Red   = {  35.0f,  90.0f, 130, 4,   nullptr, 0 };
    // red fill for Ruler_SWR
    const ruler_t Ruler_2_Green = {   0.0f,  19.0f, 130, 4,   nullptr, 0 };

    // group all ruler items
    const ruler_item_t items[] =
    {
    /*{ ruler_t,  palette color index }*/
      { &Ruler_0_Red,   2 },
      { &Ruler_2_Red,   2 },
      { &Ruler_2_Green, 3 },
      { &Ruler_S,       1 },
      { &Ruler_PWR,     1 },
      { &Ruler_SWR,     1 },
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

  }; // end namespace IC705_Gauge

  static constexpr gauge_t IC705 = IC705_Gauge::cfg;

}; // end namespace LGFXMeter

