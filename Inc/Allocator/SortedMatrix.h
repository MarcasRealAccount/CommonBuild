#pragma once

#include "BinarySearch.h"

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
		using const_iterator = SortedMatrixIterator<const V, SortedMatrix<const V, Impl>>;

	public:
		SortedMatrix(std::size_t maxRows) noexcept
			: m_MaxRows(maxRows)
		{
		}

		~SortedMatrix() noexcept
		{
			auto [lr, lc] = Indices(m_Last);
			for (std::size_t i = 0; i < m_LastArray; ++i)
			{
				std::size_t toIter = i < lr ? m_Impl.RowSize() : (i == lr ? lc : 0);
				for (std::size_t j = 0; j < toIter; ++j)
					m_Elements[i][j].~T();

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
			MoveUp(at);
			new (&Get(at)) V { std::move(value) };
			return at;
		}

		void Erase(std::size_t at) noexcept
		{
			Get(at).~T();
			MoveDown(at + 1);
		}

		template <class Comp>
		std::size_t EraseIf(Comp comp) noexcept
		{
			std::size_t count = 0;

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
		void MoveUp(std::size_t from) noexcept
		{
			if (from > m_Last)
				return;

			std::size_t rs = m_Impl.RowSize();
			if (m_Last == m_LastArray * rs)
			{
				m_Elements[m_LastArray] = m_Impl.AllocRow();
				++m_LastArray;
			}

			std::size_t toMove = m_Last - from;
			++m_Last;

			if (toMove == 0)
				return;

			auto [ii, ij] = Indices(from + toMove);
			auto [ji, jj] = Indices(from + toMove + 1);

			for (std::size_t i = 0; i < toMove; ++i)
			{
				m_Elements[ji][jj] = std::move(m_Elements[ii][ij]);
				if (ij == 0)
				{
					--ii;
					ij = rs - 1;
				}
				else
				{
					--ij;
				}
				if (jj == 0)
				{
					--ji;
					jj = rs - 1;
				}
				else
				{
					--jj;
				}
			}
		}

		void MoveDown(std::size_t from) noexcept
		{
			if (from > m_Last)
				return;

			std::size_t toMove = m_Last - from;
			--m_Last;

			std::size_t rs = m_Impl.RowSize();
			if (m_Last < (m_LastArray >> 1) * rs)
			{
				std::size_t toFree = m_LastArray >> 1;
				for (std::size_t i = 0; i < toFree; ++i)
				{
					std::size_t j = m_LastArray - toFree - 1;
					m_Impl.FreeRow(m_Elements[j]);
					m_Elements[j] = nullptr;
				}
				m_LastArray -= toFree;
			}

			if (toMove == 0)
				return;

			auto [ii, ij] = Indices(from);
			auto [ji, jj] = Indices(from - 1);


			for (std::size_t i = 0; i < toMove; ++i)
			{
				m_Elements[ji][jj] = std::move(m_Elements[ii][ij]);
				if (++ij == rs)
				{
					++ii;
					ij = 0;
				}
				if (++jj == rs)
				{
					++ji;
					jj = 0;
				}
			}
		}

		std::pair<std::size_t, std::size_t> Indices(std::size_t at) noexcept
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