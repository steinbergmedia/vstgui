// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "vstgui/uidescription/base64codec.h"
#include "vstgui/lib/malloc.h"

#include <random>

using namespace VSTGUI;

int main ()
{
	Buffer<uint8_t> origData;
	origData.allocate (1024*1024*500);

	std::independent_bits_engine<std::default_random_engine, sizeof (uint16_t) * 8, uint16_t> rbe;
	std::generate (origData.get (), origData.get () + origData.size (), std::ref (rbe));

	auto encoderResult = Base64Codec::encode (origData.get (), origData.size ());
	auto decoderResult = Base64Codec::decode (encoderResult.data.get (), encoderResult.dataSize);

	if (origData.size () != decoderResult.dataSize)
		return -1;

	if (memcmp (origData.get (), decoderResult.data.get (), origData.size ()) != 0)
		return -1;
	return 0;
}
