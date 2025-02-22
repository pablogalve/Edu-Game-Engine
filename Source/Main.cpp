//#include "MemLeaks.h"
#include "Globals.h"
#include "Application.h"

// Old school memory leak detector and other random awesomeness
#ifdef _DEBUG
	//#define TEST_MEMORY_MANAGER
#endif

#include "Leaks.h"
#include <assert.h>
#include <vector>
#include <string>

// We need to include this here because SDL overwrites main()
#include "SDL/include/SDL.h"
#pragma comment( lib, "SDL/libx86/SDL2.lib" )
#pragma comment( lib, "SDL/libx86/SDL2main.lib" )

enum main_states
{
	MAIN_CREATION,
	MAIN_START,
	MAIN_UPDATE,
	MAIN_FINISH,
	MAIN_EXIT
};

Application* App = nullptr;

void DumpLeaks(void)
{
	_CrtDumpMemoryLeaks();
}

int __cdecl CrtDbgHook(int nReportType, char* szMsg, int* pnRet)
{
	return 0;//Return false - Abort,Retry,Ignore dialog *will be displayed*
}

int main(int argc, char ** argv)
{
	_CrtSetReportHook2(_CRT_RPTHOOK_INSTALL, CrtDbgHook);
	LOG("Starting EDU Engine from [%s]", argv[0]);

#ifdef _DEBUG
	atexit(DumpLeaks);
#endif

	int main_return = EXIT_FAILURE;
	main_states state = MAIN_CREATION;

	while (state != MAIN_EXIT)
	{
		switch (state)
		{
			case MAIN_CREATION:
			{

				LOG("-------------- Application Creation --------------");
				App = new Application();
				state = MAIN_START;
			} break;

			case MAIN_START:
			{
				LOG("-------------- Application Init --------------");
				if (App->Init() == false)
				{
					LOG("Application Init exits with ERROR");
					state = MAIN_EXIT;
				}
				else
				{
					state = MAIN_UPDATE;
					LOG("-------------- Application Update --------------");
				}

			} break;

			case MAIN_UPDATE:
			{
				int update_return = App->Update();

				if (update_return == UPDATE_ERROR)
				{
					LOG("Application Update exits with ERROR");
					state = MAIN_EXIT;
				}

				if (update_return == UPDATE_STOP)
					state = MAIN_FINISH;
			} break;

			case MAIN_FINISH:
			{
				LOG("-------------- Application CleanUp --------------");
				if (App->CleanUp() == false)
				{
					LOG("Application CleanUp exits with ERROR");
				}
				else
					main_return = EXIT_SUCCESS;

				state = MAIN_EXIT;

			} break;
		}
	}

	RELEASE(App);

	LOG("Exiting engine ...");


	return main_return;
}
