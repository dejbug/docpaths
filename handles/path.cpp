#include "path.h"
#include <WinIoCtl.h>

path::SplitPath::SplitPath(char const * fullpath)
		: fullpath(nullptr), path(nullptr),
		name(nullptr), ext(nullptr) {

	try {
		if(!fullpath || !*fullpath)
			throw std::invalid_argument(
				"SplitPath(fullpath): must be string");
		if(strlen(fullpath) < 3)
			throw std::invalid_argument(
				"SplitPath(fullpath): must have 3+ characters");
		
		//TODO: Check whether fullpath is a valid filepath.
		// For this we would better create our own filepath
		// parser anyway, and not use GetFullPathName. When
		// implementing it, we must not forget to also handle
		// the UNC filepath format.
		
		this->fullpath = _strdup(fullpath);
		if(!this->fullpath)
			throw ex::OutOfMemoryError("SplitPath::fullpath");
		
		path = new char[MAX_PATH + 1];
		if(!path) throw ex::OutOfMemoryError("SplitPath::path");
		
		LPSTR filepart;
		DWORD len = GetFullPathName(this->fullpath,
			MAX_PATH, path, &filepart);
		if(!len) throw PathError("SplitPath: full path emtpy");

		// TODO: Add support for UNC-stype extra long paths.

		if(len >= MAX_PATH)
			throw PathError("SplitPath: full path > MAX_PATH");
		
		name = _strdup(filepart ? filepart : "");
		if(!name) throw ex::OutOfMemoryError("SplitPath::name");
		
		if(filepart) *filepart = '\0';
		
		char * dot = strrchr(name, '.');
		ext = strdup(dot ? dot+1 : "");
		if(!ext) throw ex::OutOfMemoryError("SplitPath::ext");
		
		if(dot) *dot = '\0';
	}
	
	catch(...) {
		//TODO: Use shared_ptr instead of try-catch block.

		if(this->fullpath) delete[] this->fullpath;
		if(path) delete[] path;
		if(name) delete[] name;
		if(ext) delete[] ext;
		throw;
	}
}

path::SplitPath::~SplitPath() {
	delete[] fullpath;
	delete[] path;
	delete[] name;
	delete[] ext;
}

size_t path::GetFileSize(char const * path) {
	HANDLE handle = CreateFile(path, 0, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL |
		FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED |
		FILE_FLAG_RANDOM_ACCESS, NULL);
	if(INVALID_HANDLE_VALUE == handle)
		throw PathError("GetFileSize: file won't open;"
			" make sure it exists");
	size_t size = ::GetFileSize(handle, NULL);
	CloseHandle(handle);
	return size;
}

path::FileInfo::FileInfo(char const * path)
		: exists(false), size(0), ctime(0), atime(0) {

	HANDLE handle = CreateFile(path, 0, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL |
		FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED |
		FILE_FLAG_RANDOM_ACCESS, NULL);
		
	if(INVALID_HANDLE_VALUE == handle)
	{
		DWORD const error = GetLastError();
		if(ERROR_FILE_NOT_FOUND != error)
			throw PathError("FileInfo: can't open file");
		
		exists = false;
		return;
	}
	
	exists = true;
	
	size = ::GetFileSize(handle, NULL);
	
	FILETIME fctime, fatime, lctime, latime;
	if(GetFileTime(handle, &fctime, &fatime, NULL)) {
		if(FileTimeToLocalFileTime(&fctime, &lctime))
			ctime = lctime.dwLowDateTime;
		if(FileTimeToLocalFileTime(&fatime, &latime))
			atime = latime.dwLowDateTime;
	}
	
	CloseHandle(handle);
}

path::FileInfo::~FileInfo() {
}

path::DosPathTranslator::DosPathTranslator() {
	EnumerateDrives();
	QueryMapping();
}

void path::DosPathTranslator::Print() const {
	for(auto it = drivemap.begin(); it != drivemap.end(); ++it)
		printf("%s => %s\n", it->first.c_str(), it->second.c_str());
}

bool path::DosPathTranslator::Translate(typ::string & path) {
	bool result = false;

	for(auto it = drivemap.begin(); it != drivemap.end(); ++it)
		if(0 == path.find(it->first)) {
			path.replace(0, it->first.length(), it->second);
			result = true;
		}
	
	/* NOTE: With the following code i'm addressing a particular
	problem i keep having, where a path to one of my (external)
	USB HDDs would come out in a format that i have never seen
	before. Instead of "F:\\New\\_Literature\\Society of Mind.pdf"
	the path would read "C:3\\New\\_Literature\\Society of Mind.pdf".
	I've noticed a regularity where the post-colon index (the "3"
	in this example) would match the partition number in the DOS
	device name of the respective HDD. At the time of the above
	example, an entry in the DosPathTranslator map read as follows:
	"\Device\HarddiskVolume23 => F:" and another
	"\Device\HarddiskVolume2 => C:". So it became clear what had
	to be done (though i still am at a loss about the reasons for
	this strange formatting to be happening, even while i'm
	suspecting that it has something to do with my disconnecting
	the HDDs occasionally). This is on WinXP SP3.
	*/
	
	if(':' == path[1] && isdigit(path[2]))
		for(auto i=drivemap.begin(); i != drivemap.end(); ++i)
			if(0 == path.find(i->second))
				for(auto j=drivemap.begin(); j != drivemap.end(); ++j)
					if(0 == j->first.find(i->first)
						&& j->first[i->first.length()] == path[2]) {
						path.replace(0, 3, j->second);
						return true;
					}
	
	return result;
}

void path::DosPathTranslator::EnumerateDrives() {
	DWORD flags = GetLogicalDrives();
	char drive[3] = " :";
	for(int i=0; i<32; ++i)
		if(((flags >> i) & 1) == 1) {
			drive[0] = 'A' + i;
			drives.push_back(drive);
		}
}

void path::DosPathTranslator::QueryMapping() {
	char buf[MAX_PATH];
	
	for(auto it = drives.begin(); it != drives.end(); ++it) {
		if(QueryDosDevice(it->c_str(), buf, sizeof(buf)))
			drivemap[buf] = it->c_str();
		else
			drivemap[buf] = typ::string("");
	}
}


path::FileFinder::FileFinder(LPCSTR pattern)
		: handle(NULL), search(pattern) {
}

path::FileFinder::~FileFinder() {
	if(INVALID_HANDLE_VALUE != handle)
		FindClose(handle);
}

bool path::FileFinder::Next() {
	if(INVALID_HANDLE_VALUE == handle)
		return false;
	
	if(!handle) {
		handle = FindFirstFile(search.c_str(), &data);
		return INVALID_HANDLE_VALUE != handle;
	}
	
	return TRUE == FindNextFile(handle, &data);
}

LPCSTR path::FileFinder::GetFileName() const {
	return data.cFileName;
}

//szPath without trailing backslash like:
//  "\\\\.\\X:"
//  "\\\\.\\PhysicalDrive0"
//  "\\\\\?\\Volume{433619ed-c6ea-11d9-a3b2-806d6172696f}
DWORD path::GetDeviceBusTypeId(LPCSTR szPath)
{
	/* TODO: I'm not sure that exceptions are necessary here.
	I will need to check the possible return values and see
	which values are free for me to return error codes with.
	My main concern is efficiency: one may have large listings
	of paths that will cause this function to throw if the USB
	drive these paths are pointing to were to be detached.
	*/

	HANDLE hDevice = CreateFile(szPath, 0,
		FILE_SHARE_READ|FILE_SHARE_WRITE, NULL,
		OPEN_EXISTING, 0, NULL);
	
	if(INVALID_HANDLE_VALUE == hDevice)
		//return -1;
		throw PathError("GetDeviceBusTypeId: unable to open path");
	
	DWORD dwOutBytes = 0;           // IOCTL output length
	STORAGE_PROPERTY_QUERY query;   // input param for query
	
	// specify the query type
	query.PropertyId = StorageDeviceProperty;
	query.QueryType = PropertyStandardQuery;
	
	char OutBuf[1024] = {0};  // good enough, usually about 100 bytes
	PSTORAGE_DEVICE_DESCRIPTOR pDevDesc =
		reinterpret_cast<PSTORAGE_DEVICE_DESCRIPTOR>(OutBuf);
	pDevDesc->Size = sizeof(OutBuf);
	
	// query using IOCTL_STORAGE_QUERY_PROPERTY 
	BOOL ok = DeviceIoControl(hDevice,	// device handle
		IOCTL_STORAGE_QUERY_PROPERTY,	// info of device property
		&query, sizeof(STORAGE_PROPERTY_QUERY),	// input data buffer
		pDevDesc, pDevDesc->Size,		// output data buffer
		&dwOutBytes,					// out's length
		NULL);
	
	CloseHandle(hDevice);
	
	if(ok)
		return pDevDesc->BusType;
	
	throw PathError("GetDeviceBusTypeId: unable to query device");
}

LPCSTR path::GetBusTypeString(DWORD busTypeId) {
	static const char busTypes[][32] = { 
	  "unknown",
	  "scsi",
	  "atapi",
	  "ata",
	  "1394",
	  "ssa",
	  "fibre",
	  "usb",
	  "raid",
	  "iscsi",
	  "sas",
	  "sata",
	  "sd",
	  "mmc",
	  "virtual",
	  "filebackedvirtual"
	};
	
	if(busTypeId >= 0 && busTypeId <= 15)
		return busTypes[busTypeId];
	
	return NULL;
}

//e.g. 'C'
DWORD path::GetDeviceBusTypeIdByDriveLetter(CHAR driveLetter)
{
	typ::string path("\\\\.\\");
	path += driveLetter;
	path += ":";
	return GetDeviceBusTypeId(path.data());
}

//e.g. "PhysicalDrive0"
DWORD path::GetDeviceBusTypeIdByPhysicalDiskPath(LPCSTR physicalPath)
{
	typ::string path("\\\\.\\");
	path += physicalPath;
	return GetDeviceBusTypeId(path.data());
}

//e.g. "433619ed-c6ea-11d9-a3b2-806d6172696f"
DWORD path::GetDeviceBusTypeIdByVolumeGUIDString(LPCSTR guidString)
{
	typ::string path("\\\\\?\\Volume{");
	path += guidString;
	path += "}";
	return GetDeviceBusTypeId(path.data());
}
