# eth2i3c_target：基于 STM32H563 的 I3C Target 寄存器模型

`eth2i3c_target` 是运行在 STM32H563 上的 I3C Target 测试固件，用于配合
[eth2i3c](https://github.com/hezy2005/eth2i3c) Controller 完成端到端通信验证。

项目基于 STM32H5 I3C LL 驱动和中断机制，实现动态地址分配、private transfer 以及一个简单、可预测的
256 字节寄存器模型。它不依赖 RTOS，适合分析 I3C Target 收发时序和验证 Controller 行为。

## 系统角色

```text
┌──────────────────┐    TCP    ┌────────────────────────┐
│ PC / Python 客户端 │ ────────> │ eth2i3c Controller     │
└──────────────────┘           │ STM32H563              │
                                └───────────┬────────────┘
                                            │ I3C1: PB8 / PB9
                                            ▼
                                ┌────────────────────────┐
                                │ eth2i3c_target         │
                                │ STM32H563 Target       │
                                └───────────┬────────────┘
                                            │
                                            ▼
                                ┌────────────────────────┐
                                │ 256 字节寄存器模型     │
                                └────────────────────────┘
```

Controller 负责网络通信、命令解析和 I3C 总线控制；本项目只实现 Target 端行为。

## 主要功能

- 使用 STM32H563 I3C1 Target 模式。
- 支持 `RSTDAA` 和 `ENTDAA` 流程，并在 DA 更新中断中保存新的动态地址。
- 配合 Controller 验证 `GETPID`、`GETBCR`、`GETDCR`、`GETMWL`、`GETMRL`、`GETSTATUS`、
  `SETMWL`、`SETMRL`、`ENEC` 和 `DISEC`。
- 支持 private random write、random read 和 current read。
- 使用 RX FIFO、TX FIFO 和 frame-complete 中断完成收发。
- 支持 repeated-start random read；收到寄存器地址后立即预装 TX FIFO。
- 同时维护 IAR EWARM 和 STM32CubeIDE 工程。

## 寄存器模型

完整寄存器地址空间为 `0x00`～`0xFF`，分为两个 128 字节 bank：

```c
#define TARGET_MEMORY_BANK_SIZE 128U

uint8_t lower_memory[TARGET_MEMORY_BANK_SIZE] = {0};
uint8_t high_memory[TARGET_MEMORY_BANK_SIZE] = {0};
```

| 地址范围 | 存储区 |
| --- | --- |
| `0x00`～`0x7F` | `lower_memory` |
| `0x80`～`0xFF` | `high_memory` |

顺序访问到达 bank 末尾后会在当前 bank 内回绕。例如，从 `0x7E` 连续访问 4 字节时，地址依次为
`0x7E`、`0x7F`、`0x00`、`0x01`。

### Random write

Controller 发送一个寄存器地址字节，后面跟随待写数据：

```text
[register address] [data 0] [data 1] ...
```

写操作完成后，current address 指向最后一个已写字节的下一个地址。

### Random read

Controller 先发送一个寄存器地址字节，再通过 repeated-start 发起读操作。Target 在收到地址字节时立即
刷新并预装 TX FIFO，以满足紧接着发生的读阶段。

### Current read

Controller 不重新发送寄存器地址，Target 从内部维护的 current address 继续返回数据。读操作结束后，
current address 按实际传输字节数前移。

## 硬件连接

| 项目 | 配置 |
| --- | --- |
| 开发板 | NUCLEO-H563ZI（STM32H5 Nucleo-144，MB1404） |
| MCU | STM32H563ZIT6 |
| 系统时钟 | 250 MHz |
| I3C 外设 | I3C1 Target 模式 |
| I3C 速率 | 12.5 MHz，由 Controller 提供时钟 |
| SCL | PB8；Arduino D15 / CN6 pin 2；Morpho CN5 pin 3 |
| SDA | PB9；Arduino D14 / CN6 pin 4；Morpho CN5 pin 5 |

Controller 和 Target 必须共地，I3C 连线应尽量短。信号质量不理想时，可分别使用一根地线与 SCL、SDA
绞合。板卡接口和跳线信息可参考 ST 的
[NUCLEO-H563ZI 产品页](https://www.st.com/en/evaluation-tools/nucleo-h563zi.html)及 MB1404 用户手册
UM3115。

## 中断处理流程

1. `RXFNE`：读取 Controller 写入的数据；首字节作为寄存器地址，并为可能发生的 repeated-start read
   预装 TX FIFO。
2. `FC`：根据方向处理一帧写入或读取，更新 current address，刷新 FIFO，并准备下一次传输。
3. `TXFNF`：继续向 TX FIFO 填入当前 bank 中的数据。
4. `DAUPD`：读取并保存 ENTDAA 分配的新动态地址。
5. `ERR`：进入统一错误处理。

TX FIFO 采用 preload 方式。每次读取完成后都会重新刷新并预装下一段数据，避免后续读取返回旧数据。

## 仓库布局

```text
eth2i3c_target/
├── Inc/                     # 应用及中断头文件
├── Src/                     # 初始化与 I3C Target 中断实现
├── EWARM/                   # IAR Embedded Workbench 工程
├── STM32CubeIDE/            # STM32CubeIDE 工程
├── DEPENDENCIES.md          # 依赖版本与目录搭建方法
├── THIRD_PARTY_NOTICES.md
└── eth2i3c_target.ioc       # STM32CubeMX 配置
```

工程通过 `../common` 引用 STM32H5 Drivers 和 CMSIS：

```text
workspace/
├── common/
├── eth2i3c/
└── eth2i3c_target/
```

准确版本和依赖获取方式见 [构建依赖](DEPENDENCIES.md)。

## 编译与使用

### 1. 编译固件

- IAR EWARM：打开 `EWARM/Project.eww`，选择 `eth2i3c_target` configuration。
- STM32CubeIDE：导入 `STM32CubeIDE` 目录，选择 `Debug` configuration。

### 2. 连接并烧录

1. 将 Controller 与 Target 的 PB8、PB9 和 GND 对应连接。
2. 分别编译并烧录 `eth2i3c` 与 `eth2i3c_target`。
3. 先启动 Controller 固件，再复位 Target，避免 Controller 尚未驱动总线时 Target 看到无效启动状态。
4. 启动 Controller 侧 Python 客户端，通过 RSTDAA 和 ENTDAA 重新建立动态地址状态。

### 3. 在 Controller 侧测试

```python
from i3c import I3CController

c = I3CController()
c.rstdaa()
targets = c.entdaa()
print(targets)

target = next(target for target in targets if target.da != 0)
c.target_set_active_by_da(target.da)

c.w(0x80, [0x10, 0x20, 0x30, 0x40])
assert c.r(0x80, 4) == [0x10, 0x20, 0x30, 0x40]
print(c.r_current(4))
```

完整的网络配置、Python API 和命令说明请参阅
[eth2i3c README](https://github.com/hezy2005/eth2i3c)。

## 当前限制

- 这是用于 Controller 联调的简化寄存器模型，不模拟具体量产器件。
- 不支持 IBI、Hot-Join 或 Controller Role Request。
- 每个 128 字节 bank 独立回绕，顺序访问不会自动跨越 bank 边界。
- 公共 Drivers 和 CMSIS 通过仓库外的 `../common` 引用，构建时需要保持上述目录结构。

## 许可证

项目代码按 [BSD 3-Clause License](LICENSE) 发布。STM32CubeH5、CMSIS Device 和 HAL/LL Drivers 的
版权及许可证见 [第三方软件声明](THIRD_PARTY_NOTICES.md)。
