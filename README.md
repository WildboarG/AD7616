# AD7616硬件并行工作驱动

---

## 说明

> 实现AD7616的并行工作时序

平台无关：核心驱动不依赖特定单片机

易于移植：只需实现硬件回调函数

1. 对要使用的GPIO写操作
2. GPIO读操作
3. 微妙级的延时
4. MS级的延时
5. 并行读取数据的寄存器/引脚

```c
typedef struct {
    // 设置指定引脚的电平状态
    void (*write_pin)(void *user_ctx, uint8_t pin_id, ad7616_pin_state_t state);
    
    // 读取指定引脚的电平状态
    ad7616_pin_state_t (*read_pin)(void *user_ctx, uint8_t pin_id);
    
    // 微秒级延时
    void (*delay_us)(void *user_ctx, uint32_t us);
    
    // 毫秒级延时
    void (*delay_ms)(void *user_ctx, uint32_t ms);
    
    // 读取数据总线（16位）
    uint16_t (*read_data_bus)(void *user_ctx);
    
    // 用户自定义上下文指针
    void *user_ctx;
} ad7616_hw_ops_t;

```

使用
---

- 导入`ad7616.c` `ad7616.h`到项目中
- GPIO引脚初始化
- 调用AD7616初始化
- 根据配置方式读取转换结果



## 例程

---



以stm32h750为例

在main中引用AD7616.h后



1. 自定义硬件结构体

```c
typedef struct {
    GPIO_TypeDef *GPIOx[NUM_PINS]; // 每个pin_id对应的GPIO端口
    uint16_t GPIO_Pin[NUM_PINS];   // 每个pin_id对应的引脚号
    // 也可以加定时器、DMA、总线等自定义成员
} my_hw_ctx_t;
```

2. 实例化硬件

- 将AD7616与真实的STM32相连接的引脚顺序对应起来，引脚顺序在AD7616.h 中有定义

```txt
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
//这里总共有13个引脚，下方实例化端口多了一个GPIOD其作为数据总线使用（D0~D15连接GPIOD0~GPIOD15)
```



```c
my_hw_ctx_t g_hw_ctx = {
    .GPIOx = {GPIOB,GPIOB,GPIOB,GPIOB,GPIOB,GPIOB,GPIOB,
              GPIOB,GPIOB,GPIOB,GPIOB,GPIOE,GPIOE,GPIOD}, // 填你每个pin_id对应的端口
    .GPIO_Pin = {GPIO_PIN_7,GPIO_PIN_6,GPIO_PIN_4,GPIO_PIN_5,GPIO_PIN_3,
			     GPIO_PIN_15,GPIO_PIN_14,GPIO_PIN_13,GPIO_PIN_12,GPIO_PIN_11,
               	 GPIO_PIN_10,GPIO_PIN_14,GPIO_PIN_15} //对应连接AD7616的真实引脚
};
```



3. 实现驱动所需的5个回调函数

```c
// 写GPIO
void my_write_pin(void *user_ctx, uint8_t pin_id, ad7616_pin_state_t state) {
    my_hw_ctx_t *ctx = (my_hw_ctx_t *)user_ctx;
    HAL_GPIO_WritePin(ctx->GPIOx[pin_id], ctx->GPIO_Pin[pin_id], (GPIO_PinState)state);
}
// 读GPIO
ad7616_pin_state_t my_read_pin(void *user_ctx, uint8_t pin_id) {
    my_hw_ctx_t *ctx = (my_hw_ctx_t *)user_ctx;
    GPIO_PinState val = HAL_GPIO_ReadPin(ctx->GPIOx[pin_id], ctx->GPIO_Pin[pin_id]);
    return (ad7616_pin_state_t)val;
}
// 微妙延时
void my_delay_us(void *user_ctx, uint32_t us) {
    // 若使用HAL库，可以用例如如下实现
    //注意：标准HAL库没有us延时，可以用DWT，或用空循环，或用外部库
		Delay_us((uint32_t)us);
}
// 毫秒延时
void my_delay_ms(void *user_ctx, uint32_t ms) {
    HAL_Delay((uint32_t)ms);
}

// 读写AD7616采集的数据 
uint16_t my_read_data_bus(void *user_ctx) {
    // 假设你用某组GPIO端口作为ADC数据总线（D0~D15）
    // 我在实例化端口中多填写一个GPIOD,用它最为连接数据总线的端口
    my_hw_ctx_t *ctx = (my_hw_ctx_t *)user_ctx;
    uint16_t value = 0;
    // 这里实际读取方法取决于你的硬件连接方式
    // 假设所有在PORTD, D0~D15依次为PD0~PD15
    value = (uint16_t)(user_ctx[13]->IDR & 0xFFFF);
    return value;
}
```





至此，移植的准备工作完成

```C
//注册回调函数
ad7616_t ad7616_device = {
    // 初始化内部的 hw 成员
    .hw = {
          .write_pin = my_write_pin,
					.read_pin = my_read_pin,
					.delay_us = my_delay_us,
					.delay_ms = my_delay_ms,
					.read_data_bus = my_read_data_bus,
					.user_ctx = &g_hw_ctx
    },
    .config = NULL,  //留空自动加载默认配置
    .is_initialized = 0  
};

int main(void)
{
    MX_GPIO_Init();
    ...
        
    ad7616_init(&ad7616_device);  //初始化AD7616,
    
    int16_t ch1=0,ch2=0;
    while(1){
        //开启AD7616采集
			ad7616_start_convst(&ad7616_device);  //给触发信号
			while(ad7616_get_busy_status(&ad7616_device) == AD7616_PIN_SET){
				my_delay_us(&ad7616_device,2);
			}
			ad7616_read_data(&ad7616_device,&ch1,&ch2);//读取数据结果
        
        // 延时(可选)
        HAL_Delay(1000);
    }
        
    
```

