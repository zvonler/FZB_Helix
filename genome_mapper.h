#ifndef genome_mapper_h
#define genome_mapper_h

#include <FastLED.h>

class GenomeMapper
{
    /* Constant state. */
    const char* _genome;
    unsigned _len;
    /* Working state. */
    float _offset = 0;

  public:
    GenomeMapper(const char* genome, unsigned len)
      : _genome(genome)
      , _len(len)
    { }

    float offset() const
    {
      return _offset;
    }
    
    void update()
    {
      const unsigned base_period = 500; // milliseconds
      const unsigned steps = 20;

      EVERY_N_MILLISECONDS(base_period / steps)
      {
        _offset += 1.0 / steps;

        if (_offset >= _len)
          _offset = 0;
      }
    }

    char base_at(unsigned pos) const
    {
      unsigned idx = (pos + unsigned(_offset)) % _len;
      return _genome[idx];
    }
};

#endif
