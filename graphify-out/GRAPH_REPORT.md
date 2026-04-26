# Graph Report - stm32f7  (2026-04-26)

## Corpus Check
- 2187 files · ~1,400,103 words
- Verdict: corpus is large enough that graph structure adds value.

## Summary
- 5204 nodes · 4331 edges · 25 communities detected
- Extraction: 86% EXTRACTED · 14% INFERRED · 0% AMBIGUOUS · INFERRED: 605 edges (avg confidence: 0.8)
- Token cost: 0 input · 0 output

## Community Hubs (Navigation)
- [[_COMMUNITY_Community 2|Community 2]]
- [[_COMMUNITY_Community 5|Community 5]]
- [[_COMMUNITY_Community 7|Community 7]]
- [[_COMMUNITY_Community 8|Community 8]]
- [[_COMMUNITY_Community 11|Community 11]]
- [[_COMMUNITY_Community 12|Community 12]]
- [[_COMMUNITY_Community 14|Community 14]]
- [[_COMMUNITY_Community 15|Community 15]]
- [[_COMMUNITY_Community 18|Community 18]]
- [[_COMMUNITY_Community 19|Community 19]]
- [[_COMMUNITY_Community 20|Community 20]]
- [[_COMMUNITY_Community 21|Community 21]]
- [[_COMMUNITY_Community 23|Community 23]]
- [[_COMMUNITY_Community 24|Community 24]]
- [[_COMMUNITY_Community 27|Community 27]]
- [[_COMMUNITY_Community 34|Community 34]]
- [[_COMMUNITY_Community 35|Community 35]]
- [[_COMMUNITY_Community 36|Community 36]]
- [[_COMMUNITY_Community 37|Community 37]]
- [[_COMMUNITY_Community 42|Community 42]]
- [[_COMMUNITY_Community 46|Community 46]]
- [[_COMMUNITY_Community 48|Community 48]]
- [[_COMMUNITY_Community 51|Community 51]]
- [[_COMMUNITY_Community 55|Community 55]]
- [[_COMMUNITY_Community 2127|Community 2127]]

## God Nodes (most connected - your core abstractions)
1. `HAL_GetTick()` - 74 edges
2. `TIM_CCxChannelCmd()` - 39 edges
3. `HAL_DMA_Start_IT()` - 30 edges
4. `App_Run()` - 29 edges
5. `I2C_Enable_IRQ()` - 28 edges
6. `App_Run()` - 27 edges
7. `main()` - 25 edges
8. `I2C_TransferConfig()` - 24 edges
9. `HAL_Delay()` - 24 edges
10. `LCD_Display_Update()` - 22 edges

## Surprising Connections (you probably didn't know these)
- `cmd_cb()` --calls--> `HAL_GetTick()`  [INFERRED]
  stm32f7disco/Src/app.c → /home/ramana/stm32f7_github_compare/stm32f7disco/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal.c
- `Motor_GetDuty()` --calls--> `LCD_Display_Update()`  [INFERRED]
  stm32f7disco/Src/motor.c → /home/ramana/stm32f7_github_compare/stm32f7disco/Src/lcd_display.c
- `boot_test_apply()` --calls--> `Motor_StopAll()`  [INFERRED]
  /home/ramana/stm32f7_github_compare/stm32f7disco/Src/app.pin-diagnostic-backup.2026-04-24.c → stm32f7disco/Src/motor.c
- `publish_imu()` --calls--> `HAL_GetTick()`  [INFERRED]
  stm32f7disco/Src/app.c → /home/ramana/stm32f7_github_compare/stm32f7disco/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal.c
- `App_MotorEsp32Diag_Run()` --calls--> `LCD_Display_BootStatus()`  [INFERRED]
  stm32f7disco/Src/app.c → /home/ramana/stm32f7_github_compare/stm32f7disco/Src/lcd_display.c

## Communities

### Community 2 - "Community 2"
Cohesion: 0.02
Nodes (155): main(), Error_Handler(), HAL_DMA_Abort_IT(), HAL_TIMEx_Break2Callback(), HAL_TIMEx_BreakCallback(), HAL_TIMEx_CommutCallback(), HAL_TIMEx_CommutHalfCpltCallback(), HAL_TIMEx_HallSensor_DeInit() (+147 more)

### Community 5 - "Community 5"
Cohesion: 0.02
Nodes (93): MX_FMC_Init(), BSP_SDRAM_DeInit(), BSP_SDRAM_Init(), BSP_SDRAM_Initialization_sequence(), BSP_SDRAM_MspDeInit(), BSP_SDRAM_ReadData(), BSP_SDRAM_ReadData_DMA(), BSP_SDRAM_Sendcmd() (+85 more)

### Community 7 - "Community 7"
Cohesion: 0.03
Nodes (102): HAL_FMC_MspDeInit(), HAL_FMC_MspInit(), HAL_SDRAM_MspDeInit(), HAL_SDRAM_MspInit(), ft5336_Get_I2C_InitializedStatus(), ft5336_I2C_InitializeIfRequired(), ft5336_Init(), ft5336_ReadID() (+94 more)

### Community 8 - "Community 8"
Cohesion: 0.04
Nodes (105): bank0(), detect_imu(), draw(), IMU_Min_Test_Run(), init_min(), rd(), wr(), LCD_BSP_Test_Run() (+97 more)

### Community 11 - "Community 11"
Cohesion: 0.03
Nodes (79): cubemx_transport_close(), cubemx_transport_open(), cubemx_transport_write(), cubemx_transport_close(), cubemx_transport_open(), cubemx_transport_write(), HAL_UART_RxCpltCallback(), HAL_UART_ErrorCallback() (+71 more)

### Community 12 - "Community 12"
Cohesion: 0.05
Nodes (92): DMA_CalcBaseAndBitshift(), DMA_CheckFifoParam(), DMA_SetConfig(), HAL_DMA_DeInit(), HAL_DMA_GetState(), HAL_DMA_Init(), HAL_DMA_PollForTransfer(), HAL_DMA_Start() (+84 more)

### Community 14 - "Community 14"
Cohesion: 0.02
Nodes (37): NVIC_DecodePriority(), NVIC_EncodePriority(), SysTick_Config(), TZ_NVIC_SetPriority_NS(), TZ_SysTick_Config_NS(), Error_Handler(), MPU_Config(), SystemClock_Config() (+29 more)

### Community 15 - "Community 15"
Cohesion: 0.05
Nodes (78): App_MotorEsp32Diag_Run(), App_Run(), clamp01f(), cmd_cb(), motor_control(), App_Run(), boot_test_apply(), boot_test_apply_single() (+70 more)

### Community 18 - "Community 18"
Cohesion: 0.06
Nodes (13): DSI_ConfigPacketHeader(), DSI_ShortWrite(), HAL_DSI_DeInit(), HAL_DSI_EndOfRefreshCallback(), HAL_DSI_ErrorCallback(), HAL_DSI_Init(), HAL_DSI_IRQHandler(), HAL_DSI_LongWrite() (+5 more)

### Community 19 - "Community 19"
Cohesion: 0.07
Nodes (21): MX_DMA2D_Init(), LL_ConvertLineToARGB8888(), LL_FillBuffer(), DMA2D_SetConfig(), HAL_DMA2D_Abort(), HAL_DMA2D_BlendingStart(), HAL_DMA2D_BlendingStart_IT(), HAL_DMA2D_CLUTLoading_Abort() (+13 more)

### Community 20 - "Community 20"
Cohesion: 0.06
Nodes (11): BaseStatusNode, main(), generate_launch_description(), bringup.launch.py  —  Full Raspberry Pi 4B hardware launch, Node, main(), Saver, clamp() (+3 more)

### Community 21 - "Community 21"
Cohesion: 0.1
Nodes (33): FLASH_Erase_Sector(), FLASH_MassErase(), FLASH_OB_BootAddressConfig(), FLASH_OB_BOR_LevelConfig(), FLASH_OB_DisableWRP(), FLASH_OB_EnableWRP(), FLASH_OB_GetBootAddress(), FLASH_OB_GetBOR() (+25 more)

### Community 23 - "Community 23"
Cohesion: 0.05
Nodes (2): TZ_NVIC_SetPriority_NS(), TZ_SysTick_Config_NS()

### Community 24 - "Community 24"
Cohesion: 0.05
Nodes (2): TZ_NVIC_SetPriority_NS(), TZ_SysTick_Config_NS()

### Community 27 - "Community 27"
Cohesion: 0.06
Nodes (2): TZ_NVIC_SetPriority_NS(), TZ_SysTick_Config_NS()

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

### Community 48 - "Community 48"
Cohesion: 0.39
Nodes (5): build_packet(), configure_encoder_pcnt(), loop(), setup(), update_encoder_state()

### Community 51 - "Community 51"
Cohesion: 0.6
Nodes (3): DMA_MultiBufferSetConfig(), HAL_DMAEx_MultiBufferStart(), HAL_DMAEx_MultiBufferStart_IT()

### Community 55 - "Community 55"
Cohesion: 1.0
Nodes (2): clock_gettime(), UTILS_NanosecondsToTimespec()

### Community 2127 - "Community 2127"
Cohesion: 1.0
Nodes (1): Convert terrain AI speed_factor → risk score (1 - speed_factor).

## Knowledge Gaps
- **3 isolated node(s):** `bringup.launch.py  —  Full Raspberry Pi 4B hardware launch`, `Convert terrain AI speed_factor → risk score (1 - speed_factor).`, `Convert terrain AI speed_factor → risk score (1 - speed_factor).`
  These have ≤1 connection - possible missing edges or undocumented components.
- **Thin community `Community 23`** (39 nodes): `ITM_CheckChar()`, `ITM_ReceiveChar()`, `ITM_SendChar()`, `__NVIC_ClearPendingIRQ()`, `NVIC_ClearTargetState()`, `NVIC_DecodePriority()`, `__NVIC_DisableIRQ()`, `__NVIC_EnableIRQ()`, `NVIC_EncodePriority()`, `__NVIC_GetActive()`, `__NVIC_GetEnableIRQ()`, `__NVIC_GetPendingIRQ()`, `__NVIC_GetPriority()`, `__NVIC_GetPriorityGrouping()`, `NVIC_GetTargetState()`, `__NVIC_GetVector()`, `__NVIC_SetPendingIRQ()`, `__NVIC_SetPriority()`, `__NVIC_SetPriorityGrouping()`, `NVIC_SetTargetState()`, `__NVIC_SetVector()`, `__NVIC_SystemReset()`, `SCB_GetFPUType()`, `SysTick_Config()`, `TZ_NVIC_ClearPendingIRQ_NS()`, `TZ_NVIC_DisableIRQ_NS()`, `TZ_NVIC_EnableIRQ_NS()`, `TZ_NVIC_GetActive_NS()`, `TZ_NVIC_GetEnableIRQ_NS()`, `TZ_NVIC_GetPendingIRQ_NS()`, `TZ_NVIC_GetPriority_NS()`, `TZ_NVIC_GetPriorityGrouping_NS()`, `TZ_NVIC_SetPendingIRQ_NS()`, `TZ_NVIC_SetPriority_NS()`, `TZ_NVIC_SetPriorityGrouping_NS()`, `TZ_SAU_Disable()`, `TZ_SAU_Enable()`, `TZ_SysTick_Config_NS()`, `core_cm33.h`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Community 24`** (39 nodes): `ITM_CheckChar()`, `ITM_ReceiveChar()`, `ITM_SendChar()`, `__NVIC_ClearPendingIRQ()`, `NVIC_ClearTargetState()`, `NVIC_DecodePriority()`, `__NVIC_DisableIRQ()`, `__NVIC_EnableIRQ()`, `NVIC_EncodePriority()`, `__NVIC_GetActive()`, `__NVIC_GetEnableIRQ()`, `__NVIC_GetPendingIRQ()`, `__NVIC_GetPriority()`, `__NVIC_GetPriorityGrouping()`, `NVIC_GetTargetState()`, `__NVIC_GetVector()`, `__NVIC_SetPendingIRQ()`, `__NVIC_SetPriority()`, `__NVIC_SetPriorityGrouping()`, `NVIC_SetTargetState()`, `__NVIC_SetVector()`, `__NVIC_SystemReset()`, `SCB_GetFPUType()`, `SysTick_Config()`, `TZ_NVIC_ClearPendingIRQ_NS()`, `TZ_NVIC_DisableIRQ_NS()`, `TZ_NVIC_EnableIRQ_NS()`, `TZ_NVIC_GetActive_NS()`, `TZ_NVIC_GetEnableIRQ_NS()`, `TZ_NVIC_GetPendingIRQ_NS()`, `TZ_NVIC_GetPriority_NS()`, `TZ_NVIC_GetPriorityGrouping_NS()`, `TZ_NVIC_SetPendingIRQ_NS()`, `TZ_NVIC_SetPriority_NS()`, `TZ_NVIC_SetPriorityGrouping_NS()`, `TZ_SAU_Disable()`, `TZ_SAU_Enable()`, `TZ_SysTick_Config_NS()`, `core_armv8mml.h`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Community 27`** (32 nodes): `__NVIC_ClearPendingIRQ()`, `NVIC_ClearTargetState()`, `NVIC_DecodePriority()`, `__NVIC_DisableIRQ()`, `__NVIC_EnableIRQ()`, `NVIC_EncodePriority()`, `__NVIC_GetActive()`, `__NVIC_GetEnableIRQ()`, `__NVIC_GetPendingIRQ()`, `__NVIC_GetPriority()`, `NVIC_GetTargetState()`, `__NVIC_GetVector()`, `__NVIC_SetPendingIRQ()`, `__NVIC_SetPriority()`, `NVIC_SetTargetState()`, `__NVIC_SetVector()`, `__NVIC_SystemReset()`, `SCB_GetFPUType()`, `SysTick_Config()`, `TZ_NVIC_ClearPendingIRQ_NS()`, `TZ_NVIC_DisableIRQ_NS()`, `TZ_NVIC_EnableIRQ_NS()`, `TZ_NVIC_GetActive_NS()`, `TZ_NVIC_GetEnableIRQ_NS()`, `TZ_NVIC_GetPendingIRQ_NS()`, `TZ_NVIC_GetPriority_NS()`, `TZ_NVIC_SetPendingIRQ_NS()`, `TZ_NVIC_SetPriority_NS()`, `TZ_SAU_Disable()`, `TZ_SAU_Enable()`, `TZ_SysTick_Config_NS()`, `core_cm23.h`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Community 46`** (8 nodes): `mpu_armv7.h`, `ARM_MPU_ClrRegion()`, `ARM_MPU_Disable()`, `ARM_MPU_Enable()`, `ARM_MPU_Load()`, `ARM_MPU_SetRegion()`, `ARM_MPU_SetRegionEx()`, `orderedCpy()`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Community 55`** (3 nodes): `microros_time.c`, `clock_gettime()`, `UTILS_NanosecondsToTimespec()`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Community 2127`** (1 nodes): `Convert terrain AI speed_factor → risk score (1 - speed_factor).`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `HAL_GetTick()` connect `Community 12` to `Community 34`, `Community 5`, `Community 7`, `Community 8`, `Community 11`, `Community 14`, `Community 15`, `Community 18`, `Community 19`, `Community 21`?**
  _High betweenness centrality (0.030) - this node is a cross-community bridge._
- **Why does `HAL_Delay()` connect `Community 15` to `Community 5`, `Community 7`, `Community 8`, `Community 12`, `Community 14`, `Community 18`?**
  _High betweenness centrality (0.008) - this node is a cross-community bridge._
- **Why does `HAL_DMA_Start_IT()` connect `Community 12` to `Community 2`, `Community 11`, `Community 5`?**
  _High betweenness centrality (0.006) - this node is a cross-community bridge._
- **Are the 72 inferred relationships involving `HAL_GetTick()` (e.g. with `IMU_Min_Test_Run()` and `cmd_cb()`) actually correct?**
  _`HAL_GetTick()` has 72 INFERRED edges - model-reasoned connections that need verification._
- **Are the 10 inferred relationships involving `TIM_CCxChannelCmd()` (e.g. with `HAL_TIMEx_HallSensor_Start()` and `HAL_TIMEx_HallSensor_Stop()`) actually correct?**
  _`TIM_CCxChannelCmd()` has 10 INFERRED edges - model-reasoned connections that need verification._
- **Are the 28 inferred relationships involving `HAL_DMA_Start_IT()` (e.g. with `HAL_I2C_Master_Transmit_DMA()` and `HAL_I2C_Master_Receive_DMA()`) actually correct?**
  _`HAL_DMA_Start_IT()` has 28 INFERRED edges - model-reasoned connections that need verification._
- **Are the 18 inferred relationships involving `App_Run()` (e.g. with `LCD_Display_Init()` and `LCD_Display_BootStatus()`) actually correct?**
  _`App_Run()` has 18 INFERRED edges - model-reasoned connections that need verification._