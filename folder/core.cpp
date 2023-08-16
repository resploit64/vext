#include "core.hpp"

LONG WINAPI ExHandler( EXCEPTION_POINTERS* pExceptionInfo )
{
    if ( pExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_ACCESS_VIOLATION )
    {
        return EXCEPTION_EXECUTE_HANDLER;
    }

    return EXCEPTION_CONTINUE_SEARCH;
}

//bool set_timer_resolution()
//{
//    static NTSTATUS(NTAPI * nt_set_timer_resolution)
//        (IN ULONG desired_resolution, IN BOOLEAN set_resolution, OUT PULONG current_resolution) =
//        (NTSTATUS(NTAPI*)(ULONG, BOOLEAN, PULONG))
//        ::GetProcAddress(GetModuleHandle(_("ntdll.dll")), _("NtSetTimerResolution"));
//
//    ULONG desired_resolution{ 5000UL }, current_resolution{ };
//    if (nt_set_timer_resolution(desired_resolution, TRUE, &current_resolution)) { return false; }
//
//    return true;
//}

static float last_render = 0.0f;

#ifndef _WINDLL
int main( )
#else
void main( )
#endif
{
    srand(time(0));
    AddVectoredExceptionHandler( 0, ExHandler );
    AllocConsole( );
    freopen( _( "CONOUT$" ), _("w"), stdout );
    freopen( _( "CONIN$" ), _("w"), stdin );
    SetConsoleTitleA( _( "nasa" ) );
    ShowWindow( GetConsoleWindow( ), SW_SHOW );

    core::get_time_since_startup( );

    //set_timer_resolution();

    core::init( );
    uworld = UWorld::Instance( );
    persistent_level = uworld->persistent_level( );
    game_instance = uworld->game_instance( );
    localplayer = game_instance->localplayer( );
    local_controller = localplayer->controller( );
    local_actor = local_controller->acknowledged_pawn( );
    local_state = local_actor->player_state( );
    local_team_id = local_state->team_component()->team_id();
    camera_manager = local_controller->camera_manager( );

    printf(_("guard_ptr -> 0x%p\n"), kernel->guard_address);
    printf(_("uworld -> 0x%p\n"), uworld);
    printf(_("persistent_level -> 0x%p\n"), persistent_level);
    printf(_("game_instance -> 0x%p\n"), game_instance);
    printf(_("localplayer -> 0x%p\n"), localplayer);
    printf(_("local_controller -> 0x%p\n"), local_controller);
    printf(_("local_actor -> 0x%p\n"), local_actor);
    printf(_("local_state -> 0x%p\n"), local_state);
    printf(_("local_team_id -> 0x%p\n"), local_team_id);
    printf(_("camera_manager -> 0x%p\n"), camera_manager);

    std::thread cache_thread(&threads::cache::thread);
    cache_thread.detach();


    last_render = core::get_time_since_startup( );
    while (!GetAsyncKeyState(VK_END))
    {
        last_render = core::get_time_since_startup( );
        threads::render::thread();
    }
    render_ejected.store(true);
    thread_ejection.store(true);

    while (!cache_ejected.load() && !render_ejected.load())
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

#ifndef _WINDLL
    return 0;
#endif
}

#ifdef _WINDLL

bool __stdcall DllMain( HMODULE hMod, const std::uint32_t call_reason, LPVOID )
{
    if ( call_reason == DLL_PROCESS_ATTACH )
    {
        const auto handle = CreateThread( nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>( main ), nullptr, 0, nullptr );
    }

    return true;
}

#endif