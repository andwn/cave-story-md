void setRandomSeed(uint16_t seed);
uint16_t random();

// util.s routines
void slz_unpack(const void *in, void *out);
void uftc_unpack(const void *in, void *out, uint16_t start, uint16_t num);

void ssf_reset();
void ssf_setbank(uint16_t bank, uint16_t chunk);
