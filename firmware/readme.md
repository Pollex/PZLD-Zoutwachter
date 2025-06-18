# MFM Saltwatcher firmware

## TODOs

- [x] Orient on existing code
- [x] Refactor part of existing code
- [ ] Update LED driver to use TIM2_CH1 w/ DMA
- [ ] Write to RS485
- [ ] Introduce watchdog
- [ ] Setup RIOT I2C Slave using RIOT-OS patch
- [ ] Validate incoming I2C commands from master
- [ ] Determine how MFM Module slot is set? 
    - Hardcoded since this entire board is an expansion
- [ ] Determine measurement packet size and format

### Testing
- [x] Setup FAT build that can run test commands
- [ ] Setup test routine for MFM Core communication
- [ ] Test UART via RS485
