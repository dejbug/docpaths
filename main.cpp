#define WIN32_LEAN_AND_MEAN
#define STRICT
#include <windows.h>
#include <stdio.h>
#include <stdexcept>

#include <handles/typ.h>
#include <handles/ex.h>
#include <handles/handles.h>


int main(int argc, char const ** argv)
{
	STANDARD_TRY_BEGIN

	char const * process_name_pattern = "foxit";
	typ::strings exts{"pdf"};

	typ::strings filtered_paths;
	typ::strings unfiltered_paths;
	
	handles::HandleEnumerator he;
	he.GetFilePathsForProcess(
		unfiltered_paths, process_name_pattern);

	for(auto it=exts.begin(); it<exts.end(); ++it) {
		handles::FilterPaths(
			filtered_paths, unfiltered_paths, it->c_str());
	}

	typ::strings & paths =
		exts.empty() ? unfiltered_paths : filtered_paths;

	for(auto it=paths.begin(); it<paths.end(); ++it) {
		printf("%s\n", it->c_str());
	}

	STANDARD_TRY_END

	return 0;
}
