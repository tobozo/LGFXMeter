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

// normalize object names across LGFX implementations

#if __has_include(<M5Unified.h>)
  #include <M5Unified.h> // https://github.com/m5stack/M5Unified
  #define ICS_Sprite LGFX_Sprite
  #define ICS_Display LGFX_Device
#elif __has_include(<LovyanGFX.h>)
  #define LGFX_AUTODETECT
  #define LGFX_USE_V1
  #include <LovyanGFX.h> // https://github.com/lovyan03/LovyanGFX
  #define ICS_Sprite LGFX_Sprite
  #define ICS_Display LGFX
#elif __has_include(<ESP32-Chimera-Core.h>)
  #include <ESP32-Chimera-Core.h> // https://github.com/tobozo/ESP32-Chimera-Core
  #define ICS_Sprite LGFX_Sprite
  #define ICS_Display LGFX
#else
  #error "This demo requires a LGFX family driver"
#endif

namespace LGFXMeter
{

  enum image_type_t
  {
    IMAGE_PNG,
    IMAGE_JPG,
    IMAGE_BMP,
    IMAGE_QOI,
    IMAGE_RAW
  };

  // image data/len holder
  struct image_t
  {
    uint8_t       bit_depth; // Note: this is NOT the image bit depth but the minimal *canvas* recommended bit depth (e.g. 24bit image can be rendered as 16bit)
    const uint8_t *data;
    uint32_t      len;
    image_type_t  type;
    int32_t       width;
    int32_t       height;
  };

  // coordinates
  struct coord_t
  {
    int32_t x, y;
  };


  struct minmax_t
  {
    int32_t minx, miny, maxx, maxy;
  };


  // clipRect zone
  struct clipRect_t
  {
    int32_t x, y, w, h;
  };

  // color palette for gauge, arrow and transparency
  struct gauge_palette_t
  {
    union
    {
      struct
      {
        uint32_t transparent_color; // must be unique and opposite to fill_color
        uint32_t fill_color;        // must be unique and opposite to transparent_color
        uint32_t warn_color;
        uint32_t ok_color;
        uint32_t needle_color;
        uint32_t needle_border_color;
        uint32_t needle_shadow_color;
      };
      uint32_t colors[7];
    };
  };

  // unit properties
  struct ruler_unit_t
  {
    const uint8_t     idx;       // unit index
    const float       angle;     // position (relative to -45 degrees)
    const char        *label;    // optional text label
    const int8_t      size;      // optional ruler size
    const int8_t      distance;  // text distance
    const lgfx::IFont *fontFace; // can use u8g2 font too
    const float       fontSize;  // e.g use a bigger font, and reduce after for antialias
    const textdatum_t textDatum; // optional text alignment
  };

  // ruler properties
  struct ruler_t
  {
    const float        angleStart;  // starting angle (relative to -45 degrees)
    const float        angleEnd;    // ending angle
    const int32_t      radius;      // radius
    const int32_t      width;       // thickness
    const ruler_unit_t *units;      // array of rules/labels in the ruler
    const size_t       units_count; // array size
  };

  // ruler item
  struct ruler_item_t
  {
    const ruler_t *ruler; // ruler properties
    uint32_t color_index; // palette color index for fill color
  };

  // gauge set
  struct gauge_t
  {
    const ruler_item_t  *items;      // collection of ruler items
    size_t              items_count; // items count
    gauge_palette_t     palette;     // color palette
    float               start;       // ruler angle start
    float               end;         // ruler angle end
  };

  // gauge needle config
  struct needle_cfg_t
  {
    ICS_Display   *display;          // output media (tft or sprite)
    ICS_Sprite    *gaugeSprite;      // background media
    const image_t *needleImg;        // optional needle image
    const image_t *shadowImg;        // optional needle shadow image (same size)
    int32_t       width;             // needle sprite size
    int32_t       height;            // needle sprite size

    clipRect_t    clipRect;          // write zone in output media
    float         start;             // starting angle
    float         end;               // ending angle
    coord_t       axis;              // needle origin
    uint32_t      fill_color;        // fill color
    uint32_t      border_color;      // border color
    uint32_t      transparent_color; // transparency color
    uint32_t      shadow_color;      // shadow color
    bool          drop_shadow;       // render shadow under needle
    float         shadowOffX;        // [-1.0...1.0] shadow offset X, % relative to needle width
    float         shadowOffY;        // [0.0...1.0]  shadow offset Y, % relative to needle width
    float         radius;            // [0...1] // needle max radius
    float         scaleX;            // horizontal scale
  };

  // gauge config
  struct gauge_cfg_t
  {
    ICS_Sprite            *dstCanvas; // output canvas (sprite)
    ICS_Display           *display;   // output media (tft, but sprite works too)
    clipRect_t            clipRect;   // gauge coords + surface
    gauge_t               gauge;   // gauge geometry + rulers/labels
    float                 zoomAA;     // scaling level used for antialiasing
    const image_t         *bgImage;   // background png image
    needle_cfg_t          needleCfg;  // needle config
    const gauge_palette_t *palette;
  };




  namespace utils
  {
    const float deg2rad   = PI/180.0f;
    const float deg2width = 2*deg2rad;

    // apply angular rotation to given coordinates
    void coord_rotate( coord_t *point, float angle )
    {
      coord_t _pt = *point;
      angle *= deg2rad;
      point->x = _pt.x * cos(angle) - _pt.y * sin(angle);
      point->y = _pt.x * sin(angle) + _pt.y * cos(angle);
    }


    // check if 'x' value is between 'low' and 'high'
    bool inRange(int low, int high, int x)
    {
      return (low <= x && x <= high);
    }


    float mapFloat(float x, float in_min, float in_max, float out_min, float out_max)
    {
      return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }


    minmax_t getMinMax( clipRect_t a, clipRect_t b )
    {
      return {
        /*.minx =*/ min( a.x, b.x ),
        /*.miny =*/ min( a.y, b.y ),
        /*.maxx =*/ max( a.x+a.w, b.x+b.w ),
        /*.maxy =*/ max( a.y+a.h, b.y+b.h )
      };
    }


    minmax_t getMinMax( minmax_t minMax, coord_t b )
    {
      return {
        /*.minx =*/ min( minMax.minx, b.x ),
        /*.miny =*/ min( minMax.miny, b.y ),
        /*.maxx =*/ max( minMax.maxx, b.x ),
        /*.maxy =*/ max( minMax.maxy, b.y )
      };
    }


    clipRect_t constrainClipRect( clipRect_t in, const clipRect_t ref )
    {
      int32_t xdiff=0,ydiff=0,wdiff=0,hdiff=0;

      if ( in.x < ref.x ) {
        xdiff = ref.x - in.x;
        in.x = ref.x;
        in.w -= xdiff;
      }
      if ( in.y < ref.y ) {
        ydiff = ref.y - in.y;
        in.y = ref.y;
        in.h -= ydiff;
      }
      if ( in.x+in.w > ref.x+ref.w ) {
        wdiff = (in.x+in.w) - (ref.x+ref.w);
        in.w -= wdiff;
      }
      if ( in.y+in.h > ref.y+ref.h ) {
        hdiff = (in.y+in.h) - (ref.y+ref.h);
        in.h -= hdiff;
      }

      return in;
    }


    clipRect_t getBoundingRect( clipRect_t r1, clipRect_t r2 )
    {
      minmax_t mm = getMinMax( r1, r2 );
      return
      {
        /* x.=*/ mm.minx,
        /* y.=*/ mm.miny,
        /* w.=*/ mm.maxx - mm.minx,
        /* h.=*/ mm.maxy - mm.miny
      };
    }



    void drawImage( LovyanGFX* dst, const image_t *img, int32_t x = 0, int32_t y = 0, int32_t maxWidth = 0, int32_t maxHeight = 0, int32_t offX = 0, int32_t offY = 0 )
    {
      float scale_x = 1.0f;
      float scale_y = 0.0f;

      switch( img->type ) {
        case IMAGE_PNG: dst->drawPng( img->data, img->len, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y ); break;
        case IMAGE_JPG: dst->drawJpg( img->data, img->len, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y ); break;
        case IMAGE_BMP: dst->drawBmp( img->data, img->len, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y ); break;
        case IMAGE_QOI: dst->drawQoi( img->data, img->len, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y ); break;
        case IMAGE_RAW: dst->pushImage( -offX, -offY, maxWidth, maxHeight, img->data, img->len );   break;
      }
    }



  };

  namespace easing
  {
    /*
    * Easing Functions - inspired from http://gizma.com/easing/
    * only considering the t value for the range [0, 1] => [0, 1]
    */
    typedef float (*easingFunc_t)(float in);

      // no easing, no acceleration
    float easing_linear( float t ) { return t; }
      // accelerating from zero velocity
    float easing_easeInQuad( float t ) { return t*t; }
      // decelerating to zero velocity
    float easing_easeOutQuad( float t ) { return  t*(2-t); }
      // acceleration until halfway, then deceleration
    float easing_easeInOutQuad( float t ) { return  t<.5 ? 2*t*t : -1+(4-2*t)*t; }
      // accelerating from zero velocity
    float easing_easeInCubic( float t ) { return t*t*t; }
      // decelerating to zero velocity
    float easing_easeOutCubic( float t ) { return (--t)*t*t+1; }
      // acceleration until halfway, then deceleration
    float easing_easeInOutCubic( float t ) { return  t<.5 ? 4*t*t*t : (t-1)*(2*t-2)*(2*t-2)+1; }
      // accelerating from zero velocity
    float easing_easeInQuart( float t ) { return t*t*t*t; }
      // decelerating to zero velocity
    float easing_easeOutQuart( float t ) { return 1-(--t)*t*t*t; }
      // acceleration until halfway, then deceleration
    float easing_easeInOutQuart( float t ) { return t<.5 ? 8*t*t*t*t : 1-8*(--t)*t*t*t; }
      // accelerating from zero velocity
    float easing_easeInQuint( float t ) { return t*t*t*t*t; }
      // decelerating to zero velocity
    float easing_easeOutQuint( float t ) { return 1+(--t)*t*t*t*t; }
      // acceleration until halfway, then deceleration
    float easing_easeInOutQuint( float t ) { return t<.5 ? 16*t*t*t*t*t : 1+16*(--t)*t*t*t*t; }
  };


};



