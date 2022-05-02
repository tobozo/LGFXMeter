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



namespace LGFXMeter
{


  namespace needle
  {

    using namespace utils;
    using namespace easing;

    needle_cfg_t cfg =
    {
      .display           = nullptr,
      .gaugeSprite       = nullptr,
      .needleImg         = nullptr, // default is a 2bpp triangle
      .shadowImg         = nullptr, // default is copy of needleImg with palette forced to shadow_color
      .width             = 8,
      .height            = 160,
      //.angleOffset       = 0,       // 180.0 if upside down
      .clipRect          = {0,0,0,0},
      .start             = -45.0,
      .end               =  45.0,
      .axis              = {0,0},
      .fill_color        = 0x222222U,
      .border_color      = 0xaaaaaaU,
      .transparent_color = 0xffffffU,
      .shadow_color      = 0x888888U,
      .drop_shadow       = true,
      .shadowOffX        = 0,   // px
      .shadowOffY        = 10,  // px
      .radius            = 1.0, // [0...1] needle radius, fraction of axis.y
      .scaleX            = 1.0  // arrow hscale
    };

    needle_cfg_t config() { return cfg; }


    class Needle_Class
    {
    public:

      Needle_Class( needle_cfg_t _cfg )
      {
        assert( _cfg.gaugeSprite );
        assert( _cfg.display );
        cfg          = _cfg;
        ICSDisplay   = cfg.display;
        gaugeSprite  = cfg.gaugeSprite;
        lastAngle    = cfg.start;
        createNeedle();
      };

      easingFunc_t easingFunc = easing_easeInOutQuart;

      void render( float angle );
      void animate( float_t angle);
      bool ready() { return _ready; }

    private:

      ICS_Display *ICSDisplay  = nullptr;
      ICS_Sprite *clipSprite   = nullptr;
      ICS_Sprite *needleSprite = nullptr;
      ICS_Sprite *shadowSprite = nullptr;
      ICS_Sprite *gaugeSprite  = nullptr;

      bool _has_rendered = false;
      bool _ready        = false;
      bool _debug        = false;

      uint16_t xMiddle; // for pivot
      int32_t shadowOffX, shadowOffY; // shadow offset
      float scaleX, scaleY;
      uint16_t yhigh, ylow;

      clipRect_t lastclipRect = {0,0,0,0};

      float lastAngle = -45.0f;

      void createNeedle();
      clipRect_t getArrowBoundingRect( coord_t *pt_high, coord_t *pt_low, coord_t *pt_axis, float angle );
      void pushNeedle(LovyanGFX* dst, float dst_x, float dst_y, float angle, float zoom_x, float zoom_y, uint32_t transparent_color );

    };



    void Needle_Class::createNeedle()
    {

      if( ! clipSprite ) {
        clipSprite = new ICS_Sprite( ICSDisplay );
        clipSprite->setColorDepth( gaugeSprite->getColorDepth() );
        // psram sprites are slow, force dram use
        clipSprite->setPsram( false );
      }

      if( cfg.needleImg ) {
        cfg.width  = cfg.needleImg->width;
        cfg.height = cfg.needleImg->height;
      }

      xMiddle = cfg.width/2; // pointy end horizontal pos

      if( !needleSprite ) {

        needleSprite = new ICS_Sprite( ICSDisplay );
        needleSprite->setColorDepth( cfg.needleImg ? cfg.needleImg->bit_depth : 4 );
        // psram sprites are slow, force dram use, the sprite is small anyway
        needleSprite->setPsram( false );

        if( ! needleSprite->createSprite( cfg.width, cfg.height ) ) {
          log_e("Unable to create needle sprite :(");
          return;
        }

        needleSprite->setPaletteColor( 0, cfg.transparent_color );
        needleSprite->setPaletteColor( 1, cfg.fill_color );
        needleSprite->setPaletteColor( 2, cfg.border_color );
        needleSprite->setPaletteColor( 3, cfg.shadow_color );

        if( cfg.needleImg ) { // 32x40 png data was provided

          if( ! cfg.shadowImg ) {
            cfg.drop_shadow = false;
          }

          needleSprite->fillSprite( cfg.transparent_color );
          drawImage( needleSprite, cfg.needleImg, 0, 1, cfg.width, cfg.height );

        } else {

          // this needle will be drawn with pointy end to the bottom, so let's compensate
          needleSprite->setRotation( 2 );

          // since it's a filled shape, it can be re-used to drop a shadow
          cfg.drop_shadow = true;

          coord_t bottom_left  = { 0,         0          },
                  bottom_right = { cfg.width, 0          },
                  top_middle   = { xMiddle,   cfg.height },
                  top_left     = { 0,         cfg.height },
                  top_right    = { cfg.width, cfg.height }
          ;

          // Proceed by filling with opaque color and cutting edges with transparent color.
          // The triangle gets a larger pointy end so that the antialias doesn't discard it.
          needleSprite->fillSprite( 2 );
          // draw a triangle, axis will be in the middle like a compass
          needleSprite->fillTriangle( bottom_left.x,bottom_left.y,     bottom_right.x,bottom_right.y, top_middle.x,   top_middle.y, 1 ); // fill red
          needleSprite->fillTriangle( bottom_left.x-2,bottom_left.y,   top_left.x-2,top_left.y,       top_middle.x-2, top_middle.y, cfg.transparent_color ); // crop corner
          needleSprite->fillTriangle( bottom_right.x+2,bottom_right.y, top_right.x+2,top_right.y,     top_middle.x+2, top_middle.y, cfg.transparent_color ); // crop corner
          needleSprite->fillRect( 0, top_middle.y-1, needleSprite->width(), 1, cfg.transparent_color ); // delete top scanline

        }
      }

      // set pivot at bottom center of the sprite
      needleSprite->setPivot( needleSprite->width()/2, needleSprite->height() );
      if( _debug ) needleSprite->drawRect( 0,0, needleSprite->width(), needleSprite->height(), cfg.shadow_color );

      if( cfg.drop_shadow ) {

        shadowOffX = cfg.shadowOffX;
        shadowOffY = cfg.shadowOffY;

        shadowSprite = new ICS_Sprite( ICSDisplay );
        shadowSprite->setColorDepth( cfg.shadowImg ? cfg.shadowImg->bit_depth : 4 );
        // psram sprites are slow, force dram use, the sprite is small anyway
        shadowSprite->setPsram( false );

        if( ! shadowSprite->createSprite( cfg.width, cfg.height ) ) {
          log_e("Unable to create shadow sprite, disabling drop shadow");
          cfg.drop_shadow = false;
        } else {

          shadowSprite->setPaletteColor( 3, cfg.transparent_color );
          shadowSprite->setPaletteColor( 2, cfg.shadow_color );
          shadowSprite->setPaletteColor( 1, cfg.shadow_color );

          if( cfg.shadowImg ) { // shadow image was provided
            shadowSprite->fillSprite( cfg.transparent_color );
            drawImage( shadowSprite, cfg.shadowImg, 0, 1, cfg.width, cfg.height );
          } else { // copy needle image with palette forced to shadow_color
            needleSprite->pushSprite( shadowSprite, 0, 0 );
          }

          // set pivot at bottom center of the sprite
          shadowSprite->setPivot( shadowSprite->width()/2, shadowSprite->height() );
        }

      }

      // figure out the horizon line since no pixel shoud be drawn underneath
      float minRadius = mapFloat( cfg.axis.y-cfg.clipRect.h, 0, cfg.axis.y, 0.0, 1.0 );
      // apply custom radius if any
      ylow  = minRadius  *cfg.axis.y;
      yhigh = cfg.radius *cfg.axis.y;

      scaleX = cfg.scaleX;
      scaleY = float(yhigh)/float(needleSprite->height()); // match half needle size to radius size

      log_d("Needle CFG: high=%d, low=%d, scaleX=%.2f, scaleY=%.2f, shadow: %s", yhigh, ylow, scaleX, scaleY, cfg.drop_shadow?(cfg.shadowImg?"img":"true"):"false" );

      _ready = true;
    }



    void Needle_Class::animate( float_t angle )
    {
      if( angle == lastAngle ) return;
      if( !_ready ) return;

      // constrain to gauge values
      //if( angle<cfg.start ) angle = cfg.start;
      //if( angle>cfg.end )   angle = cfg.end;

      uint32_t animationDuration = 300;
      uint32_t animationStart    = millis();
      uint32_t animationElapsed  = 0;
      uint32_t animationFrames   = 0;

      while( animationElapsed < animationDuration ) {

        float fElapsed, tmp_angle, angleEased;
        if( angle > lastAngle ) {

          fElapsed = mapFloat( float(animationElapsed), 0.0f, float(animationDuration), 0.0f, 1.0f );
          angleEased = easingFunc( fElapsed );
          tmp_angle = mapFloat( angleEased, 0.0, 1.0, lastAngle, angle );

        } else {

          fElapsed = mapFloat( float(animationElapsed), 0, float(animationDuration), 1.0, 0.0 );
          angleEased = easingFunc( fElapsed );
          tmp_angle = mapFloat( angleEased, 0.0, 1.0, angle, lastAngle );

        }

        render( tmp_angle );
        animationFrames++;
        animationElapsed = millis() - animationStart;

      }

      uint32_t animationEnd = millis();
      animationDuration = animationEnd-animationStart;
      float fps = float(animationFrames)/float(animationDuration) * 1000.0;
      log_d("[%+06.2f=>%+06.2f]@[%3d:%-3d][%3d*%-3d] %d frames in %d ms (=%.2f fps)", lastAngle, angle, lastclipRect.x, lastclipRect.y, lastclipRect.w, lastclipRect.h, animationFrames, animationDuration, fps );

      lastAngle = angle;
    }




    void Needle_Class::render( float absangle )
    {
      if( !_ready ) return;
      // TODO: constrain to gauge values
      //if( absangle<cfg.start ) absangle = cfg.start;
      //if( absangle>cfg.end )   absangle = cfg.end;

      float angle            = -cfg.start - absangle; // translate to relative
      coord_t pt_high        = {0, yhigh};
      coord_t pt_low         = {0, ylow};

      int32_t x              = cfg.axis.x;
      int32_t y              = cfg.axis.y;

      // calculate clip rect for the needle
      clipRect_t currentClip = getArrowBoundingRect( &pt_high, &pt_low, &cfg.axis, angle );

      if( cfg.drop_shadow ) {
        // extend clipRect accordingly
        coord_t shadow_axis = cfg.axis;
        shadow_axis.x += shadowOffX;
        shadow_axis.y += shadowOffY;
        clipRect_t shadowClip = getArrowBoundingRect( &pt_high, &pt_low, &shadow_axis, angle );
        currentClip = getBoundingRect( currentClip, shadowClip );
      }

      if( !_has_rendered ) {
        // avoid clipping with null coords on first render
        lastclipRect = currentClip;
        _has_rendered = true;
      }

      clipRect_t mergedClip = getBoundingRect( currentClip, lastclipRect );
      // constrain clip height to draw zone
      //mergedClip.h = (mergedClip.y+mergedClip.h < y) ? mergedClip.h : mergedClip.h-( mergedClip.y - y );

      // used by pushSprite, based on clipRect
      clipRect_t absClip = constrainClipRect({
        mergedClip.x+cfg.clipRect.x,
        mergedClip.y+cfg.clipRect.y,
        mergedClip.w,
        mergedClip.h
      }, cfg.clipRect );

      // used by pushRotateZoomWithAA, based on axis
      clipRect_t relClip = {
        x-mergedClip.x,
        y-mergedClip.y,
        mergedClip.w,
        mergedClip.h
      };

      bool merge_render  = false;
      bool sprite_needle = false;

      angle = 360-(angle/*+cfg.angleOffset*/); // translate to lgfx pivot/rotate defaults

      // test if last and current clip overlap
      if( !inRange( currentClip.x, currentClip.x+currentClip.w, lastclipRect.x )
      && !inRange( lastclipRect.x, lastclipRect.x+lastclipRect.w, currentClip.x ) ) {
        // no overlapping, two zones need redraw
        sprite_needle = clipSprite->createSprite( currentClip.w, currentClip.h );
      } else {
        // overlapping, will create a sprite to clear last needle then draw the new needle
        merge_render = clipSprite->createSprite( absClip.w, absClip.h );
        //if(!merge_render) log_d("Duh!");
      }

      if( merge_render ) { // clear + draw needle in a single sprite

        ICSDisplay->setClipRect( absClip.x, absClip.y, absClip.w, absClip.h );
        // restore to background
        gaugeSprite->pushSprite( clipSprite, cfg.clipRect.x-absClip.x, cfg.clipRect.y-absClip.y );
        // draw needle
        pushNeedle( clipSprite, relClip.x, relClip.y, angle, scaleX, scaleY, cfg.transparent_color );

        // DEBUG
        if( _debug ) clipSprite->drawRect( 0, 0, clipSprite->width(),clipSprite->height(), TFT_BLACK );
        clipSprite->pushSprite(  absClip.x, absClip.y );
        clipSprite->deleteSprite();

      } else {

        // clear last needle
        ICSDisplay->setClipRect( lastclipRect.x+cfg.clipRect.x, lastclipRect.y+cfg.clipRect.y, lastclipRect.w, lastclipRect.h );
        gaugeSprite->pushSprite( ICSDisplay, cfg.clipRect.x, cfg.clipRect.y );

        // draw new needle
        ICSDisplay->setClipRect( currentClip.x, cfg.clipRect.y, currentClip.w, cfg.clipRect.h );

        if( sprite_needle ) { // have to do this to avoid antialiasing with TFT

          clipSprite->fillSprite( cfg.transparent_color );
          pushNeedle( clipSprite, x - currentClip.x + cfg.clipRect.x, y - currentClip.y + cfg.clipRect.y, angle, scaleX, scaleY, cfg.transparent_color );
          clipSprite->pushSprite(  currentClip.x, currentClip.y, cfg.transparent_color );
          clipSprite->deleteSprite();

        } else { // duh! not enough memory to use a sprite, antialias will blend to default black from TFT :(

          pushNeedle( ICSDisplay, x+cfg.clipRect.x, y+cfg.clipRect.y, angle, scaleX, scaleY, cfg.transparent_color );

        }
      }

      ICSDisplay->clearClipRect();
      lastclipRect = currentClip;
    }



    void Needle_Class::pushNeedle(LovyanGFX* dst, float dst_x, float dst_y, float angle, float zoom_x, float zoom_y, uint32_t transparent_color )
    {
      if( cfg.drop_shadow ) shadowSprite->pushRotateZoomWithAA(dst, dst_x+shadowOffX, dst_y+shadowOffY, angle, zoom_x, zoom_y, transparent_color  );
      needleSprite->pushRotateZoomWithAA(dst, dst_x, dst_y, angle, zoom_x, zoom_y, transparent_color  );
    }



    clipRect_t Needle_Class::getArrowBoundingRect( coord_t *pt_high, coord_t *pt_low, coord_t *pt_axis, float angle )
    {

      int32_t x = pt_axis->x, y = pt_axis->y;

      coord_t pt_ul=*pt_high, // top left
              pt_ur=*pt_high, // top right
              pt_bl=*pt_low,  // bottom left
              pt_br=*pt_low   // bottom right
      ;

      // calculate box edges + 1px outline
      pt_ul.x -= (xMiddle*scaleX)+1;
      pt_ur.x += (xMiddle*scaleX)+1;
      pt_bl.x -= (xMiddle*scaleX)+1;
      pt_br.x += (xMiddle*scaleX)+1;

      pt_ul.y -= 1;
      pt_ur.y -= 1;
      pt_bl.y += 1;
      pt_br.y += 1;

      // rotate coords
      coord_rotate( &pt_ul, angle );
      coord_rotate( &pt_ur, angle );
      coord_rotate( &pt_bl, angle );
      coord_rotate( &pt_br, angle );
      // translate to axis axis coords
      pt_ul = { x+pt_ul.x, y-pt_ul.y };
      pt_ur = { x+pt_ur.x, y-pt_ur.y };
      pt_bl = { x+pt_bl.x, y-pt_bl.y };
      pt_br = { x+pt_br.x, y-pt_br.y };
      // get max/min values
      minmax_t minMax =
      {
        .minx = pt_ul.x,
        .miny = pt_ul.y,
        .maxx = pt_ul.x,
        .maxy = pt_ul.y
      };
      minMax = getMinMax( minMax, pt_ur );
      minMax = getMinMax( minMax, pt_bl );
      minMax = getMinMax( minMax, pt_br );
      // deduce clip rect
      clipRect_t ret = {
        minMax.minx,
        minMax.miny,
        minMax.maxx-minMax.minx,
        minMax.maxy-minMax.miny
      };
      // constrain to height
      if( ret.y+ret.h > cfg.clipRect.h ) {
        float vdiff = (ret.y+ret.h) - cfg.clipRect.h;
        ret.h -= vdiff;

        /*
        // TODO:
        // more "after resize" narrowing can be done
        // if discarded zone is behind horizon

        float hyp   = vdiff/cos( abs(angle) );
        float hdiff = sqrt(hyp*hyp - vdiff*vdiff);

        if( _debug ) {
          log_d("angle=%.2f, vdiff=%.2f, hdiff=%.2f", angle, vdiff, hdiff );
        }
        if( vdiff > 1 ) {
          if( angle <= 0 ) {
            ret.w -= hdiff;
            ret.x += hdiff;
          }
        }

        */
      }


      return ret;
    }


  }; // end namespace needle


  using Needle_Class = needle::Needle_Class;

}; // end namespace LGFXMeter
