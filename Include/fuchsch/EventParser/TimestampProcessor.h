/*
 * TimestampProcessor.h
 *
 *  Created on: Apr 25, 2020
 *      Author: cf
 */

#pragma once

#include <cstdint>
#include <type_traits>

#include <fuchsch/Utilities/Observer.h>

#include "EventParser.h"

namespace fuchsch::eventparser {

class TimestampProcessor: public utilities::Observable<TPIUEvent>, public utilities::Observer<TPIUPacket> {
public:

	TimestampProcessor()
		: localTimestamp{0}
	{ }

	void operator()(TPIUPacket const &packet) noexcept override {
		if ((packet[0] & 0xC0) == 0xC0) {
			uint32_t timestamp = (packet[0] & 0x70) >> 4;
			size_t shift = 2;
			for (auto c : packet.subspan(1)) {
				timestamp |= (static_cast<uint32_t>(c) & 0x7F) << shift;
				shift += 6;
			}
			localTimestamp += TPIUToTimestamp(timestamp);
		} else if ((packet[0] & 0x8F) == 0 && (packet[0] & 0x70) != 0x70) {
			// Single byte local timestamp packet
			localTimestamp += TPIUToTimestamp((packet[0] & 0x70) >> 4);
		} else if (packet[0] == 0 || packet[0] == 0x94 || packet[0] == 0xB4) {
			// Ignore synchronization and global timestamp packets for now
		} else {
			Notify({ localTimestamp, packet });
		}
	}

	Timestamp LocalTimestamp() {
		return localTimestamp;
	}

private:

	Timestamp TPIUToTimestamp(uint64_t ts) const noexcept {
		return std::chrono::milliseconds(ts * 64 * 1000 / 120000000);
	}

	Timestamp localTimestamp = { };

};

} // namespace fuchsch::eventparser
