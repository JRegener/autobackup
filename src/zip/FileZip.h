#include <cstdint>
#include <filesystem>

namespace autobackup::zip {

class FileZip {
public:
	explicit FileZip (std::string_view outputFile);
	~FileZip ();

	void add (const std::filesystem::path& root, const std::filesystem::path & path);

private:
	class Impl; 
	Impl * impl;
};


}