/*
    Message Package
        1byte   1byte    1 byte    max 256 byte  3bit
    |preamble|header|   hand_shaking/ ack/nack
      1 byte
    |preamble|header|package_size|   raw_data   |crc |

*/
#include "esp_task_wdt.h"
#include "HardwareSerial.h"
#include "SoftwareSerial.h"

#define CRC_Generator 0xD5
#define Preamble_CODE 0xAA
#define ACK_CODE 0xEE
#define NACK_CODE 0x11
#define START_CODE 0xCC
#define STOP_CODE 0xBB
#define PACKAGE_CODE 0x77
#define REQUEST_PACKAGE_CODE 0x66
#define SEND_PACKAGE_CODE 0x55

#define XOR(x,n) 

#include "esp32-hal-uart.h"
#include "esp32-hal-uart.c"

class ARQ
{
    public:
#ifdef HardwareSH
        ARQ(uint16_t _package_length, HardwareSerial *my_Serial);
#endif
#ifdef SoftwareSH
		ARQ(uint16_t _package_length, SoftwareSerial *my_Serial);
#endif
		bool IsConnected();
		bool TryConnect(uint16_t _timeout); //try to connect slave device
		bool DisConnect(uint16_t _timeout, bool force);
        bool SendBytes(uint8_t * _buffer,uint16_t _length,uint16_t _timeout,uint8_t _try_number); //send user data
		bool IsSent();
		bool IsReceived();
		uint16_t BeginReceive(uint8_t *buffer, uint16_t _timeout,uint8_t _try_number);
        void SlaveLoop(uint8_t *user_data, uint16_t _length, uint16_t _timeout,uint8_t _try_number);
		bool RequestPackage(uint16_t _timeout,uint8_t user_data_index);
		bool RequestSendPackage(uint16_t _length, uint16_t _timeout,uint8_t user_data_index);
		uint8_t UserIndex();
        //virtual ~ARQ();
		//void (*SlaveConnectEvent)();
		
		void attachConnectInterrupt(void (*f)()) __attribute__((always_inline)) {
			isrConnectCallback=f;
		}
		
		void attachStatusInterrupt (void (*f)()) __attribute__((always_inline)) {
			isrStatusCallback=f;
		}
		void attachRequestInterrupt (void (*f)()) __attribute__((always_inline)) {
			isrRequestCallback=f;
		}
	
    protected:

    private:
#ifdef HardwareSH
		HardwareSerial  *_my_Serial;
#endif
#ifdef SoftwareSH
		SoftwareSerial  *_my_Serial;
#endif
		void (*isrConnectCallback)();
		void (*isrStatusCallback)();
		void (*isrRequestCallback)();
		
		uint8_t user_index=0;
		uint16_t _package_len=0;
		void crcInit();
		//uint8_t crcFast(uint8_t header, uint8_t *message, uint16_t nBytes);
		uint8_t  crcTable[256];
        uint8_t *_arq_package;
        uint16_t _arq_package_length=255;
		uint16_t _current_slave_package_size = 0;
		long _last_connection_time = 0;
        bool _isconnected=false;
		bool _issent = false;
		bool _isreceived=false;
        bool _wait_ACK(uint16_t _timeout);
		void _send_ACK();
		void _send_NACK();
        bool _send_package(uint8_t *_package, uint16_t _length,uint16_t _timeout,uint8_t _try_number);   //send package
        //bool _EOP(); //end of package
		bool _check_crc_error(uint8_t *_package, uint16_t length); //check crc bits
        uint8_t _generate_crc(uint8_t header,uint8_t *_package, uint16_t length);
       
		bool _read_package(uint8_t *buffer, uint16_t &_length, uint16_t _timeout,uint8_t _try_number);
		bool tx_stat=true;
		void Set_Tx(bool stat);
		void flushRxBuffer();
		bool immdStop=false;
		
};


//#include "ARQ.h"

#ifdef HardwareSH
ARQ::ARQ(uint16_t _buffer_length, HardwareSerial *my_Serial)
{
	_package_len=_buffer_length+2;
	_arq_package = new uint8_t[_buffer_length+2];
	_arq_package_length = _buffer_length;
	_my_Serial = my_Serial;
	crcInit();
}
#endif

#ifdef SoftwareSH
ARQ::ARQ(uint16_t _buffer_length, SoftwareSerial *my_Serial)
{
	_arq_package = new uint8_t[_buffer_length];
	_arq_package_length = _buffer_length;
	_my_Serial = my_Serial;
}
#endif
void ARQ::flushRxBuffer()
{
	while(_my_Serial->available())
		_my_Serial->read();
}
void ARQ::Set_Tx(bool stat)
{
	/*if(!stat&&tx_stat)
	{
		pinMatrixOutDetach(17, false, false);
		pinMode(17,OUTPUT);
		digitalWrite(17,0);
		tx_stat=false;
	}
	else
	{
		if(stat&&!tx_stat)
		{	
			pinMatrixOutAttach(17, UART_TXD_IDX(2), false, false);
			tx_stat=true;
		}
	}*/
}

bool ARQ::TryConnect(uint16_t _timeout)
{
	_isconnected = false;

	_current_slave_package_size = 0;

	_my_Serial->flush();
	_my_Serial->write(Preamble_CODE);	//preamble
	_my_Serial->write(Preamble_CODE);	//preamble
	_my_Serial->write(Preamble_CODE);	//preamble
	_my_Serial->write(Preamble_CODE);	//preamble
	_my_Serial->write(Preamble_CODE);	//preamble
	_my_Serial->write(START_CODE);		//header
	if (_wait_ACK(_timeout))
	{
		_isconnected = true;		
		//_current_slave_package_size = _my_Serial->read(); //read the total package size 
	}
	return _isconnected;
}

bool ARQ::DisConnect(uint16_t _timeout, bool force)
{
	_current_slave_package_size = 0;
	_my_Serial->flush();
	_my_Serial->write(Preamble_CODE);	//preamble
	_my_Serial->write(Preamble_CODE);	//preamble
	_my_Serial->write(Preamble_CODE);	//preamble
	_my_Serial->write(Preamble_CODE);	//preamble
	_my_Serial->write(Preamble_CODE);	//preamble
	_my_Serial->write(STOP_CODE);		//header
	
	if (force)
	{
		_isconnected = false;
		return true;
	}
	if (_wait_ACK(_timeout))
	{
		_isconnected = false;
	}
	return !_isconnected; //if true => disconnected
}

bool ARQ::IsConnected()
{
	return _isconnected;
}

bool ARQ::IsReceived()
{
	return _isreceived;
}

bool ARQ::IsSent()
{
	return _issent;
}

uint8_t ARQ::UserIndex()
{
	return user_index;
}

bool ARQ::SendBytes(uint8_t * _buffer, uint16_t _length, uint16_t _timeout,uint8_t _try_number) //send user data
{
	bool _stat=false;
	if (_isconnected)
	{
		uint16_t _total_cyle = ceil((((float)_length) / _arq_package_length));// + ((_length%_arq_package_length) > 0); //ceiling total cycle size;
		for (uint16_t i=0; i < _total_cyle - 1; i++)
		{
			_my_Serial->flush();
			
			_stat=_send_package(_buffer + _arq_package_length * i, _arq_package_length, _timeout,_try_number); //shifting array
			if(!_stat)
				return false;
			//delay(100);
			_length -= _arq_package_length; // total sent byte length
			
		}
		_stat=_send_package(_buffer + _arq_package_length*(_total_cyle-1), _length, _timeout,_try_number); //shifting array last bytes
		if(!_stat)
			return false;
		
	}
	return true;
}

bool ARQ::_send_package(uint8_t *_package, uint16_t _length, uint16_t _timeout,uint8_t _try_number)    //can be added trial number
{
	uint8_t crc_bits = _generate_crc(PACKAGE_CODE, _package, _length);
	long try_number = 0;
send_again:
	
	//delay(100);
    try_number++;
	_my_Serial->write(Preamble_CODE);	//preamble
	_my_Serial->write(PACKAGE_CODE);	//header
	_my_Serial->write((_length + 1)>>8);			//length +1 crc
	_my_Serial->write((_length + 1)&0xFF);			//length +1 crc

	_my_Serial->write(_package, _length);	//write whole the raw data

	_my_Serial->write(crc_bits);	
	//printf("sending:%d %lu len:%lu\n", crc_bits, try_number, (long)_length);
	//flushRxBuffer();
	if (!_wait_ACK(_timeout+try_number*500))
	{
	//	for(int i=0;i<_length;i++)
		//	printf("read_package:%X", _package[i]);
	//	printf("\n");
	//	delay(1000);
		if(immdStop)
		{
			immdStop=false;
			return false;
		}
	
		printf("Send again\n");
		if (try_number > _try_number)
			return false;
		//flushRxBuffer();
		goto send_again;
	}
	return true;
}

bool ARQ::_wait_ACK(uint16_t _timeout)
{
	printf("Waiting for ack\n");
	long _temp = millis();
	bool preamble = false;
	char incoming = 0;
	while ((millis() - _temp)< _timeout)
	{	esp_task_wdt_reset();
		while (!_my_Serial->available()){
			esp_task_wdt_reset();
			if ((millis() - _temp) > _timeout)
				return false;
		}
		incoming = _my_Serial->read();		
		//printf("w= 0x%X\n", (unsigned int)incoming);
		//if (!preamble) //wait preamble
		{			
			if (incoming == Preamble_CODE)											
				preamble = true;											
		}
		//else
		{
			
		if (incoming == ACK_CODE&&preamble)
		{		
			printf("Got Ack\n");		
			return true;		
		}
		else
			if(incoming==NACK_CODE&&preamble)
			{	
				printf("Got Nack\n");		
				delay(50);
				return false;
			}else 
				if(incoming==START_CODE&&preamble)
				{			
					printf("Got Connect Code\n");
					immdStop=true;
					return false;
				}
				
		}
		
	}
	return false;
}

void ARQ::_send_ACK()
{
	//printf("waiting to write %u\n",_my_Serial->availableForWrite());
	//delay(10);
	//printf("Sending Ack\n");
	_my_Serial->write(0x00);	//preamble
	_my_Serial->write(0xFF);	//preamble
	_my_Serial->write(Preamble_CODE);	//preamble
	_my_Serial->write(Preamble_CODE);	//preamble
	_my_Serial->write(Preamble_CODE);	//preamble
	_my_Serial->write(Preamble_CODE);	//preamble
	_my_Serial->write(ACK_CODE);	    //header
}

void ARQ::_send_NACK()
{
	//printf("Sending Nack\n");
	_my_Serial->write(0x00);	//preamble
	_my_Serial->write(0xFF);	//preamble
	_my_Serial->write(Preamble_CODE);	//preamble
	_my_Serial->write(Preamble_CODE);	//preamble
	_my_Serial->write(Preamble_CODE);	//preamble
	_my_Serial->write(Preamble_CODE);	//preamble
	_my_Serial->write(NACK_CODE);	    //header
}

bool ARQ::RequestPackage(uint16_t _timeout,uint8_t user_data_index)
{
	_my_Serial->flush();
	_my_Serial->write(Preamble_CODE);	//preamble
	_my_Serial->write(REQUEST_PACKAGE_CODE);	    //header
	_my_Serial->write(user_data_index);	    //user index part of the image index
	if(_wait_ACK(_timeout))
	{ 
		_current_slave_package_size = _my_Serial->read() << 8;
		_current_slave_package_size |= (_my_Serial->read()) & 0xFF;
		printf("cycles size:%lu\n", (long)_current_slave_package_size);
		return true;
	}
	
	return false;
}

bool ARQ::RequestSendPackage(uint16_t _length,uint16_t _timeout,uint8_t user_data_index)
{
	uint16_t _total_cyle = ceil((((float)_length) / _arq_package_length));
//printf("size:%lu\n", (long)_total_cyle);
	_my_Serial->write(Preamble_CODE);	//preamble
	_my_Serial->write(SEND_PACKAGE_CODE);	    //header
	_my_Serial->write(_total_cyle >> 8);	//header
	_my_Serial->write(_total_cyle & 0xFF);	//header
	_my_Serial->write(user_data_index);	//sending which part of the image
	if (_wait_ACK(_timeout))			
		return true;
	
	return false;
}

uint16_t ARQ::BeginReceive(uint8_t *user_array, uint16_t _timeout,uint8_t _try_number)
{
	if (_current_slave_package_size == 0)
		return 0;
	uint16_t length = 0, index = 0;
	_my_Serial->flush();
	_send_ACK();
	//printf("size:%d\n",_current_slave_package_size);
	long temp=millis();
	for (uint16_t i = 0; i < _current_slave_package_size; i++)
	{
		if (_read_package(_arq_package, length,_timeout,_try_number))
		{
			//printf("OK1111\n");
			//printf("User_array:%p\n",user_array);
			for (uint16_t j = 0; j < length; j++)
			{
				user_array[index++] = _arq_package[j];	//copy array			
			}
			_send_ACK();
		/*	_send_ACK();
			flushRxBuffer();
			while(!_my_Serial->available())
			{
				if((millis()-temp)>10)
				{
					printf("ack again\n");
					_my_Serial->flush();
					_send_ACK();
					flushRxBuffer();
					temp=millis();
				}					
			}*/
		}
		else
		{
			//printf("NOT\n");
			return 0;
		}
	}
	_current_slave_package_size = 0;
	return index;
}

bool ARQ::_read_package(uint8_t *buffer, uint16_t &_length, uint16_t _timeout,uint8_t _try_number)
{
	long _temp = millis();
	bool preamble = false;
	bool pkg_code = false;
	uint8_t incoming = 0;
	long try_number = 0;
	//printf("Rx time: %lu\n",(millis() - _temp));
	while ((millis() - _temp) < _timeout)
	{
	wait_again:
		esp_task_wdt_reset();
		try_number++;
		printf("Try number %lu\n", try_number);	
		while (!_my_Serial->available())
		if ((millis() - _temp) > _timeout)
		{	printf("Serial is not available");
			delay(100);
			esp_task_wdt_reset();
			return false;
		}
		while(_my_Serial->available()&&((millis() - _temp) < _timeout))
		{
			incoming = _my_Serial->read();
			printf("read_package:%X - %d\n", incoming, incoming);
			if (!preamble) //wait preamble
			{
				if (incoming == Preamble_CODE)
				{
				//	printf("preamble\n");
					preamble = true;
					_temp = millis();
				}
			}
			else
			{
				if (!pkg_code)
				{
					if (incoming == PACKAGE_CODE)
					{
						//printf("package code\n");
						pkg_code = true;
						_temp = millis();
					}
					
					if (incoming == Preamble_CODE)
					{
					//	printf("preamble\n");
						preamble = true;
						_temp = millis();
					}
				}
				else
				{
					_temp = millis();
					_length = incoming<<8;
					_length |= (_my_Serial->read())&0xFF;
					//long _temp_new=micros();
					if(_length>(_package_len+2))
					{
						printf("NACK len %lu\n", (unsigned long)_length);		
						_my_Serial->flush();
						_send_NACK();
						flushRxBuffer();
						pkg_code=false;
						preamble=false;
						if (try_number > _try_number)
							return false;
						_temp = millis();
						goto wait_again;					
					}
					while (!_my_Serial->available())
						esp_task_wdt_reset();
					//while (_my_Serial->available())	
						_my_Serial->readBytes(buffer,_length);
				//	printf("Last:%d, len:%d\n", buffer[_length-1], _length);
					//printf("crc:%d\n",_generate_crc(PACKAGE_CODE, buffer, _length));
					if (!_check_crc_error(buffer, _length))
					{					
						printf("NACK %lu\n", try_number);		
						//for(int i=0;i<_length;i++)
						//	printf("read_package:%X", buffer[i]);
						//printf("\n");
						//delay(1000);
						_my_Serial->flush();
						_send_NACK();
						if (try_number > _try_number)
							return false;					
						pkg_code=false;
						preamble=false;
						_temp = millis();
						goto wait_again;
					}
					else
					{
						printf("ACK\n");
						//_send_ACK();
						//delay(10);
						//_send_ACK();
						//_my_Serial->flush();
						_length--;
						return true;
					}
				}
			}
		}
	}	
	return false;
}

bool ARQ::_check_crc_error(uint8_t *_package, uint16_t length) //check crc bits
{
	//printf("Fast crc:%d\n", crcFast(PACKAGE_CODE, _package, length));
	//printf("Crc chek:%d\n", _generate_crc(PACKAGE_CODE, _package, length));
	return _generate_crc(PACKAGE_CODE, _package, length) == 0;
}

/*uint8_t ARQ::_generate_crc(uint8_t header, uint8_t *_package, uint16_t length)
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

	for (uint16_t byte = 0; byte < length; ++byte)
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
}*/

void ARQ::SlaveLoop(uint8_t *user_data, uint16_t _length, uint16_t _timeout,uint8_t _try_number)
{
	char incoming = 0;
	_issent = false;
	_isreceived=false;
	if (_my_Serial->available())
	{		
		incoming = _my_Serial->read();
		if (incoming == Preamble_CODE)
		{			
			incoming = _my_Serial->read();//Command
			if (!_isconnected)
			{				
				if (incoming == START_CODE)
				{				
			
					_last_connection_time = millis(); //Valid info then reset time
					_send_ACK();				
					
					printf("Connected\n");
					_isconnected = true;
					this->isrConnectCallback();
				}
			}
			else
			{
				//printf("Waiting Ack\n");
				uint16_t _total_cyle = ceil((((float)_length) / _arq_package_length));
				uint16_t total_len=0;
				switch (incoming)
				{
					case START_CODE:
						_issent = false;
						_isreceived=false;
						
						_last_connection_time = millis(); //Valid info then reset time
						_send_ACK();				
						
						printf("Reconnected\n");
						_isconnected = true;
						this->isrConnectCallback();
					break;
					case REQUEST_PACKAGE_CODE:
						
						user_index=_my_Serial->read();	//user index
						_last_connection_time = millis();
						
						this->isrRequestCallback();
						_send_ACK();
						
						_my_Serial->write(_total_cyle >> 8);	//header
						_my_Serial->write(_total_cyle & 0xFF);	//header
						printf("cycles size:%lu\n", (long)_total_cyle);						
						
						_wait_ACK(_timeout);						
						printf("Tranmitting package\n");
						_issent=SendBytes(user_data+7680*user_index, _length, _timeout,_try_number);
						_last_connection_time = millis();
						//_issent = true;
						printf("Tranmitted\n");
						this->isrStatusCallback();
						break;
					case SEND_PACKAGE_CODE:
						_current_slave_package_size = _my_Serial->read() << 8;
						_current_slave_package_size |= (_my_Serial->read()) & 0xFF;			
						user_index=_my_Serial->read();	//user index
						_last_connection_time = millis();
						this->isrRequestCallback();	//to ready the slave 
						
						printf("size:%lu\n",(long)_current_slave_package_size);
						printf("Receiving package\n");
						//user_data=user_data+7680*user_index;
						total_len=BeginReceive(user_data+7680*user_index, _timeout,_try_number);
						_last_connection_time = millis();
						_isreceived=true;
						printf("%lu byte recieved\n", (long)total_len);			
						this->isrStatusCallback();						
						break;
					case STOP_CODE:
						_last_connection_time = millis(); //Valid info then reset time
						printf("Disconnected\n");
						_send_ACK();
						_isconnected = false;
						this->isrConnectCallback();
						break;
					default:
						break;
				}

			}
		}
	}
	if (_isconnected &&( (millis() - _last_connection_time) > _timeout))
	{
		printf("Connection Timeout\n");
		_isconnected = false;
		this->isrConnectCallback();
	}
}

void ARQ::crcInit()
{
	uint8_t  remainder;


	/*
	 * Compute the remainder of each possible dividend.
	 */
	for (int dividend = 0; dividend < 256; ++dividend)
	{
		/*
		 * Start with the dividend followed by zeros.
		 */
		remainder = dividend ;

		/*
		 * Perform modulo-2 division, a bit at a time.
		 */
		for (uint8_t bit = 8; bit > 0; --bit)
		{
			/*
			 * Try to divide the current data bit.
			 */
			if (remainder & 0x80)
			{
				remainder = (remainder << 1) ^ CRC_Generator;
			}
			else
			{
				remainder = (remainder << 1);
			}
		}

		/*
		 * Store the result into the table.
		 */
		crcTable[dividend] = remainder;
	}

} 

uint8_t ARQ::_generate_crc(uint8_t header,uint8_t *message, uint16_t nBytes)
{
	uint8_t data;
	uint8_t remainder = 0;
	data = header ^ remainder;
	for (uint16_t byte = 0; byte < nBytes; ++byte)
	{
		data = message[byte] ^ remainder;
		remainder = crcTable[data] ^ (remainder << 8);
	}

	return (remainder);
}