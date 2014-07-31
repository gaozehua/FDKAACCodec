
#ifndef __INVF_DEF_H
#define __INVF_DEF_H
typedef enum
{
  INVF_OFF = 0,
  INVF_LOW_LEVEL,
  INVF_MID_LEVEL,
  INVF_HIGH_LEVEL,
  INVF_SWITCHED /* not a real choice but used here to control behaviour */
}
INVF_MODE;
#endif