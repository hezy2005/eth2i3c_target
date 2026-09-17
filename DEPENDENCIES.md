# 构建依赖

[返回项目 README](README.md)

工程文件通过相对路径引用同级 `common` 目录，因此单独 clone 本仓库后，需要先准备以下依赖。

## 已验证版本

| 组件 | 已验证版本 | 本地版本依据 |
| --- | --- | --- |
| STM32CubeH5 / STM32H5 HAL/LL | V1.5.0 | `stm32h5xx_hal.c` 中 HAL 版本为 1.5.0 |
| IAR C/C++ Compiler for Arm | 9.50.2 | EWARM 工程已验证 |
| STM32CubeIDE | 1.17.0 | Debug 工程配置已维护 |

## 目录结构

```text
workspace/
├── common/
│   └── Drivers/
│       ├── CMSIS/
│       └── STM32H5xx_HAL_Driver/
├── eth2i3c/
└── eth2i3c_target/
```

STM32H5 CMSIS Device 和 HAL/LL Drivers 来自
[STM32CubeH5](https://github.com/STMicroelectronics/STM32CubeH5) V1.5.0。当前 `common` 是经过实机验证的
源码快照，不包含用于确认精确 commit 的 Git 元数据。替换依赖版本后，应重新 clean build，并复测
RSTDAA、ENTDAA、random write、random read 和 current read。
