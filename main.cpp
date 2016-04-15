#include <iostream>
#include <cpprest/http_client.h>
#include <boost/program_options.hpp>

enum class FormatType {
	json
};

FormatType formatFromString(const std::string& typeStr)
{
	if (typeStr == "json")
		return FormatType::json;

	return FormatType::json;
}

std::ostream& operator << (std::ostream& ostr, const FormatType& type )
{
	switch (type) {
	case FormatType::json:
		return ostr << "json";
	default:
		return ostr;
	}
}

std::istream& operator >> (std::istream& istr, FormatType& type)
{
	std::string typeStr;
	istr >> typeStr;
	type = formatFromString(typeStr);
	return istr;
}

pplx::task<std::string> RequestJsonAsync(std::string serverAddress, FormatType)
{
	web::http::client::http_client client(serverAddress);
	return client.request(web::http::methods::GET)
		.then ([] (web::http::http_response response) -> pplx::task<web::json::value>
		{
			if (response.status_code() == web::http::status_codes::OK)
				return response.extract_json();
			throw std::runtime_error("Unable to get response from server");
		})
		.then([] (pplx::task<web::json::value> jsonTask)
		{
			return jsonTask.get().at(U("time_zone")).as_string();
		});
}

using namespace std;
namespace ProgramOption = boost::program_options;

template<typename Type, typename Value>
bool exist(const Type& type, const Value& value)
{
	return type.find(value) != type.end();
}

std::string extractAddress(ProgramOption::variables_map& variableMap)
{
	return exist(variableMap, "address") ? variableMap["address"].as<std::string>()
			: std::string("http://freegeoip.net/json/");
}

FormatType extractFormatType(ProgramOption::variables_map& variableMap)
{
	return exist(variableMap, "type") ? variableMap["type"].as<FormatType>()
			: FormatType::json;
}

boost::program_options::variables_map initializeParserArgument(int argc, char *argv[])
{
	ProgramOption::options_description optionDescription("Allowed option");
	optionDescription.add_options()
			("help", "produce help message")
			("type", ProgramOption::value<FormatType>(), "format type: for now can be only json [json]")
			("address", ProgramOption::value<std::string>(), "server address [http://freegeoip.net/json/]")
			;

	ProgramOption::variables_map variableMap;
	ProgramOption::store(ProgramOption::parse_command_line(argc, argv, optionDescription), variableMap);
	ProgramOption::notify(variableMap);

	return variableMap;
}

int main(int argc, char *argv[])
{
	ProgramOption::variables_map variableMap = initializeParserArgument(argc, argv);

	auto serverAddress = extractAddress(variableMap);
	auto formatType = extractFormatType(variableMap);

	std::cout << "address is " << serverAddress << '\n'
			  << "type is " << formatType << endl;

	ucout << "time zone is: " << RequestJsonAsync(serverAddress, formatType).get() << std::endl;
	return 0;
}
