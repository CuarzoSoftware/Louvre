#include <LGammaTable.h>

void Louvre::LGammaTable::fill(Float64 gamma, Float64 brightness,
                               Float64 contrast) noexcept {
  if (size() == 0) return;

  UInt16 *r{red()};
  UInt16 *g{green()};
  UInt16 *b{blue()};

  if (size() == 1) {
    *r = *g = *b = UINT16_MAX / 2;
    return;
  }

  if (gamma <= 0.0) gamma = 0.1;

  Float64 n{(Float64)size() - 1};
  Float64 val;

  for (UInt32 i = 0; i < size(); i++) {
    val = contrast * pow((Float64)i / n, 1.0 / gamma) + (brightness - 1);

    if (val > 1.0)
      val = 1.0;
    else if (val < 0.0)
      val = 0.0;

    r[i] = g[i] = b[i] = (UInt16)(UINT16_MAX * val);
  }
}
