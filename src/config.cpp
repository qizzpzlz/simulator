#in/*clude <config.h>
#include <fstream>
#include "json11.hpp"

namespace ClusterSimulator
{
	Config Config::parse(std::string_view path)
	{
		using namespace json11;

		std::ifstream fin;
		fin.open(path);
	}
}*/
