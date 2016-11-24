#define WIN32_LEAN_AND_MEAN
#define STRICT
#include <windows.h>
#include <stdio.h>
#include <stdexcept>

#include <handles/typ.h>
#include <handles/ex.h>
#include <handles/sys.h>
#include <handles/text.h>


int main(int argc, char const ** argv)
{
	STANDARD_TRY_BEGIN

	char buffer[1024];
	text::Format(buffer, sizeof(buffer), "%d:%d", 12, 13);
	printf("1* |%s|\n", buffer);

	text::Ansi ansi(buffer);
	printf("2* |%s|\n", ansi.GetText());

	STANDARD_TRY_END

	return 0;
}
