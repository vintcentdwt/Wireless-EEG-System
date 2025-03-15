#ifndef _ADS1299_H_
#define _ADS1299_H_

#include "stm32f10x.h" 
#include "stm32f10x_spi.h"
#include "stm32f10x_gpio.h"
#include "Delay.h"  

/* Interface macro definitions */
#define SPI_CS_HIGH     GPIO_SetBits(GPIOA, SPI_CS)
#define SPI_CS_LOW      GPIO_ResetBits(GPIOA, SPI_CS)

#define ADS_DRDY_G		GPIOA     // Data ready indicator (active low)
#define ADS_SCLK_G		GPIOA	  // SPI communication clock line
#define ADS_DIN_G		GPIOA	  // ADS1299 data input pin
#define ADS_DOUT_G		GPIOA	  // ADS1299 data output pin

// Define GPIO pins and SPI configuration
#define ADS_DRDY	    GPIO_Pin_1
#define ADS_SCLK	    GPIO_Pin_5
#define ADS_DIN		    GPIO_Pin_7
#define ADS_DOUT	    GPIO_Pin_6
#define SPI_CS          GPIO_Pin_4  

// ADS1299 command definitions
#define WAKEUP          0x02    // Wake up ADS1299 from standby mode
#define STANDBY         0x04    // Enter standby mode to reduce power consumption
#define ADS_RESET       0x06    // Reset ADS1299 to default state
#define START           0x08    // Start data conversion
#define STOP            0x0A    // Stop data conversion
#define RDATAC          0x10    // Enable continuous data read mode
#define SDATAC          0x11    // Stop continuous data read mode
#define RDATA           0x12    // Read data once via command
#define RREG            0x20    // Read register command base (to be combined with register address)
#define WREG            0x40    // Write register command base (to be combined with register address)

// ADS1299 register address definitions
#define ID              0x00    // ID register, read device identification
#define CONFIG1         0x01    // Configuration register 1, set sample rate, etc.
#define CONFIG2         0x02    // Configuration register 2, control test signals, etc.
#define CONFIG3         0x03    // Configuration register 3, set reference voltage and BIAS
#define LOFF            0x04    // Lead-off detection control register
#define CH1SET          0x05    // Channel 1 settings register
#define CH2SET          0x06    // Channel 2 settings register
#define CH3SET          0x07    // Channel 3 settings register
#define CH4SET          0x08    // Channel 4 settings register
#define BIAS_SENSP      0x0D    // BIAS positive electrode selection register
#define BIAS_SENSN      0x0E    // BIAS negative electrode selection register
#define LOFF_SENSP      0x0F    // Lead-off positive electrode selection register
#define LOFF_SENSN      0x10    // Lead-off negative electrode selection register
#define LOFF_FLIP       0x11    // Lead-off flip control register
#define LOFF_STATP      0x12    // Lead-off positive electrode status register
#define LOFF_STATN      0x13    // Lead-off negative electrode status register
#define GPIO            0x14    // GPIO control register
#define MISC1           0x15    // Miscellaneous control register 1
#define MISC2           0x16    // Miscellaneous control register 2
#define CONFIG4         0x17    // Configuration register 4, set bandwidth mode, etc.

// Data packet structure (4 channels, each channel data is 24-bit sign-extended to 32-bit; additional 4-byte timestamp and 2-byte checksum)
#pragma pack(push, 1) // 1-byte alignment
typedef struct {
    uint32_t timestamp;   // Timestamp (4 bytes)
    int32_t ch1_data;     // Channel 1 data (24-bit sign-extended to 32-bit)
    int32_t ch2_data;     // Channel 2 data
    int32_t ch3_data;     // Channel 3 data
    int32_t ch4_data;     // Channel 4 data
    uint16_t checksum;    // Checksum (simple sum, excluding the checksum field itself)
} ADS_DataPacket;
#pragma pack(pop)
	
// Function prototypes
void ADS_Init(void);
unsigned char ADS_SPI(unsigned char com);
unsigned char ADS_REG(unsigned char com, unsigned data);
void ADS_PowerOnInit(void);
void ADS_Read(ADS_DataPacket *packet);

#endif // _ADS1299_H_
