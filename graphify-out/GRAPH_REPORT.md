# Graph Report - stm32f7_github_compare  (2026-04-25)

## Corpus Check
- 2184 files · ~1,394,793 words
- Verdict: corpus is large enough that graph structure adds value.

## Summary
- 5177 nodes · 4295 edges · 23 communities detected
- Extraction: 86% EXTRACTED · 14% INFERRED · 0% AMBIGUOUS · INFERRED: 605 edges (avg confidence: 0.8)
- Token cost: 0 input · 0 output

## Community Hubs (Navigation)
- [[_COMMUNITY_Community 2|Community 2]]
- [[_COMMUNITY_Community 3|Community 3]]
- [[_COMMUNITY_Community 7|Community 7]]
- [[_COMMUNITY_Community 9|Community 9]]
- [[_COMMUNITY_Community 10|Community 10]]
- [[_COMMUNITY_Community 12|Community 12]]
- [[_COMMUNITY_Community 14|Community 14]]
- [[_COMMUNITY_Community 15|Community 15]]
- [[_COMMUNITY_Community 18|Community 18]]
- [[_COMMUNITY_Community 19|Community 19]]
- [[_COMMUNITY_Community 21|Community 21]]
- [[_COMMUNITY_Community 22|Community 22]]
- [[_COMMUNITY_Community 25|Community 25]]
- [[_COMMUNITY_Community 26|Community 26]]
- [[_COMMUNITY_Community 30|Community 30]]
- [[_COMMUNITY_Community 34|Community 34]]
- [[_COMMUNITY_Community 35|Community 35]]
- [[_COMMUNITY_Community 36|Community 36]]
- [[_COMMUNITY_Community 37|Community 37]]
- [[_COMMUNITY_Community 42|Community 42]]
- [[_COMMUNITY_Community 46|Community 46]]
- [[_COMMUNITY_Community 50|Community 50]]
- [[_COMMUNITY_Community 54|Community 54]]

## God Nodes (most connected - your core abstractions)
1. `HAL_GetTick()` - 74 edges
2. `TIM_CCxChannelCmd()` - 39 edges
3. `HAL_DMA_Start_IT()` - 30 edges
4. `App_Run()` - 28 edges
5. `I2C_Enable_IRQ()` - 28 edges
6. `App_Run()` - 27 edges
7. `main()` - 25 edges
8. `I2C_TransferConfig()` - 24 edges
9. `HAL_Delay()` - 24 edges
10. `LCD_Display_Update()` - 22 edges

## Surprising Connections (you probably didn't know these)
- `cmd_cb()` --calls--> `HAL_GetTick()`  [INFERRED]
  stm32f7disco/Src/app.pin-diagnostic-backup.2026-04-24.c → stm32f7disco/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal.c
- `cmd_cb()` --calls--> `HAL_GetTick()`  [INFERRED]
  stm32f7disco/Src/app.c → stm32f7disco/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal.c
- `LCD_Display_Update()` --calls--> `Motor_GetDuty()`  [INFERRED]
  stm32f7disco/Src/lcd_display.c → stm32f7disco/Src/motor.c
- `HAL_LTDC_MspDeInit()` --calls--> `HAL_GPIO_DeInit()`  [INFERRED]
  stm32f7disco/Src/ltdc.c → stm32f7disco/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_gpio.c
- `_gettimeofday()` --calls--> `HAL_GetTick()`  [INFERRED]
  stm32f7disco/Src/syscalls.c → stm32f7disco/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal.c

## Communities

### Community 2 - "Community 2"
Cohesion: 0.02
Nodes (152): HAL_DMA_Abort_IT(), HAL_TIMEx_Break2Callback(), HAL_TIMEx_BreakCallback(), HAL_TIMEx_CommutCallback(), HAL_TIMEx_CommutHalfCpltCallback(), HAL_TIMEx_HallSensor_DeInit(), HAL_TIMEx_HallSensor_Init(), HAL_TIMEx_HallSensor_MspDeInit() (+144 more)

### Community 3 - "Community 3"
Cohesion: 0.02
Nodes (132): MX_DMA2D_Init(), bank0(), detect_imu(), draw(), IMU_Min_Test_Run(), init_min(), rd(), wr() (+124 more)

### Community 7 - "Community 7"
Cohesion: 0.04
Nodes (103): HAL_DMA_Start_IT(), HAL_GetTick(), HAL_I2C_AbortCpltCallback(), HAL_I2C_AddrCallback(), HAL_I2C_DeInit(), HAL_I2C_DisableListen_IT(), HAL_I2C_EnableListen_IT(), HAL_I2C_ER_IRQHandler() (+95 more)

### Community 9 - "Community 9"
Cohesion: 0.03
Nodes (90): cubemx_transport_close(), cubemx_transport_open(), cubemx_transport_write(), cubemx_transport_close(), cubemx_transport_open(), cubemx_transport_write(), HAL_UART_RxCpltCallback(), clock_gettime() (+82 more)

### Community 10 - "Community 10"
Cohesion: 0.03
Nodes (63): NVIC_DecodePriority(), SysTick_Config(), HAL_I2C_MspInit(), HAL_LTDC_MspDeInit(), HAL_LTDC_MspInit(), MPU_Config(), Error_Handler(), HAL_TIM_PeriodElapsedCallback() (+55 more)

### Community 12 - "Community 12"
Cohesion: 0.02
Nodes (81): HAL_FMC_MspDeInit(), HAL_FMC_MspInit(), HAL_SDRAM_MspDeInit(), HAL_SDRAM_MspInit(), MX_FMC_Init(), BSP_SDRAM_DeInit(), BSP_SDRAM_Init(), BSP_SDRAM_Initialization_sequence() (+73 more)

### Community 14 - "Community 14"
Cohesion: 0.05
Nodes (81): App_MotorEsp32Diag_Run(), App_Run(), clamp01f(), cmd_cb(), motor_pid(), App_Run(), boot_test_apply(), boot_test_apply_single() (+73 more)

### Community 15 - "Community 15"
Cohesion: 0.04
Nodes (57): ft5336_Get_I2C_InitializedStatus(), ft5336_I2C_InitializeIfRequired(), ft5336_Init(), ft5336_ReadID(), ft5336_TS_Configure(), ft5336_TS_DetectTouch(), ft5336_TS_DisableIT(), ft5336_TS_EnableIT() (+49 more)

### Community 18 - "Community 18"
Cohesion: 0.06
Nodes (13): DSI_ConfigPacketHeader(), DSI_ShortWrite(), HAL_DSI_DeInit(), HAL_DSI_EndOfRefreshCallback(), HAL_DSI_ErrorCallback(), HAL_DSI_Init(), HAL_DSI_IRQHandler(), HAL_DSI_LongWrite() (+5 more)

### Community 19 - "Community 19"
Cohesion: 0.1
Nodes (33): FLASH_Erase_Sector(), FLASH_MassErase(), FLASH_OB_BootAddressConfig(), FLASH_OB_BOR_LevelConfig(), FLASH_OB_DisableWRP(), FLASH_OB_EnableWRP(), FLASH_OB_GetBootAddress(), FLASH_OB_GetBOR() (+25 more)

### Community 21 - "Community 21"
Cohesion: 0.05
Nodes (2): TZ_NVIC_SetPriority_NS(), TZ_SysTick_Config_NS()

### Community 22 - "Community 22"
Cohesion: 0.05
Nodes (2): TZ_NVIC_SetPriority_NS(), TZ_SysTick_Config_NS()

### Community 25 - "Community 25"
Cohesion: 0.06
Nodes (2): TZ_NVIC_SetPriority_NS(), TZ_SysTick_Config_NS()

### Community 26 - "Community 26"
Cohesion: 0.07
Nodes (3): NVIC_EncodePriority(), TZ_NVIC_SetPriority_NS(), TZ_SysTick_Config_NS()

### Community 30 - "Community 30"
Cohesion: 0.1
Nodes (5): generate_launch_description(), bringup.launch.py  —  Full Raspberry Pi 4B hardware launch, Node, main(), Saver

### Community 34 - "Community 34"
Cohesion: 0.11
Nodes (3): _exit(), _gettimeofday(), _kill()

### Community 35 - "Community 35"
Cohesion: 0.17
Nodes (13): ARM_MPU_ClrRegion(), ARM_MPU_ClrRegion_NS(), ARM_MPU_ClrRegionEx(), ARM_MPU_Load(), ARM_MPU_Load_NS(), ARM_MPU_LoadEx(), ARM_MPU_SetMemAttr(), ARM_MPU_SetMemAttr_NS() (+5 more)

### Community 36 - "Community 36"
Cohesion: 0.12
Nodes (3): HAL_PWR_EnableBkUpAccess(), HAL_PWR_PVD_IRQHandler(), HAL_PWR_PVDCallback()

### Community 37 - "Community 37"
Cohesion: 0.16
Nodes (8): get_messages_arrays(), get_messages_basic_types(), get_messages_bounded_plain_sequences(), get_messages_bounded_sequences(), get_messages_multi_nested(), get_messages_nested(), get_messages_unbounded_sequences(), get_services_arrays()

### Community 42 - "Community 42"
Cohesion: 0.23
Nodes (11): getBlockSize(), prvHeapInit(), prvInsertBlockIntoFreeList(), pvPortCallocMicroROS(), pvPortMallocMicroROS(), pvPortReallocMicroROS(), vPortFreeMicroROS(), microros_allocate() (+3 more)

### Community 46 - "Community 46"
Cohesion: 0.29
Nodes (2): ARM_MPU_Load(), orderedCpy()

### Community 50 - "Community 50"
Cohesion: 0.6
Nodes (3): DMA_MultiBufferSetConfig(), HAL_DMAEx_MultiBufferStart(), HAL_DMAEx_MultiBufferStart_IT()

### Community 54 - "Community 54"
Cohesion: 1.0
Nodes (2): clock_gettime(), UTILS_NanosecondsToTimespec()

## Knowledge Gaps
- **1 isolated node(s):** `bringup.launch.py  —  Full Raspberry Pi 4B hardware launch`
  These have ≤1 connection - possible missing edges or undocumented components.
- **Thin community `Community 21`** (39 nodes): `ITM_CheckChar()`, `ITM_ReceiveChar()`, `ITM_SendChar()`, `__NVIC_ClearPendingIRQ()`, `NVIC_ClearTargetState()`, `NVIC_DecodePriority()`, `__NVIC_DisableIRQ()`, `__NVIC_EnableIRQ()`, `NVIC_EncodePriority()`, `__NVIC_GetActive()`, `__NVIC_GetEnableIRQ()`, `__NVIC_GetPendingIRQ()`, `__NVIC_GetPriority()`, `__NVIC_GetPriorityGrouping()`, `NVIC_GetTargetState()`, `__NVIC_GetVector()`, `__NVIC_SetPendingIRQ()`, `__NVIC_SetPriority()`, `__NVIC_SetPriorityGrouping()`, `NVIC_SetTargetState()`, `__NVIC_SetVector()`, `__NVIC_SystemReset()`, `SCB_GetFPUType()`, `SysTick_Config()`, `TZ_NVIC_ClearPendingIRQ_NS()`, `TZ_NVIC_DisableIRQ_NS()`, `TZ_NVIC_EnableIRQ_NS()`, `TZ_NVIC_GetActive_NS()`, `TZ_NVIC_GetEnableIRQ_NS()`, `TZ_NVIC_GetPendingIRQ_NS()`, `TZ_NVIC_GetPriority_NS()`, `TZ_NVIC_GetPriorityGrouping_NS()`, `TZ_NVIC_SetPendingIRQ_NS()`, `TZ_NVIC_SetPriority_NS()`, `TZ_NVIC_SetPriorityGrouping_NS()`, `TZ_SAU_Disable()`, `TZ_SAU_Enable()`, `TZ_SysTick_Config_NS()`, `core_cm33.h`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Community 22`** (39 nodes): `ITM_CheckChar()`, `ITM_ReceiveChar()`, `ITM_SendChar()`, `__NVIC_ClearPendingIRQ()`, `NVIC_ClearTargetState()`, `NVIC_DecodePriority()`, `__NVIC_DisableIRQ()`, `__NVIC_EnableIRQ()`, `NVIC_EncodePriority()`, `__NVIC_GetActive()`, `__NVIC_GetEnableIRQ()`, `__NVIC_GetPendingIRQ()`, `__NVIC_GetPriority()`, `__NVIC_GetPriorityGrouping()`, `NVIC_GetTargetState()`, `__NVIC_GetVector()`, `__NVIC_SetPendingIRQ()`, `__NVIC_SetPriority()`, `__NVIC_SetPriorityGrouping()`, `NVIC_SetTargetState()`, `__NVIC_SetVector()`, `__NVIC_SystemReset()`, `SCB_GetFPUType()`, `SysTick_Config()`, `TZ_NVIC_ClearPendingIRQ_NS()`, `TZ_NVIC_DisableIRQ_NS()`, `TZ_NVIC_EnableIRQ_NS()`, `TZ_NVIC_GetActive_NS()`, `TZ_NVIC_GetEnableIRQ_NS()`, `TZ_NVIC_GetPendingIRQ_NS()`, `TZ_NVIC_GetPriority_NS()`, `TZ_NVIC_GetPriorityGrouping_NS()`, `TZ_NVIC_SetPendingIRQ_NS()`, `TZ_NVIC_SetPriority_NS()`, `TZ_NVIC_SetPriorityGrouping_NS()`, `TZ_SAU_Disable()`, `TZ_SAU_Enable()`, `TZ_SysTick_Config_NS()`, `core_armv8mml.h`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Community 25`** (32 nodes): `__NVIC_ClearPendingIRQ()`, `NVIC_ClearTargetState()`, `NVIC_DecodePriority()`, `__NVIC_DisableIRQ()`, `__NVIC_EnableIRQ()`, `NVIC_EncodePriority()`, `__NVIC_GetActive()`, `__NVIC_GetEnableIRQ()`, `__NVIC_GetPendingIRQ()`, `__NVIC_GetPriority()`, `NVIC_GetTargetState()`, `__NVIC_GetVector()`, `__NVIC_SetPendingIRQ()`, `__NVIC_SetPriority()`, `NVIC_SetTargetState()`, `__NVIC_SetVector()`, `__NVIC_SystemReset()`, `SCB_GetFPUType()`, `SysTick_Config()`, `TZ_NVIC_ClearPendingIRQ_NS()`, `TZ_NVIC_DisableIRQ_NS()`, `TZ_NVIC_EnableIRQ_NS()`, `TZ_NVIC_GetActive_NS()`, `TZ_NVIC_GetEnableIRQ_NS()`, `TZ_NVIC_GetPendingIRQ_NS()`, `TZ_NVIC_GetPriority_NS()`, `TZ_NVIC_SetPendingIRQ_NS()`, `TZ_NVIC_SetPriority_NS()`, `TZ_SAU_Disable()`, `TZ_SAU_Enable()`, `TZ_SysTick_Config_NS()`, `core_cm23.h`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Community 46`** (8 nodes): `ARM_MPU_ClrRegion()`, `ARM_MPU_Disable()`, `ARM_MPU_Enable()`, `ARM_MPU_Load()`, `ARM_MPU_SetRegion()`, `ARM_MPU_SetRegionEx()`, `orderedCpy()`, `mpu_armv7.h`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Community 54`** (3 nodes): `clock_gettime()`, `UTILS_NanosecondsToTimespec()`, `microros_time.c`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `HAL_GetTick()` connect `Community 7` to `Community 34`, `Community 3`, `Community 9`, `Community 10`, `Community 12`, `Community 14`, `Community 18`, `Community 19`?**
  _High betweenness centrality (0.025) - this node is a cross-community bridge._
- **Why does `HAL_Delay()` connect `Community 14` to `Community 3`, `Community 7`, `Community 9`, `Community 10`, `Community 12`, `Community 15`, `Community 18`?**
  _High betweenness centrality (0.009) - this node is a cross-community bridge._
- **Why does `HAL_DMA_Start_IT()` connect `Community 7` to `Community 9`, `Community 2`, `Community 12`?**
  _High betweenness centrality (0.005) - this node is a cross-community bridge._
- **Are the 72 inferred relationships involving `HAL_GetTick()` (e.g. with `IMU_Min_Test_Run()` and `cmd_cb()`) actually correct?**
  _`HAL_GetTick()` has 72 INFERRED edges - model-reasoned connections that need verification._
- **Are the 10 inferred relationships involving `TIM_CCxChannelCmd()` (e.g. with `HAL_TIMEx_HallSensor_Start()` and `HAL_TIMEx_HallSensor_Stop()`) actually correct?**
  _`TIM_CCxChannelCmd()` has 10 INFERRED edges - model-reasoned connections that need verification._
- **Are the 28 inferred relationships involving `HAL_DMA_Start_IT()` (e.g. with `HAL_I2C_Master_Transmit_DMA()` and `HAL_I2C_Master_Receive_DMA()`) actually correct?**
  _`HAL_DMA_Start_IT()` has 28 INFERRED edges - model-reasoned connections that need verification._
- **Are the 18 inferred relationships involving `App_Run()` (e.g. with `LCD_Display_Init()` and `LCD_Display_BootStatus()`) actually correct?**
  _`App_Run()` has 18 INFERRED edges - model-reasoned connections that need verification._