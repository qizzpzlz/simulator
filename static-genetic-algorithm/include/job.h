#pragma once
#include <ostream>
#include <vector>
#include <string>
#include <sstream>
#include <cstring>
#include <iostream>

//#define USE_LEVEL_DB

#ifdef USE_LEVEL_DB
#include "leveldb/db.h"
#include "leveldb/cache.h"
#endif

namespace genetic
{
	constexpr std::size_t DB_CACHE_SIZE = 1024 * 1048576; // 1GB
	static const std::string DB_PATH = "../static-genetic-algorithm/eligible-hosts";

	static constexpr bool use_old_entry = true;
	
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
	struct Str_20
	{
		UInt32_padded a;
		UInt32_padded b;
		UInt32_padded c;
		UInt32_padded d;
	};
	struct _old_RawJoBMetaData
	{
		Str_20 values;
		std::vector<UInt16_padded> hosts;
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

	template <bool Old>
	struct _EntryImpl{};
	
	template <>
	struct _EntryImpl<false>
	{
		RawJobMetaData values;
#ifdef USE_LEVEL_DB
		inline static leveldb::DB* db;
		inline static leveldb::ReadOptions read_option;
#endif

		[[nodiscard]] uint32_t submit_time() const { return values.a; }
		[[nodiscard]] uint32_t slots() const { return values.b; }
		[[nodiscard]] uint32_t cpu_time() const { return values.c; }
		[[nodiscard]] uint32_t non_cpu_time() const { return values.d; }
		[[nodiscard]] uint32_t db_entry_index() const { return values.e; }
		[[nodiscard]] uint16_t num_eligible_hosts() const { return values.f; }

		[[nodiscard]] std::vector<uint16_t> get_hosts() const noexcept
		{
#ifdef USE_LEVEL_DB
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
#endif
			return std::vector<uint16_t>{};
		}

		[[nodiscard]] uint16_t get_host(const int& index) const noexcept
		{
#ifdef USE_LEVEL_DB
			std::string value;
			leveldb::Status s = db->Get(read_option, std::to_string(db_entry_index()), &value);

			if (!s.ok()) {
				std::cerr << db_entry_index() << '\t' << s.ToString() << std::endl;
			}

			uint16_t* eligible_hosts = reinterpret_cast<uint16_t*>(value.data());

			return *(eligible_hosts + index);
#endif
			return 0;
		}

#ifdef USE_LEVEL_DB
	private:
		inline static bool db_initialized = []
		{
			//leveldb::Options options;
			//options.create_if_missing = true;
			//options.block_cache = leveldb::NewLRUCache(1024 * 1048576);  // 1GB cache
			//leveldb::Status s = leveldb::DB::Open(options, genetic::DB_PATH, &db);
			//if (!s.ok()) std::cerr << s.ToString() << std::endl;

			//read_option = leveldb::ReadOptions();

			return true;
		}();
#endif

		static _EntryImpl<false> from_binary(char** ptr)
		{
			char* current = *ptr;
			auto* front_ptr = reinterpret_cast<RawJobMetaData*>(current);
			_EntryImpl<false> entry;
			entry.values = *front_ptr;

			*ptr = current + sizeof(RawJobMetaData);
			return entry;
		}
	};

	/**
	 * Specialization for old data type.
	 */
	template<>
	struct _EntryImpl<true>
	{
		_old_RawJoBMetaData data;
		[[nodiscard]] uint32_t submit_time() const noexcept { return data.values.a; }
		[[nodiscard]] uint32_t slots() const noexcept { return data.values.b; }
		[[nodiscard]] uint32_t cpu_time() const noexcept { return data.values.c; }
		[[nodiscard]] uint32_t non_cpu_time() const noexcept { return data.values.d; }
		[[nodiscard]] std::vector<UInt16_padded>& get_hosts() noexcept { return data.hosts; }
		[[nodisacrd]] uint16_t num_eligible_hosts() const noexcept { return data.hosts.size(); }
		[[nodiscard]] uint16_t get_host(std::size_t index) const { return data.hosts.at(index); }

		static _EntryImpl<true> from_binary(char** ptr)
		{
			char* current = *ptr;
			
			const Str_20* front_ptr = reinterpret_cast<const Str_20*>(current);
			current += sizeof(Str_20);

			_EntryImpl<true> entry;
			entry.data.values = *front_ptr;

			UInt16_padded array_size = *reinterpret_cast<const UInt16_padded*>(current);
			current += sizeof(UInt16_padded);

			const std::size_t array_length_in_bytes = sizeof(UInt16_padded) * array_size.value;

			entry.data.hosts = std::vector<UInt16_padded>(array_size.value);
			UInt16_padded* hosts_ptr = entry.data.hosts.data();
			memcpy(hosts_ptr, current, array_length_in_bytes);
			current += array_length_in_bytes;

			*ptr = current;
			return entry;
		}
	};

	using Entry = _EntryImpl<use_old_entry>;
	
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

