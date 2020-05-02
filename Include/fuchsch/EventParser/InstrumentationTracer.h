/*
 * TimestampProcessor.h
 *
 *  Created on: Apr 25, 2020
 *      Author: cf
 */

#pragma once

#include <algorithm>
#include <cstdint>

#include <fuchsch/Utilities/Observer.h>

#include "EventParser.h"

namespace fuchsch::eventparser {

class InstrumentationTracer: public utilities::Observable<InstrumentationTrace>, public utilities::Observer<TPIUEvent> {
public:

	void operator()(TPIUEvent const &event) noexcept override {
		const auto data = event.data;

		if ((data[0] & 0x0F) == 0x08 && data.size() == 1) {
			page = (data[0] >> 4) & 0x0F;
		}

		if ((data[0] & 0x03) > 0 && (data[0] & 0x04) == 0) {
			const uint8_t port = page * 32 + ((data[0] >> 3) & 0x1F);
			uint32_t trace = 0;
			for (int i = data.size() - 1; i > 0; i--) {
				trace = (trace << 8) | data[i];
			}
			Notify({ event.timestamp, port, trace });
		}
	}

private:

	uint8_t page = 0;

};

} // namespace fuchsch::eventparser
