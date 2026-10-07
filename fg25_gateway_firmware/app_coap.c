#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app_coap.h"
#include "sl_cli.h"
#include "sl_wisun_coap.h"
#include "sl_wisun_coap_rhnd.h"

#define EV_RESOURCE_URI "/ev"
#define EV_RESPONSE_SIZE 128

/*
 * Latest EV charger data.
 *
 * This is what the CoAP /ev resource returns.
 */
static char ev_response[EV_RESPONSE_SIZE] =
    "{\"v\":0.0,\"i\":0.0,\"p\":0.0,\"e\":0.0,\"s\":0}";

/* ------------------------------------------------------------------------- */
/* Forward declarations                                                      */
/* ------------------------------------------------------------------------- */

static void ev_update_command(sl_cli_command_arg_t *arguments);

static sl_wisun_coap_packet_t *
coap_callback_ev(
    const sl_wisun_coap_packet_t * const req_packet);

/* ------------------------------------------------------------------------- */
/* Custom CLI command                                                        */
/* ------------------------------------------------------------------------- */

/*
 * Command:
 *
 * ev_update <voltage> <current> <power> <energy_kwh> <status>
 *
 * Example:
 *
 * ev_update 229.2 0.44 100.8 0.125 1
 *
 * status:
 *   0 = IDLE
 *   1 = CHARGING
 *   2 = FAULT
 *   3 = DONE
 */

static const sl_cli_command_info_t cmd_ev_update =
    SL_CLI_COMMAND(
        ev_update_command,
        "Update EV charger readings",
        "voltage current power energy_kWh status",
        {
            SL_CLI_ARG_STRING,
            SL_CLI_ARG_STRING,
            SL_CLI_ARG_STRING,
            SL_CLI_ARG_STRING,
            SL_CLI_ARG_STRING,
            SL_CLI_ARG_END,
        });

static const sl_cli_command_entry_t ev_cli_table[] = {
    { "ev_update", &cmd_ev_update, false },
    { NULL, NULL, false },
};

static sl_cli_command_group_t ev_cli_group = {
    { NULL },
    false,
    ev_cli_table
};

/* ------------------------------------------------------------------------- */
/* EV update command handler                                                 */
/* ------------------------------------------------------------------------- */

static void ev_update_command(sl_cli_command_arg_t *arguments)
{
    char *voltage_str;
    char *current_str;
    char *power_str;
    char *energy_str;
    char *status_str;

    char *endptr;

    float voltage;
    float current;
    float power;
    float energy_kwh;

    unsigned long status;

    /*
     * We require exactly 5 arguments.
     */
    if (sl_cli_get_argument_count(arguments) != 5) {
        printf(
            "Usage: ev_update <voltage> <current> "
            "<power> <energy_kwh> <status>\r\n");
        return;
    }

    voltage_str = sl_cli_get_argument_string(arguments, 0);
    current_str = sl_cli_get_argument_string(arguments, 1);
    power_str   = sl_cli_get_argument_string(arguments, 2);
    energy_str  = sl_cli_get_argument_string(arguments, 3);
    status_str  = sl_cli_get_argument_string(arguments, 4);

    /*
     * Convert the numeric strings.
     */
    voltage = strtof(voltage_str, &endptr);
    if (*endptr != '\0') {
        printf("ERROR: Invalid voltage\r\n");
        return;
    }

    current = strtof(current_str, &endptr);
    if (*endptr != '\0') {
        printf("ERROR: Invalid current\r\n");
        return;
    }

    power = strtof(power_str, &endptr);
    if (*endptr != '\0') {
        printf("ERROR: Invalid power\r\n");
        return;
    }

    energy_kwh = strtof(energy_str, &endptr);
    if (*endptr != '\0') {
        printf("ERROR: Invalid energy\r\n");
        return;
    }

    status = strtoul(status_str, &endptr, 10);
    if (*endptr != '\0' || status > 3) {
        printf("ERROR: Invalid status\r\n");
        return;
    }

    /*
     * Build the JSON returned by CoAP /ev.
     */
    snprintf(
        ev_response,
        sizeof(ev_response),
        "{\"v\":%.1f,\"i\":%.2f,\"p\":%.1f,\"e\":%.3f,\"s\":%lu}",
        voltage,
        current,
        power,
        energy_kwh,
        status);

    printf("EV data updated: %s\r\n", ev_response);
}

/* ------------------------------------------------------------------------- */
/* CoAP callback                                                             */
/* ------------------------------------------------------------------------- */

static sl_wisun_coap_packet_t *
coap_callback_ev(
    const sl_wisun_coap_packet_t * const req_packet)
{
    sl_wisun_coap_packet_t *resp_packet;

    resp_packet = sl_wisun_coap_build_response(
        req_packet,
        COAP_MSG_CODE_RESPONSE_CONTENT);

    if (resp_packet == NULL) {
        return NULL;
    }

    /*
     * Keep the content format compatible with the CoAP API
     * currently used by this project.
     */
    resp_packet->content_format = COAP_CT_TEXT_PLAIN;

    resp_packet->payload_ptr =
        (uint8_t *)ev_response;

    resp_packet->payload_len =
        (uint16_t)strlen(ev_response);

    return resp_packet;
}

/* ------------------------------------------------------------------------- */
/* CoAP initialization                                                       */
/* ------------------------------------------------------------------------- */

void app_coap_init(void)
{
    sl_wisun_coap_rhnd_resource_t coap_resource = { 0 };

    /*
     * Initialize CoAP resource structure.
     */
    sl_wisun_coap_rhnd_resource_init(
        &coap_resource);

    /*
     * /ev
     */
    coap_resource.data.uri_path =
        EV_RESOURCE_URI;

    /*
     * Resource type.
     */
    coap_resource.data.resource_type =
        "json";

    /*
     * Interface.
     */
    coap_resource.data.interface =
        "ev";

    /*
     * GET /ev callback.
     */
    coap_resource.auto_response =
        coap_callback_ev;

    /*
     * Allow discovery through /.well-known/core
     */
    coap_resource.discoverable =
        true;

    /*
     * Register the CoAP resource.
     */
    if (sl_wisun_coap_rhnd_resource_add(
            &coap_resource) == SL_STATUS_OK) {

        printf(
            "CoAP resource registered: /ev\r\n");

    } else {

        printf(
            "ERROR: Failed to register /ev\r\n");
    }

    /*
     * Register our custom CLI command.
     */
    if (sl_cli_command_add_command_group(
            sl_cli_default_handle,
            &ev_cli_group)) {

        printf(
            "EV CLI command registered: ev_update\r\n");

    } else {

        printf(
            "ERROR: Failed to register ev_update command\r\n");
    }
}