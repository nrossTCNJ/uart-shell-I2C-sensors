# UART Shell with I2C Sensor Communication
This project adds onto my previous project, the UART Command Shell, by exploring the I2C communication protocol. New commands were added for the user to read data from a MPU-6500 sensor via I2C. Also, bare-metal programming was utilized to emphasize register configuration, reading and writing protocols, ACKs, and flag checking when communicating. Similar to the previous project, this one uses the UART communication protocol to send commands from my laptop to my STM32-NUCLEO F411RE. This project builds on the embedded systems foundations established in the previous implementation.

# What was done
1. I2C Initialization\
   To start, I2C1 peripheral registers were configured so the data bus was ready to use. The clock was enabled, the pins were set to open-drain, pull-up pull-down was cleared due to on-board pull-up resistors, alternate function mode was used, the peripheral clock was set to 42 MHz, the SCL clock was set to 100 kHz, rise time set to 43 ns, the peripheral was enabled, and ACK was enabled.
2. I2C Read\
   An I2C read function was created that both reads the MPU-6500's device address and sends it back via UART. This command, "whoami", reads the WHO_AM_I register in the MPU sensor to confirm the GPIO, pheripheral configuration, and bus protocol all works. The i2c_read() function works by establishing two START conditions, switching between write and read mode, ACK when necessary, and NACKing for the last byte to create a STOP condition.
3. I2C Write\
   The MPU-6500 was waken from sleep so that the accelerometer, temperature, and gyroscope metrics could be configured. To do this, an i2c_write() function was created that was similar to the read function, except that it only has one START condition and never switches to read mode. 
4. Burst Read\
   An I2C burst read function was created that reads all 14 bytes of sensor data in one transaction and converts the raw data into physical units. This meant reading all 14 bytes in one transaction instead of reading one byte for 14 transactions. Next, the raw MPU data was converted to physical units using formulas provided on the MPU-6500 datasheet. 
5. Stream Read\
   A "stream" command was created to get frequent, real-time readings of MPU sensor data, with an additional "stop" command ended to end this stream. 

# What was learned overall
This project reinforced and introduced topics related to embedded systems including the I2C protocol, register and clock configuration, open-drain and alternate function modes, master and slave devices, bare-metal programming, and blocking vs. non-blocking mode. The next part to be implented in the next project will be timers for interrupts, which will make the stream read of sensor data much smoother. Additionally, this project still has great potential for future additions such as LED PWM manipulation or communication with an LCD.

