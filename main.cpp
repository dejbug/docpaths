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

	ArgumentParser parser;
	parser.addArgument("-n", "--name", 1, false);
	parser.addArgument("-e", "--exts", '*');
	parser.parse(argc, argv);

	char const * process_name_pattern =
		parser.retrieve<typ::string>("name").c_str();

	typ::strings exts = 
		parser.retrieve<typ::strings>("exts");

	typ::strings filtered_paths, unfiltered_paths;

	handles::HandleEnumerator he;
	he.GetFilePathsForProcess(unfiltered_paths, process_name_pattern);

	for(auto it=exts.begin(); it < exts.end(); ++it) {
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
