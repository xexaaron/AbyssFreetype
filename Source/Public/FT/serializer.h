#pragma once
#include <filesystem>
#include <vector>

#include "FT/common.h"

namespace aby::ft {

	enum class ESerializeMode {
		READ,
		WRITE,
	};

	struct SerializeOpts {
		std::filesystem::path file;
		ESerializeMode mode;
	};

	class Serializer {
	public:
		explicit Serializer(const SerializeOpts& opts);

		void save();
		void reset();
		void seek(i64 offset);

		void set_mode(ESerializeMode mode);

		template <typename T>
		void write(const T& data) {
			FT_ASSERT(m_Opts.mode == ESerializeMode::WRITE, "Cannot write when mode is set to read");
			if constexpr (std::is_same_v<T, std::string>) {
				size_t length            = data.size();
				const auto* length_bytes = reinterpret_cast<const std::byte*>(&length);
				m_Data.insert(m_Data.end(), length_bytes, length_bytes + sizeof(length));
				const auto* string_bytes = reinterpret_cast<const std::byte*>(data.data());
				m_Data.insert(m_Data.end(), string_bytes, string_bytes + length);
			} else if constexpr (std::is_same_v<T, const char*>) {
				size_t length     = std::strlen(data);
				const auto* bytes = reinterpret_cast<const std::byte*>(data);
				m_Data.insert(m_Data.end(), bytes, bytes + length);
			} else if constexpr (std::is_trivially_constructible_v<T>) {
				const auto* bytes = reinterpret_cast<const std::byte*>(&data);
				m_Data.insert(m_Data.end(), bytes, bytes + sizeof(T));
			}
		}

		template <typename T>
		T& read(T& buffer) {
			FT_ASSERT(m_Opts.mode == ESerializeMode::READ, "Cannot read when mode is set to write");
			if constexpr (std::is_same_v<T, std::string>) {
				i64 length = 0;
				std::memcpy(&length, &m_Data[m_Offset], sizeof(length));
				m_Offset += sizeof(length);
				buffer.assign(reinterpret_cast<const char*>(&m_Data[m_Offset]), length);
				m_Offset += length;
			} else if constexpr (std::is_same_v<T, const char*>) {
				i64 length = 0;
				constexpr std::byte null{ 0 };
				while (m_Offset + length < m_Data.size() && m_Data[m_Offset + length] != null) {
					++length;
				}
				buffer    = reinterpret_cast<const char*>(&m_Data[m_Offset]);
				m_Offset += length + 1;
			} else if constexpr (std::is_trivially_constructible_v<T>) {
				FT_ASSERT(m_Offset + sizeof(T) <= m_Data.size(), "Out of range");
				std::memcpy(&buffer, &m_Data[m_Offset], sizeof(T));
				m_Offset += sizeof(T);
			}
			return buffer;
		}
	protected:
		void read_file();
		void create_file();
	private:
		SerializeOpts m_Opts;
		i64 m_Offset;
		std::vector<std::byte> m_Data;
	};

} // namespace aby::ft
