/*
 * Main.cpp
 *
 *  Created on: Apr 17, 2020
 *      Author: cf
 */

#include <QtCore/QCommandLineParser>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

#include <spdlog/spdlog.h>

#include <fuchsch/EventParser/EventParser.h>
#include <fuchsch/EventParser/ExceptionTracer.h>
#include <fuchsch/EventParser/InstrumentationTracer.h>
#include <fuchsch/EventParser/PacketSplitter.h>
#include <fuchsch/EventParser/TimestampProcessor.h>

using namespace ::fuchsch::eventparser;

class TraceReceiver: public fuchsch::utilities::Observer<InstrumentationTrace> {
public:

	void operator()(InstrumentationTrace const &trace) noexcept override {
		spdlog::info("[{0}] {1}: {2}", trace.timestamp.count(), trace.port, trace.data);
	}

};

static void Run(QString portName, int baud) {
	PacketSplitter splitter;
	TimestampProcessor processor;
	InstrumentationTracer tracer;
	TraceReceiver receiver;
	splitter >> processor >> tracer >> receiver;

	QSerialPort port(portName);
	if (!port.open(QIODevice::ReadWrite)) {
		spdlog::error(port.errorString().toStdString());
		return;
	}
	port.setDataBits(QSerialPort::Data8);
	port.setStopBits(QSerialPort::OneStop);
	port.setFlowControl(QSerialPort::HardwareControl);
	port.setBaudRate(baud, QSerialPort::AllDirections);

	while (true) {
		auto bytes = port.readAll();
		port.waitForReadyRead(0);
		splitter({ reinterpret_cast<unsigned char *>(bytes.begin()), static_cast<size_t>(bytes.count()) });
	}
}

int main(int argc, char *argv[]) {

	spdlog::info("Starting Event Viewer");

	QCommandLineParser parser;
	parser.setApplicationDescription("Receives Cortex-M trace data");
	parser.addHelpOption();
	parser.addVersionOption();
	parser.addOptions({
		{{"p", "port"}, QCoreApplication::translate("main", "Port to connect to"), QCoreApplication::translate("main", "port")},
		{{"b", "baud"}, QCoreApplication::translate("main", "Baud rate used for the connection"), QCoreApplication::translate("main", "baud")}
	});

	QCoreApplication app(argc, argv);
	QCoreApplication::setApplicationName("Event Viewer");
	QCoreApplication::setApplicationVersion("1.0");

	parser.process(app);
	QString port;
	int baud = 1000000;
	if (parser.isSet("port")) {
		port = parser.value("port");
	}
	if (parser.isSet("baud")) {
		baud = parser.value("baud").toInt();
	}

	spdlog::info("Connecting to \"{0}\" at {1}", port.toStdString(), baud);

	Run(port, baud);

	return 0;
}
