
#include "cstream.h"
#include <algorithm>

namespace VSTGUI {

//-----------------------------------------------------------------------------
CMemoryStream::CMemoryStream (int initialSize, int inDelta, ByteOrder byteOrder)
: OutputStream (byteOrder)
, InputStream (byteOrder)
, buffer (0)
, size (0)
, pos (0)
, delta (inDelta)
, ownsBuffer (true)
{
	resize (initialSize);
}

//-----------------------------------------------------------------------------
CMemoryStream::CMemoryStream (const char* inBuffer, int bufferSize, ByteOrder byteOrder)
: OutputStream (byteOrder)
, InputStream (byteOrder)
, buffer (const_cast<char*> (inBuffer))
, size (bufferSize)
, pos (0)
, delta (0)
, ownsBuffer (false)
{
}

//-----------------------------------------------------------------------------
CMemoryStream::~CMemoryStream ()
{
	if (ownsBuffer && buffer)
		free (buffer);
}

//-----------------------------------------------------------------------------
bool CMemoryStream::resize (int inSize)
{
	if (size >= inSize)
		return true;

	if (ownsBuffer == false)
		return false;

	int newSize = size + delta;
	while (newSize < inSize)
		newSize += delta;

	char* newBuffer = (char*)malloc (newSize);
	if (newBuffer && buffer)
		memcpy (newBuffer, buffer, size);
	if (buffer)
		free (buffer);
	buffer = newBuffer;
	size = newSize;
	
	return buffer != 0;
}

//-----------------------------------------------------------------------------
int CMemoryStream::writeRaw (const void* inBuffer, int size)
{
	if (!resize (pos + size))
		return -1;
	
	memcpy (buffer + pos, inBuffer, size);
	pos += size;
	
	return size;
}

//-----------------------------------------------------------------------------
int CMemoryStream::readRaw (void* outBuffer, int outSize)
{
	if ((size - pos) <= 0)
		return 0;

	outSize = std::min (outSize, size - pos);
	memcpy (outBuffer, buffer + pos, outSize);
	pos += outSize;

	return outSize;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool OutputStream::operator<< (const int& input)
{
	if (byteOrder == kNativeByteOrder)
	{
		return writeRaw (&input, sizeof (int)) == sizeof (int);
	}
	else
	{
		const unsigned char* p = (const unsigned char*)&input;
		if (writeRaw (&p[3], sizeof (char)) != sizeof (char)) return false;
		if (writeRaw (&p[2], sizeof (char)) != sizeof (char)) return false;
		if (writeRaw (&p[1], sizeof (char)) != sizeof (char)) return false;
		if (writeRaw (&p[0], sizeof (char)) != sizeof (char)) return false;
		return true;
	}
}

//-----------------------------------------------------------------------------
bool OutputStream::operator<< (const unsigned int& input)
{
	if (byteOrder == kNativeByteOrder)
	{
		return writeRaw (&input, sizeof (unsigned int)) == sizeof (unsigned int);
	}
	else
	{
		const unsigned char* p = (const unsigned char*)&input;
		if (writeRaw (&p[3], sizeof (char)) != sizeof (char)) return false;
		if (writeRaw (&p[2], sizeof (char)) != sizeof (char)) return false;
		if (writeRaw (&p[1], sizeof (char)) != sizeof (char)) return false;
		if (writeRaw (&p[0], sizeof (char)) != sizeof (char)) return false;
		return true;
	}
}

//-----------------------------------------------------------------------------
bool OutputStream::operator<< (const short& input)
{
	if (byteOrder == kNativeByteOrder)
	{
		return writeRaw (&input, sizeof (short)) == sizeof (short);
	}
	else
	{
		const unsigned char* p = (const unsigned char*)&input;
		if (writeRaw (&p[1], sizeof (char)) != sizeof (char)) return false;
		if (writeRaw (&p[0], sizeof (char)) != sizeof (char)) return false;
		return true;
	}
}

//-----------------------------------------------------------------------------
bool OutputStream::operator<< (const unsigned short& input)
{
	if (byteOrder == kNativeByteOrder)
	{
		return writeRaw (&input, sizeof (unsigned short)) == sizeof (unsigned short);
	}
	else
	{
		const unsigned char* p = (const unsigned char*)&input;
		if (writeRaw (&p[1], sizeof (char)) != sizeof (char)) return false;
		if (writeRaw (&p[0], sizeof (char)) != sizeof (char)) return false;
		return true;
	}
}

//-----------------------------------------------------------------------------
bool OutputStream::operator<< (const char& input)
{
	return writeRaw (&input, sizeof (char)) == sizeof (char);
}

//-----------------------------------------------------------------------------
bool OutputStream::operator<< (const unsigned char& input)
{
	return writeRaw (&input, sizeof (unsigned char)) == sizeof (unsigned char);
}

//-----------------------------------------------------------------------------
bool OutputStream::operator<< (const double& input)
{
	if (byteOrder == kNativeByteOrder)
	{
		return writeRaw (&input, sizeof (double)) == sizeof (double);
	}
	else
	{
		const unsigned char* p = (const unsigned char*)&input;
		if (writeRaw (&p[7], sizeof (char)) != sizeof (char)) return false;
		if (writeRaw (&p[6], sizeof (char)) != sizeof (char)) return false;
		if (writeRaw (&p[5], sizeof (char)) != sizeof (char)) return false;
		if (writeRaw (&p[4], sizeof (char)) != sizeof (char)) return false;
		if (writeRaw (&p[3], sizeof (char)) != sizeof (char)) return false;
		if (writeRaw (&p[2], sizeof (char)) != sizeof (char)) return false;
		if (writeRaw (&p[1], sizeof (char)) != sizeof (char)) return false;
		if (writeRaw (&p[0], sizeof (char)) != sizeof (char)) return false;
		return true;
	}
}

//-----------------------------------------------------------------------------
bool OutputStream::operator<< (const std::string& str)
{
	if (!(*this << 'str ')) return false;
	if (!(*this << (int)str.length ())) return false;
	return writeRaw (str.c_str (), str.length ()) == str.length ();
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool InputStream::operator>> (int& output)
{
	if (readRaw (&output, sizeof (int)) == sizeof (int))
	{
		if (byteOrder != kNativeByteOrder)
		{
			unsigned char* p = (unsigned char*)&output;
			char temp = p[0];
			p[0] = p[3];
			p[3] = temp;
			temp = p[1];
			p[1] = p[2];
			p[2] = temp;
		}
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool InputStream::operator>> (unsigned int& output)
{
	if (readRaw (&output, sizeof (unsigned int)) == sizeof (unsigned int))
	{
		if (byteOrder != kNativeByteOrder)
		{
			unsigned char* p = (unsigned char*)&output;
			char temp = p[0];
			p[0] = p[3];
			p[3] = temp;
			temp = p[1];
			p[1] = p[2];
			p[2] = temp;
		}
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool InputStream::operator>> (short& output)
{
	if (readRaw (&output, sizeof (short)) == sizeof (short))
	{
		if (byteOrder != kNativeByteOrder)
		{
			unsigned char* p = (unsigned char*)&output;
			char temp = p[0];
			p[0] = p[1];
			p[1] = temp;
		}
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool InputStream::operator>> (unsigned short& output)
{
	if (readRaw (&output, sizeof (unsigned short)) == sizeof (unsigned short))
	{
		if (byteOrder != kNativeByteOrder)
		{
			unsigned char* p = (unsigned char*)&output;
			char temp = p[0];
			p[0] = p[1];
			p[1] = temp;
		}
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool InputStream::operator>> (char& output)
{
	return readRaw (&output, sizeof (char)) == sizeof (char);
}

//-----------------------------------------------------------------------------
bool InputStream::operator>> (unsigned char& output)
{
	return readRaw (&output, sizeof (unsigned char)) == sizeof (unsigned char);
}

//-----------------------------------------------------------------------------
bool InputStream::operator>> (double& output)
{
	if (readRaw (&output, sizeof (double)) == sizeof (double))
	{
		if (byteOrder != kNativeByteOrder)
		{
			unsigned char* p = (unsigned char*)&output;
			char temp = p[0];
			p[0] = p[7];
			p[7] = temp;
			temp = p[6];
			p[1] = p[6];
			p[6] = temp;
			temp = p[5];
			p[2] = p[5];
			p[2] = temp;
			temp = p[3];
			p[3] = p[4];
			p[4] = temp;
		}
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool InputStream::operator>> (std::string& string)
{
	int identifier;
	if (!(*this >> identifier)) return false;
	if (identifier == 'str ')
	{
		int length;
		if (!(*this >> length)) return false;
		char* buffer = (char*)malloc (length);
		int read = readRaw (buffer, length);
		if (read == length)
			string.assign (buffer, length);
		free (buffer);
		return read == length;
	}
	return false;
}

} // namespace
