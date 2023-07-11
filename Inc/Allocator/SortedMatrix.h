#pragma once

#include "BinarySearch.h"
#include "Memory/Memory.h"

#include <cstddef>
#include <cstring>

#include <compare>
#include <concepts>
#include <utility>

namespace Allocator
{
	template <class T, class V>
	concept SortedMatrixImplC = requires(T& t, V* a) {
		{
			t.AllocRow()
		} noexcept -> std::same_as<V*>;
		{
			t.FreeRow(a)
		} noexcept -> std::same_as<void>;
	} && requires(const T& t, const V& v) {
		{
			t.RowSize()
		} noexcept -> std::same_as<std::size_t>;
	};

	template <class V, class SM>
	struct SortedMatrixIterator
	{
	public:
		SortedMatrixIterator() noexcept
			: m_Matrix(nullptr),
			  m_Index(0) {}

		SortedMatrixIterator(const SortedMatrixIterator& copy) noexcept
			: m_Matrix(copy.m_Matrix),
			  m_Index(copy.m_Index) {}

		SortedMatrixIterator& operator=(const SortedMatrixIterator& copy) noexcept
		{
			m_Matrix = copy.m_Matrix;
			m_Index  = copy.m_Index;
			return *this;
		}

		V& operator*()
		{
			return m_Matrix->Get(m_Index);
		}

		V* operator->()
		{
			return &m_Matrix->Get(m_Index);
		}

		SortedMatrixIterator& operator++()
		{
			++m_Index;
			return *this;
		}

		SortedMatrixIterator operator++(int)
		{
			auto copy = *this;
			++(*this);
			return copy;
		}

		SortedMatrixIterator& operator--()
		{
			--m_Index;
			return *this;
		}

		SortedMatrixIterator operator--(int)
		{
			auto copy = *this;
			--(*this);
			return copy;
		}

		SortedMatrixIterator& operator+=(std::ptrdiff_t inc)
		{
			m_Index += inc;
			return *this;
		}

		SortedMatrixIterator& operator-=(std::ptrdiff_t inc)
		{
			m_Index -= inc;
			return *this;
		}

		friend SortedMatrixIterator operator+(SortedMatrixIterator itr, std::ptrdiff_t inc)
		{
			return itr += inc;
		}

		friend SortedMatrixIterator operator-(SortedMatrixIterator itr, std::ptrdiff_t inc)
		{
			return itr -= inc;
		}

		friend std::ptrdiff_t operator-(SortedMatrixIterator lhs, SortedMatrixIterator rhs)
		{
			return lhs.m_Index - rhs.m_Index;
		}

		friend bool operator==(SortedMatrixIterator lhs, SortedMatrixIterator rhs)
		{
			return lhs.m_Matrix == rhs.m_Matrix && lhs.m_Index == rhs.m_Index;
		}

		friend bool operator!=(SortedMatrixIterator lhs, SortedMatrixIterator rhs)
		{
			return !(lhs == rhs);
		}

		friend bool operator<(SortedMatrixIterator lhs, SortedMatrixIterator rhs)
		{
			if (lhs.m_Matrix > rhs.m_Matrix)
				return false;
			return lhs.m_Matrix < rhs.m_Matrix || lhs.m_Index < rhs.m_Index;
		}

		friend bool operator>(SortedMatrixIterator lhs, SortedMatrixIterator rhs)
		{
			if (lhs.m_Matrix < rhs.m_Matrix)
				return false;
			return lhs.m_Matrix > rhs.m_Matrix || lhs.m_Index > rhs.m_Index;
		}

		friend bool operator<=(SortedMatrixIterator lhs, SortedMatrixIterator rhs)
		{
			return !(lhs > rhs);
		}

		friend bool operator>=(SortedMatrixIterator lhs, SortedMatrixIterator rhs)
		{
			return !(lhs < rhs);
		}

	private:
		friend SM;

		SortedMatrixIterator(SM& matrix, std::size_t index) noexcept
			: m_Matrix(&matrix),
			  m_Index(index) {}

		SM*         m_Matrix;
		std::size_t m_Index;
	};

	template <class V, SortedMatrixImplC<V> Impl>
	struct SortedMatrix
	{
	public:
		using iterator       = SortedMatrixIterator<V, SortedMatrix<V, Impl>>;
		using const_iterator = SortedMatrixIterator<const V, const SortedMatrix<V, Impl>>;

	public:
		SortedMatrix(std::size_t maxRows) noexcept
			: m_MaxRows(maxRows),
			  m_Last(0),
			  m_LastArray(0)
		{
		}

		~SortedMatrix() noexcept
		{
			auto [lr, lc] = Indices(m_Last);
			for (std::size_t i = 0; i < m_LastArray; ++i)
			{
				std::size_t toIter = i < lr ? m_Impl.RowSize() : (i == lr ? lc : 0);
				for (std::size_t j = 0; j < toIter; ++j)
					m_Elements[i][j].~V();

				m_Impl.FreeRow(m_Elements[i]);
			}
		}

		V& operator[](std::size_t at) noexcept
		{
			auto [i, j] = Indices(at);
			return m_Elements[i][j];
		}

		const V& operator[](std::size_t at) const noexcept
		{
			auto [i, j] = Indices(at);
			return m_Elements[i][j];
		}

		V& Get(std::size_t at) noexcept
		{
			auto [i, j] = Indices(at);
			return m_Elements[i][j];
		}

		const V& Get(std::size_t at) const noexcept
		{
			auto [i, j] = Indices(at);
			return m_Elements[i][j];
		}

		template <class Find>
		std::size_t Insert(V&& value, Find find) noexcept
		{
			std::size_t at = find(*this, value);
			MoveUp(at, m_Last, 1);
			new (&Get(at)) V { std::move(value) };
			return at;
		}

		void Erase(std::size_t at) noexcept
		{
			Get(at).~V();
			MoveDown(at + 1, m_Last, 1);
		}

		template <class Comp>
		std::size_t EraseIf(Comp comp) noexcept
		{
			std::size_t rs    = m_Impl.RowSize();
			std::size_t count = 0;
			std::size_t ii = 0, ij = 0;
			std::size_t lastIndex = 0;
			for (std::size_t i = 0; i < m_Last; ++i)
			{
				if (comp(m_Elements[ii][ij]))
				{
					m_Elements[ii][ij].~V();
					if (lastIndex)
						MoveDown(lastIndex + 1, i, count);
					lastIndex = i;
					++count;
				}
				if (++ij == rs)
				{
					++ii;
					ij = 0;
				}
			}
			if (lastIndex)
				MoveDown(lastIndex + 1, m_Last, count);
			return count;
		}

		std::size_t Size() const { return m_Last; }

		std::size_t Capacity() const { return m_LastArray * m_Impl.RowSize(); }

		iterator begin() { return iterator { *this, 0 }; }

		const_iterator begin() const { return const_iterator { *this, 0 }; }

		const_iterator cbegin() const { return const_iterator { *this, 0 }; }

		iterator end() { return iterator { *this, m_Last }; }

		const_iterator end() const { return const_iterator { *this, m_Last }; }

		const_iterator cend() const { return const_iterator { *this, m_Last }; }

	private:
		void MoveUp(std::size_t from, std::size_t to, std::size_t count) noexcept
		{
			if (from > m_Last)
				return;

			std::size_t rs    = m_Impl.RowSize();
			std::size_t toAdd = Memory::AlignCountCeil(m_Last + count, rs) - m_LastArray;
			for (std::size_t i = 0; i < toAdd; ++i)
			{
				m_Elements[m_LastArray] = m_Impl.AllocRow();
				++m_LastArray;
			}

			std::size_t toMove = to - from;
			m_Last            += count;

			if (toMove == 0)
				return;

			auto [ii, ij] = Indices(from + toMove - 1);
			auto [ji, jj] = Indices(from + toMove + count - 1);
		}

		void MoveDown(std::size_t from, std::size_t to, std::size_t count) noexcept
		{
			if (from > m_Last)
				return;

			std::size_t toMove = to - from;
			m_Last            -= count;

			if (toMove == 0)
				return;

			auto [ii, ij] = Indices(from);
			auto [ji, jj] = Indices(from - count);

			std::size_t rs = m_Impl.RowSize();
		}

		std::pair<std::size_t, std::size_t> Indices(std::size_t at) const noexcept
		{
			std::size_t rs = m_Impl.RowSize();
			return { at / rs, at % rs };
		}

	private:
		Impl        m_Impl;
		std::size_t m_MaxRows;
		std::size_t m_Last;
		std::size_t m_LastArray;

		V* m_Elements[1];
	};

	template <class Matrix, class K, class KeyF, class Comp>
	std::size_t FindGreaterEqual(const Matrix& matrix, const K& key, KeyF keyF, Comp comp)
	{
		return BSGreaterEqual(key, 0, matrix.Size(), [&](std::size_t element, const K& key) -> std::weak_ordering {
			return comp(keyF(matrix[element]), key);
		});
	}

	template <class Matrix, class K, class KeyF, class Comp>
	std::size_t FindGreater(const Matrix& matrix, const K& key, KeyF keyF, Comp comp)
	{
		return BSGreater(key, 0, matrix.Size(), [&](std::size_t element, const K& key) -> std::weak_ordering {
			return comp(keyF(matrix[element]), key);
		});
	}

	template <class Matrix, class K, class KeyF, class Comp>
	std::size_t FindLessEqual(const Matrix& matrix, const K& key, KeyF keyF, Comp comp)
	{
		return BSLessEqual(key, 0, matrix.Size(), [&](std::size_t element, const K& key) -> std::weak_ordering {
			return comp(keyF(matrix[element]), key);
		});
	}

	template <class Matrix, class K, class KeyF, class Comp>
	std::size_t FindEqual(const Matrix& matrix, const K& key, KeyF keyF, Comp comp)
	{
		return BSEqual(key, 0, matrix.Size(), [&](std::size_t element, const K& key) -> std::weak_ordering {
			return comp(keyF(matrix[element]), key);
		});
	}

	template <class Matrix, class K, class KeyF, class Comp>
	std::size_t FindEqualIter(const Matrix& matrix, const K& key, KeyF keyF, Comp comp)
	{
		std::size_t index = 0;
		for (const auto& value : matrix)
		{
			if (comp(keyF(value), key) == 0)
				return index;
			++index;
		}
		return index;
	}
} // namespace Allocator