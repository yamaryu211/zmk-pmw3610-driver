#define DT_DRV_COMPAT zmk_behavior_pmw3610

#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>

#include <zmk/behavior.h>
#include <zmk/hid.h>
#include <zephyr/input/input.h>
#include <zephyr/dt-bindings/input/input-event-codes.h>
#include "../pmw3610.h"

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

struct behavior_cpi_config {
    const struct device *pixart_dev;
    bool increase_cpi;
    bool decrease_cpi;
};





static int behavior_cpi_init(const struct device *dev) { 
    return 0; 
}

static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
                                        
     const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    if (!dev) {
        LOG_ERR("Failed to get device.");
        return -ENODEV;
    }

    const struct behavior_cpi_config *cfg = dev->config;
    const struct device *pixart_dev = cfg->pixart_dev;
    struct pixart_data *data = pixart_dev->data;
    
    if (!data) {
        LOG_ERR("Failed to get pixart_data.");
        return -ENODEV;
    }

    // CPIを増加または減少させる処理
    if (cfg->increase_cpi) {
        // CPIを200増加
        if (increase_cpi(dev) != 0) {
            LOG_ERR("Failed to increase CPI.");
        }
    }

    if (cfg->decrease_cpi) {
        // CPIを200減少
        if (decrease_cpi(dev) != 0) {
            LOG_ERR("Failed to decrease CPI.");
        }
    }

    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_keymap_binding_released(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_cpi_driver_api = {
    .binding_pressed = on_keymap_binding_pressed,
    .binding_released = on_keymap_binding_released,
    .locality = BEHAVIOR_LOCALITY_GLOBAL,
#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
    .get_parameter_metadata = zmk_behavior_get_empty_param_metadata,
#endif
};

#define CPI_INST(n)                                                                                \
    static struct behavior_cpi_config behavior_cpi_config_##n = {                                   \  
        .pixart_dev = DEVICE_DT_GET(DT_INST_PHANDLE(n, pixart_dev)),                                \
        .increase_cpi = DT_INST_PROP(n, increase_cpi),                                              \
        .decrease_cpi = DT_INST_PROP(n, decrease_cpi),                                              \
    };                                                                                             \
    BEHAVIOR_DT_INST_DEFINE(n, behavior_cpi_init, NULL, NULL, &behavior_cpi_config_##n,            \
                            POST_KERNEL, 34,                      \
                            &behavior_cpi_driver_api);

DT_INST_FOREACH_STATUS_OKAY(CPI_INST)
