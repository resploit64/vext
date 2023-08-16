#include <Windows.h>
#include <iostream>
#include <tuple>
#include <xhash>
//using namespace std::chrono_literals;
#include "settings.hpp"
#include "util/xorstr.hpp"
#include "driver/driver.hpp"
#include "util/util.hpp"
#include "vdm_ctx/vdm_ctx.hpp"
#include "util/vector.hpp"
#include "util/kernel.hpp"
#include "util/renderer.hpp"
std::unique_ptr<kernel_mgr> kernel = nullptr;
std::unique_ptr<game_renderer> renderer = nullptr;

namespace core
{
	bool timer_initialized = false;

	float timeSinceStartup = 0;
	float timeFrequency = 0;

	float get_time_since_startup()
	{
		LARGE_INTEGER PerformanceCount;
		LARGE_INTEGER FrequencyCount;

		if (!timer_initialized)
		{
			timer_initialized = true;

			PerformanceCount.QuadPart = 0;
			QueryPerformanceCounter(&PerformanceCount);

			FrequencyCount.QuadPart = 0;
			QueryPerformanceFrequency(&FrequencyCount);

			timeFrequency = float(FrequencyCount.QuadPart);

			timeSinceStartup = float(PerformanceCount.QuadPart);
		}

		PerformanceCount.QuadPart = 0;
		QueryPerformanceCounter(&PerformanceCount);

		return float(PerformanceCount.QuadPart - timeSinceStartup) / timeFrequency;
	}

	template<typename T>
	bool valid_ptr(T ptr )
	{
		return ptr != 0 && kernel->read<T>( ptr ) != 0;
	}

	void init( )
	{
		vdm::read_phys_t _read_phys =
			[ & ]( void* addr, void* buffer, std::size_t size ) -> bool
		{
			// read code
		};
		vdm::write_phys_t _write_phys =
			[ & ]( void* addr, void* buffer, std::size_t size ) -> bool
		{
			// write code
		};

		kernel = std::make_unique<kernel_mgr>( vdm, _("VALORANT-Win64-Shipping.exe") );

		EnumWindows( [ ]( HWND hWnd, LPARAM lParam ) -> BOOL {
			DWORD pid = 0;
			GetWindowThreadProcessId( hWnd, &pid );
			if ( pid == kernel->process_id ) {
				game_window = hWnd;
				return false;
			}

			return true;
		}, 0 );

		renderer = std::make_unique<game_renderer>( game_window );
	}
}

#include "sdk/offsets.hpp"
#include "sdk/sdk.hpp"

std::vector<APawn*> player_list = {};
std::mutex player_list_mutex{};

static UWorld* uworld;
static ULevel* persistent_level;
static UGameInstance* game_instance;
static ULocalPlayer* localplayer = nullptr;
static APlayerController* local_controller = nullptr;
static APawn* local_actor = nullptr;
static PlayerState* local_state = nullptr;
static int local_team_id = 0;

std::atomic<bool> thread_ejection{};
std::atomic<bool> render_ejected{};
std::atomic<bool> cache_ejected{};


#include "threads/cache.hpp"
#include "threads/render.hpp"