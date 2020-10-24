To make Comet support on both Vivado HSL and Catapult

1. Replace ac_int<w, false> with HLS_UINT(w)
2. Replace ac_int<w, true> with HLS_INT(w)