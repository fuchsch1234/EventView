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

class ExceptionTracer: public utilities::Observable<ExceptionTrace>, public utilities::Observer<TPIUEvent> {
public:

	void operator()(TPIUEvent const &event) noexcept override {
		const auto data = event.data;
		if (data[0] == 0x0E && data.size() == 3) {
			auto type = ExceptionTrace::Event::RESUME;
			if ((data[2] & 0x30) == 0x10) {
				type = ExceptionTrace::Event::START;
			} else if ((data[2] & 0x30) == 0x20) {
				type = ExceptionTrace::Event::STOP;
			}
			auto exception = (int)(data[2] & 0x01) << 8;
			exception |= data[1];
			Notify(ExceptionTrace{ event.timestamp, exception, type });
		}
	}

};

} // namespace fuchsch::eventparser
