/*
 * Span.h
 *
 *  Created on: Apr 20, 2020
 *      Author: cf
 */

#pragma once

#include <array>
#include <cstddef>
#include <type_traits>

namespace fuchsch::eventparser {

template<typename T>
class Span {

public:

	using element_type = T;
	using value_type = std::remove_cv_t<T>;

	Span() noexcept
		: ptr{nullptr}
		, extent{0}
	{ }

	template<typename It>
	Span(It first, std::size_t count) noexcept
		: ptr{first}
		, extent{count}
	{ }

	template<std::size_t N>
	Span(T (&arr)[N]) noexcept
		: ptr{arr}
		, extent{N}
	{ }

	template<std::size_t N>
	Span(std::array<T, N>& arr) noexcept
		: ptr{arr.data()}
		, extent{N}
	{ }

	template<std::size_t N>
	Span(std::array<T, N> const& arr) noexcept
		: ptr{arr.data()}
		, extent{N}
	{ }

	Span(Span const &sp) noexcept = default;

	Span& operator=(Span const& sp) noexcept = default;

	constexpr T& operator[](std::size_t index) const {
		if (index < extent) {
			return ptr[index];
		}
		throw std::out_of_range("Span operator[]: Index out of range");
	}

	constexpr T* begin() const noexcept {
		return ptr;
	}

	constexpr T* end() const noexcept {
		return ptr + extent;
	}

	constexpr T* data() const noexcept {
		return ptr;
	}

	std::size_t size() const noexcept {
		return extent;
	}

	Span<T> first(std::size_t count) const {
		return {ptr, count};
	}

	Span<T> subspan(std::size_t offset) const {
		if (offset < extent) {
			return {ptr + offset, extent - offset};
		}
		return {};
	}

private:

	T *ptr;

	std::size_t extent;

};

} // namespace fuchsch::eventparser
