#include "vstgui/uidescription/base64codec.h"
#include "vstgui/lib/malloc.h"

#include <random>

using namespace VSTGUI;

int main ()
{
	Malloc<uint8_t> origData;
	origData.allocate (1024*1024*500);

	std::independent_bits_engine<std::default_random_engine, sizeof (uint8_t) * 8, uint8_t> rbe;
	std::generate (origData.get (), origData.get () + origData.size (), std::ref (rbe));

	Base64Codec encoder;
	if (!encoder.encode (origData.get (), origData.size ()))
		return -1;
	Base64Codec decoder;
	if (!decoder.decode (encoder.getData (), encoder.getDataSize ()))
		return -1;

	if (origData.size () != decoder.getDataSize ())
		return -1;

	if (memcmp (origData.get (), decoder.getData (), origData.size ()) != 0)
		return -1;
	return 0;
}
