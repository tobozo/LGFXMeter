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

//#include <ESP32-Chimera-Core.h>
#include <M5Unified.h>

#include "assets.h"
#include "IC705.hpp"

const int32_t GaugeWidth  = 320;
const int32_t GaugeHeight = 160;
const int32_t GaugePosX   = 0;
const int32_t GaugePosY   = 0;

const image_t bgImg       = { 16, bg_png,                 bg_png_len,                 IMAGE_PNG, 320, 240 };
const image_t pixelArrow  = { 16, arrow_zelda_png,        arrow_zelda_png_len,        IMAGE_PNG, 32, 40 };
const image_t shadowArrow = { 16, arrow_zelda_shadow_png, arrow_zelda_shadow_png_len, IMAGE_PNG, 32, 40 };


Gauge_Class  *ICSGauge    = nullptr;


void setup()
{

  M5.begin();

  auto cfg = LGFXMeter::config( IC705 );

  // attach the gauge to TFT
  cfg.display = &M5.Lcd;

  // The gauge will create its own canvas unless cfg.dstCanvas is provided.
  // Use case: background is variable, generated from gradients or vector drawings.
  // /!\ Don't use this feature if your device has no PSRam as it comes with a
  // memory consumption overhead that may get the mask canvas creation to fail.
  // cfg.dstCanvas = myGaugeSprite;

  // set gauge geometry
  cfg.clipRect  = {
    .x         = GaugePosX,
    .y         = GaugePosY,
    .w         = GaugeWidth,
    .h         = GaugeHeight
  };

  // - Mask zoom level rendering setting affects mask size and antialias
  // - The antialias trick is to draw gauge rulers and labels in an upscaled, low bit depth, canvas, and
  //   then downscale it into the final gauge canvas
  // - The zoom level value is expressed as a fraction [0....1] and is applied to the mask canvas.
  // - Although the mask is discarded after initial rendering, it will need some available ram.
  //   A lower cfg.zoomAA will need more ram. Set it to 1.0 to disable Antialias and consume minimal ram.
  //
  // e.g.
  //   - 320*160 gauge with cfg.zoomAA=0.5 will use a 640*320 mask with antialias
  //   - 320*160 gauge with cfg.zoomAA=1.0 will use a 320*160 mask with NO antialias
  //
  // cfg.zoomAA = psramInit() ? 0.5 : 1.0;
  //cfg.zoomAA = 1.0;

  // Optionally use pre-rendered arrow and custom shadow
  // - Image can be any size (scale will constrained) but smaller is faster, transparent png works
  // - Pointy end of the arrow goes on the top
  // - Shadow image must have same dimensions as arrow image
  //cfg.needle.img    = &pixelArrow;
  //cfg.needle.shadow = &shadowArrow;
  //cfg.needle.scaleX = 0.5; // scaling down a stretched image to produce nicer antialiased result
  //cfg.needle.axis = { GaugeWidth/2, GaugePosY+GaugeHeight }; // will be automatically positioned with a clunky calculation otherwise

  // Optionally share a background image between TFT and the gauge sprite
  // - Image must be png/jpg/bmp/qoi or byte array
  // - Image dimensions must be tft.width() * tft.height()
  // - Image will also be rendered on tft: M5.Lcd.drawPng( img->data, img->len, 0, 0 );
  cfg.bgImage   = &bgImg;

  // fill screen with a color from the gauge palette
  // M5.Lcd.fillScreen( cfg.palette->transparent_color );

  // or draw the gauge background shared image
  M5.Lcd.drawPng( bgImg.data, bgImg.len );

  ICSGauge = new Gauge_Class( cfg );
  ICSGauge->pushGauge(); // render empty gauge (no needle yet)

}



void loop()
{

  static float random_angle = 0.0;

  ICSGauge->animateNeedle( random_angle, true ); // random_angle should be between 0 and 90 (=degrees)
  delay( 500 + rand()%1000 );
  random_angle = ( (rand()%9000) / 100.0 );

}



