#include "suli2.h"
#include "rpc_server.h"
#include "rpc_stream.h"
#include "Main.h"

GenericPWMOut *GenericPWMOutD0_ins;

void rpc_server_register_resources()
{
    uint8_t arg_types[MAX_INPUT_ARG_LEN];
    EVENT_T *event;
    

    //GenericPWMOutD0
    GenericPWMOutD0_ins = new GenericPWMOut(14);
    memset(arg_types, TYPE_NONE, MAX_INPUT_ARG_LEN);
    rpc_server_register_method("GenericPWMOutD0", "pwm", METHOD_READ, __grove_generic_pwm_out_read_pwm, GenericPWMOutD0_ins, arg_types);

    memset(arg_types, TYPE_NONE, MAX_INPUT_ARG_LEN);
    arg_types[0] = TYPE_FLOAT;
    rpc_server_register_method("GenericPWMOutD0", "pwm", METHOD_WRITE, __grove_generic_pwm_out_write_pwm, GenericPWMOutD0_ins, arg_types);
    memset(arg_types, TYPE_NONE, MAX_INPUT_ARG_LEN);
    arg_types[0] = TYPE_FLOAT;
    arg_types[1] = TYPE_UINT32;
    rpc_server_register_method("GenericPWMOutD0", "pwm_with_freq", METHOD_WRITE, __grove_generic_pwm_out_write_pwm_with_freq, GenericPWMOutD0_ins, arg_types);
}

void print_well_known()
{
    writer_print(TYPE_STRING, "{\"well_known\":");
    writer_print(TYPE_STRING, "[");
    writer_block_print(TYPE_STRING, "\"GET " OTA_SERVER_URL_PREFIX "/node/GenericPWMOutD0/pwm -> float duty_percent, uint32_t freq\",");
    writer_block_print(TYPE_STRING, "\"POST " OTA_SERVER_URL_PREFIX "/node/GenericPWMOutD0/pwm/{float duty_percent}\",");
    writer_block_print(TYPE_STRING, "\"POST " OTA_SERVER_URL_PREFIX "/node/GenericPWMOutD0/pwm_with_freq/{float duty_percent}/{uint32_t freq}\"");
    writer_print(TYPE_STRING, "]}");
}
