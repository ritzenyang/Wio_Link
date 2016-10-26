#include "grove_generic_pwm_out_gen.h"
#include "rpc_server.h"
#include "rpc_stream.h"

bool __grove_generic_pwm_out_read_pwm(void *class_ptr, char *method_name, void *input_pack)
{
    GenericPWMOut *grove = (GenericPWMOut *)class_ptr;
    uint8_t *arg_ptr = (uint8_t *)input_pack;
    float duty_percent;
    uint32_t freq;


    if(grove->read_pwm(&duty_percent, &freq))
    {
        writer_print(TYPE_STRING, "{");
        writer_print(TYPE_STRING, "\"duty_percent\":");
        writer_print(TYPE_FLOAT, &duty_percent);
        writer_print(TYPE_STRING, ",");
        writer_print(TYPE_STRING, "\"freq\":");
        writer_print(TYPE_UINT32, &freq);
        writer_print(TYPE_STRING, "}");
        return true;
    }else
    {
        writer_print(TYPE_STRING, "null");
        return false;
    }
}

bool __grove_generic_pwm_out_write_pwm(void *class_ptr, char *method_name, void *input_pack)
{
    GenericPWMOut *grove = (GenericPWMOut *)class_ptr;
    uint8_t *arg_ptr = (uint8_t *)input_pack;
    float duty_percent;

    memcpy(&duty_percent, arg_ptr, sizeof(float)); arg_ptr += sizeof(float);

    if(grove->write_pwm(duty_percent))
    {
        writer_print(TYPE_STRING, "\"OK\"");
        return true;
    }
    else
    {
        writer_print(TYPE_STRING, "null");
        return false;
    }
}

bool __grove_generic_pwm_out_write_pwm_with_freq(void *class_ptr, char *method_name, void *input_pack)
{
    GenericPWMOut *grove = (GenericPWMOut *)class_ptr;
    uint8_t *arg_ptr = (uint8_t *)input_pack;
    float duty_percent;
    uint32_t freq;

    memcpy(&duty_percent, arg_ptr, sizeof(float)); arg_ptr += sizeof(float);
    memcpy(&freq, arg_ptr, sizeof(uint32_t)); arg_ptr += sizeof(uint32_t);

    if(grove->write_pwm_with_freq(duty_percent,freq))
    {
        writer_print(TYPE_STRING, "\"OK\"");
        return true;
    }
    else
    {
        writer_print(TYPE_STRING, "null");
        return false;
    }
}

