/*
 * Copyright (c) 2015-2017, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== uartecho.c ========
 */
#include <stdint.h>
#include <stddef.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/UART.h>
#include <ti/drivers/I2C.h>
#include <ti/drivers/SPI.h>
#include <ti/drivers/Watchdog.h>
#include <ti/drivers/Timer.h>
#include <ti/drivers/ADC.h>
//#include <ti/devices/msp432p4xx/driverlib/cs.h>

/* Example/Board Header files */
#include "OBC_Board.h"

#include "satellite.h"
#include "devices.h"
#include "queue.h"

#include "INA226.h"
#include "TMP100.h"

#include "parameters.h"

#include "osal.h"

extern UART_Handle uart_dbg_bus;
extern UART_Handle uart_pq9_bus;

bool start_flag = false;

/*
 *  ======== mainThread ========
 */
void *mainThread(void *arg0)
{

    /* Call driver init functions */
    GPIO_init();
    UART_init();
    I2C_init();
    SPI_init();
    Timer_init();
    ADC_init();
    Watchdog_init();

    uint32_t wdg_time = OSAL_sys_GetTick();
    uint32_t now_time;

    GPIO_write(EXT_WDG, 1);
    usleep(19600);
    GPIO_write(EXT_WDG, 0);

    //in Hz
    uint32_t freq = CS_getSMCLK();

    /* Turn on user LED */
    GPIO_write(PQ9_EN, 0);


    /*ECSS services start*/
    pkt_pool_INIT();
    device_init();
    init_parameters();
    OSAL_init();

    uint16_t boot_counter=0, size;
    uint8_t buf[4];

   // get_parameter(OBC_boot_counter_param_id, &boot_counter, buf, &size);
   // boot_counter=0;
   // set_parameter(OBC_boot_counter_param_id, boot_counter);

    get_parameter(OBC_boot_counter_param_id, &boot_counter, buf, &size);
    boot_counter++;
    set_parameter(OBC_boot_counter_param_id, boot_counter);


    start_flag = true;

    uint32_t sen_loop = 100000;

    /* Loop forever echoing */
    while (1) {

        now_time = OSAL_sys_GetTick();
        if(now_time - wdg_time > 2100) {
          GPIO_write(EXT_WDG, 1);
          usleep(19600);
          GPIO_write(EXT_WDG, 0);
          wdg_time = now_time;
        }

        set_parameter(SBSYS_reset_clr_int_wdg_param_id, NULL);

        update_device(OBC_MON_DEV_ID);
        usleep(1);

        update_device(OBC_TEMP_DEV_ID);
        usleep(1);

        get_parameter(SBSYS_sensor_loop_param_id, &sen_loop, buf, &size);
        usleep(sen_loop);

    }
}


/*  ======== ecssThread ========
 *  This thread runs on a higher priority, since wdg pin
 *  has to be ready for master.
 */
void *pqReceiveThread(void *arg0)
{

    while(!start_flag) {
        usleep(1000);
    }

    /* Loop forever */
    while (1) {
         //import_pkt();
         sleep(100);
    }

    return (NULL);
}

void *pqTransmitThread(void *arg0)
{

    while(!start_flag) {
        usleep(1000);
    }

    /* Loop forever */
    while (1) {
         PQ9_primary();
         usleep(1);
    }

    return (NULL);
}


char msg[100];

/*  ======== senThread ========
 *  This a dbg thread for outputing sensor readings
 */
void *senThread(void *arg0)
{

    struct ina_device ina_dev;
    struct tmp_device tmp_dev;

    sprintf(msg, "Reset\n");
    UART_write(uart_dbg_bus, msg, strlen(msg));

    sleep(1);

    /* Loop forever */
    while (1) {

        //read_device_parameters(OBC_MON_DEV_ID, &ina_dev);
        //read_device_parameters(OBC_TEMP_DEV_ID, &tmp_dev);

       // sprintf(msg, "INA: C %d, V %d, W %d, Temp: %d\n", (int)(ina_dev.current*1000), (int)ina_dev.voltage, (int)ina_dev.power, (int)tmp_dev.temp);
       // UART_write(uart_dbg_bus, msg, strlen(msg));

        sleep(1);

        //master_();
    }

    return (NULL);
}
/*  ======== pqmasterThread ========
 *  This thread generates the for outputing sensor readings
 */
void *pqMasterThread(void *arg0)
{

    while(!start_flag) {
        usleep(1000);
    }

    //PQ9_master_init();

    sleep(1);

    /* Loop forever */
    while (1) {
      PQ9_master();
      sleep(1);
    }

    return (NULL);
}
