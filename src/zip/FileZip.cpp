#include "FileZip.h"

#include <string_view>
#include <fstream>

#include <minizip-ng/mz.h>
#include <minizip-ng/mz_os.h>
#include <minizip-ng/mz_compat.h>
#include <minizip-ng/mz_strm_os.h>

#include <iostream>
#include <numeric>
#include <functional>
#include <Windows.h>

namespace autobackup::zip {

static bool isLargeFile (const std::filesystem::path & filename) {
	std::error_code ec{};
	auto size = std::filesystem::file_size (filename, ec);

	return !ec ? 
		size >= std::numeric_limits<uint32_t>::max () / 4 :
		true;
}

class FileZip::Impl {
public:
	Impl (std::string_view zipFile) :
		m_zipPath (zipFile)
	{ 
		open ();
	}

	Impl (const char * zipFile) :
		m_zipPath (zipFile)
	{
		open ();
	}

	~Impl () {
		close ();
	}

private:
	void open () {
		m_zipArchive = zipOpen64 (m_zipPath.data (), APPEND_STATUS_CREATE);
	}

	bool close () {
		if (m_zipArchive) {
			return zipClose (m_zipArchive, nullptr);
		}
		return true;
	}

	void add (const std::filesystem::path & internalZipFile) {
		zip_fileinfo fileinfo{};
		
		char* unicodeName = (char*)mz_os_utf8_string_create (
			internalZipFile.generic_string ().c_str (), CP_ACP);
		const auto unicodeNameDeleter = [](void ** str) {
			mz_os_utf8_string_delete ((uint8_t**)str);
		};
		std::unique_ptr<void*, decltype(unicodeNameDeleter)> fileInZip ((void**)&unicodeName, unicodeNameDeleter);

		// unicode filename
		int err = zipOpenNewFileInZip4_64 (m_zipArchive,	/* file */
					unicodeName,		/* internal filename */
					&fileinfo,					/* fileinfo */
					nullptr,					/* extrafield_local */
					0,							/* size_extrafield_local */
					nullptr,					/* extrafield_global */
					0,							/* size_extrafield_global */
					nullptr,					/* comment */
					MZ_COMPRESS_METHOD_DEFLATE,	/* compression_method */
					MZ_COMPRESS_LEVEL_DEFAULT,  /* level */
					0,							/* raw */
					0,							/* windowBits */
					0,							/* memLevel */
					0,							/* strategy */
					nullptr,					/* password */
					0,							/* crc_for_crypting */
					MZ_VERSION_MADEBY,
					MZ_ZIP_FLAG_UTF8,
					false						/* zip64 large file */
					);

		if (err == MZ_OK) {
			const auto closeFileInZip = [](void * file) {
				int err = zipCloseFileInZip64 ((zipFile)file);
				if (err != MZ_OK) {
					// send message to logger
				}
			};
			
			std::unique_ptr<void, decltype(closeFileInZip)> fileInZip (m_zipArchive, closeFileInZip);
			writeInZipInternal (internalZipFile);
		}
	}

	void add (const char* internalZipFile) {
		add (std::string_view (internalZipFile));
	}

	void writeInZipInternal (const std::filesystem::path & internalZipFile) {
		constexpr size_t BUFFER_SIZE = std::numeric_limits<int16_t>::max ();

		void* buff = ::operator new[] (BUFFER_SIZE);
		if (buff) {
			const auto deleteBuffer = [](void * p) {
				::operator delete[](p);
			};
			std::unique_ptr<void, decltype(deleteBuffer)> bufferDeleter (buff, deleteBuffer);

			const auto fileClose = [](std::ifstream * ifs) {
				if (ifs) {
					ifs->close ();
					delete ifs;
				}
			};

			std::unique_ptr<std::ifstream, decltype(fileClose)> fileReader (
				new std::ifstream (internalZipFile, std::ios_base::in | std::ios_base::binary),
				fileClose);

			if (fileReader->is_open ()) {
				while (fileReader->read ((char*)buff, BUFFER_SIZE)) {
					auto readSize = fileReader->gcount ();
					if (readSize < BUFFER_SIZE) {
						if (!fileReader->eof ()) {
							//MZ_STREAM_ERROR
							//break
						}
					}
					
					if (readSize > 0) {
						int result = zipWriteInFileInZip (m_zipArchive, buff, static_cast<uint32_t>(readSize));
						if (result != MZ_OK) {
							// error
						}
					}
				}
			}

		}
	}

public:

	const std::string_view m_zipPath;
	zipFile m_zipArchive;

	friend class FileZip;
};


FileZip::FileZip (std::string_view outputFile) :
	impl (new Impl(outputFile))
{ }

FileZip::~FileZip () 
{ 
	if (impl) delete impl; 
}

static std::filesystem::path getInternalPath (const std::filesystem::path& root, const std::filesystem::path& sub) 
{
	auto rootBegin = root.begin ();
	auto rootEnd = root.end ();

	auto subBegin = sub.begin ();
	auto subEnd = sub.end ();

	while (rootBegin != rootEnd && *rootBegin == *subBegin) {
		++rootBegin;
		++subBegin;
	}

	return std::accumulate (subBegin, subEnd,
							std::filesystem::path{}, std::divides{});
}

void FileZip::add (const std::filesystem::path& root, const std::filesystem::path& path)
{
	if (std::filesystem::is_directory (path)) {
		for (const auto & internalFile : std::filesystem::directory_iterator (path)) {
			if (internalFile.is_directory ()) {
				add (root, internalFile);
			}
			else {
				impl->add (getInternalPath (root, internalFile).generic_string ());
			}
		}
	}
	else {
		impl->add (getInternalPath (root, path).generic_string ());
	}
}

}