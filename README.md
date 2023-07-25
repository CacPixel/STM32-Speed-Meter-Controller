# STM32-Speed-Meter-Controller
一个基于STM32的速度表控制器

使用STM32F103C8T6最小系统板驱动东洋计器（TOYO KEIKI）的DVF-11ED速度表。
表针部分使用PWM驱动，荧光灯使用高压板驱动，高压板上有使能引脚连接至单片机的GPIO。

STM32使用HAL库+STM32CubeIDE开发，`/MDK-ARM`目录下存有Keil的工程文件，可用于烧录或调试其他兼容STM32的单片机（比如GD32）。

51单片机版本：https://github.com/CacPixel/8051-Speed-Meter-Controller
