// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "model.h"
#include <algorithm>
#include <random>
#include <cassert>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Minesweeper {

//------------------------------------------------------------------------
Model::Model (uint32_t numberOfRows, uint32_t numberOfCols, uint32_t numberOfMines,
			  IListener* listener)
: mines (numberOfMines), listener (listener)
{
	assert (numberOfMines < numberOfRows * numberOfCols);
	allocateModel (numberOfRows, numberOfCols);
	clearModel ();
	setMines ();
	calcNeighbours ();
}

//------------------------------------------------------------------------
uint32_t Model::getNumberOfMines () const
{
	return mines;
}

//------------------------------------------------------------------------
uint32_t Model::getNumberOfFlags () const
{
	return flagged;
}

//------------------------------------------------------------------------
bool Model::isTrapped () const
{
	return trapped;
}

//------------------------------------------------------------------------
bool Model::isDone () const
{
	if (flagged == mines)
	{
		for (auto row = 0u; row < matrix.size (); ++row)
		{
			for (auto col = 0u; col < matrix.front ().size (); ++col)
			{
				auto cell = matrix[row][col];
				if (cell.isFlag () && !cell.isMine ())
					return false;
			}
		}
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
void Model::open (uint32_t row, uint32_t col)
{
	if (openInternal (row, col))
	{
		auto cell = matrix[row][col];
		if (cell.isOpen () && cell.neighbours == 0)
		{
			openCleanNearby (row, col);
		}
	}
}

//------------------------------------------------------------------------
void Model::mark (uint32_t row, uint32_t col)
{
	auto& cell = matrix[row][col];
	if (cell.isFlag ())
	{
		--flagged;
		cell.unsetFlag ();
		cell.setQuestion ();
	}
	else if (cell.isQuestion ())
	{
		cell.unsetQuestion ();
	}
	else
	{
		++flagged;
		cell.setFlag ();
	}
	if (listener)
		listener->onCellChanged (row, col);
}

//------------------------------------------------------------------------
bool Model::isOpen (uint32_t row, uint32_t col) const
{
	return matrix[row][col].isOpen ();
}

//------------------------------------------------------------------------
bool Model::isFlag (uint32_t row, uint32_t col) const
{
	return matrix[row][col].isFlag ();
}

//------------------------------------------------------------------------
bool Model::isQuestion (uint32_t row, uint32_t col) const
{
	return matrix[row][col].isQuestion ();
}

//------------------------------------------------------------------------
bool Model::isMine (uint32_t row, uint32_t col) const
{
	return matrix[row][col].isMine ();
}

//------------------------------------------------------------------------
bool Model::isTrapMine (uint32_t row, uint32_t col) const
{
	return matrix[row][col].isTrap ();
}

//------------------------------------------------------------------------
uint32_t Model::getNumberOfMinesNearby (uint32_t row, uint32_t col) const
{
	return matrix[row][col].neighbours;
}

//------------------------------------------------------------------------
void Model::openCleanNearby (uint32_t row, uint32_t col)
{
	const auto maxCol = matrix.front ().size () - 1;
	const auto maxRow = matrix.size () - 1;
	if (row > 0)
	{
		if (col > 0)
			open (row - 1, col - 1);
		open (row - 1, col);
		if (col < maxCol)
			open (row - 1, col + 1);
	}
	if (col > 0)
		open (row, col - 1);
	if (col < maxCol)
		open (row, col + 1);
	if (row < maxRow)
	{
		if (col > 0)
			open (row + 1, col - 1);
		open (row + 1, col);
		if (col < maxCol)
			open (row + 1, col + 1);
	}
}

//------------------------------------------------------------------------
bool Model::openInternal (uint32_t row, uint32_t col)
{
	auto& cell = matrix[row][col];
	if (!cell.isOpen ())
	{
		cell.setOpen ();
		++opened;
		if (cell.isFlag ())
		{
			cell.unsetFlag ();
			--flagged;
		}
		else if (cell.isQuestion ())
			cell.unsetQuestion ();
		if (listener)
			listener->onCellChanged (row, col);
		if (cell.isMine ())
		{
			cell.setTrap ();
			trapped = true;
			return false;
		}
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
void Model::allocateModel (uint32_t numberOfRows, uint32_t numberOfCols)
{
	matrix.resize (numberOfRows);
	std::for_each (matrix.begin (), matrix.end (),
				   [numberOfCols] (auto& r) { r.resize (numberOfCols); });
}

//------------------------------------------------------------------------
void Model::clearModel ()
{
	std::for_each (matrix.begin (), matrix.end (), [] (auto& r) {
		std::for_each (r.begin (), r.end (), [] (auto& cell) { cell = {}; });
	});
}

//------------------------------------------------------------------------
void Model::setMines ()
{
	std::random_device rd;
	std::mt19937 gen (rd ());
	std::uniform_int_distribution<> rowDist (0, static_cast<int> (matrix.size () - 1));
	std::uniform_int_distribution<> colDist (0, static_cast<int> (matrix.front ().size () - 1));
	for (auto i = 0u; i < mines; ++i)
	{
		auto row = rowDist (gen);
		auto col = colDist (gen);
		if ((matrix[row][col]).isMine ())
		{
			assert (i > 0u);
			--i;
			continue;
		}
		matrix[row][col].setMine ();
	}
}

//------------------------------------------------------------------------
void Model::calcNeighbours ()
{
	const auto maxCol = matrix.front ().size () - 1;
	const auto maxRow = matrix.size () - 1;
	for (auto row = 0u; row < matrix.size (); ++row)
	{
		for (auto col = 0u; col < matrix.front ().size (); ++col)
		{
			auto& cell = matrix[row][col];
			if (cell.isMine ())
				continue;
			if (row > 0)
			{
				if (col > 0)
					cell.neighbours += matrix[row - 1][col - 1].isMine () ? 1 : 0;
				cell.neighbours += matrix[row - 1][col].isMine () ? 1 : 0;
				if (col < maxCol)
					cell.neighbours += matrix[row - 1][col + 1].isMine () ? 1 : 0;
			}
			if (col > 0)
				cell.neighbours += matrix[row][col - 1].isMine () ? 1 : 0;
			if (col < maxCol)
				cell.neighbours += matrix[row][col + 1].isMine () ? 1 : 0;
			if (row < maxRow)
			{
				if (col > 0)
					cell.neighbours += matrix[row + 1][col - 1].isMine () ? 1 : 0;
				cell.neighbours += matrix[row + 1][col].isMine () ? 1 : 0;
				if (col < maxCol)
					cell.neighbours += matrix[row + 1][col + 1].isMine () ? 1 : 0;
			}
		}
	}
}

//------------------------------------------------------------------------
} // Minesweeper
} // Standalone
} // VSTGUI
