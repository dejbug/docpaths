#include "handles.h"

handles::ProcessIdFinder::ProcessIdFinder()
		: result(STATUS_SUCCESS) {

	auto ntdll = sys::Dll("ntdll.dll");
	
	ULONG size = GetProcessInformationMinimalOutputBufferSize();
	
	buffer.reset(new char[size]);
	result = NtQuerySystemInformation(SystemProcessInformation,
		reinterpret_cast<PVOID>(buffer.get()), size, nullptr);
	
	if(STATUS_SUCCESS != result)
		throw HandlesError("ProcessIdFinder:"
			" unable to query process information\n");
}

void handles::ProcessIdFinder::Find(PIDNAMES &pidnames,
		char const * text) {

	if(!text || !*text)
		throw HandlesError("ProcessIdFinder::Find(text):"
			" must be a valid string");

	for(int offset=0, i=0; true; ++i) {
		nt::SYSTEM_PROCESS_INFORMATION &spi =
			*reinterpret_cast<nt::SYSTEM_PROCESS_INFORMATION*>(
				buffer.get() + offset);
		offset += spi.NextEntryOffset;
		
		text::Ansi ansi(spi.ImageName);
		if(Matches(ansi.GetText(), ansi.GetLength(),
			text, strlen(text))) {

			pidnames.push_back(PIDNAME(reinterpret_cast<ULONG>(
				spi.UniqueProcessId), typ::string(ansi.GetText())));
		}
		
		if(!spi.NextEntryOffset)
			break;
	}
}

bool handles::ProcessIdFinder::Matches(char const * p,
		int plen, char const * s, int slen) {

	if(plen < slen) return false;
	// 's'earchstring must be a strict prefix of 'p'rocessName.

	int minlen = plen <= slen ? plen : slen;
	return minlen && !_strnicmp(p, s, minlen);
}

ULONG handles::ProcessIdFinder::
	GetProcessInformationMinimalOutputBufferSize() {

	ULONG requiredSize = 0;
	result = NtQuerySystemInformation(SystemProcessInformation,
		nullptr, 0, &requiredSize);
	if(STATUS_INFO_LENGTH_MISMATCH != result)
		throw HandlesError("ProcessIdFinder::"
			"GetProcessInformationMinimalOutputBufferSize(): "
				"unable to query output data size");
	return requiredSize;
}

handles::RemoteHandleLocalizer::RemoteHandleLocalizer()
		: myProcess(nullptr), myPid(-1) {

	myPid = GetCurrentProcessId();
	myProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, myPid);
}

handles::RemoteHandleLocalizer::~RemoteHandleLocalizer() {
	if(myProcess) CloseHandle(myProcess);
}

HANDLE handles::RemoteHandleLocalizer::Localize(
	nt::SYSTEM_HANDLE_ENTRY const & h) {

	return Localize(h.OwnerPid,
		reinterpret_cast<HANDLE>(h.HandleValue));
}

HANDLE handles::RemoteHandleLocalizer::Localize(
	ULONG ownerPid, unsigned short handleValue) {

	return Localize(ownerPid, reinterpret_cast<HANDLE>(handleValue));
}

HANDLE handles::RemoteHandleLocalizer::Localize(
	ULONG ownerPid, HANDLE remoteHandle) {

	HANDLE localHandle = nullptr;
	HANDLE ownerProcess = OpenProcess(
		PROCESS_ALL_ACCESS, FALSE, ownerPid);
	
	if(!DuplicateHandle(ownerProcess, remoteHandle,
		myProcess, (LPHANDLE)&localHandle,
		0, FALSE, DUPLICATE_SAME_ACCESS)) {

		CloseHandle(ownerProcess);
		throw HandlesError("RemoteHandleLocalizer::Localize():"
			" unable to localize handle");
	}
	
	CloseHandle(ownerProcess);
	return localHandle;
}

handles::FileHandlePathGetter::FileHandlePathGetter()
		: ntdll("ntdll.dll") {

	_QueryObject = ntdll.find<nt::NtQueryObjectType>("NtQueryObject");
}

void handles::FileHandlePathGetter::GetPath(typ::string & path,
	nt::SYSTEM_HANDLE_ENTRY const & h) {

	RemoteHandleLocalizer rhl;
	HANDLE handle = rhl.Localize(h);
	GetPath(path, handle);
	CloseHandle(handle);
}

void handles::FileHandlePathGetter::GetPath(
	typ::string & path, HANDLE handle, bool closeHandle) {

	NTSTATUS result = STATUS_SUCCESS;
	ULONG size = 8 + MAX_PATH * 2 + 2;
	
	std::unique_ptr<char> buffer(new char[size]);
	OBJECT_NAME_INFORMATION & oni =
		*reinterpret_cast<OBJECT_NAME_INFORMATION *>(buffer.get());
	
	result = _QueryObject(handle, ObjectNameInformation,
		buffer.get(), size, &size);
	
	if(STATUS_SUCCESS != result) {
		if(closeHandle)
			CloseHandle(handle);
		throw QueryError("FileHandlePathGetter::GetPath():"
			" couldn't query object name");
	}

	text::Ansi ansi(oni.Name);
	typ::string dp(ansi.GetText());
	
	path::DosPathTranslator dpt;
	dpt.Translate(dp);
	
	path.swap(dp);
	
	if(closeHandle)
		CloseHandle(handle);
}

void handles::PrintHandleInformation(
	nt::SYSTEM_HANDLE_ENTRY const & h) {

	printf("Owner PID     : %08lX\nObject Type   :"
		" %08X\nHandle Flags  : %08X\n",
		h.OwnerPid, h.ObjectType, h.HandleFlags);
	printf("Handle Value  : %08X\n", h.HandleValue);
	printf("Object Pointer: %08lX\nAccess Mask   : %08lX\n",
		reinterpret_cast<long unsigned int>(h.ObjectPointer),
		h.AccessMask);
}

handles::HandleInformationPrinter::HandleInformationPrinter()
		: ntdll("ntdll.dll") {

	_QueryObject = ntdll.find<nt::NtQueryObjectType>("NtQueryObject");
}

void handles::HandleInformationPrinter::Print(
	nt::SYSTEM_HANDLE_ENTRY const & h) {

	PrintHandleInformation(h);
	
	typ::string p;
	FileHandlePathGetter pg;
	pg.GetPath(p, h);
	printf("Resource Name : '%s'\n", p.c_str());
}

handles::HandleEnumerator::HandleEnumerator(size_t initialBufferSize)
		: result(STATUS_SUCCESS), ntdll("ntdll.dll"),
		size(initialBufferSize), buffer(new char[size]) {

		EnumerateHandles();
}

void handles::HandleEnumerator::GetFilePaths(typ::strings & paths) {
	FileHandlePathGetter pg;
	//HandleInformationPrinter hip;
	
	nt::SYSTEM_HANDLE_ENTRY * he = GetHandleEntries();
	
	for(size_t i=0; i<GetHandleEntriesCount(); ++i) {
		nt::SYSTEM_HANDLE_ENTRY &h = he[i];
		
		if(25 == h.ObjectType || 28 == h.ObjectType) {
			try {
				typ::string p;
				pg.GetPath(p, h);
				paths.push_back(p);
			}
			catch(QueryError &e) {
				OutputDebugString(e.what());
			}
		}
	}
}

void handles::HandleEnumerator::GetFilePathsForProcess(
	typ::strings & paths) {

	GetFilePathsForProcess(paths, GetCurrentProcessId());
}

void handles::HandleEnumerator::GetFilePathsForProcess(
	typ::strings & paths, unsigned long pid) {

	FileHandlePathGetter pg;
	//HandleInformationPrinter hip;
	
	nt::SYSTEM_HANDLE_ENTRY * he = GetHandleEntries();
	
	for(size_t i=0; i<GetHandleEntriesCount(); ++i) {
		nt::SYSTEM_HANDLE_ENTRY &h = he[i];
		
		if(pid == h.OwnerPid)
			if(25 == h.ObjectType || 28 == h.ObjectType) {
				try {
					typ::string p;
					pg.GetPath(p, h);
					paths.push_back(p);
				}
				catch(QueryError &e) {
					OutputDebugString(e.what());
				}
			}
	}
}

void handles::HandleEnumerator::GetFilePathsForProcess(
	typ::strings &paths, char const * name) {

	if(!name || !*name) return;
	
	handles::ProcessIdFinder pif;
	handles::ProcessIdFinder::PIDNAMES pidnames;
	pif.Find(pidnames, name);
	
	if(!pidnames.empty()) {
		for(auto it = pidnames.begin(); it != pidnames.end(); ++it) {
			typ::strings temp;
			GetFilePathsForProcess(temp, it->first);
			paths.insert(paths.end(), temp.begin(), temp.end());
		}
	}
}

unsigned long
handles::HandleEnumerator::GetHandleEntriesCount() const {

	nt::SYSTEM_HANDLE_INFORMATION & hi =
		*reinterpret_cast<nt::SYSTEM_HANDLE_INFORMATION*>(
			buffer.get());
	return hi.Count;
}

nt::SYSTEM_HANDLE_ENTRY *
handles::HandleEnumerator::GetHandleEntries() const {

	return reinterpret_cast<nt::SYSTEM_HANDLE_ENTRY*>(
		buffer.get() + 4);
}

void handles::HandleEnumerator::EnumerateHandles() {
	result = NtQuerySystemInformation(SystemHandleInformation,
		buffer.get(), size, nullptr);
	
	for(int i=0, tries=3;
		i<tries && STATUS_INFO_LENGTH_MISMATCH == result; ++i) {

		size <<= 1;
		buffer.reset(new char[size]);
		result = NtQuerySystemInformation(
			SystemHandleInformation, buffer.get(), size, nullptr);
	}
	
	if(STATUS_SUCCESS != result) {
		if(STATUS_INFO_LENGTH_MISMATCH == result)
			throw HandlesError("HandleEnumerator::EnumerateHandles():"
				" output buffer still too small\n");
		throw HandlesError("HandleEnumerator::EnumerateHandles():"
			" unable to query process information\n");
	}
}

void handles::FilterPaths(typ::strings & paths, char const * ext) {
	using MARKS = std::vector<typ::strings::iterator>;
	MARKS marks;
	
	for(auto it = paths.begin(); it != paths.end(); ++it) {
		path::SplitPath sp(it->c_str());
		if(ext && *ext && stricmp(sp.GetExt(), ext))
			marks.push_back(it);
	}
	
	for(auto it = marks.rbegin(); it != marks.rend(); ++it)
		paths.erase(*it);
}

void handles::FilterPaths(typ::strings & out,
	typ::strings const & paths, char const * ext) {

	for(auto it = paths.begin(); it != paths.end(); ++it) {
		path::SplitPath sp(it->c_str());
		if(!(ext && *ext) || !stricmp(sp.GetExt(), ext))
			out.push_back(*it);
	}
}

bool handles::ListHandlesForProcess(char const * name,
	char const * ext) {

	handles::ProcessIdFinder pif;
	handles::ProcessIdFinder::PIDNAMES pidnames;
	pif.Find(pidnames, name);
	
	if(pidnames.empty()) {
		printf("\nFOUND NO PROCESS THAT MATCHES"
			" THE PREFIX '%s'.\n", name);
		return false;
	}
	
	printf("\nFOUND %d PROCESS(ES) THAT MATCH(ES)"
		" THE PREFIX '%s':\n\n", pidnames.size(), name);
	
	int h=0;
	for(auto it = pidnames.begin(); it != pidnames.end(); ++it)
		printf(" #%2d: %ld (%s)\n",
			++h, it->first, it->second.c_str());
	
	HandleEnumerator hh;
	
	int i=0;
	for(auto it = pidnames.begin(); it != pidnames.end(); ++it) {
		typ::strings paths;
		hh.GetFilePathsForProcess(paths, it->first);
		
		FilterPaths(paths, ext);
		
		if(paths.empty()) {
			printf("\nNo file paths with extension '%s'"
				" found for process #%d: %ld (%s)\n\n",
				ext, ++i, it->first, it->second.c_str());
			continue;
		}
		
		printf("\nEnumerating file paths for process"
			" #%d: %ld (%s)\n\n",
			++i, it->first, it->second.c_str());
		
		int j=0;
		for(auto it = paths.begin(); it != paths.end(); ++it) {
			path::SplitPath sp(it->c_str());
			printf("%4d: '%s'\n      [%s]\n", ++j,
				sp.GetName(), sp.GetPath());
		}
	}
	
	return true;
}

void handles::GetHandlesForProcess(typ::strings &paths,
	char const * name, char const * ext) {

	handles::ProcessIdFinder pif;
	handles::ProcessIdFinder::PIDNAMES pidnames;
	pif.Find(pidnames, name);
	
	if(!pidnames.empty()) {
		HandleEnumerator hh;
		
		for(auto it = pidnames.begin(); it != pidnames.end(); ++it) {
			typ::strings temp;
			hh.GetFilePathsForProcess(temp, it->first);
			if(ext) FilterPaths(temp, ext);
			paths.insert(paths.end(), temp.begin(), temp.end());
		}
	}
}
