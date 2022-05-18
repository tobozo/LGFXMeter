/*\
 *
 * LGFX VU Meter Gauge
 *
 * Code derived from https://github.com/m5stack/M5Unified/blob/master/examples/Advanced/MP3_with_ESP8266Audio/MP3_with_ESP8266Audio.ino
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

#include <SD.h>
#include <HTTPClient.h>
#include <math.h>

/// need ESP8266Audio library. ( URL : https://github.com/earlephilhower/ESP8266Audio/ )
#include <AudioOutput.h>
#include <AudioFileSourceSD.h>
#include <AudioFileSourceID3.h>
#include <AudioGeneratorMP3.h>

#include <M5Unified.h>

#include "assets.h"
#include "VUMeter.hpp"

const int32_t GaugeWidth  = 320;
const int32_t GaugeHeight = 160;
const int32_t GaugePosX   = 0;
const int32_t GaugePosY   = 45;

const image_t bgImg         = { 16, bg_png,                 bg_png_len,                 IMAGE_PNG, 320, 240 };
const image_t vuMeterArrow  = { 16, clock_arrow_png,        clock_arrow_png_len,        IMAGE_PNG, 16, 144 };
const image_t vuMeterShadow = { 16, clock_arrow_shadow_png, clock_arrow_shadow_png_len, IMAGE_PNG, 16, 144 };

Gauge_Class  *VUMeterGauge    = nullptr;

/// set M5Speaker virtual channel (0-7)
static constexpr uint8_t m5spk_virtual_channel = 0;

/// set your mp3 filename
static constexpr const char* filename[] =
{
  "/mp3/file01.mp3",
  "/mp3/file02.mp3",
  "/mp3/file03.mp3",
  "/mp3/file04.mp3",
};
static constexpr const size_t filecount = sizeof(filename) / sizeof(filename[0]);

class AudioOutputM5Speaker : public AudioOutput
{
  public:
    AudioOutputM5Speaker(m5::Speaker_Class* m5sound, uint8_t virtual_sound_channel = 0)
    {
      _m5sound = m5sound;
      _virtual_ch = virtual_sound_channel;
    }
    virtual ~AudioOutputM5Speaker(void) {};
    virtual bool begin(void) override { return true; }
    virtual bool ConsumeSample(int16_t sample[2]) override
    {
      if (_tri_buffer_index < tri_buf_size)
      {
        _tri_buffer[_tri_index][_tri_buffer_index  ] = sample[0];
        _tri_buffer[_tri_index][_tri_buffer_index+1] = sample[1];
        _tri_buffer_index += 2;

        return true;
      }

      flush();
      return false;
    }
    virtual void flush(void) override
    {
      if (_tri_buffer_index)
      {
        _m5sound->playRaw(_tri_buffer[_tri_index], _tri_buffer_index, hertz, true, 1, _virtual_ch);
        _tri_index = _tri_index < 2 ? _tri_index + 1 : 0;
        _tri_buffer_index = 0;
      }
    }
    virtual bool stop(void) override
    {
      flush();
      _m5sound->stop(_virtual_ch);
      return true;
    }

    const int16_t* getBuffer(void) const { return _tri_buffer[(_tri_index + 2) % 3]; }

  protected:
    m5::Speaker_Class* _m5sound;
    uint8_t _virtual_ch;
    static constexpr size_t tri_buf_size = 1536;
    int16_t _tri_buffer[3][tri_buf_size];
    size_t _tri_buffer_index = 0;
    size_t _tri_index = 0;
};


#define FFT_SIZE 256
class fft_t
{
  float _wr[FFT_SIZE + 1];
  float _wi[FFT_SIZE + 1];
  float _fr[FFT_SIZE + 1];
  float _fi[FFT_SIZE + 1];
  uint16_t _br[FFT_SIZE + 1];
  size_t _ie;

public:
  fft_t(void)
  {
#ifndef M_PI
#define M_PI 3.141592653
#endif
    _ie = logf( (float)FFT_SIZE ) / log(2.0) + 0.5;
    static constexpr float omega = 2.0f * M_PI / FFT_SIZE;
    static constexpr int s4 = FFT_SIZE / 4;
    static constexpr int s2 = FFT_SIZE / 2;
    for ( int i = 1 ; i < s4 ; ++i)
    {
    float f = cosf(omega * i);
      _wi[s4 + i] = f;
      _wi[s4 - i] = f;
      _wr[     i] = f;
      _wr[s2 - i] = -f;
    }
    _wi[s4] = _wr[0] = 1;

    size_t je = 1;
    _br[0] = 0;
    _br[1] = FFT_SIZE / 2;
    for ( size_t i = 0 ; i < _ie - 1 ; ++i )
    {
      _br[ je << 1 ] = _br[ je ] >> 1;
      je = je << 1;
      for ( size_t j = 1 ; j < je ; ++j )
      {
        _br[je + j] = _br[je] + _br[j];
      }
    }
  }

  void exec(const int16_t* in)
  {
    memset(_fi, 0, sizeof(_fi));
    for ( size_t j = 0 ; j < FFT_SIZE / 2 ; ++j )
    {
      float basej = 0.25 * (1.0-_wr[j]);
      size_t r = FFT_SIZE - j - 1;

      /// perform han window and stereo to mono convert.
      _fr[_br[j]] = basej * (in[j * 2] + in[j * 2 + 1]);
      _fr[_br[r]] = basej * (in[r * 2] + in[r * 2 + 1]);
    }

    size_t s = 1;
    size_t i = 0;
    do
    {
      size_t ke = s;
      s <<= 1;
      size_t je = FFT_SIZE / s;
      size_t j = 0;
      do
      {
        size_t k = 0;
        do
        {
          size_t l = s * j + k;
          size_t m = ke * (2 * j + 1) + k;
          size_t p = je * k;
          float Wxmr = _fr[m] * _wr[p] + _fi[m] * _wi[p];
          float Wxmi = _fi[m] * _wr[p] - _fr[m] * _wi[p];
          _fr[m] = _fr[l] - Wxmr;
          _fi[m] = _fi[l] - Wxmi;
          _fr[l] += Wxmr;
          _fi[l] += Wxmi;
        } while ( ++k < ke) ;
      } while ( ++j < je );
    } while ( ++i < _ie );
  }

  uint32_t get(size_t index)
  {
    return (index < FFT_SIZE / 2) ? (uint32_t)sqrtf(_fr[ index ] * _fr[ index ] + _fi[ index ] * _fi[ index ]) : 0u;
  }
};

static constexpr size_t WAVE_SIZE = 320;
static AudioFileSourceSD file;
static AudioOutputM5Speaker out(&M5.Speaker, m5spk_virtual_channel);
static AudioGeneratorMP3 mp3;
static AudioFileSourceID3* id3 = nullptr;
static fft_t fft;
static bool fft_enabled = false;
static bool wave_enabled = false;
static uint16_t prev_y[(FFT_SIZE / 2)+1];
static uint16_t peak_y[(FFT_SIZE / 2)+1];
static int16_t wave_y[WAVE_SIZE];
static int16_t wave_h[WAVE_SIZE];
static int16_t raw_data[WAVE_SIZE * 2];
static int header_height = 0;
static size_t fileindex = 0;

void MDCallback(void *cbData, const char *type, bool isUnicode, const char *string)
{
  (void)cbData;
  if (string[0] == 0) { return; }
  if (strcmp(type, "eof") == 0)
  {
    M5.Display.display();
    return;
  }
  int y = M5.Display.getCursorY();
  if (y+1 >= header_height) { return; }
  M5.Display.fillRect(0, y, M5.Display.width(), 12, M5.Display.getBaseColor());
  M5.Display.printf("%s: %s", type, string);
  M5.Display.setCursor(0, y+12);
}

void stop(void)
{
  if (id3 == nullptr) return;
  out.stop();
  mp3.stop();
  id3->RegisterMetadataCB(nullptr, nullptr);
  id3->close();
  file.close();
  delete id3;
  id3 = nullptr;
}

void play(const char* fname)
{
  if (id3 != nullptr) { stop(); }
  M5.Display.setCursor(0, 8);
  file.open(fname);
  id3 = new AudioFileSourceID3(&file);
  id3->RegisterMetadataCB(MDCallback, (void*)"ID3TAG");
  id3->open(fname);
  mp3.begin(id3, &out);
}

uint32_t bgcolor(LGFX_Device* gfx, int y)
{
  auto h = gfx->height();
  auto dh = h - header_height;
  int v = ((h - y)<<5) / dh;
  if (dh > 44)
  {
    int v2 = ((h - y - 1)<<5) / dh;
    if ((v >> 2) != (v2 >> 2))
    {
      return 0x666666u;
    }
  }
  return gfx->color888(v + 2, v, v + 6);
}

void gfxSetup(LGFX_Device* gfx)
{
  if (gfx == nullptr) { return; }
  if (gfx->width() < gfx->height())
  {
    gfx->setRotation(gfx->getRotation()^1);
  }


  {
    auto cfg = LGFXMeter::config( VUMeter );

    // attach the gauge to TFT
    cfg.display = gfx;

    // set gauge geometry
    cfg.clipRect  = {
      .x         = GaugePosX,
      .y         = GaugePosY,
      .w         = GaugeWidth,
      .h         = GaugeHeight
    };
    cfg.needle.img = &vuMeterArrow;
    cfg.needle.axis = { GaugeWidth/2, GaugePosY+GaugeHeight };
    cfg.bgImage    = &bgImg;
    VUMeterGauge = new Gauge_Class( cfg );
  }

  gfx->setFont(&fonts::lgfxJapanGothic_12);
  gfx->setEpdMode(epd_mode_t::epd_fastest);
  gfx->setCursor(0, 8);
  gfx->println("MP3 player");
  gfx->setTextWrap(false);
  gfx->fillRect(0, 6, gfx->width(), 2, TFT_BLACK);

  header_height = 45;
  fft_enabled = !gfx->isEPD();
  if (fft_enabled)
  {
    wave_enabled = (gfx->getBoard() != m5gfx::board_M5UnitLCD);
    for (int y = header_height; y < gfx->height(); ++y)
    {
      gfx->drawFastHLine(0, y, gfx->width(), bgcolor(gfx, y));
    }
  }

  for (int x = 0; x < (FFT_SIZE/2)+1; ++x)
  {
    prev_y[x] = INT16_MAX;
    peak_y[x] = INT16_MAX;
  }
  for (int x = 0; x < WAVE_SIZE; ++x)
  {
    wave_y[x] = gfx->height();
    wave_h[x] = 0;
  }

  VUMeterGauge->pushGauge(); // render empty gauge (no needle yet)
}

void gfxLoop(LGFX_Device* gfx)
{
  if (gfx == nullptr) { return; }

  if (!gfx->displayBusy())
  { // draw volume bar
    static int px;
    uint8_t v = M5.Speaker.getChannelVolume(m5spk_virtual_channel);
    int x = v * (gfx->width()) >> 8;
    if (px != x)
    {
      gfx->fillRect(x, 6, px - x, 2, px < x ? 0xAAFFAAu : 0u);
      gfx->display();
      px = x;
    }
  }

  if (fft_enabled && !gfx->displayBusy() && M5.Speaker.isPlaying(m5spk_virtual_channel) > 1)
  {
    static int prev_x[2];
    static int peak_x[2];
    float avglevel=0, lastavglevel=0;

    auto buf = out.getBuffer();
    if (buf)
    {
      memcpy(raw_data, buf, WAVE_SIZE * 2 * sizeof(int16_t)); // stereo data copy
      gfx->startWrite();


      float levels[2];

      // draw stereo level meter
      for (size_t i = 0; i < 2; ++i)
      {
        int32_t level = 0;
        for (size_t j = i; j < 640; j += 32)
        {
          uint32_t lv = abs(raw_data[j]);
          if (level < lv) { level = lv; }
        }

        levels[i] = float(level * 90.0) / INT16_MAX;

        int32_t x = (level * gfx->width()) / INT16_MAX;
        int32_t px = prev_x[i];
        if (px != x)
        {
          gfx->fillRect(x, i * 3, px - x, 2, px < x ? 0xFF9900u : 0x330000u);
          prev_x[i] = x;
        }
        px = peak_x[i];
        if (px > x)
        {
          gfx->writeFastVLine(px, i * 3, 2, TFT_BLACK);
          px--;
        }
        else
        {
          px = x;
        }
        if (peak_x[i] != px)
        {
          peak_x[i] = px;
          gfx->writeFastVLine(px, i * 3, 2, TFT_WHITE);
        }
      }

      avglevel = (levels[0]+levels[1])*0.5f;
      if( avglevel!=lastavglevel ) {
        VUMeterGauge->drawNeedle( avglevel );
        lastavglevel = avglevel;
      }

      gfx->display();
      gfx->endWrite();
    }
  }
}





void setup()
{

  auto cfg = M5.config();

  cfg.external_spk = true;

  M5.begin(cfg);


  { /// custom setting
    auto spk_cfg = M5.Speaker.config();
    /// Increasing the sample_rate will improve the sound quality instead of increasing the CPU load.
    spk_cfg.sample_rate = 96000; // default:64000 (64kHz)  e.g. 48000 , 50000 , 80000 , 96000 , 100000 , 128000 , 144000 , 192000 , 200000
    M5.Speaker.config(spk_cfg);
  }


  M5.Speaker.begin();

  while (false == SD.begin(GPIO_NUM_4, SPI, 25000000))
  {
    delay(500);
  }


  gfxSetup(&M5.Display);

  play(filename[fileindex]);

}



void loop()
{

  gfxLoop(&M5.Display);

  if (mp3.isRunning())
  {
    if (!mp3.loop()) { mp3.stop(); }
  }
  else
  {
    delay(1);
  }

  M5.update();
  if (M5.BtnA.wasClicked())
  {
    M5.Speaker.tone(1000, 100);
    stop();
    if (++fileindex >= filecount) { fileindex = 0; }
    play(filename[fileindex]);
  }
  else
  if (M5.BtnA.isHolding() || M5.BtnB.isPressed() || M5.BtnC.isPressed())
  {
    size_t v = M5.Speaker.getChannelVolume(m5spk_virtual_channel);
    if (M5.BtnB.isPressed()) { --v; } else { ++v; }
    if (v <= 255 || M5.BtnA.isHolding())
    {
      M5.Speaker.setChannelVolume(m5spk_virtual_channel, v);
    }
  }

}



