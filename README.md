# AD7616硬件并行工作驱动

---

## 说明

> 由GPIO模拟实现AD7616的并行工作时序

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
