// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include <cstdint>
#include <vector>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Minesweeper {

//------------------------------------------------------------------------
class Model
{
private:
	enum class Bit : uint16_t
	{
		Mine = 1 << 0,
		Open = 1 << 1,
		Flag = 1 << 2,
		Trap = 1 << 3,
		Question = 1 << 4,
	};

	static bool hasBit (uint16_t s, Bit flag) { return (s & static_cast<uint16_t> (flag)) != 0; }
	static void setBit (uint16_t& s, Bit flag) { s |= static_cast<uint16_t> (flag); }
	static void unsetBit (uint16_t& s, Bit flag) { s &= ~static_cast<uint16_t> (flag); }

	struct Cell
	{
		uint16_t neighbours {0};
		uint16_t flags {0};

		bool isMine () const { return hasBit (flags, Bit::Mine); }
		bool isOpen () const { return hasBit (flags, Bit::Open); }
		bool isFlag () const { return hasBit (flags, Bit::Flag); }
		bool isTrap () const { return hasBit (flags, Bit::Trap); }
		bool isQuestion () const { return hasBit (flags, Bit::Question); }

		void setMine () { setBit (flags, Bit::Mine); }
		void setOpen () { setBit (flags, Bit::Open); }
		void setTrap () { setBit (flags, Bit::Trap); }
		void setFlag () { setBit (flags, Bit::Flag); }
		void unsetFlag () { unsetBit (flags, Bit::Flag); }
		void setQuestion () { setBit (flags, Bit::Question); }
		void unsetQuestion () { unsetBit (flags, Bit::Question); }
	};

	using Row = std::vector<Cell>;
	using Matrix = std::vector<Row>;

public:
	struct IListener
	{
		virtual void onCellChanged (uint32_t row, uint32_t col) = 0;
	};

	Model (uint32_t numberOfRows, uint32_t numberOfCols, uint32_t numberOfMines,
	       IListener* listener);

	uint32_t getNumberOfMines () const;
	uint32_t getNumberOfFlags () const;

	void open (uint32_t row, uint32_t col);
	void mark (uint32_t row, uint32_t col);

	bool isTrapped () const;
	bool isDone () const;
	bool isOpen (uint32_t row, uint32_t col) const;
	bool isFlag (uint32_t row, uint32_t col) const;
	bool isQuestion (uint32_t row, uint32_t col) const;
	bool isMine (uint32_t row, uint32_t col) const;
	bool isTrapMine (uint32_t row, uint32_t col) const;
	uint32_t getNumberOfMinesNearby (uint32_t row, uint32_t col) const;

private:
	void openCleanNearby (uint32_t row, uint32_t col);
	bool openInternal (uint32_t row, uint32_t col);
	void allocateModel (uint32_t numberOfRows, uint32_t numberOfCols);
	void clearModel ();
	void setMines ();
	void calcNeighbours ();

	Matrix matrix;
	uint32_t mines {0};
	uint32_t opened {0};
	uint32_t flagged {0};
	bool trapped {false};
	IListener* listener {nullptr};
};

//------------------------------------------------------------------------
} // Minesweeper
} // Standalone
} // VSTGUI
