#ifndef SD_READER_CONFIG_H
#define SD_READER_CONFIG_H

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * \addtogroup config Sd-reader configuration
 *
 * @{
 */

/**
 * \file
 * Common sd-reader configuration used by all modules (license: GPLv2 or LGPLv2.1)
 *
 * \note This file contains only configuration items relevant to
 * all sd-reader implementation files. For module specific configuration
 * options, please see the files fat_config.h, partition_config.h
 * and sd_raw_config.h.
 */

/**
 * Controls allocation of memory.
 *
 * Set to 1 to use malloc()/free() for allocation of structures
 * like file and directory handles, set to 0 to use pre-allocated
 * fixed-size handle arrays.
 */
#define USE_DYNAMIC_MEMORY 0

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif

