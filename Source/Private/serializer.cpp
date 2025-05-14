#include "FT/serializer.h"

#include <fstream>

namespace aby::ft {

	Serializer::Serializer(const SerializeOpts& opts) :
	    m_Opts(opts), m_Offset(0) {
		set_mode(opts.mode);
	}

	void Serializer::reset() {
		m_Data.clear();
		m_Offset = 0;
	}
	void Serializer::seek(i64 offset) {
		m_Offset += offset;
		FT_ASSERT(m_Offset >= 0, "Out of range");
		FT_ASSERT(m_Offset < static_cast<int64_t>(m_Data.size()), "Out of range");
	}
	void Serializer::set_mode(ESerializeMode mode) {
		m_Opts.mode = mode;
		if (mode == ESerializeMode::READ) {
			reset();
			read_file();
		} else if (mode == ESerializeMode::WRITE) {
			create_file();
		}
	}
	void Serializer::save() {
		if (m_Data.empty()) {
			FT_WARN("Attempting to save serialized data but Serializer::m_Data is empty");
			return;
		}
		std::ofstream ofs(m_Opts.file, std::ios::binary);
		if (ofs.is_open()) {
			ofs.write(reinterpret_cast<const char*>(m_Data.data()), m_Data.size());
			ofs.close();
		} else {
			FT_ERROR("Failed to open file for writing: {}", m_Opts.file.string());
		}
	}

	void Serializer::read_file() {
		std::ifstream ifs(m_Opts.file, std::ios::binary);
		if (ifs.is_open()) {
			ifs.seekg(0, std::ios::end);
			std::streamsize size = ifs.tellg();
			ifs.seekg(0, std::ios::beg);
			m_Data.resize(size);
			ifs.read(reinterpret_cast<char*>(m_Data.data()), size);
			ifs.close();
		} else {
			FT_ERROR("Failed to open file for reading: {}", m_Opts.file.string());
		}
	}

	void Serializer::create_file() {
		if (!std::filesystem::exists(m_Opts.file.parent_path())) {
			std::filesystem::create_directories(m_Opts.file.parent_path());
		}
		std::ofstream ofs(m_Opts.file, std::ios::binary | std::ios::trunc);
		if (ofs.is_open()) {
			ofs.close();
		} else {
			FT_ERROR("Failed to open file for writing: {}", m_Opts.file.string());
		}
	}

} // namespace aby::ft
