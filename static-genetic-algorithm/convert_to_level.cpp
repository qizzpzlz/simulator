#include <string>
#include <string_view>
#include <fstream>
#include <iostream>
#include <vector>
#include <filesystem>
#include <cstring>
#include "job.h"
#include "leveldb/db.h"
#include "leveldb/cache.h"

using namespace genetic;

int main(int argc, char* argv[])
{
	leveldb::Status s;
	leveldb::DB* db = Entry::db;
	leveldb::WriteOptions writeOptions = leveldb::WriteOptions();
	writeOptions.sync = true;

	constexpr char file_name[] = "../static-genetic-algorithm/job-eligibility-with-hosts.bin";

	auto file = std::ifstream(file_name, std::ios::binary | std::ios::ate);

	if (!file)
		throw std::runtime_error("Can't find a binary file for the job table.");

	file.seekg(0, std::ifstream::end);
	const size_t size = file.tellg();
	file.seekg(0, std::ifstream::beg);

	if (size == 0) return 0;

	// buffer to load line size in bytes, uint_32
	std::vector<std::byte> meta_buffer(22);
	// buffer to load line data, format: {job meta data}{eligiblehost array in uint_16}
	std::vector<std::byte> eligible_hosts_buffer(50000);

	std::size_t read = 0;
	std::size_t count = 0;
	int line_size;

	std::cout << "total size of file " << size << std::endl;

	while (read < size)
	{
		// read total line size from first 4 byte
		if (!file.read(reinterpret_cast<char*>(meta_buffer.data()), 22))
			throw std::runtime_error("");

		RawJobMetaData* front_ptr = reinterpret_cast<RawJobMetaData*>(meta_buffer.data());
		Entry entry;
		entry.values = *front_ptr;

		std::size_t eligible_hosts_length_in_bytes = sizeof(uint16_t) * entry.num_eligible_hosts();
		std::string hostsBinaryString(eligible_hosts_length_in_bytes, ' ');

		// read line based on line size
		if (!file.read(reinterpret_cast<char*>(hostsBinaryString.data()), eligible_hosts_length_in_bytes))
			throw std::runtime_error("");

		// write to db instance
		s = db->Put(writeOptions, std::to_string(entry.db_entry_index()), hostsBinaryString);
		if (!s.ok()) {
			std::cerr << s.ToString() << std::endl;
		}

		// 4 for line_size
		read += 22 + eligible_hosts_length_in_bytes;

		if(count % 10000 == 0) std::cout << entry.db_entry_index()<< '\t' << entry.num_eligible_hosts() << '\t' << entry.submit_time() << '\t' << entry.num_eligible_hosts() << '\t' << read << std::endl;
		count++;
	}

	delete db;

	return 0;
}

