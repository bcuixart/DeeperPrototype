#include "ByteStream.hh"

/* WRITER */

void ByteWriter::U8(uint8_t v)
{ 
	_data.push_back(v); 
}

void ByteWriter::U16(uint16_t v) 
{
	U8(v & 0xFF); U8((v >> 8) & 0xFF); 
}

void ByteWriter::U32(uint32_t v)
{ 
	U16(v & 0xFFFF); U16((v >> 16) & 0xFFFF); 
}

void ByteWriter::F32(float v)    
{ 
	uint32_t bits;
	std::memcpy(&bits, &v, sizeof(bits)); 

	U32(bits);
}


/* READER */

uint8_t ByteReader::U8()  
{ 
	if (_pos >= _size) 
	{ 
		_ok = false; 
		return 0; 
	} 
	return _data[_pos++]; 
}

uint16_t ByteReader::U16() 
{ 
	uint16_t lo = U8();
	uint16_t hi = U8();

	return (uint16_t)(lo | (hi << 8)); 
}

uint32_t ByteReader::U32() 
{ 
	uint32_t lo = U16();
	uint32_t hi = U16();
	
	return lo | (hi << 16); 
}

float ByteReader::F32() 
{ 
	uint32_t bits = U32();

	float v;
	std::memcpy(&v, &bits, sizeof(v)); 

	return v; 
}