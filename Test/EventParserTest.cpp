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

/******************************************************************************
 * Parser Tests
 *****************************************************************************/

TEST(EventParserTest, ParseSynchronizationPacket) {
	unsigned char data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x80};
	auto event = ParseTPIUPacket(data);
	ASSERT_TRUE(std::holds_alternative<SynchronizationPacket>(event));
}

TEST(EventParserTest, ParseOverflowPacket) {
	unsigned char data[] = {0x70};
	auto event = ParseTPIUPacket(data);
	ASSERT_TRUE(std::holds_alternative<OverflowPacket>(event));
}

TEST(EventParserTest, ParseLocalTimestampPacket) {
	unsigned char data[] = {0xF0, 0x80, 0x00};
	auto event = ParseTPIUPacket(data);
	ASSERT_TRUE(std::holds_alternative<LocalTimestampPacket>(event));
}

TEST(EventParserTest, ParseShortLocalTimestampPacket) {
	unsigned char data[] = {0x30};
	auto event = ParseTPIUPacket(data);
	ASSERT_TRUE(std::holds_alternative<LocalTimestampPacket>(event));
}

TEST(EventParserTest, ParseGlobalTimestamp1Packet) {
	unsigned char data[] = {0x94, 0x80, 0x80, 0xC7, 0x00};
	auto event = ParseTPIUPacket(data);
	ASSERT_TRUE(std::holds_alternative<GlobalTimestamp1Packet>(event));
}

TEST(EventParserTest, ParseGlobalTimestamp2Packet) {
	unsigned char data[] = {0xB4, 0x80, 0x80, 0xC7, 0x01};
	auto event = ParseTPIUPacket(data);
	ASSERT_TRUE(std::holds_alternative<GlobalTimestamp2Packet>(event));
}

TEST(EventParserTest, ParseExtensionPacket) {
	unsigned char data[] = {0x78};
	auto event = ParseTPIUPacket(data);
	ASSERT_TRUE(std::holds_alternative<ExtensionPacket>(event));
}

TEST(EventParserTest, ParseInstrumentationPacket) {
	unsigned char data[] = {0x0B, 0x00, 0x00, 0x00, 0x00};
	auto event = ParseTPIUPacket(data);
	ASSERT_TRUE(std::holds_alternative<InstrumentationPacket>(event));
}

TEST(EventParserTest, ParseEventCounterPacket) {
	unsigned char data[] = {0x05, 0x3};
	auto event = ParseTPIUPacket(data);
	ASSERT_TRUE(std::holds_alternative<EventCounterPacket>(event));
}

TEST(EventParserTest, ParseExceptionTracePacket) {
	unsigned char data[] = {0x0E, 0x10, 0x10};
	auto event = ParseTPIUPacket(data);
	ASSERT_TRUE(std::holds_alternative<ExceptionTracePacket>(event));
}

TEST(EventParserTest, ParsePCSamplingPacket) {
	unsigned char data[] = {0x17, 0x12, 0x34, 0x56, 0x78};
	auto event = ParseTPIUPacket(data);
	ASSERT_TRUE(std::holds_alternative<PCSamplingPacket>(event));
}

TEST(EventParserTest, ParseDataTracePacket) {
	unsigned char data[] = {0x8D, 0x55};
	auto event = ParseTPIUPacket(data);
	ASSERT_TRUE(std::holds_alternative<DataTracePacket>(event));
}
