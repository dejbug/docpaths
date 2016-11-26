#define WIN32_LEAN_AND_MEAN
#define STRICT
#include <windows.h>
#include <stdio.h>
#include <stdexcept>

#include <argparse/argparse.hpp>

#include <handles/typ.h>
#include <handles/ex.h>
#include <handles/handles.h>


int main(int argc, char const ** argv)
{
	STANDARD_TRY_BEGIN

	// -- Create a little command-line parser, then parse it.

	ArgumentParser parser;
	parser.addArgument("-n", "--name", '+');
	parser.addArgument("-e", "--ext", '+');
	parser.parse(argc, argv);

	// TODO: Add a help option.
	// FIXME: Access to flag-arguments seem to be missing.
	//	Maybe use the boost library?

	// -- Retrieve the command-line arguments we need.

	typ::strings names =
		parser.retrieve<typ::strings>("name");

	typ::strings exts = 
		parser.retrieve<typ::strings>("ext");

	// -- Enumerate the file-handles.

	typ::strings unfiltered_paths;
	handles::HandleEnumerator he;

	if(names.empty()) {
		// -- Enumerate file-handles for all processes.
		he.GetFilePaths(unfiltered_paths);
	}
	else {
		// -- Enumerate file-handles for the specified
		//	processes only. GetFilePathsForProcess() will never
		//	touch the handles that were in the string-vector
		//	already.
		for(auto it=names.begin(); it<names.end(); ++it) {
			he.GetFilePathsForProcess(unfiltered_paths, it->c_str());
		}
	}

	// -- Filter file-paths based on extension.

	typ::strings filtered_paths;

	for(auto it=exts.begin(); it < exts.end(); ++it) {
		handles::FilterPaths(filtered_paths,
			unfiltered_paths, it->c_str());
	}

	typ::strings const & paths =
		exts.empty() ? unfiltered_paths : filtered_paths;

	// -- Print.

	for(auto it=paths.begin(); it<paths.end(); ++it) {
		printf("%s\n", it->c_str());
	}

	STANDARD_TRY_END

	return 0;
}
