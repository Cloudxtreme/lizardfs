#include "common/platform.h"
#include "admin/metadataserver_status_command.h"

#include <iomanip>
#include <iostream>

#include "common/cltoma_communication.h"
#include "common/matocl_communication.h"

std::string MetadataserverStatusCommand::name() const {
	return "metadataserver-status";
}

void MetadataserverStatusCommand::usage() const {
	std::cerr << name() << " <master ip> <master port>" << std::endl;
	std::cerr << "    Prints status of a master or shadow master server" << std::endl;
}

LizardFsProbeCommand::SupportedOptions MetadataserverStatusCommand::supportedOptions() const {
	return { {kPorcelainMode, kPorcelainModeDescription} };
}

void MetadataserverStatusCommand::run(const Options& options) const {
	if (options.arguments().size() != 2) {
		throw WrongUsageException("Expected <master ip> and <master port> for " + name());
	}

	ServerConnection connection(options.argument(0), options.argument(1));
	MetadataserverStatus s = MetadataserverStatusCommand::getStatus(connection);

	if (options.isSet(kPorcelainMode)) {
		std::cout << s.personality << "\t" << s.serverStatus << "\t"
				<< s.metadataVersion << std::endl;
	} else {
		std::cout << "     personality: " << s.personality << std::endl;
		std::cout << "   server status: " << s.serverStatus << std::endl;
		std::cout << "metadata version: " << s.metadataVersion << std::endl;
	}
}

MetadataserverStatus MetadataserverStatusCommand::getStatus(ServerConnection& connection) {
	std::vector<uint8_t> request;
	request = cltoma::metadataserverStatus::build(1);
	auto response = connection.sendAndReceive(request, LIZ_MATOCL_METADATASERVER_STATUS);

	uint32_t messageId;
	uint8_t status;
	uint64_t metadataVersion;
	matocl::metadataserverStatus::deserialize(response, messageId, status, metadataVersion);

	std::string personality, serverStatus;
	switch (status) {
	case LIZ_METADATASERVER_STATUS_MASTER:
		personality = "master";
		serverStatus = "running";
		break;
	case LIZ_METADATASERVER_STATUS_SHADOW_CONNECTED:
		personality = "shadow";
		serverStatus = "connected";
		break;
	case LIZ_METADATASERVER_STATUS_SHADOW_DISCONNECTED:
		personality = "shadow";
		serverStatus = "disconnected";
		break;
	default:
		personality = "<unknown>";
		serverStatus = "<unknown>";
	}
	return MetadataserverStatus{personality, serverStatus, metadataVersion};
}
