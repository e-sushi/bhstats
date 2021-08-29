#include "deshi.h"
#include "winload.h"
#include "math/Math.h"

HANDLE csgoProc;
DWORD csgopid;
DWORD clientBaseAddr;

//designed to allow a direct copy from CSGO's memory to the struct
struct PlayerData {
	vec3 velocity;

	float UNKN0{}, UNKN1{}, UNKN2{}, UNKN3{};

	vec3 lookangle;
	vec3 lookangle_duplicate; //not sure why theres two of these 

	float UNKN4{};

	vec3 acceleration; 

	// all of this seems to be padding, as it never changes with movement or other things
	float UNKN5{}, UNKN6{}, UNKN7{}, UNKN8{}, UNKN9{}, UNKN10{}, UNKN11{}, UNKN12{}, UNKN13{}, UNKN14{};

	vec3 position;
};

void GetPlayerMemory(PlayerData& pdata) {
	if (!ReadProcessMemory(csgoProc, (LPCVOID)(clientBaseAddr + 0xDA4788), &pdata, sizeof(PlayerData), NULL)) {
		std::cout << "\ReadProcessMemory failed! Last error: \n" << std::dec << GetLastError() << std::endl;
		throw 0;
	}
}

int main() {
	deshi::init(700, 300);
	Render::UseDefaultViewProjMatrix();

	csgoProcInfo cpi = winload::GetCSGOInfo();

	csgoProc = cpi.handle;
	csgopid = cpi.pid;
	clientBaseAddr = cpi.clientBaseAddr;

	PlayerData pdata;

	array<float> prevSpeeds;

	prevSpeeds.reserve(1000);

	TIMER_START(t_d); TIMER_START(t_f);
	while (!deshi::shouldClose()) {
		DeshiImGui::NewFrame();
		DeshTime->Update();
		DeshWindow->Update();
		DeshInput->Update();
		DeshConsole->Update(); Console2::Update();

		GetPlayerMemory(pdata);

		vec3 velzzero = pdata.velocity;
		velzzero.z = 0;

		if (prevSpeeds.count == 1000) {
			prevSpeeds.pop();
			prevSpeeds.insert(velzzero.mag(), 0);
		}
		else {
			prevSpeeds.insert(velzzero.mag(), 0);
		}


		using namespace ImGui;

		SetNextWindowPos(ImVec2{ 0, 0 });
		SetNextWindowSize(ImVec2{ 700, 300 });
		Begin("main", 0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
		
		Text(TOSTRING("position:     ", pdata.position).str);
		Text(TOSTRING("velocity:     ", pdata.velocity).str);
		Text(TOSTRING("acceleration: ", pdata.acceleration).str);
		Text(TOSTRING("lookang:      ", pdata.lookangle).str);

		PlotLines("#lab", prevSpeeds.data, 1000, 0, 0, 0, 1000, ImVec2{300, 200});

		End();




		UI::Update();
		Render::Update();

		DeshTime->frameTime = TIMER_END(t_f); TIMER_RESET(t_f);
	}


}