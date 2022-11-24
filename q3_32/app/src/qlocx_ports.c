/***************************************************************************
 * qlocx_ports.c 				                                                
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
 * 0.1  220905  DH	Y        	Initial version
 *
 *  
 * Handling I/O expansion and individual sense inputs added to the original code 										
 ***************************************************************************/

#include "app_timer.h"
#include "nrf_delay.h"
#include "nrf_drv_gpiote.h"
#include "nrf_gpio.h"
#include "nrf_log.h"
#include "qlocx_config.h"
#include "qlocx_ports.h"
#include "qlocx_power_meter.h"
#include "qlocx_ioexp.h"
#include <stdint.h>
#include <stdlib.h>


static qlocx_port_t qlocx_ports[] = QLOCX_PORTS_INIT;

/**
 * Clear all ports.
 */
static void qlocx_ports_clear_all_ports()
{
uint8_t i;

    for (i = 0; i < sizeof(qlocx_ports) / sizeof(qlocx_port_t); i++){
        if (qlocx_ports[i].direction == QLOCX_PORT_OUTPUT){
            if(qlocx_ports[i].gpio < 128){
                nrf_gpio_pin_clear(qlocx_ports[i].gpio);
            }
            else{
                qlocx_ioexp_write_output((qlocx_ports[i].gpio - 128), false);
            }
            qlocx_ports[i].status = QLOCX_PORT_STATUS_DISABLED;
        }
    }  
}

/**
 * Handler to run when the timer recieves a timeout.
 */
static void qlocx_ports_timer_handler(void* ptr)
{

qlocx_port_t* context = (qlocx_port_t*) ptr;

    if (context->status == QLOCX_PORT_STATUS_ENABLED) {
        if(context->gpio < 128){
            nrf_gpio_pin_clear(context->gpio);
        }
        else{
            qlocx_ioexp_write_output((context->gpio - 128), false);
        }
        context->status = QLOCX_PORT_STATUS_DISABLED;
        return;
    } else if (context->status == QLOCX_PORT_STATUS_DISABLED) {
        if(context->gpio < 128){
            nrf_gpio_pin_set(context->gpio);
         }else{
            qlocx_ioexp_write_output((context->gpio - 128), true);
        }  
        context->status = QLOCX_PORT_STATUS_ENABLED;
  #ifdef QLOCX_POWER_METER_ENABLED
        /* Clear all ports if current consumption is too high */
        if (qlocx_power_meter_get_current_milliampere() > 2000){
            qlocx_ports_clear_all_ports();
            return;
        }
  #endif
        app_timer_start(context->timer, APP_TIMER_TICKS(context->timeout), context);
   }
}

/**
 * Handler to run when the sense port is triggered.
 */
static void qlocx_ports_on_sense_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
uint8_t sense_config[QLOCX_CONFIG_SENSE_CONFIG_SIZE] = {0};
int i;

  qlocx_config_get_sense_config(sense_config);
  for (i = 0; i < QLOCX_CONFIG_SENSE_CONFIG_SIZE; i += 3) {
      qlocx_ports_enable(sense_config[i], sense_config[i+1] * 100, sense_config[i+2] * 100);
  }

}

/**
 * Initialize the ports module.
 * This function should be called before any of
 * the members of this module is used.
 */
void qlocx_ports_init()
{
size_t i;

    NRF_LOG_INFO("Ports initializing.");
    // Init IO expansion ports
    qlocx_ioexp_init();
    /* Initialize gpiote */
    nrf_drv_gpiote_init();

    /* Initialize the timers */
    for (i = 0; i < sizeof(qlocx_ports) / sizeof(qlocx_port_t); i++) {
        qlocx_ports[i].timer = &qlocx_ports[i].timer_data;
        app_timer_create(&qlocx_ports[i].timer, APP_TIMER_MODE_SINGLE_SHOT, qlocx_ports_timer_handler);
    }

    /* Initialize the ports */
    for (i = 0; i < sizeof(qlocx_ports) / sizeof(qlocx_port_t); i++) {
      /* Initialize outputs */
        if (qlocx_ports[i].direction == QLOCX_PORT_OUTPUT){
            if(qlocx_ports[i].gpio < 128){
                nrf_gpio_cfg_output(qlocx_ports[i].gpio);
                nrf_gpio_pin_clear(qlocx_ports[i].gpio);
            }
            qlocx_ports[i].status = QLOCX_PORT_STATUS_DISABLED;
        }
        /* Initialize inputs */
        else{
            if(qlocx_ports[i].gpio < 128){
                nrf_drv_gpiote_in_config_t config = GPIOTE_CONFIG_IN_SENSE_HITOLO(false);
                config.pull = NRF_GPIO_PIN_NOPULL;
                nrf_drv_gpiote_in_init(qlocx_ports[i].gpio, &config, qlocx_ports_on_sense_handler);
                nrf_drv_gpiote_in_event_enable(qlocx_ports[i].gpio, true);
            }
        }
    }
}

/**
 * Enable a port.
 *
 * @param duration duration in milliseconds for the port to be enabled
 * @param delay delay in milliseconds to wait before enabling the port
 */
void qlocx_ports_enable(uint8_t index, uint16_t duration, uint16_t delay)
{

  /* Assert item has an actual duration */
  if (duration == 0) return;

  /* Assert index is not out of bounds */
  if (index >= (sizeof(qlocx_ports) / sizeof(qlocx_port_t))) return;

  /* Assert port is an output port */
  if (qlocx_ports[index].direction != QLOCX_PORT_OUTPUT) return;

  /* Assert port is currently disabled */
  if (qlocx_ports[index].status != QLOCX_PORT_STATUS_DISABLED) return;

  qlocx_ports[index].timeout = duration;

  if (delay) {
    app_timer_start(qlocx_ports[index].timer, 
        APP_TIMER_TICKS(delay),
        &qlocx_ports[index]);
  } else {
    app_timer_start(qlocx_ports[index].timer,
        QLOCX_PORTS_MINIMUM_TIMER_TICKS,
        &qlocx_ports[index]);
  }

}

/**
 * Get the port status, zero when the port is a closed circuit, one
 * when the port is an open circuit.
 *
 * @param index index of the port according to silkscreen on the board
 */
uint8_t qlocx_ports_get_status(uint8_t index)
{
bool status;  

  nrf_gpio_pin_set(14);
  nrf_delay_ms(20);
  status = qlocx_ioexp_read_input(index);
  nrf_delay_ms(20);
  nrf_gpio_pin_clear(14);
  return !status;  
}

/**
 * Get the port status, zero when the port is a closed circuit, one
 * when the port is an open circuit.
 *
 * @param index index of the port according to silkscreen on the board
 */
uint32_t qlocx_ports_get_all_status(void)
{
uint32_t status;  

  nrf_gpio_pin_set(14);
  nrf_delay_ms(20);
  status = qlocx_ioexp_read_all_inputs();
  nrf_delay_ms(20);
  nrf_gpio_pin_clear(14);
  return (status);  
}