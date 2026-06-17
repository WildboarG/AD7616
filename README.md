
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
        
    
