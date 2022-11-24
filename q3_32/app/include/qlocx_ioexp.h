/***************************************************************************
 * qlocx_ioexp.h
 *
 *
 *
 *
 * Functional level: 1
 *
 * Prepared: Dirk Handzic
 *
 *
 * Rev	Date	Author	Ext int. (Y/N) 	Reason/description
 * 0.1  220901  DH	Y	        Initial version
 *
 *
 *PCA9535 handler
 ***************************************************************************/
#include <stdint.h>
#include <stdbool.h>

#ifndef qlocx_ioexp_def
#define qlocx_ioexp_def


#ifdef __cplusplus
extern "C" {
#endif

void  qlocx_ioexp_init(void);

bool  qlocx_ioexp_read_input(uint8_t index);

void  qlocx_ioexp_write_output(uint8_t index, bool state);

uint32_t  qlocx_ioexp_read_all_inputs(void);

#ifdef __cplusplus
}
#endif

#endif 
