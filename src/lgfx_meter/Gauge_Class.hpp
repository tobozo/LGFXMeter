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

#include "lgfxmeter_types.hpp"
#include "Needle_Class.hpp"
#include <vector>


namespace LGFXMeter
{

  namespace gauge
  {

    using namespace utils;

    const image_t default_background = // default background (transparent)
    {
      .bit_depth = 8,
      .data      = nullptr,
      .len       = 0,
      .type      = IMAGE_RAW
    };

    const gauge_palette_t default_palette = // default color palette
    {
      0xffffffU, /*.transparent_color  */
      0x000000U, /*.fill_color         */
      0xff4444U, /*.warn_color         */
      0x66ff66U, /*.ok_color           */
      0xff2222U, /*.arrow_color        */
      0xaaaaaaU, /*.arrow_border_color */
      0x888888U, /*.arrow_shadow_color */
    };

    const gauge_t default_gauge_set =
    {
      .items       = nullptr,
      .items_count = 0,
      .palette     = default_palette,
      .start       = -45.0f,
      .end         =  45.0f,
    };

    gauge_cfg_t cfg = // default gauge config
    {
      .dstCanvas = nullptr, // sprite
      .display   = nullptr, // lcd display
      .clipRect  = {
        .x         = 0,   // relative to the display
        .y         = 0,   // relative to the display
        .w         = 320, // gauge canvas width
        .h         = 160  // gauge canvas height
      },
      .gauge     = default_gauge_set,
      .zoomAA    = 0.5f, // antialias scale value, set to 0.5 or lower for smoothing, 1.0 if ram issues or 2.0 for ugly pixelated result
      .bgImage   = &default_background,
      .needleCfg = needle::config(),
      .palette   = &default_gauge_set.palette
    };


    class Gauge_Class
    {
    public:

      Gauge_Class( gauge_cfg_t _cfg = gauge::cfg )
      {
        assert( _cfg.display );
        cfg = _cfg;
        clipRect = &cfg.clipRect;
        setupGauge();
      };

      void pushGauge();
      void drawNeedle( float angle, bool render_value = false );
      void animateNeedle( float_t angle, bool render_value = false );

    private:

      Needle_Class  *Needle      = nullptr;
      ICS_Sprite    *spriteMask  = nullptr;
      ICS_Sprite    *gaugeSprite = nullptr;
      const image_t *bgImage     = nullptr;

      //const float deg2width = 2*PI/180.0f;
      const float _offset = -90.0; // "0 degree middle top" ref angle for drawing

      coord_t axis;
      int32_t dstPosX, dstPosY;

      clipRect_t *clipRect = nullptr;

      float dstShrinkLevel; // = destination_canvas_size / mask_size, shrunk for forcing antialias
      float maskScale; // = mask_size / destination_canvas_size, used for downscaling/upscaling

      bool _is_transparent = false;
      bool _ready          = false;
      bool _debug          = false;

      std::vector<ruler_item_t> rulers;

      void setupGauge();
      void initNeedle();
      bool initMask();

      void addRulerItems();
      void addRulerItem( const ruler_item_t item );

      void drawRulers();
      void drawRuler( const ruler_t *ruler, uint32_t ruler_color );

      void drawAngleValue( float angle );

      void drawInfiniteSign( coord_t coords, uint32_t color_index );
      void maskFillArcZoom( int32_t x, int32_t y, int32_t radius0, int32_t radius1, float angle0, float angle1, float zoom, int32_t color_index );

    };


    void Gauge_Class::setupGauge()
    {
      // background image behind the gauge
      bgImage        = cfg.bgImage;
      axis           = { clipRect->w/2, int(clipRect->w*.6875f) }; // TODO: find a better way to position the axis
      dstShrinkLevel = cfg.zoomAA;
      maskScale      = 1.0/cfg.zoomAA;
      // adjust to screen proportions
      maskScale     *= clipRect->w/cfg.display->width();

      if( cfg.dstCanvas ) {
        log_d("Using provided background canvas");
        gaugeSprite = cfg.dstCanvas;
      } else {
        uint8_t bit_depth = bgImage ? bgImage->bit_depth : default_background.bit_depth;
        gaugeSprite = new ICS_Sprite( cfg.display );
        gaugeSprite->setColorDepth( bit_depth );
        // psram sprites are slow, default behaviour is to use dram, override this with cfg.dstCanvas
        gaugeSprite->setPsram( false );
        if( !gaugeSprite->createSprite( clipRect->w, clipRect->h ) ) {
          log_e("Can't create gauge canvas :(");
          return;
        }

        bool has_background_image = bgImage && bgImage->data && bgImage->len > 0;

        if( has_background_image ) { // render the provided background image

          drawImage( gaugeSprite, bgImage, -clipRect->x, -clipRect->y/*, clipRect->w, clipRect->h*/ );

        } else {
          // no background image provided, fill with transparent color
          gaugeSprite->fillSprite( cfg.palette->transparent_color );
          _is_transparent = true;
          // TODO: cropped circle mask - gaugeSprite->fillCircle( axis.x, axis.y, axis.y-clipRect->h, 0xeeeeee);
          // gaugeSprite->pushSprite( clipRect->x, clipRect->y, cfg.palette->transparent_color );
        }
        log_d("Using Generated %dbpp gauge canvas with %s backgound", bit_depth, has_background_image ? "png" : "transparent" );
      }

      // pushRotated destination coords
      dstPosX  = clipRect->w/2;
      dstPosY  = clipRect->h/2;
      // render
      addRulerItems();
      if( initMask() ) {
        drawRulers();
        spriteMask->deleteSprite();
        initNeedle();
      }
    }



    bool Gauge_Class::initMask()
    {
      uint8_t bit_depth = 4;
      spriteMask = new ICS_Sprite();
      spriteMask->setColorDepth( bit_depth );
      // this sprite will be discarded after initial rendering and doesn't need to explicitely sit in dram
      spriteMask->setPsram( psramInit() );
      // calculate mask size
      int32_t maskWidth   = clipRect->w/dstShrinkLevel;
      int32_t maskHeight  = clipRect->h/dstShrinkLevel;
      // create mask sprite
      if( !spriteMask->createSprite( maskWidth, maskHeight ) ) {
        log_e("Not enough ram to create mask canvas. Hint: create the object earlier in the setup, or set cfg.zoomAA to 1.0.");
        return false;
      }
      log_d("Successfully Created %dbpp mask canvas %dx%d with %.2f scale factor", bit_depth, maskWidth, maskHeight, dstShrinkLevel );
      log_d("clipRect[%3d:%-3d][%3dx%-3d] axis[%3d:%-3d] scale=%.2f", clipRect->x, clipRect->y, clipRect->w, clipRect->h, axis.x,  axis.y, maskScale );

      spriteMask->setTextDatum( MC_DATUM );
      spriteMask->setPaletteColor( 0, cfg.palette->transparent_color );
      spriteMask->setPaletteColor( 1, cfg.palette->fill_color );
      spriteMask->setPaletteColor( 2, cfg.palette->warn_color );
      spriteMask->setPaletteColor( 3, cfg.palette->ok_color );
      spriteMask->fillSprite( cfg.palette->transparent_color );

      return true;
    }


    void Gauge_Class::initNeedle()
    {
      // prepare needle config
      auto needleCfg = &cfg.needleCfg;
      needleCfg->display           = cfg.display;
      needleCfg->gaugeSprite       = gaugeSprite;
      needleCfg->clipRect          = cfg.clipRect;
      needleCfg->start             = cfg.gauge.start;
      needleCfg->end               = cfg.gauge.end;
      needleCfg->axis              = axis;
      needleCfg->fill_color        = cfg.palette->needle_color;
      needleCfg->border_color      = cfg.palette->needle_border_color;
      needleCfg->transparent_color = cfg.palette->transparent_color;
      needleCfg->shadow_color      = cfg.palette->needle_shadow_color;

      Needle = new Needle_Class( cfg.needleCfg );
      _ready = Needle->ready();
    }



    void Gauge_Class::maskFillArcZoom( int32_t x, int32_t y, int32_t radius0, int32_t radius1, float angle0, float angle1, float zoom, int32_t color_index )
    {
      spriteMask->fillArc( x*zoom, y*zoom, radius0*zoom, radius1*zoom, angle0, angle1, color_index );
      // DEBUG destination zone
      if( _debug ) spriteMask->drawRect(0,0, spriteMask->width(), spriteMask->height(), 1 );
    }


    void Gauge_Class::drawRuler( const ruler_t *ruler, uint32_t color_index )
    {
      int32_t x        = axis.x;
      int32_t y        = axis.y;
      int32_t radius0  = ruler->radius;
      int32_t radius1  = ruler->radius + ruler->width;
      float angleStart = ruler->angleStart + _offset + cfg.gauge.start;
      float angleEnd   = ruler->angleEnd   + _offset + cfg.gauge.start;

      log_v("Filling initial arc [%d:%d] radius0=%d, radius1=%d, zoom:%.2f, angle[%.2f-%.2f]", x, y, radius0, radius1, maskScale, angleStart, angleEnd );
      maskFillArcZoom( x, y, radius0, radius1, angleStart, angleEnd, maskScale, color_index ); // initial arc

      if( ruler->units_count > 0 ) {
        log_v("Parsing %d units", ruler->units_count );
        for( int i=0; i<ruler->units_count; i++ ) {

          ruler_unit_t *unit = (ruler_unit_t *)&ruler->units[i];
          float angle        = angleStart+unit->angle;
          bool has_ruler     = unit->size != 0;
          bool has_label     = unit->label != nullptr;

          if( has_ruler ) {
            // translated ruler width is based on radius1
            radius1 = ruler->radius + ruler->width * unit->size;
            float rulerWidth = 1.0f/(radius0*deg2width);
            log_v("Unit %2d has ruler: [%d:%d] radius0=%d, radius1=%d, zoom:%.2f, angle[%.2f-%.2f]", i, x, y, radius0, radius1, maskScale, angle - rulerWidth, angle + rulerWidth );
            maskFillArcZoom( x, y, radius0, radius1, angle - rulerWidth, angle + rulerWidth, maskScale, color_index );
          }
          if( has_label ) {
            spriteMask->setTextDatum( unit->textDatum );
            spriteMask->setFont( unit->fontFace );
            // reusing radius1 as font position offset, based on downscaled font size for centering
            spriteMask->setTextSize( unit->fontSize );
            radius1 = radius0 + unit->distance + float(spriteMask->fontHeight()/2.0) * (unit->distance>0?1.0:-1.0);
            // now apply mask zoom level to font
            spriteMask->setTextSize( maskScale * unit->fontSize );
            // find x/y position relative to axis using trigonometry
            coord_t fontPos = {0, radius1 };
            coord_rotate( &fontPos, 360.0f-(angle-_offset) );
            fontPos = { x + fontPos.x, y - fontPos.y };

            if( unit->label == "∞" ) {
              // draw infinite sign since no utf8 capable embedded font seem to provide it :-(
              drawInfiniteSign( fontPos, color_index );
            } else {
              spriteMask->setTextColor( color_index );
              spriteMask->drawString( unit->label, fontPos.x * maskScale, fontPos.y * maskScale );
            }
            log_v("Unit %2d has label: '%s' [%d:%d]", i, unit->label, int(fontPos.x*maskScale), int(fontPos.y*maskScale) );
          }
        }
      }

      spriteMask->pushRotateZoomWithAA( gaugeSprite, dstPosX, dstPosY, 0.0, dstShrinkLevel, dstShrinkLevel, cfg.palette->transparent_color );
    }


    void Gauge_Class::pushGauge()
    {
      if( ! _ready ) return;
      if( _is_transparent ) {
        gaugeSprite->pushSprite( clipRect->x, clipRect->y, cfg.palette->transparent_color );
      } else {
        gaugeSprite->pushSprite( clipRect->x, clipRect->y/*, cfg.palette->transparent_color*/ );
      }
    }


    void Gauge_Class::drawInfiniteSign( coord_t coords, uint32_t color_index )
    {
      float arcRadius0 = spriteMask->fontHeight()/8.0;
      float arcRadius1 = arcRadius0 + spriteMask->fontHeight()/24.0;
      float x1         = coords.x-arcRadius0;
      float x2         = coords.x+arcRadius0;
      float y1         = coords.y-arcRadius0;
      maskFillArcZoom( x1, y1, arcRadius0, arcRadius1, 0, 360, maskScale, color_index );
      maskFillArcZoom( x2, y1, arcRadius0, arcRadius1, 0, 360, maskScale, color_index );
    }


    void Gauge_Class::drawAngleValue( float angle )
    {
      spriteMask->setTextColor( TFT_BLACK );
      spriteMask->setFont( &FreeMonoBold9pt7b );
      spriteMask->setTextDatum( ML_DATUM );
      spriteMask->setTextSize(1);
      spriteMask->setPsram( false );
      char degreeStr[12] = {0};
      snprintf(degreeStr, 11, "< %.2f", angle );
      int32_t clipWidth  = spriteMask->textWidth("<=00.00") + 3;
      int32_t clipHeight = spriteMask->fontHeight()*1.3;
      spriteMask->setColorDepth( gaugeSprite->getColorDepth() );

      if( spriteMask->createSprite( clipWidth, clipHeight ) ) {
        spriteMask->setClipRect( 0, 0, clipWidth, clipHeight );
        gaugeSprite->pushSprite( spriteMask, 0, 0 );
        spriteMask->drawString( degreeStr, 3, clipHeight/2 );
        spriteMask->pushSprite( cfg.display, 0, 0 );
        spriteMask->deleteSprite();
      }
    }



    void Gauge_Class::drawRulers()
    {
      for( int i=0; i<rulers.size(); i++ ) {
        drawRuler( rulers[i].ruler, rulers[i].color_index );
      }
    }


    void Gauge_Class::addRulerItems()
    {
      for( int i=0; i<cfg.gauge.items_count; i++ ) {
        addRulerItem( cfg.gauge.items[i] );
      }
    }


    void Gauge_Class::addRulerItem( const ruler_item_t item )
    {
      rulers.push_back( item );
    }


    void Gauge_Class::drawNeedle( float angle, bool render_value )
    {
      if( Needle )
        Needle->render( angle );

      if( render_value ) {
        drawAngleValue( angle );
      }
    }


    void Gauge_Class::animateNeedle( float_t angle, bool render_value )
    {
      if( Needle )
        Needle->animate( angle );

      if( render_value ) {
        drawAngleValue( angle );
      }

    }

  }; // end namespace gauge



  gauge_cfg_t config()
  {
    return gauge::cfg;
  }

  gauge_cfg_t config( const gauge_t _gauge )
  {
    gauge::cfg.gauge = _gauge;
    gauge::cfg.palette = &gauge::cfg.gauge.palette;
    return gauge::cfg;
  }

  // export class to local namespace
  using Gauge_Class = gauge::Gauge_Class;


}; // end namespace LGFXMeter

using namespace LGFXMeter;

