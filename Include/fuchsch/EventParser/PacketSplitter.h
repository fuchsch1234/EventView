/*
 * TimestampProcessor.h
 *
 *  Created on: Apr 25, 2020
 *      Author: cf
 */

#pragma once

#include <algorithm>
#include <vector>

#include <fuchsch/Utilities/Observer.h>

#include "EventParser.h"

namespace fuchsch::eventparser {

class PacketSplitter: public utilities::Observable<TPIUPacket>, public utilities::Observer<unsigned char>, public utilities::Observer<Span<unsigned char>> {
public:

	void operator()(Span<unsigned char> const &bytes) noexcept override {
		std::copy(bytes.begin(), bytes.end(), std::back_inserter(residual));
		TryNextPacket();
	}

	void operator()(unsigned char const &byte) noexcept override {
		residual.push_back(byte);
		TryNextPacket();
	}

private:

	void TryNextPacket() {
		auto packet = SplitPacket({residual.data(), residual.size()});
		if (packet.size() > 0) {
			// Found a complete packet in the bytestream
			Notify(packet);
			// Keep the residual
			residual.erase(residual.begin(), residual.begin() + packet.size());
		}
	}

	std::vector<unsigned char> residual = {};

};

} // namespace fuchsch::eventparser
