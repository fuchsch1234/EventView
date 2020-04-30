#include <gtest/gtest.h>

#include <fuchsch/EventParser/EventParser.h>

using namespace ::fuchsch::eventparser;

/******************************************************************************
 * Packet Split Tests
 *****************************************************************************/

TEST(SplitPacket, OneBytePacket) {
	unsigned char data[] = { 0x10 };
	auto packet = SplitPacket(data);
	ASSERT_EQ(packet.size(), 1U);
}

TEST(SplitPacket, ContinuationPacket) {
	unsigned char data[] = { 0x80, 0x80, 0x17 };
	auto packet = SplitPacket(data);
	ASSERT_EQ(packet.size(), 3U);
}

TEST(SplitPacket, 2ByteSourcePacket) {
	unsigned char data[] = { 0x81, 0x80 };
	auto packet = SplitPacket(data);
	ASSERT_EQ(packet.size(), 2U);
}

TEST(SplitPacket, 3ByteSourcePacket) {
	unsigned char data[] = { 0x82, 0x80, 0x17 };
	auto packet = SplitPacket(data);
	ASSERT_EQ(packet.size(), 3U);
}

TEST(SplitPacket, 5ByteSourcePacket) {
	unsigned char data[] = { 0x83, 0x80, 0x00, 0x00, 0x80 };
	auto packet = SplitPacket(data);
	ASSERT_EQ(packet.size(), 5U);
}

TEST(SplitPacket, SynchronizationPacket) {
	unsigned char data[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x80 };
	auto packet = SplitPacket(data);
	ASSERT_EQ(packet.size(), sizeof(data));
}

TEST(SplitPacket, LongSynchronizationPacket) {
	unsigned char data[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80 };
	auto packet = SplitPacket(data);
	ASSERT_EQ(packet.size(), sizeof(data));
}

TEST(SplitPacket, MissingBytesSynchronizationPacket) {
	unsigned char data[] = { 0x00, 0x00, 0x00, 0x00 };
	Span<unsigned char> residual;
	auto packet = SplitPacket(data, &residual);
	ASSERT_EQ(packet.size(), 0U);
	ASSERT_EQ(residual.size(), 4U);
}

TEST(SplitPacket, MissingBytesContinuationPacket) {
	unsigned char data[] = { 0x80, 0x80 };
	auto packet = SplitPacket(data);
	ASSERT_EQ(packet.size(), 0U);
}

TEST(SplitPacket, MissingBytesSourcePacket) {
	unsigned char data[] = { 0x82, 0x80 };
	auto packet = SplitPacket(data);
	ASSERT_EQ(packet.size(), 0U);
}

TEST(SplitPacket, NoResidualContinuationPacket) {
	unsigned char data[] = { 0x80, 0x00 };
	Span<unsigned char> residual = {};
	auto packet = SplitPacket(data, &residual);
	ASSERT_EQ(packet.size(), 2U);
	ASSERT_EQ(residual.size(), 0U);
}

TEST(SplitPacket, NoResidualSourcePacket) {
	unsigned char data[] = { 0x81, 0x00 };
	Span<unsigned char> residual = {};
	auto packet = SplitPacket(data, &residual);
	ASSERT_EQ(packet.size(), 2U);
	ASSERT_EQ(residual.size(), 0U);
}

TEST(SplitPacket, AdditionalBytesContinuationPacket) {
	unsigned char data[] = { 0x80, 0x00, 0x17 };
	Span<unsigned char> residual = {};
	auto packet = SplitPacket(data, &residual);
	ASSERT_EQ(packet.size(), 2U);
	ASSERT_EQ(residual.data(), &data[2]);
}

TEST(SplitPacket, AdditionalBytesSourcePacket) {
	unsigned char data[] = { 0x83, 0x00, 0x00, 0x00, 0x00, 0x17 };
	Span<unsigned char> residual = {};
	auto packet = SplitPacket(data, &residual);
	ASSERT_EQ(packet.size(), 5U);
	ASSERT_EQ(residual.data(), &data[5]);
}

TEST(SplitPacket, BrokenSynchronizationPacket) {
	unsigned char data[] = { 0x00, 0x00, 0x00, 0x00, 0x80 };
	Span<unsigned char> residual = {};
	auto packet = SplitPacket(data, &residual);
	ASSERT_EQ(packet.size(), 5U);
	ASSERT_EQ(residual.size(), 0U);
}

TEST(SplitPacket, BrokenSynchronizationPacketAdditionalBytes) {
	unsigned char data[] = { 0x00, 0x00, 0x00, 0x00, 0x80, 0x10 };
	Span<unsigned char> residual = {};
	auto packet = SplitPacket(data, &residual);
	ASSERT_EQ(packet.size(), 5U);
	ASSERT_EQ(residual.data(), &data[5]);
}

TEST(SplitPacket, Garbage) {
	unsigned char data[] = { 0x00, 0x00, 0x00, 0x00, 0x10 };
	Span<unsigned char> residual = {};
	auto packet = SplitPacket(data, &residual);
	ASSERT_EQ(packet.size(), 5U);
	ASSERT_EQ(residual.data(), nullptr);
}
