# LGFXMeter library


![image](https://user-images.githubusercontent.com/1893754/166322961-1f34e81a-ebb4-4c7b-8c05-4857d09a3456.png)



[![PlatformIO Registry](https://badges.registry.platformio.org/packages/tobozo/library/LGFXMeter.svg)](https://registry.platformio.org/packages/libraries/tobozo/LGFXMeter)

This library was inspired by [ICSMeter](https://github.com/armel/ICSMeter).


## Library requirements:

  - M5Unified
  - M5GFX


## Usage




### Creating a custom gauge.



```C

  #include <LGFXMeter.h>


  {
    // First describe the gauge.

    // Units and Labels.
    const ruler_unit_t My_Units[] = {
    /*{ idx, angle,   label, size, distance,        fontFace, fontSize, textDatum }*/
      {   0,  0.0f, nullptr,    0,        0,         nullptr,     0.0f,  MC_DATUM },
      {   1,  2.0f,     "0",   -8,      -11,  &FreeSans9pt7b,     0.75f, MC_DATUM },
      {   2, 45.0f,    "50",   -8,      -11,  &FreeSans9pt7b,     0.75f, MC_DATUM },
      {   3, 90.0f,  "100%",   -8,      -11,  &FreeSans9pt7b,     0.75f, MC_DATUM },
    };

    // Units and Labels
    const ruler_unit_t My_Other_Units[] = {
    /*{ idx, angle,   label, size, distance,        fontFace, fontSize, textDatum }*/
      {   0,  0.0f, nullptr,    0,        0,         nullptr,     0.0f,  MC_DATUM },
      {   1,  2.0f,   "+10",    8,       11,  &FreeSans9pt7b,     0.75f, MC_DATUM },
      {   2, 45.0f,   "+50",    8,       11,  &FreeSans9pt7b,     0.75f, MC_DATUM },
      {   3, 90.0f, "+100%",    8,       11,  &FreeSans9pt7b,     0.75f, MC_DATUM },
    };

    // Attach units to ruler
    const ruler_t My_Ruler       = {  0.0f,  90.0f, 150, 1, My_Units,       sizeof(My_Units)/sizeof(ruler_unit_t) };

    // Attach units to ruler
    const ruler_t My_Other_Ruler = { 45.0f,  90.0f, 160, 1, My_Other_Units, sizeof(My_Other_Units)/sizeof(ruler_unit_t) };

    // group all rulers
    const ruler_item_t items[] =
    {
    /*{ ruler_t,          palette color index }*/
      { &My_Ruler,        1                   },
      { &My_Other_Ruler,  2                   }
    };

    // define a color palette
    const gauge_palette_t palette =
    {
      0xffffffU, /*.transparent_color  */
      0x222222U, /*.fill_color         */
      0xff4444U, /*.warn_color         */
      0x00ff00U, /*.ok_color           */
      0xff2222U, /*.arrow_color        */
      0xaaaaaaU, /*.arrow_border_color */
      0x888888U, /*.arrow_shadow_color */
    };

    //
    const gauge_t MY_CUSTOM_GAUGE =
    {
      .items       = items,
      .items_count = sizeof(items)/sizeof(ruler_item_t),
      .palette     = palette,
      .start       = -45.0f,
      .end         =  45.0f
    };

  }

```




### Integrating the gauge

```C++

#include <M5Unified.h>
#include <LGFXMeter.h>


void setup()
{
  M5.begin();

  auto cfg = LGFXMeter::config( MY_CUSTOM_GAUGE );

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

  // fill screen with a color from the gauge palette
  M5.Lcd.fillScreen( cfg.palette->transparent_color );

  ICSGauge = new Gauge_Class( cfg );
  ICSGauge->pushGauge(); // render empty gauge (no needle yet)
}


void loop()
{

  // measure some value
  int mySensorValue = analogRead( mySensorPinNumber );

  // map() it to the gauge angular range [0...90]
  float my_angle = utils::mapFloat( mySensorValue, 0, 4095, 0.0, 90.0 );

  // Either animate (300ms blocking) ...
  ICSGauge->animateNeedle( my_angle );

  // .. or use eased drawing (300ms non blocking) ...
  ICSGauge->setNeedle( my_angle );
  ICSGauge->easeNeedle( 300 );
  // ICSGauge->easeNeedle( 300, easing::easeOutBounce );
  // /!\ See lgfxmeter_types.hpp for complete list of available easing function
  // Function names

  // .. or just render the needle without easing or animation
  ICSGauge->drawNeedle( my_angle );

}



```


### Custom needle and background

Background, needle and needle shadow images can be any of the following formats:

  - PNG: `IMAGE_PNG`
  - QOI: `IMAGE_QOI`
  - JPG: `IMAGE_JPG`
  - BMP: `IMAGE_BMP`
  - RAW: `IMAGE_RAW` (e.g. Sprite, untested)


As seen in the examples, image data can be stored in byte arrays.

```C++

  const image_t bgImg         = { 16, bg_png,                 bg_png_len,                 IMAGE_PNG, 320, 240 };
  const image_t vuMeterArrow  = { 16, clock_arrow_png,        clock_arrow_png_len,        IMAGE_PNG, 16, 144 };
  const image_t vuMeterShadow = { 16, clock_arrow_shadow_png, clock_arrow_shadow_png_len, IMAGE_PNG, 16, 144 };

```

Custom background, needle and shadow can be setup as follows:


```C++
  // Optionally use pre-rendered arrow and custom shadow
  // - Image can be any size (scale will constrained) but smaller is faster, transparent png works
  // - Pointy end of the arrow goes on the top
  // - Shadow image must have same dimensions as arrow image
  cfg.needleCfg.needleImg = &vuMeterArrow;
  cfg.needleCfg.shadowImg = &vuMeterShadow;
  //cfg.needleCfg.scaleX = 0.5; // scaling down a stretched image to produce nicer antialiased result
  //cfg.needle.axis = { GaugeWidth/2, GaugePosY + GaugeHeight }; // will be automatically positioned with a clunky calculation otherwise


  // Optionally share a background image between TFT and the gauge sprite
  // - Image must be png/jpg/bmp/qoi or byte array
  // - Image dimensions must be tft.width() * tft.height()
  cfg.bgImage   = &bgImg;

  // draw the gauge background shared image
  M5.Lcd.drawPng( bgImg.data, bgImg.len );

```

Background can eventually be changed after gauge creation, but it will remove any previously drawn rulers.
However if the gauge is built without rulers and uses a simple background image, then custom modes (e.g. dark/light) are possible.

```C++

  // firt create your image entity
  const image_t alternateBgImage = { 16, my_image_data, my_image_data_len, IMAGE_PNG, GaugeWidth, GaugeWidth };

  // overwrite the gauge background (will also remove the rulers !)
  utils::drawImage( ICSGauge->getGaugeSprite(), alternateBgImage, 0, 0 );

  // eventually change the transparency color depending on the saturation
  needle::cfg.transparent_color = is_background_dark ? 0x000000U : 0xffffffU;

  // or toggle needle shadow in dark mode
  needle::cfg.drop_shadow       = is_background_dark ? false : true;


```



## Credits:

- [@armel](https://github.com/armel) A.K.A. F4HWN
- [@lovyan03](https://github.com/lovyan03)


