// SQUiXL GT911 touch controller
// https://squicl.io
//
// 2025 Unexpected Maker
//
// Inspired by TAM_GT911, thanks for your library.

#include "UM_GT911.h"
#include "UM_LCA9555.h" // ioex support

UM_GT911::UM_GT911(uint8_t _addr)
{
	addr = _addr;
}

void UM_GT911::begin(uint8_t _int, uint8_t _rst, uint16_t _width, uint16_t _height, bool wake, TwoWire *_wire)
{
	pinInt = _int;
	pinRst = _rst;
	width = _width;
	height = _height;
	wire = _wire;

	reset();
}

// Bit Banged Reset via IO Expander
void UM_GT911::reset()
{
	pinMode(pinInt, OUTPUT);
	ioex.pin_mode(pinRst, OUTPUT); // pinMode(pinRst, OUTPUT);
	digitalWrite(pinInt, 0);
	ioex.write(pinRst, LOW); // digitalWrite(pinRst, 0);
	delay(10);
	digitalWrite(pinInt, LOW);
	delay(5);
	ioex.write(pinRst, HIGH); // digitalWrite(pinRst, 1);
	delay(100);				  // wait 100ms before setting INT to INPUT
	pinMode(pinInt, INPUT);
	delay(100);
	read_block(config_buffer, GT911_CONFIG_START, GT911_CONFIG_SIZE);

	// // reapply the resoltion settings
	refresh_resolution();
}
void UM_GT911::calculate_checksum()
{
	uint8_t checksum;
	for (uint8_t i = 0; i < GT911_CONFIG_SIZE; i++)
	{
		checksum += config_buffer[i];
	}
	checksum = (~checksum) + 1;
	config_buffer[GT911_CONFIG_CHKSUM - GT911_CONFIG_START] = checksum;
}

void UM_GT911::reflash_config()
{
	calculate_checksum();
	write_bytes(GT911_CONFIG_CHKSUM, config_buffer[GT911_CONFIG_CHKSUM - GT911_CONFIG_START]);
	write_bytes(GT911_CONFIG_FRESH, 1);
}
void UM_GT911::set_rotation(uint8_t rot)
{
	rotation = rot;
}
void UM_GT911::refresh_resolution()
{
	config_buffer[GT911_X_OUTPUT_MAX_LOW - GT911_CONFIG_START] = lowByte(width);
	config_buffer[GT911_X_OUTPUT_MAX_HIGH - GT911_CONFIG_START] = highByte(width);
	config_buffer[GT911_Y_OUTPUT_MAX_LOW - GT911_CONFIG_START] = lowByte(height);
	config_buffer[GT911_Y_OUTPUT_MAX_HIGH - GT911_CONFIG_START] = highByte(height);
	reflash_config();
}

void UM_GT911::read(void)
{
	uint8_t data[7];
	uint8_t id;
	uint16_t x, y, size;

	uint8_t touch_point_info = read_bytes(GT911_POINT_INFO);
	uint8_t bufferStatus = touch_point_info >> 7 & 1;
	uint8_t proximityValid = touch_point_info >> 5 & 1;
	uint8_t haveKey = touch_point_info >> 4 & 1;
	is_detect_large = touch_point_info >> 6 & 1;
	touches = touch_point_info & 0xF;
	is_touched = touches > 0;
	if (bufferStatus == 1 && is_touched)
	{
		for (uint8_t i = 0; i < touches; i++)
		{
			read_block(data, GT911_POINT_1 + i * 8, 7);
			points[i] = read_touch_point(data);
		}
	}
	write_bytes(GT911_POINT_INFO, 0);
}
TouchPoint UM_GT911::read_touch_point(uint8_t *data)
{
	uint16_t temp;
	uint8_t id = data[0];
	uint16_t x = data[1] + (data[2] << 8);
	uint16_t y = data[3] + (data[4] << 8);
	uint16_t size = data[5] + (data[6] << 8);
	switch (rotation)
	{
	case ROTATION_NORMAL:
		x = width - x;
		y = height - y;
		break;
	case ROTATION_LEFT:
		temp = x;
		x = width - y;
		y = temp;
		break;
	case ROTATION_INVERTED:
		x = x;
		y = y;
		break;
	case ROTATION_RIGHT:
		temp = x;
		x = y;
		y = height - temp;
		break;
	default:
		break;
	}
	return TouchPoint(id, x, y, size);
}
void UM_GT911::write_bytes(uint16_t reg, uint8_t val)
{
	Wire.beginTransmission(addr);
	Wire.write(highByte(reg));
	Wire.write(lowByte(reg));
	Wire.write(val);
	Wire.endTransmission();
}
uint8_t UM_GT911::read_bytes(uint16_t reg)
{
	uint8_t x;
	Wire.beginTransmission(addr);
	Wire.write(highByte(reg));
	Wire.write(lowByte(reg));
	Wire.endTransmission();
	Wire.requestFrom(addr, (uint8_t)1);
	x = Wire.read();
	return x;
}
void UM_GT911::write_block(uint16_t reg, uint8_t *val, uint8_t size)
{
	Wire.beginTransmission(addr);
	Wire.write(highByte(reg));
	Wire.write(lowByte(reg));
	for (uint8_t i = 0; i < size; i++)
		Wire.write(val[i]);
	Wire.endTransmission();
}
void UM_GT911::read_block(uint8_t *buf, uint16_t reg, uint8_t size)
{
	Wire.beginTransmission(addr);
	Wire.write(highByte(reg));
	Wire.write(lowByte(reg));
	Wire.endTransmission();
	Wire.requestFrom(addr, size);
	for (uint8_t i = 0; i < size; i++)
		buf[i] = Wire.read();
}

UM_GT911 touch;
