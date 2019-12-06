#pragma once

namespace ClusterSimulator
{
	class Config
	{
		Config();
	public:
		static Config parse(char* path);
	};
}
