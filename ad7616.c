/*
 * @file ad7616.c
 * @brief AD7616 digital-to-analog converter driver implementation
 * @author WildboarG
 * @date 2025-08-05
*/
#include "ad7616.h"
/**
 * @brief set the state of a specific pin
 * @param dev: AD7616 device pointer
 * @param pin_id: pin id
 * @param state: pin state
 */
static void set_pin(ad7616_t *dev, u8 pin_id, ad7616_pin_state_t state) {
    if (dev && dev->hw.write_pin) {
        dev->hw.write_pin(dev->hw.user_ctx, pin_id, state);
    }
}

/**
 * @brief get the state of a specific pin
 * @param dev: AD7616 device pointer
 * @param pin_id: pin id
 * @return pin state
 */
static ad7616_pin_state_t get_pin(ad7616_t *dev, u8 pin_id) {
    if (dev && dev->hw.read_pin) {
        return dev->hw.read_pin(dev->hw.user_ctx, pin_id);
    }
    return AD7616_PIN_RESET;
}

/**
 * @brief delay in microseconds
 * @param dev: AD7616 device pointer
 * @param us: delay time (microseconds)
 */
static void delay_us(ad7616_t *dev, u16 us) {
    if (dev && dev->hw.delay_us) {
        dev->hw.delay_us(dev->hw.user_ctx, us);
    }
}

/**
 * @brief delay in milliseconds
 * @param dev: AD7616 device pointer
 * @param ms: delay time (milliseconds)
 */
static void delay_ms(ad7616_t *dev, u16 ms) {
    if (dev && dev->hw.delay_ms) {
        dev->hw.delay_ms(dev->hw.user_ctx, ms);
    }
}

/**
 * @brief configure pins based on the current configuration
 * @param dev: AD7616 device pointer
 */
static void configure_pins_by_config(ad7616_t *dev) {
    if (!dev) return;
    
    const ad7616_config_t *config = &dev->config;
    
    // set enable sequence pin
    if (config->enable_sequence) {
        set_pin(dev, AD7616_PIN_SEQEN, AD7616_PIN_SET);
    } else {
        set_pin(dev, AD7616_PIN_SEQEN, AD7616_PIN_RESET);
    }
    
    // set range pins
    switch (config->range) {
        case AD7616_RANGE_10V:
            set_pin(dev, AD7616_PIN_RNGSEL0, AD7616_PIN_SET);
            set_pin(dev, AD7616_PIN_RNGSEL1, AD7616_PIN_SET);
            break;
        case AD7616_RANGE_5V:
            set_pin(dev, AD7616_PIN_RNGSEL0, AD7616_PIN_RESET);
            set_pin(dev, AD7616_PIN_RNGSEL1, AD7616_PIN_SET);
            break;
        case AD7616_RANGE_2V5:
            set_pin(dev, AD7616_PIN_RNGSEL0, AD7616_PIN_SET);
            set_pin(dev, AD7616_PIN_RNGSEL1, AD7616_PIN_RESET);
            break;
    }
    
    // set interface
    if (config->interface == AD7616_INTERFACE_SERIAL) {
        set_pin(dev, AD7616_PIN_SER, AD7616_PIN_SET);
    } else {
        set_pin(dev, AD7616_PIN_SER, AD7616_PIN_RESET);
    }
    
    // set mode
    if (config->mode == AD7616_MODE_SOFTWARE) {
        // software mode
        set_pin(dev, AD7616_PIN_WR, AD7616_PIN_RESET);
    } else {
        set_pin(dev, AD7616_PIN_WR, AD7616_PIN_SET);
    }
    
    // reset RD and CS pins to high
    set_pin(dev, AD7616_PIN_RD, AD7616_PIN_SET);
    set_pin(dev, AD7616_PIN_CS, AD7616_PIN_SET);
    
    // set channel pins
    uint8_t channel_bits = (uint8_t)config->channel;
    set_pin(dev, AD7616_PIN_CHSEL0, (channel_bits & 0x01) ? AD7616_PIN_SET : AD7616_PIN_RESET);
    set_pin(dev, AD7616_PIN_CHSEL1, (channel_bits & 0x02) ? AD7616_PIN_SET : AD7616_PIN_RESET);
    set_pin(dev, AD7616_PIN_CHSEL2, (channel_bits & 0x04) ? AD7616_PIN_SET : AD7616_PIN_RESET);
    
    // reset the CONVST pins to high
    set_pin(dev, AD7616_PIN_CONVST, AD7616_PIN_RESET);
}

/* ==================== Public Functions ==================== */

/**
 * @brief init AD7616 (with default configuration)
 * @param dev: AD7616 device pointer
 */
void ad7616_init(ad7616_t *dev) {
    if (!dev) return; 
    ad7616_config_t default_config = AD7616_DEFAULT_CONFIG;
    ad7616_init_with_config(dev, &default_config);
}

/**
 * @brief init AD7616 with specific configuration
 * @param dev: AD7616 device pointer
 * @param config: configuration
 */
void ad7616_init_with_config(ad7616_t *dev, const ad7616_config_t *config) {
    if (!dev || !config) return;
    
    // copy configuration
    dev->config = *config;
    
    // set configured pins
    configure_pins_by_config(dev);
    
    // make sure AD7616 is reset
    delay_ms(dev, 1);
    
    // reset AD7616
    ad7616_reset(dev);
    
    // wait for AD7616 to initialize
    delay_ms(dev, 10);
    
    // mark as initialized
    dev->is_initialized = 1;
}

/**
 * @brief reset AD7616
 * @param dev: AD7616 device pointer
 */
void ad7616_reset(ad7616_t *dev) {
    if (!dev) return;
    
    set_pin(dev, AD7616_PIN_ENRESET, AD7616_PIN_SET);
    delay_us(dev, 1); // ensure the pin is high for at least 100ns 
    
    set_pin(dev, AD7616_PIN_ENRESET, AD7616_PIN_RESET);
    delay_us(dev, 1);
    
    set_pin(dev, AD7616_PIN_ENRESET, AD7616_PIN_SET);
}

/**
 * @brief start AD7616 conversion
 * @param dev: AD7616 device pointer
 */
void ad7616_start_convst(ad7616_t *dev) {
    if (!dev) return;
    
    set_pin(dev, AD7616_PIN_CONVST, AD7616_PIN_SET);
    delay_us(dev, 1);
    
    set_pin(dev, AD7616_PIN_CONVST, AD7616_PIN_RESET);
    delay_us(dev, 1);
}

/**
 * @brief read data from AD7616
 * @param dev: AD7616 device pointer
 * @param cha_data: channel A data
 * @param chb_data: channel B data
 */
void myad7616_read_data(ad7616_t *dev, int16_t *cha_data, int16_t *chb_data) {
    if (!dev) return;
    uint16_t rxdata_a = 0;
    uint16_t rxdata_b = 0;
    
    set_pin(dev, AD7616_PIN_CS, AD7616_PIN_RESET);    
    set_pin(dev, AD7616_PIN_RD, AD7616_PIN_RESET);    
    
    if (dev->hw.read_data_bus) {
        rxdata_a = dev->hw.read_data_bus(dev->hw.user_ctx);
    }
    
    set_pin(dev, AD7616_PIN_RD, AD7616_PIN_SET);      
    set_pin(dev, AD7616_PIN_CS, AD7616_PIN_SET);      
    
    delay_us(dev, 1); // ensure a small delay between reads
    set_pin(dev, AD7616_PIN_CS, AD7616_PIN_RESET);    
    set_pin(dev, AD7616_PIN_RD, AD7616_PIN_RESET);   
    
    if (dev->hw.read_data_bus) {
        rxdata_b = dev->hw.read_data_bus(dev->hw.user_ctx);
    }
    
    set_pin(dev, AD7616_PIN_RD, AD7616_PIN_SET);     
    set_pin(dev, AD7616_PIN_CS, AD7616_PIN_SET);      
    
    // 返回数据
    if (cha_data) *cha_data = (int16_t)rxdata_a;
    if (chb_data) *chb_data = (int16_t)rxdata_b;
}

/**
 * @brief check if AD7616 is busy
 * @param dev: AD7616 device pointer
 * @return  AD7616_PIN_SET: busy, AD7616_PIN_RESET: not busy
 */
ad7616_pin_state_t myad7616_get_busy_status(ad7616_t *dev) {
    if (dev && dev->hw.read_pin) {
        return dev->hw.read_pin(dev->hw.user_ctx, AD7616_PIN_BUSY);
    }
    return AD7616_PIN_RESET;
}

/**
 * @brief update AD7616 configuration
 * @param dev: AD7616 device pointer
 * @param config: configuration
 * @return 0: success, -1: error
 */
int ad7616_update_config(ad7616_t *dev, const ad7616_config_t *config) {
    if (!dev || !config) return -1;
    
    dev->config = *config;

    configure_pins_by_config(dev);
    
    return 0;
}

/**
 * @brief get current configuration
 * @param dev: AD7616 device pointer
 * @return configuration
 */
const ad7616_config_t* ad7616_get_config(ad7616_t *dev) {
    if (!dev) return NULL;
    return &dev->config;
} 
