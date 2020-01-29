#pragma once
#include <ostream>
#include <vector>

namespace genetic
{
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
#pragma pack(pop)
	
	inline std::ostream& operator << (std::ostream& out, const UInt32_padded& value)
	{
		out << value.value;
		return out;
	}
	
	struct Entry
	{
		Str_20 values;
		std::vector<UInt16_padded> hosts;

		[[nodiscard]] uint32_t submit_time() const { return values.a; }
		[[nodiscard]] uint32_t slots() const { return values.b; }
		[[nodiscard]] uint32_t cpu_time() const { return values.c; }
		[[nodiscard]] uint32_t non_cpu_time() const { return values.d; }
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

