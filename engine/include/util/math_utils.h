#pragma once

#define lise_min(x, y) (x < y ? x : y)

#define lise_max(x, y) (x > y ? x : y)

#define lise_clamp(x, lo, hi) lise_min(hi, lise_max(lo, x))
