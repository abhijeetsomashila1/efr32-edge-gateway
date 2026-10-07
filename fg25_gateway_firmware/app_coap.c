#include <stdio.h>
#include <string.h>

#include "app_coap.h"
#include "sl_wisun_coap.h"
#include "sl_wisun_coap_rhnd.h"

#define EV_RESOURCE_URI "/ev"

static char ev_response[] =
    "{\"v\":229.2,\"i\":0.44,\"p\":100.8,\"e\":0.125,\"s\":1}";

/*
 * CoAP callback for GET /ev
 *
 * Silicon Labs callback type:
 *
 * sl_wisun_coap_packet_t *
 * (*)(const sl_wisun_coap_packet_t * const req_packet)
 */
static sl_wisun_coap_packet_t *
coap_callback_ev(
    const sl_wisun_coap_packet_t * const req_packet)
{
  sl_wisun_coap_packet_t *resp_packet;

  /*
   * Build a standard CoAP Content response.
   */
  resp_packet = sl_wisun_coap_build_response(
      req_packet,
      COAP_MSG_CODE_RESPONSE_CONTENT);

  if (resp_packet == NULL) {
    return NULL;
  }

  /*
   * Tell the client that the payload is JSON.
   */
  resp_packet->content_format = COAP_CT_TEXT_PLAIN;

  /*
   * JSON payload.
   */
  resp_packet->payload_ptr =
      (uint8_t *)ev_response;

  resp_packet->payload_len =
      (uint16_t)strlen(ev_response);

  return resp_packet;
}


/*
 * Register the /ev resource.
 */
void app_coap_init(void)
{
  sl_wisun_coap_rhnd_resource_t coap_resource = { 0 };

  /*
   * Initialize empty resource descriptor.
   */
  sl_wisun_coap_rhnd_resource_init(
      &coap_resource);

  /*
   * URI:
   *
   * coap://[FG25-IP]:5683/ev
   */
  coap_resource.data.uri_path =
      EV_RESOURCE_URI;

  /*
   * Resource type.
   */
  coap_resource.data.resource_type =
      "json";

  /*
   * Interface description.
   */
  coap_resource.data.interface =
      "ev";

  /*
   * Callback executed when /ev is requested.
   */
  coap_resource.auto_response =
      coap_callback_ev;

  /*
   * Make /ev visible through:
   *
   * /.well-known/core
   */
  coap_resource.discoverable = true;

  /*
   * Register the resource.
   */
  if (sl_wisun_coap_rhnd_resource_add(
          &coap_resource) == SL_STATUS_OK) {

    printf("CoAP resource registered: /ev\r\n");

  } else {

    printf("ERROR: Failed to register /ev\r\n");
  }
}