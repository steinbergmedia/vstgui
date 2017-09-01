// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../unittests.h"
#include "../../../uidescription/cstream.h"

namespace VSTGUI {

TESTCASE(CMemoryStreamTests,

	TEST(readWrite,
		CMemoryStream s;
		uint64_t value = 1;
		uint32_t size = sizeof(value);
		EXPECT(s.writeRaw (&value, size) == size);
		EXPECT(s.tell () == size);
		s.rewind ();
		value = 20;
		EXPECT(s.readRaw(&value, size) == size);
		EXPECT(value == 1);
	);
	
	TEST(overread,
		CMemoryStream s;
		uint32_t value = 1;
		EXPECT(s.writeRaw (&value, sizeof (value)) == sizeof (value));
		s.rewind ();
		uint64_t value2;
		EXPECT(s.readRaw(&value2, sizeof(value2)) < sizeof(value2));
	);
	
	TEST(seek,
		constexpr uint32_t bufferSize = 32;
		int8_t buffer[bufferSize];
		CMemoryStream s (buffer, bufferSize);
		EXPECT(s.seek (33, CMemoryStream::kSeekEnd) == kStreamSeekError);
		EXPECT(s.seek (0, CMemoryStream::kSeekEnd) == 32);
		EXPECT(s.seek (-2, CMemoryStream::kSeekCurrent) == 30);
		EXPECT(s.seek (15, CMemoryStream::kSeekSet) == 15);
	);
	
	TEST(readWriteValueLittleEndian,
		CMemoryStream s;
		OutputStream& os = s;
		os.setByteOrder (kLittleEndianByteOrder);
		os << static_cast<int8_t> (1);
		os << static_cast<uint8_t> (2);
		os << static_cast<int16_t> (3);
		os << static_cast<uint16_t> (4);
		os << static_cast<int32_t> (5);
		os << static_cast<uint32_t> (6);
		os << static_cast<int64_t> (7);
		os << static_cast<uint64_t> (8);
		os << static_cast<double> (9);
		s.rewind ();
		InputStream& is = s;
		is.setByteOrder (kLittleEndianByteOrder);
		int8_t v1;
		EXPECT(is >> v1);
		EXPECT(v1 == 1);
		uint8_t v2;
		EXPECT(is >> v2);
		EXPECT(v2 == 2);
		int16_t v3;
		EXPECT(is >> v3);
		EXPECT(v3 == 3);
		uint16_t v4;
		EXPECT(is >> v4);
		EXPECT(v4 == 4);
		int32_t v5;
		EXPECT(is >> v5);
		EXPECT(v5 == 5);
		uint32_t v6;
		EXPECT(is >> v6);
		EXPECT(v6 == 6);
		int64_t v7;
		EXPECT(is >> v7);
		EXPECT(v7 == 7);
		uint64_t v8;
		EXPECT(is >> v8);
		EXPECT(v8 == 8);
		double v9;
		EXPECT(is >> v9);
		EXPECT(v9 == 9.0);
	);

	TEST(readWriteValueBigEndian,
		CMemoryStream s;
		OutputStream& os = s;
		os.setByteOrder (kBigEndianByteOrder);
		os << static_cast<int8_t> (1);
		os << static_cast<uint8_t> (2);
		os << static_cast<int16_t> (3);
		os << static_cast<uint16_t> (4);
		os << static_cast<int32_t> (5);
		os << static_cast<uint32_t> (6);
		os << static_cast<int64_t> (7);
		os << static_cast<uint64_t> (8);
		os << static_cast<double> (9);
		os << std::string ("Test");
		s.rewind ();
		InputStream& is = s;
		is.setByteOrder (kBigEndianByteOrder);
		int8_t v1;
		EXPECT(is >> v1);
		EXPECT(v1 == 1);
		uint8_t v2;
		EXPECT(is >> v2);
		EXPECT(v2 == 2);
		int16_t v3;
		EXPECT(is >> v3);
		EXPECT(v3 == 3);
		uint16_t v4;
		EXPECT(is >> v4);
		EXPECT(v4 == 4);
		int32_t v5;
		EXPECT(is >> v5);
		EXPECT(v5 == 5);
		uint32_t v6;
		EXPECT(is >> v6);
		EXPECT(v6 == 6);
		int64_t v7;
		EXPECT(is >> v7);
		EXPECT(v7 == 7);
		uint64_t v8;
		EXPECT(is >> v8);
		EXPECT(v8 == 8);
		double v9;
		EXPECT(is >> v9);
		EXPECT(v9 == 9.0);
		std::string str;
		EXPECT(is >> str);
		EXPECT(str == "Test");
	);
	
);

} // VSTGUI
