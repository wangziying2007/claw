<!--
 * @Author: Frt001 2067314783@qq.com
 * @Date: 2026-08-24 15:45:10
 * @LastEditors: frt 2067314783@qq.com
 * @LastEditTime: 2026-09-10 16:45:08
 * @FilePath: \f4_show\README.md
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
-->

# stm32f405rgt6 HAL库freertos版本模板

## 模块简介

stm32f405rgt6 HAL库freertos版本模板，包含了基本的工程结构和配置，适用于快速开发嵌入式应用。

## 负责人

- **开发：** [付润天]
- **测试：** [付润天]

## 软件环境

- 开发工具链（cubemx+keil / 6.22）
- 依赖库（STM32Cube FW_F4 V1.28.3）
- freertos V10.3.1
- 烧录工具 不限

## 文件说明

- `APL/` - 用户自定义扩展功能，在此目录下新建子目录并添加功能模块
- `Core/` - cubemx生成的源码，禁止添加新的文件
- `DOCS/` - 模板工程相关文档
- `FML/` - 功能模块库，包含了常用的零碎功能模块，按需补充
- `HDL/` - 板级外设驱动，包含灯和蜂鸣
- `IRQ/` - 中断服务函数，包含了中断回调函数重定义
- `Motor/` - 电机驱动，目前仅添加了z和dji

## 进度记录
9/10  添加进2027RC/，大部分功能完善，可以满足大部分情况下的调试需求
## 注意事项
使用此模板前，请仔细阅读模板移植事项，以及命名规范

