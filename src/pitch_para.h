
#ifndef __PITCH_PARA_H
#define __PITCH_PARA_H
#include "machine_type.h"
/** parameter set for one single patch */
typedef struct {
  INT    sourceStartBand;         /*!< first band in lowbands where to take the samples from */
  INT    sourceStopBand;          /*!< first band in lowbands which is not included in the patch anymore */
  INT    guardStartBand;          /*!< first band in highbands to be filled with zeros in order to
                                         reduce interferences between patches */
  INT    targetStartBand;         /*!< first band in highbands to be filled with whitened lowband signal */
  INT    targetBandOffs;          /*!< difference between 'startTargetBand' and 'startSourceBand' */
  INT    numBandsInPatch;         /*!< number of consecutive bands in this one patch */
} PATCH_PARAM;
#endif