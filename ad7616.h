#ifndef _AD7616_H_
#define _AD7616_H_

#include <stdint.h>

#define u16 uint16_t
#define u8  uint8_t
#define i16 int16_t
#define i8  int8_t

// user can define the macro in their project settings
#define AD7616_PIN_ENRESET      0   // reset pin 
#define AD7616_PIN_SEQEN      1   // sequence 
#define AD7616_PIN_RNGSEL0    2   // range select 0
#define AD7616_PIN_RNGSEL1    3   // range select 1
#define AD7616_PIN_SER        4   // serial interface select
#define AD7616_PIN_WR         5   // write enable
#define AD7616_PIN_RD         6   // read enable
#define AD7616_PIN_CS         7   // chip select
#define AD7616_PIN_CHSEL0     8   // channel select 0
#define AD7616_PIN_CHSEL1     9   // channel select 1
#define AD7616_PIN_CHSEL2     10  // channel select 2
#define AD7616_PIN_CONVST     11  // conversion start
#define AD7616_PIN_BUSY       12  // busy

#define AD7616_DEFAULT_CONFIG { \
    .range = AD7616_RANGE_10V, \
    .interface = AD7616_INTERFACE_PARALLEL, \
    .mode = AD7616_MODE_HARDWARE, \
    .channel = AD7616_CHANNEL_VA0_VB0, \
    .enable_sequence = 0, \
    .enable_busy_check = 0 \
}

typedef enum {
    AD7616_PIN_RESET = 0,    
    AD7616_PIN_SET = 1      
} ad7616_pin_state_t;


typedef enum {
    AD7616_RANGE_10V = 0,    // ±10V
    AD7616_RANGE_5V = 1,     // ±5V
    AD7616_RANGE_2V5 = 2     // ±2.5V
} ad7616_range_t;


typedef enum {
    AD7616_INTERFACE_PARALLEL = 0,  
    AD7616_INTERFACE_SERIAL = 1     
} ad7616_interface_t;


typedef enum {
    AD7616_MODE_HARDWARE = 0,  
    AD7616_MODE_SOFTWARE = 1   
} ad7616_mode_t;


typedef enum {
    AD7616_CHANNEL_VA0_VB0 = 0,  
    AD7616_CHANNEL_VA1_VB1 = 1,  
    AD7616_CHANNEL_VA2_VB2 = 2,  
    AD7616_CHANNEL_VA3_VB3 = 3,  
    AD7616_CHANNEL_VA4_VB4 = 4,  
    AD7616_CHANNEL_VA5_VB5 = 5,  
    AD7616_CHANNEL_VA6_VB6 = 6,  
    AD7616_CHANNEL_VA7_VB7 = 7   
} ad7616_channel_t;


typedef struct {
    ad7616_range_t range;           
    ad7616_interface_t interface;   
    ad7616_mode_t mode;             
    ad7616_channel_t channel;      
    uint8_t enable_sequence;         
    uint8_t enable_busy_check;       
} ad7616_config_t;


typedef struct {
    void (*write_pin)(void *user_ctx, u8 pin_id, ad7616_pin_state_t state);
    
    ad7616_pin_state_t (*read_pin)(void *user_ctx, u8 pin_id);
    
    void (*delay_us)(void *user_ctx, u16 us);
    
    void (*delay_ms)(void *user_ctx, u16 ms);
    
    uint16_t (*read_data_bus)(void *user_ctx);
    
    void *user_ctx;

} ad7616_hw_ops_t;


typedef struct {
    ad7616_hw_ops_t hw;           
    ad7616_config_t config;       
    uint8_t is_initialized;       
} ad7616_t;

/* ==================== function declaration ==================== */


void ad7616_init(ad7616_t *dev);

void ad7616_init_with_config(ad7616_t *dev, const ad7616_config_t *config);

void ad7616_reset(ad7616_t *dev);

void ad7616_start_convst(ad7616_t *dev);

void ad7616_read_data(ad7616_t *dev, int16_t *cha_data, int16_t *chb_data);

ad7616_pin_state_t ad7616_get_busy_status(ad7616_t *dev);

int ad7616_update_config(ad7616_t *dev, const ad7616_config_t *config);

const ad7616_config_t* ad7616_get_config(ad7616_t *dev);

#endif 
