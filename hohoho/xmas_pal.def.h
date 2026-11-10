enum EPalette : unsigned char {
  Pal_Red=0,
  Pal_Green=1,
  Pal_Blue=2,
  Pal_Amber=3,
  Pal_COUNT=4,
};

static constexpr EPalette kXmasPal[] = {
  Pal_Blue, Pal_Amber, Pal_Green, Pal_Amber, Pal_Blue, Pal_Amber, Pal_Green, Pal_Red, Pal_Amber, Pal_Blue, Pal_Red, Pal_Blue,
  Pal_Green, Pal_Red, Pal_Blue, Pal_Green, Pal_Red, Pal_Amber, Pal_Red, Pal_Amber, Pal_Blue, Pal_Amber, Pal_Green, Pal_Blue,
  Pal_Amber, Pal_Red, Pal_Amber, Pal_Red, Pal_Green, Pal_Blue, Pal_Red, Pal_Green, Pal_Amber, Pal_Green, Pal_Blue, Pal_Green,
  Pal_Red, Pal_Blue, Pal_Amber, Pal_Blue, Pal_Green, Pal_Red, Pal_Blue, Pal_Red, Pal_Blue, Pal_Green, Pal_Blue, Pal_Amber,
  Pal_Green, Pal_Red, Pal_Blue, Pal_Amber, Pal_Red, Pal_Green, Pal_Blue, Pal_Red, Pal_Blue, Pal_Green, Pal_Blue, Pal_Red,
  Pal_Blue, Pal_Red, Pal_Green, Pal_Red, Pal_Blue, Pal_Amber, Pal_Green, Pal_Amber, Pal_Green, Pal_Blue, Pal_Green, Pal_Amber,
  Pal_Blue, Pal_Green, Pal_Amber, Pal_Green, Pal_Blue, Pal_Red, Pal_Amber, Pal_Green, Pal_Red, Pal_Amber, Pal_Blue, Pal_Red,
  Pal_Amber, Pal_Blue, Pal_Green, Pal_Amber, Pal_Red, Pal_Green, Pal_Amber, Pal_Green, Pal_Red, Pal_Amber, Pal_Blue, Pal_Red,
  Pal_Green, Pal_Amber, Pal_Blue, Pal_Red, Pal_Green, Pal_Blue, Pal_Red, Pal_Blue, Pal_Red, Pal_Amber, Pal_Red, Pal_Blue,
  Pal_Red, Pal_Amber, Pal_Red, Pal_Amber, Pal_Red, Pal_Amber, Pal_Blue, Pal_Red, Pal_Amber, Pal_Red, Pal_Amber, Pal_Blue,
  Pal_Green, Pal_Red, Pal_Amber, Pal_Blue, Pal_Red, Pal_Amber, Pal_Blue, Pal_Amber, Pal_Green, Pal_Blue, Pal_Amber, Pal_Green,
  Pal_Blue, Pal_Green, Pal_Blue, Pal_Red, Pal_Amber, Pal_Green, Pal_Blue, Pal_Amber, Pal_Green, Pal_Blue, Pal_Amber, Pal_Red,
  Pal_Green, Pal_Amber, Pal_Green, Pal_Red, Pal_Green, Pal_Blue, Pal_Red, Pal_Blue, Pal_Red, Pal_Green, Pal_Amber, Pal_Red,
  Pal_Green, Pal_Amber, Pal_Blue, Pal_Green, Pal_Amber, Pal_Green, Pal_Red, Pal_Green, Pal_Red, Pal_Green, Pal_Amber, Pal_Green,
  Pal_Red, Pal_Green, Pal_Amber, Pal_Blue, Pal_Red, Pal_Green, Pal_Amber, Pal_Red, Pal_Blue, Pal_Green, Pal_Blue, Pal_Green,
  Pal_Amber, Pal_Green, Pal_Red, Pal_Green, Pal_Amber, Pal_Blue, Pal_Amber, Pal_Blue, Pal_Amber, Pal_Blue, Pal_Amber, Pal_Blue,
  Pal_Amber, Pal_Blue, Pal_Red, Pal_Green, Pal_Red, Pal_Blue, Pal_Green, Pal_Red, Pal_Blue, Pal_Green, Pal_Amber, Pal_Green,
  Pal_Red, Pal_Blue, Pal_Red, Pal_Blue, Pal_Amber, Pal_Blue, Pal_Amber, Pal_Red, Pal_Amber, Pal_Red, Pal_Blue, Pal_Red,
  Pal_Green, Pal_Red, Pal_Green, Pal_Amber, Pal_Blue, Pal_Red, Pal_Blue, Pal_Amber, Pal_Blue, Pal_Green, Pal_Amber, Pal_Blue,
  Pal_Red, Pal_Blue, Pal_Red, Pal_Blue, Pal_Amber, Pal_Green, Pal_Amber, Pal_Green, Pal_Red, Pal_Blue, Pal_Red, Pal_Amber,
  Pal_Green, Pal_Red, Pal_Blue, Pal_Green, Pal_Red, Pal_Green, Pal_Blue, Pal_Amber, Pal_Red, Pal_Blue, Pal_Amber, Pal_Blue,
  Pal_Red, Pal_Amber, Pal_Green, Pal_Blue, Pal_Amber, Pal_Green, Pal_Blue, Pal_Red, Pal_Green, Pal_Blue, Pal_Green,
};
static constexpr int kXmasPalLen = sizeof(kXmasPal);
static_assert(kXmasPalLen == 263);
