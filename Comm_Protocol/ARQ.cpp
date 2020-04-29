#include "ARQ.h"

ARQ::ARQ(uint8_t _buffer_length=255, HardwareSerial *my_Serial)
{
	_arq_package = new uint8_t[_buffer_length];
	_my_Serial = my_Serial;
}

/*ARQ::~ARQ()
{
	if (_arq_package != NULL)
		delete _arq_package;
}
*/
bool ARQ::TryConnect(uint16_t _timeout)
{
	_isconnected = false;
	
	_current_slave_package_size = 0;

	_my_Serial->write(Preamble_CODE);	//preamble
	_my_Serial->write(START_CODE);		//header

	if (_wait_ACK(_timeout))
		_isconnected = true;

	_current_slave_package_size = _my_Serial->read(); //read the total package size 
	return _isconnected;
}

void ARQ::SendBytes(uint8_t * _buffer,uint16_t _length,uint16_t _timeout) //send user data
{
    if(_isconnected)
    {
        uint8_t _total_cyle=_length/_arq_package_length+((_length%_arq_package_length)>0); //ceiling total cycle size;
        for(uint8_t i;i<_total_cyle-1;i++)
        {
            _send_package(_buffer+_arq_package_length*i, _arq_package_length,_timeout); //shifting array
        }
		_send_package(_buffer + _arq_package_length * i, _length%_arq_package_length, _timeout); //shifting array
    }
}

void ARQ::_send_package(uint8_t *_package,uint8_t _length uint16_t _timeout)    //can be added trial number
{
    uint8_t header=0;
    uint8_t crc_bits=_generate_crc(header,_package); 
    send_again:
	_my_Serial->write(Preamble_CODE);	//preamble
	_my_Serial->write(PACKAGE_CODE);	//header
	_my_Serial->write(_length+1);			//length +1 crc
    for(int i=0;i< _length;i++)
    {
		_my_Serial->write(_package[i]);
    }
	_my_Serial->write(crc_bits);

    if(!_wait_ACK(_timeout))
        goto send_again;
}

bool ARQ::_wait_ACK(uint16_t _timeout)
{
	long _temp = millis();

	while (!(_my_Serial->available()))	//wait until stream available
		if ((millis() - _temp) > _timeout) //check timeout
			return false;

	while(_my_Serial->read()== Preamble_CODE) //wait preamble
		if ((millis() - _temp) > _timeout) //check timeout
			return false;

	if (_my_Serial->read() == ACK_CODE)
		return true;

	return false;
}

void ARQ::_send_ACK()
{
	_my_Serial->write(Preamble_CODE);	//preamble
	_my_Serial->write(ACK_CODE);	    //header
}

void ARQ::_send_NACK()
{
	_my_Serial->write(Preamble_CODE);	//preamble
	_my_Serial->write(NACK_CODE);	    //header
}

bool ARQ::_begin_Receive(uint8_t *user_array,uint16_t _timeout)
{
	long _temp = millis();

	for (int i=0;i< _current_slave_package_size;i++)
	{	
		if (_read_package(_arq_package, _timeout))
		{
			for (int j = 0; j < _arq_package[0]; j++)
			{
				user_array[i*_arq_package[0]] = _arq_package[j];	//copy array
			}
		}
		else
			return false;
	}
	_current_slave_package_size = 0;
	return true;
}

bool ARQ::_read_package(uint8_t *buffer,uint16_t _timeout)
{
	wait_again:
	while (!(_my_Serial->available()))	//wait until stream available
		if ((millis() - _temp) > _timeout) //check timeout
			return false;

	while (_my_Serial->read() == Preamble_CODE) //wait preamble
		if ((millis() - _temp) > _timeout) //check timeout
			return false;

	while (_my_Serial->read() == PACKAGE_CODE) //wait package code
		if ((millis() - _temp) > _timeout) //check timeout
			return false;
	buffer[0] = _my_Serial->read(); //read package size

	for (int i = 0; i < buffer[0]; i++)
	{
		buffer[0] = _my_Serial->read();
	}
	if (!_check_crc_error(buffer))
	{
		_send_NACK();
		goto wait_again;
	}
	else
	{
		_send_ACK();
		return true;
	}
}

bool ARQ::_check_crc_error(uint8_t *_package, uint8_t length) //check crc bits
{
	uint8_t _crc = 0;
	for (uint8_t byte = 0; byte < length; ++byte)
	{
		_crc ^= _package[byte];
		for (uint8_t bit = 8; bit > 0; --bit)
		{
			if (_crc & 0x80)
			{
				_crc = (_crc << 1) ^ CRC_Generator;
			}
			else
			{
				_crc = (_crc << 1);
			}
		}
	}
	return _crc==0;
}
uint8_t ARQ::_generate_crc(uint8_t header, uint8_t *_package, uint8_t length)
{
	uint8_t _crc = 0;

	_crc ^= header;
	for (uint8_t bit = 8; bit > 0; --bit)
	{
		if (_crc & 0x80)
		{
			_crc = (_crc << 1) ^ CRC_Generator;
		}
		else
		{
			_crc = (_crc << 1);
		}
	}

	for (uint8_t byte = 0; byte < length; ++byte)
	{
		_crc ^= _package[byte];
		for (uint8_t bit = 8; bit > 0; --bit)
		{
			if (_crc & 0x80)
			{
				_crc = (_crc << 1) ^ CRC_Generator;
			}
			else
			{
				_crc = (_crc << 1);
			}
		}
	}
	return _crc;
}
