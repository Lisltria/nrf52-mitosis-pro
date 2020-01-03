/*
Copyright (C) 2018,2019 Jim Jiang <jim@lotlab.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "ble_bas_service.h"
#include <string.h>

#include "ble_bas.h"
#include "ble_config.h"

#include "../config/keyboard_config.h"
#include "adc_convert.h"

struct BatteryInfo battery_info;
BLE_BAS_DEF(m_bas); /**< Structure used to identify the battery service. */

/**@brief Function for performing a battery measurement, and update the Battery Level characteristic in the Battery Service.
 */
static void battery_level_update(uint8_t battery_level)
{
    ret_code_t err_code;

    err_code = ble_bas_battery_level_update(&m_bas, battery_level, BLE_CONN_HANDLE_ALL);
    if ((err_code != NRF_SUCCESS) && (err_code != NRF_ERROR_BUSY) && (err_code != NRF_ERROR_RESOURCES) && (err_code != NRF_ERROR_FORBIDDEN) && (err_code != NRF_ERROR_INVALID_STATE) && (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING)) {
        APP_ERROR_HANDLER(err_code);
    }
}

/**@brief Function for initializing Battery Service.
 */
static void bas_init(void)
{
    ret_code_t err_code;
    ble_bas_init_t bas_init_obj;

    memset(&bas_init_obj, 0, sizeof(bas_init_obj));

    bas_init_obj.evt_handler = NULL;
    bas_init_obj.support_notification = true;
    bas_init_obj.p_report_ref = NULL;
    bas_init_obj.initial_batt_level = 100;

    bas_init_obj.bl_rd_sec = SEC_CURRENT;
    bas_init_obj.bl_cccd_wr_sec = SEC_CURRENT;
    bas_init_obj.bl_report_rd_sec = SEC_CURRENT;

    err_code = ble_bas_init(&m_bas, &bas_init_obj);
    APP_ERROR_CHECK(err_code);
}

static void calculate_battery_persentage(struct BatteryInfo* info)
{
    if (info->voltage >= 4100)
        info->percentage = 100;
    else if (info->voltage >= 3335)
        info->percentage = 15 + (info->voltage - 3335) / 9;
    else if (info->voltage >= 2900)
        info->percentage = (info->voltage - 2900) / 29;
    else
        info->percentage = 0;
}

static void adc_result_handler(nrf_saadc_value_t value)
{
    // RESULT = [V(P) – V(N) ] * GAIN/REFERENCE * 2 ^ (RESOLUTION - m)
    // value  = V_in / 1.2 * 1024
    // V_in   = V_bat * 2.2 / 12.2
    battery_info.voltage = (uint32_t)value * 1200 * 122 / 1024 / 22;
    calculate_battery_persentage(&battery_info);
    battery_level_update(battery_info.percentage);
}

static nrf_saadc_channel_config_t channel_config = {
    .resistor_p = NRF_SAADC_RESISTOR_DISABLED,
    .resistor_n = NRF_SAADC_RESISTOR_DISABLED,
    .gain = NRF_SAADC_GAIN1_2,
    .reference = NRF_SAADC_REFERENCE_INTERNAL,
    .acq_time = NRF_SAADC_ACQTIME_10US,
    .mode = NRF_SAADC_MODE_SINGLE_ENDED,
    .burst = NRF_SAADC_BURST_DISABLED,
    .pin_p = (nrf_saadc_input_t)(BATTERY_ADC_PIN),
    .pin_n = NRF_SAADC_INPUT_DISABLED
};

static struct adc_channel_config batt_channel = {
    .adc_start = 0,
    .adc_finish = &adc_result_handler,
    .period = 2000,
    .config = &channel_config,
};

ADC_CONVERT_CHANNEL(batt_channel);

void battery_service_init(void)
{
    bas_init();
}