# 电磁阀板 CAN 控制模块说明

## 1. 背景

电磁阀板(STM32F0)固件:

- 监听 CAN 标准帧,ID = `0x123`;
- 取 `Data[0]` 的低 4 位,直接驱动 4 路 GPIO / 移位寄存器输出。

本工程 **claw(STM32F405)** 是**主控板**,不直接驱动电磁阀,而是作为**发送方**,
向 CAN 总线上发出 ID = `0x123` 的控制帧,由电磁阀板接收并执行。

> **历史说明**:早期本工程内曾存在一个独立的 `solenoid_new.c/.h`(从电磁阀板固件
> 改造而来的纯 CAN 发送模块),由 `solenoid.c` 转发调用。由于它的对外能力
> (`SetChannel` / `GetState` / `AllOff` 等)业务层一个都没用到,**该模块已删除**,
> CAN 发送逻辑直接合并进 `FML/Src/solenoid.c`,现在只有一个电磁阀模块。

## 2. CAN 协议

| 项目     | 取值                                                        |
| -------- | ----------------------------------------------------------- |
| 帧类型   | 标准数据帧(`CAN_ID_STD` + `CAN_RTR_DATA`)                  |
| 帧 ID    | `0x123`(宏 `SOLENOID_CAN_ID` 可改)                         |
| DLC      | 1                                                           |
| 数据     | `Data[0]` 低 4 位有效,高位忽略                              |
| 位定义   | bit0 → CH1,bit1 → CH2,bit2 → CH3,bit3 → CH4                |
| 电平含义 | `1` = 开启(ON),`0` = 关闭(OFF)                            |

示例: 点亮 CH1 与 CH3 → 控制字节 `0b0101 = 0x05`。

波特率:主控板 CAN1 = `PCLK1 42MHz / (3 × (1 + 9 + 4))` = **1 Mbps**,
与电磁阀板(48MHz / (3 × 16))一致。

## 3. 配置

配置项位于 `FML/Inc/solenoid.h`:

```c
#define SOLENOID_CAN_ID   0x123U   /* 控制帧 CAN 标准 ID */
#define SOLENOID_CAN_BUS  0U       /* 0 = CAN1, 1 = CAN2 */
```

> 注意: **电磁阀板必须与所选总线挂接在同一条 CAN 总线上**。
> 默认使用 CAN1(与 DJI 电机同总线)。

## 4. 接口

头文件: `FML/Inc/solenoid.h`

```c
void    solenoid_init(void);                        /* 初始化(绑定 CAN 句柄) */
void    solenoid_on(uint8_t channel, uint8_t cmd);  /* 按位图设置 4 路并下发 CAN */
uint8_t solenoid_get_state(void);                   /* 读回最后一次下发的控制字节(调试用) */
```

- `channel` 为**历史遗留参数**,仅用于兼容 `Claw.c` 的调用,**不参与报文组装**
  (报文内容完全由 `cmd` 的低 4 位决定)。
- **每次调用 `solenoid_on()` 都会实际下发一帧 CAN**,不做"值未变化则跳过"的去重;
  这样即使电磁阀板漏收一帧,后续相同的值也能重新发出去。

## 5. 使用示例

### 5.1 初始化

`Core/Src/main.c` 的 `USER CODE BEGIN 2` 中已调用(必须在 `MX_CANx_Init()` 之后):

```c
MX_CAN1_Init();
MX_CAN2_Init();
/* ... */
solenoid_init();
```

### 5.2 在任务中控制

```c
#include "solenoid.h"

/* 同时开启 CH1 和 CH2 */
solenoid_on(1, 0x03);

/* 只开 CH4 */
solenoid_on(1, 0x08);

/* 全部关闭 */
solenoid_on(1, 0x00);
```

### 5.3 与业务逻辑(Claw.c)的对接方式

业务层(`User/Src/Claw.c`)调用的是**原有接口** `solenoid_on(channel, cmd)`,
未作任何修改:

```c
void solenoid_on(uint8_t usart_channel, uint8_t cmd)
{
    CAN_TxHeaderTypeDef tx_header;
    uint32_t tx_mailbox;
    uint8_t tx_data[1];
    uint8_t data = cmd & 0x0FU;

    (void)usart_channel;        /* 仅用于兼容旧接口,不参与报文组装 */
    s_solenoid_state = data;

    if (s_solenoid_can == NULL) return;

    tx_data[0] = data;
    tx_header.StdId = SOLENOID_CAN_ID;
    tx_header.ExtId = 0U;
    tx_header.IDE   = CAN_ID_STD;
    tx_header.RTR   = CAN_RTR_DATA;
    tx_header.DLC   = 1U;
    tx_header.TransmitGlobalTime = DISABLE;

    if (HAL_CAN_AddTxMessage(s_solenoid_can, &tx_header, tx_data, &tx_mailbox) != HAL_OK)
    {
        (void)HAL_CAN_AbortTxRequest(s_solenoid_can, 0x07U);
    }
}
```

调用链:`Claw_Ready()` / `Claw_Sole()` / `Claw_Zero()` … → `solenoid_on(ch, cmd)`
→ CAN1 发出 ID=0x123 / DLC=1。

**要点**

- 业务层零改动,原有 `solenoid_on(...)` 的签名与调用方式完全保持。
- 只有低 4 位参与发送,与电磁阀板固件一致。
- 每次调用都下发一帧(不去重),保证"发消息一定控得了电磁阀"。

## 6. 构建集成

- 源文件 `solenoid.c` 已在 Keil 工程 `MDK-ARM/claw.uvprojx` 的 **FML** 分组中注册:
  `..\FML\Src\solenoid.c`
- 头文件搜索路径 `../FML/Inc` 已在 Target → C/C++ → Include Paths 中,
  无需额外配置。
- `solenoid_new.c/.h` **已从工程和磁盘中删除**,无需再注册。
