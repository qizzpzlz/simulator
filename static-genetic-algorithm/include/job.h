#pragma once
#include <ostream>
#include <vector>
#include <string>
#include <sstream>
#include <cstring>
#include <iostream>
#include "leveldb/db.h"
#include "leveldb/cache.h"

namespace genetic
{
	constexpr std::size_t DB_CACHE_SIZE = 1024 * 1048576; // 1GB
	static const std::string DB_PATH = "../static-genetic-algorithm/eligible-hosts";

#pragma pack(push, 1)
	struct UInt32_padded
	{
		uint32_t value;
		std::byte padding;

		operator uint32_t() const { return value; }
		friend std::ostream& operator << (std::ostream& out, const UInt32_padded& value);
	};
	struct UInt16_padded
	{
		uint16_t value;
		std::byte padding;

		operator uint16_t() const { return value; }
	};

	struct RawJobMetaData
	{
		uint32_t a;
		uint32_t b;
		uint32_t c;
		uint32_t d;
		uint32_t e;
		uint16_t f;
	};
#pragma pack(pop)
	
	inline std::ostream& operator << (std::ostream& out, const UInt32_padded& value)
	{
		out << value.value;
		return out;
	}
	
	struct Entry
	{
		RawJobMetaData values;
		inline static leveldb::DB* db;
		inline static leveldb::ReadOptions read_option;

		[[nodiscard]] uint32_t submit_time() const { return values.a; }
		[[nodiscard]] uint32_t slots() const { return values.b; }
		[[nodiscard]] uint32_t cpu_time() const { return values.c; }
		[[nodiscard]] uint32_t non_cpu_time() const { return values.d; }
		[[nodiscard]] uint32_t db_entry_index() const { return values.e; }
		[[nodiscard]] uint16_t num_eligible_hosts() const { return values.f; }

		[[nodiscard]] std::vector<uint16_t> get_hosts() const noexcept
		{
			std::string value;
			leveldb::Status s = db->Get(read_option, std::to_string(db_entry_index()), &value);

			if (!s.ok()) {
				std::cerr << db_entry_index() << '\t' << s.ToString() << std::endl;
			}

			// // create vector from retrieved binary uint16 string
			std::vector<uint16_t> eligible_hosts;
			eligible_hosts.reserve(num_eligible_hosts());
			memcpy(eligible_hosts.data(), value.data(), num_eligible_hosts() * sizeof(uint16_t));

			return eligible_hosts;
		}

		[[nodiscard]] uint16_t get_host(const int& index) const noexcept
		{
			std::string value;
			leveldb::Status s = db->Get(read_option, std::to_string(db_entry_index()), &value);

			if (!s.ok()) {
				std::cerr << db_entry_index() << '\t' << s.ToString() << std::endl;
			}

			uint16_t* eligible_hosts = reinterpret_cast<uint16_t*>(value.data());

			return *(eligible_hosts + index);
		}

	private:
		inline static bool db_initialized = []
		{
			leveldb::Options options;
			options.create_if_missing = true;
			options.block_cache = leveldb::NewLRUCache(1024 * 1048576);  // 1GB cache
			leveldb::Status s = leveldb::DB::Open(options, genetic::DB_PATH, &db);
			if (!s.ok()) std::cerr << s.ToString() << std::endl;

			read_option = leveldb::ReadOptions();

			return true;
		}();
	};
	
	class Job
	{
	public:
		explicit Job(const Entry& entry)
			: entry_ptr_{&entry}
		{
			
		}

		[[nodiscard]] uint32_t submit_time() const noexcept
		{
			return entry_ptr_->submit_time();
		}

		[[nodiscard]] uint32_t cpu_time() const noexcept
		{
			return entry_ptr_->cpu_time();
		}

		[[nodiscard]] uint32_t non_cpu_time() const noexcept
		{
			return entry_ptr_->non_cpu_time();
		}

		[[nodiscard]] uint32_t slots() const noexcept
		{
			return entry_ptr_->slots();
		}

		void set_finish_time(uint32_t estimated_time)
		{
			finish_time_ = submit_time() + estimated_time;
		}

		void set_finish_time(uint32_t estimated_time, uint32_t delay)
		{
			finish_time_ = submit_time() + estimated_time + delay;
		}

		[[nodiscard]] uint32_t finish_time() const noexcept
		{
			return finish_time_;
		}
		
	private:
		uint32_t finish_time_;
		const Entry* entry_ptr_;
	};
}

