/*
 * EventParser.h
 *
 *  Created on: Apr 20, 2020
 *      Author: cf
 */

#pragma once

#include <chrono>
#include <exception>
#include <variant>
#include <cstring>

#include "Span.h"

namespace fuchsch::eventparser {

	struct OverflowPacket {};

	struct ExtensionPacket {
		uint8_t page;
	};

	struct EventCounterPacket {};

	struct PCSamplingPacket {
		uint32_t pc;
	};

	struct DataTracePacket {
		uint8_t address;
		uint32_t data;
	};

	using Timestamp = std::chrono::milliseconds;

	using TPIUPacket = Span<unsigned char>;

	struct TPIUEvent {
		Timestamp timestamp;
		Span<unsigned char> data;
	};

	struct ExceptionTrace {
		Timestamp timestamp;
		int exception;
		enum class Event {
			START,
			STOP,
			RESUME,
		} event;
	};

	struct InstrumentationTrace {
		Timestamp timestamp;
		uint8_t port;
		uint32_t data;
	};

	/**
	 * @brief Calculate the length of a TPIU packet in a byte array.
	 *
	 * @param buffer A byte array containing TPIU packets.
	 * @return The length of the TPIU packet starting at the first byte in this buffer.
	 */
	static size_t CalculateTPIULength(Span<unsigned char> buffer) noexcept {
		switch (buffer[0] & 0x3) {
			case 0: {
				size_t length = 1;
				for (auto c : buffer) {
					if (buffer[0] == 0) {
						if (c != 0) return length;
					} else {
						if ((c & 0x80) == 0x00) return length;
					}
					length++;
				}
				break;
			}
			case 1:
				return 2 <= buffer.size() ? 2 : 0;
			case 2:
				return 3 <= buffer.size() ? 3 : 0;
			case 3:
				return 5 <= buffer.size() ? 5 : 0;
		}
		return 0;
	}

	/**
	 * @brief Split an array of bytes into a TPIU packet and the residual containing the rest of the buffer.
	 *
	 * @param buffer Bytes containing 0 or more TPIU packets.
	 * @param residual Out parameter. If not null the value will be set to the residual starting after the current TPIU packet.
	 * @return Span containing the next TPIU packet in the buffer or an empty span if buffer contains no complete TPIU packet.
	 */
	Span<unsigned char> SplitPacket(Span<unsigned char> buffer, Span<unsigned char> *residual = nullptr) noexcept {
		if (buffer.size() == 0) return {};
		auto length = CalculateTPIULength(buffer);
		if (residual) *residual = buffer.subspan(length);
		return buffer.first(length);
	}

} // namespace fuchsch::eventparser
