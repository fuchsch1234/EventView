/*
 * EventParser.h
 *
 *  Created on: Apr 20, 2020
 *      Author: cf
 */

#pragma once

#include <exception>
#include <variant>
#include <cstring>

#include "Span.h"

namespace fuchsch::eventparser {

	struct SynchronizationPacket {};

	struct OverflowPacket {};

	struct LocalTimestampPacket {
		uint32_t timestamp;
	};

	struct GlobalTimestamp1Packet {
		uint32_t timestamp;
		bool clockChange;
		bool wrap;
	};

	struct GlobalTimestamp2Packet {
		uint32_t timestamp;
	};

	struct ExtensionPacket {
		uint8_t page;
	};

	struct InstrumentationPacket {
		uint8_t port;
		uint32_t data;
	};

	struct EventCounterPacket {};

	struct ExceptionTracePacket {
		int exception;
		enum class Event {
			START,
			STOP,
			RESUME,
		} event;
	};

	struct PCSamplingPacket {
		uint32_t pc;
	};

	struct DataTracePacket {
		uint8_t address;
		uint32_t data;
	};

	using TPIUPacket = Span<unsigned char>;

	struct TPIUEvent {
		uint64_t timestamp;
		Span<unsigned char> data;
	};

	struct ExceptionTrace {
		uint64_t timestamp;
		int exception;
		enum class Event {
			START,
			STOP,
			RESUME,
		} event;
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
		auto length = CalculateTPIULength(buffer);
		if (residual) *residual = buffer.subspan(length);
		return buffer.first(length);
	}

	/**
	 * @brief Parses a TPIU packet from a bytes array.
	 *
	 * @param span Array of bytes containing TPIU packet.
	 * @return The parsed TPIU packet.
	 */
	TPIUPacket ParseTPIUPacket(Span<unsigned char> span) {
		auto length = CalculateTPIULength(span);
		if (length >= 6) {
			constexpr const unsigned char sync[] = {0, 0, 0, 0, 0, 0x80};
			if (std::memcmp(span.data(), sync, sizeof(sync)) == 0) {
				return SynchronizationPacket{};
			}
		} else if (span[0] == 0x70) {
			return OverflowPacket{};
		} else if ((span[0] & 0xC0) == 0xC0) {
			uint32_t timestamp = (span[0] & 0x70) >> 4;
			size_t shift = 2;
			for (auto c : span.subspan(1)) {
				timestamp |= (static_cast<uint32_t>(c) & 0x7F) << shift;
				shift += 6;
			}
			return LocalTimestampPacket{ timestamp };
		} else if ((span[0] & 0x8F) == 0) {
			// Single byte local timestamp packet
			return LocalTimestampPacket{ static_cast<uint8_t>((span[0] & 0x70) >> 4) };
		} else if (span[0] == 0x94) {
			return GlobalTimestamp1Packet{};
		} else if (span[0] == 0xB4) {
			return GlobalTimestamp2Packet{};
		} else if ((span[0] & 0x8F) == 0x08) {
			return ExtensionPacket{ static_cast<uint8_t>((span[0] & 0x70) >> 4) };
		} else if ((span[0] & 0x04) == 0) {
			uint32_t data = 0;
			int shift = 0;
			for (auto c : span.subspan(1)) {
				data |= static_cast<uint32_t>(c) << shift;
				shift += 8;
			}
			return InstrumentationPacket{ static_cast<uint8_t>((span[0] & 0xF8) >> 3), data };
		} else if (span[0] == 0x05) {
			return EventCounterPacket{};
		} else if (span[0] == 0x0E) {
			auto type = ExceptionTracePacket::Event::RESUME;
			if ((span[2] & 0x30) == 0x10) {
				type = ExceptionTracePacket::Event::START;
			} else if ((span[2] & 0x30) == 0x20) {
				type = ExceptionTracePacket::Event::STOP;
			}
			auto exception = (int)(span[2] & 0x01) << 8;
			exception |= span[1];
			return ExceptionTracePacket{ exception, type };
		} else if (span[0] == 0x17) {
			return PCSamplingPacket{};
		} else if ((span[0] & 0x04) == 0x04) {
			return DataTracePacket{};
		}
		return {};
	}

} // namespace fuchsch::eventparser
