#include "ads1299.h"
#include "stm32f10x.h" 
#include "SysTick.h"  // Used to get system timestamp

// Initialize ADS1299 (configure SPI and GPIO)
void ADS_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure_ADS;
    SPI_InitTypeDef SPI_InitStructure_ADS;

    // Enable GPIOA and SPI1 clock
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

    // Configure DRDY pin (PA1) as floating input
    GPIO_InitStructure_ADS.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure_ADS.GPIO_Pin = ADS_DRDY;
    GPIO_Init(ADS_DRDY_G, &GPIO_InitStructure_ADS);

    // Configure SCLK pin (PA5) as alternate function push-pull output
    GPIO_InitStructure_ADS.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure_ADS.GPIO_Pin = ADS_SCLK;
    GPIO_Init(ADS_SCLK_G, &GPIO_InitStructure_ADS);

    // Configure DIN pin (PA7) as alternate function push-pull output
    GPIO_InitStructure_ADS.GPIO_Pin = ADS_DIN;
    GPIO_Init(ADS_DIN_G, &GPIO_InitStructure_ADS);

    // Configure DOUT pin (PA6) as floating input
    GPIO_InitStructure_ADS.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure_ADS.GPIO_Pin = ADS_DOUT;
    GPIO_Init(ADS_DOUT_G, &GPIO_InitStructure_ADS);

    // Configure CS pin (PA4) as push-pull output
    GPIO_InitStructure_ADS.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure_ADS.GPIO_Pin = SPI_CS;
    GPIO_Init(GPIOA, &GPIO_InitStructure_ADS);

    // Initialize SPI1
    SPI_Cmd(SPI1, DISABLE);
    SPI_InitStructure_ADS.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure_ADS.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure_ADS.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure_ADS.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure_ADS.SPI_CPHA = SPI_CPHA_1Edge; 
    SPI_InitStructure_ADS.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure_ADS.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
    SPI_InitStructure_ADS.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure_ADS.SPI_CRCPolynomial = 7;
    SPI_Init(SPI1, &SPI_InitStructure_ADS);
    SPI_Cmd(SPI1, ENABLE);
}

// Communicate with ADS1299 via SPI (send and receive one byte)
unsigned char ADS_SPI(unsigned char com) {
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
    SPI_I2S_SendData(SPI1, com);
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
    return SPI_I2S_ReceiveData(SPI1);
}

// Read/write ADS1299 internal registers
unsigned char ADS_REG(unsigned char com, unsigned data) {
    unsigned char data_return;
    SPI_CS_LOW;
    Delay_us(10);
    ADS_SPI(com);
    Delay_us(10);
    ADS_SPI(0X00);
    Delay_us(10);
    if ((com & 0x20) == 0x20) {  // Read register
        data_return = ADS_SPI(0X00);
    } else if ((com & 0x40) == 0x40) {  // Write register
        data_return = ADS_SPI(data);
    } else {
        data_return = 0;
    }
    SPI_CS_HIGH;
    return data_return;
}

// Initialize ADS1299 after power-on
void ADS_PowerOnInit(void) {
    Delay_ms(1000); // Wait for power to stabilize
    SPI_CS_LOW;
    ADS_SPI(ADS_RESET);
    Delay_us(10); // At least 18 tCLK, approximately 8.8us
    SPI_CS_HIGH;
    Delay_ms(10);
    SPI_CS_LOW;
    
    ADS_SPI(SDATAC); // Stop continuous read mode for configuration
    SPI_CS_HIGH;
    Delay_ms(10);
    
    ADS_REG(WREG | CONFIG3, 0XEC); // Configure internal reference and BIAS buffer
    Delay_ms(10);
    ADS_REG(WREG | CONFIG1, 0x94); // 0x94 for 500Hz sample rate, 0x95 for 250Hz
    ADS_REG(WREG | CH1SET, 0X00);  // Configure channel gain
    ADS_REG(WREG | CH2SET, 0X00);
    ADS_REG(WREG | CH3SET, 0X00);
    ADS_REG(WREG | CH4SET, 0X00);
    ADS_REG(WREG | CONFIG4, 0x02); // Wideband mode, ensure 100 Hz
    
    SPI_CS_LOW;
    ADS_SPI(START); // Send START command to start continuous conversion
}

// Read ADS1299 data
void ADS_Read(ADS_DataPacket *packet) {
    uint8_t raw_data[27] = {0};
    // Wait for DRDY signal to ensure data is ready
    while (GPIO_ReadInputDataBit(ADS_DRDY_G, ADS_DRDY) != 0);
    SPI_CS_LOW;
    for (int i = 0; i < 27; i++) {
        raw_data[i] = ADS_SPI(0X00);
    }
    SPI_CS_HIGH;

    // Parse channel data
    int32_t value = (raw_data[3] << 16) | (raw_data[4] << 8) | raw_data[5];
    if (value & 0x800000) value |= 0xFF000000;
    packet->ch1_data = value;

    value = (raw_data[6] << 16) | (raw_data[7] << 8) | raw_data[8];
    if (value & 0x800000) value |= 0xFF000000;
    packet->ch2_data = value;

    value = (raw_data[9] << 16) | (raw_data[10] << 8) | raw_data[11];
    if (value & 0x800000) value |= 0xFF000000;
    packet->ch3_data = value;

    value = (raw_data[12] << 16) | (raw_data[13] << 8) | raw_data[14];
    if (value & 0x800000) value |= 0xFF000000;
    packet->ch4_data = value;

    packet->timestamp = Get_SystemTick();
    packet->checksum = 0;
    uint8_t *bytes = (uint8_t*)packet;
    for (int i = 0; i < sizeof(ADS_DataPacket) - sizeof(packet->checksum); i++) {
        packet->checksum += bytes[i];
    }
}
