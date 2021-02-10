#ifndef patterns_h
#define patterns_h

#include "genome_mapper.h"

class Pattern
{
  public:
    virtual void operator()(GenomeMapper const& gm, unsigned start, unsigned end) { }

  protected:
    virtual CRGB color_for(char base) const
    {
      switch (base)
      {
        case 'a': return CRGB::Red;
        case 'c': return CRGB::Yellow;
        case 'g': return CRGB::Green;
        case 't': return CRGB::Blue;
        default : return CRGB::Black;
      }
    }

    char complementary_base(char base)
    {
      switch (base)
      {
        case 'a': return 't';
        case 't': return 'a';
        case 'c': return 'g';
        case 'g': return 'c';
        default: return 0;
      }
    }
};

class PalettePattern : public Pattern
{
    CRGBPalette16 _palette = RainbowColors_p;

    void operator()(GenomeMapper const& gm, unsigned start, unsigned end) override
    {
      float offset = gm.offset();
      for (unsigned i = start; i < end; ++i)
      {
        leds[i] = ColorFromPalette(_palette, color_index_for(gm.base_at(i), gm.base_at(i + 1), offset), 255, LINEARBLEND);
        leds[i + LEDS_PER_PIN] = ColorFromPalette(_palette,
                                 color_index_for(complementary_base(gm.base_at(i)), complementary_base(gm.base_at(i + 1)), offset), 255, LINEARBLEND);
      }
    }

  private:
    unsigned color_index_for(char base, char next, float offset)
    {
      float integer;
      return offset_for_base(base) + 64 * modf(offset, &integer);
    }

    unsigned offset_for_base(char base)
    {      
      switch (base)
      {
        case 'a': return 0;
        case 'c': return 64;
        case 'g': return 192;
        case 't': return 128;
        default: return 0;
      }
    }
} palette_pattern;

class BasicDiscretePairs : public Pattern
{
    void operator()(GenomeMapper const& gm, unsigned start, unsigned end) override
    {
      for (unsigned i = start; i < end; ++i)
      {
        leds[i] = color_for(gm.base_at(i));
        leds[i + LEDS_PER_PIN] = color_for(complementary_base(gm.base_at(i)));
      }
    }
} basic_discrete_pairs;

class SlowRainbowPairs : public Pattern
{
    /* Working state. */
    byte _hue_base = 0;
    bool _increasing = true;

  protected:
    void operator()(GenomeMapper const& gm, unsigned start, unsigned end) override
    {
      EVERY_N_MILLISECONDS(250)
      {
        if (_increasing)
        {
          if (_hue_base == 63)
          {
            _increasing = false;
            _hue_base = 62;
          }
          else
            ++_hue_base;
        }
        else
        {
          if (_hue_base == 0)
          {
            _increasing = true;
            _hue_base = 1;
          }
          else
            --_hue_base;
        }
      }

      for (unsigned i = start; i < end; ++i)
      {
        leds[i] = color_for(gm.base_at(i));
        leds[i + LEDS_PER_PIN] = color_for(complementary_base(gm.base_at(i)));
      }
    }

    CRGB color_for(char base) const override
    {
      switch (base)
      {
        case 'g': return CHSV(_hue_base, 255, 255);
        case 't': return CHSV(_hue_base + 64, 255, 255);
        case 'c': return CHSV(_hue_base + 128, 255, 255);
        case 'a': return CHSV(_hue_base + 192, 255, 255);
      }
    }
} slow_rainbow_pairs;

class SlowRainbowWithSparkles : public SlowRainbowPairs
{
    void operator()(GenomeMapper const& gm, unsigned start, unsigned end) override
    {
      EVERY_N_MILLISECONDS(15)
      {
        SlowRainbowPairs::operator()(gm, start, end);

        for (unsigned i = 0; i < 3; ++i)
        {
          unsigned target = random(LEDS_COUNT);
          if (target < start || target >= end)
            continue;

          if (random(2))
            continue;

          target += random(2) ? LEDS_PER_PIN : 0;
          leds[target] = CRGB::White;
        }
      }
    }
} slow_rainbow_with_sparkles;

class SlowRainbowWithFastMovingDropout : public SlowRainbowPairs
{
    /* Working state. */
    unsigned _dropout = 0;

protected:
    void operator()(GenomeMapper const& gm, unsigned start, unsigned end) override
    {
      SlowRainbowPairs::operator()(gm, start, end);

      EVERY_N_MILLISECONDS(50)
      {
        if (_dropout <= 0)
          _dropout = LEDS_PER_PIN - 1;
        else
          --_dropout;
      }

      if (_dropout < start || _dropout > end - 1)
        return;

      leds[_dropout] = CRGB::Black;
      leds[_dropout + LEDS_PER_PIN] = CRGB::Black;
    }
} slow_rainbow_with_fast_moving_dropout;

class SlowRainbowWithEvenFasterMovingDropout : public SlowRainbowWithFastMovingDropout
{
    /* Working state. */
    unsigned _faster_dropout = 0;

    void operator()(GenomeMapper const& gm, unsigned start, unsigned end) override
    {
      SlowRainbowWithFastMovingDropout::operator()(gm, start, end);

      EVERY_N_MILLISECONDS(5)
      {
        if (_faster_dropout <= 0)
          _faster_dropout = LEDS_PER_PIN - 1;
        else
          --_faster_dropout;
      }

      if (_faster_dropout < start || _faster_dropout > end - 1)
        return;

      leds[_faster_dropout] = CRGB::White;
      leds[_faster_dropout + LEDS_PER_PIN] = CRGB::White;
    }
} slow_rainbow_with_even_faster_moving_dropout;

class SlowRainbowSingleSided : public SlowRainbowPairs
{
    /* Working state. */
    bool _side;

    void operator()(GenomeMapper const& gm, unsigned start, unsigned end) override
    {
      SlowRainbowPairs::operator()(gm, start, end);
      for (unsigned i = start; i < end; ++i)
        leds[i + LEDS_PER_PIN] = CRGB::Black;
    }
}
slow_rainbow_single_sided;

class SingleBasesLit : public SlowRainbowPairs
{
    /* Working state. */
    char _base_a = 'a';
    char _base_b = 'c';

    void operator()(GenomeMapper const& gm, unsigned start, unsigned end) override
    {
      SlowRainbowPairs::operator()(gm, start, end);
      for (unsigned i = start; i < end; ++i)
      {
        if (gm.base_at(i) != _base_a && gm.base_at(i) != _base_b)
          leds[i] = CRGB::Black;
        if (complementary_base(gm.base_at(i)) != _base_a && complementary_base(gm.base_at(i)) != _base_b)
          leds[i + LEDS_PER_PIN] = CRGB::Black;
      }
      EVERY_N_SECONDS(5)
      {
        _base_a = next_base();
        _base_b = alternate(_base_a);
      }
    }

  private:
    char next_base()
    {
      const char bases[] = { 'a', 'c', 'g', 't' };
      return bases[random(4)];
    }

    char alternate(char base)
    {
      if (base == 'a' || base == 't')
        return random(2) ? 'c' : 'g';
      return random(2) ? 'a' : 't';
    }
} single_base_lit;

Pattern* patterns[] = {
  &slow_rainbow_with_fast_moving_dropout,
//  &slow_rainbow_with_even_faster_moving_dropout,
  &palette_pattern,
  &single_base_lit,
  &slow_rainbow_with_sparkles,
  &basic_discrete_pairs,
  &slow_rainbow_single_sided,
  &slow_rainbow_pairs,
};
unsigned num_patterns = sizeof(patterns) / sizeof(patterns[0]);

#endif
