//--
//-- SimSystem.cpp
//--
#include "SimSystem.h"
namespace sundile {
	namespace SimSystem {

		void simRegistryQuery(const SimRegistryQuery& srq) {
			for (auto sim : sims) {
				if (sim->id == srq.id) {
					srq.wrapper->registry = sim->registry.get();
				}
			}
		}

		//Parses GLFW key events and passes appropriate information to other functions.
		void handleInput(const WindowInputEvent& wev) {
			//-- Emergency Exit
			if (wev.key == GLFW_KEY_ESCAPE) {
				printf("Exiting on Keypress: Esc");
				exit(EXIT_FAILURE);
			}

			for (auto sim : sims) {
				SimInputEvent ev;
				ev.registry = sim->registry;
				ev.deltaTime = sim->deltaTime;
				ev.key = wev.key;
				ev.scancode = wev.scancode;
				ev.action = wev.action;
				ev.mods = wev.mods;
				sim->evw->dispatcher.trigger<SimInputEvent>(ev);
			}
		}

		void catchInit(const initEvent& ev) {
			for (auto sim : sims) {
				SimInitEvent ev;
				ev.registry = sim->registry;
				ev.deltaTime = 0.f;
				ev.evw = sim->evw;
				ev.id = sim->id;
				ev.evw->dispatcher.trigger<SimInitEvent>(ev);
			}
		}

		SmartSim init(SmartEVW evw) {
			// Initialize
			SmartSim sim = std::make_shared<Sim>();
			sim->registry = std::make_shared<entt::registry>();
			sim->evw = evw;
			sim->id = rand();

			// Connect event listeners
			evw->dispatcher.sink<stepEvent>().connect<updateAll>();
			evw->dispatcher.sink<WindowInputEvent>().connect<handleInput>();
			evw->dispatcher.sink<initEvent>().connect<catchInit>();
			evw->dispatcher.sink<SimRegistryQuery>().connect<simRegistryQuery>();


			//Add to sims
			sims.push_back(sim);

			return sim;
		}

		//--
		//-- Main Loop
		//--
		void update(SmartSim sim) {

			//-- Time
			sim->currentTime = glfwGetTime();
			sim->deltaTime = sim->currentTime - sim->lastTime;
			sim->lastTime = sim->currentTime;

			SmartEVW evw = sim->evw;

			//-- Update Events
			SimStepEvent sev;
			sev.registry = sim->registry;
			sev.deltaTime = sim->deltaTime;
			sev.currentTime = sim->currentTime;
			evw->dispatcher.trigger<SimStepEvent>(sev);

			RenderEvent rev;
			rev.registry = sim->registry;
			rev.deltaTime = sim->deltaTime;
			rev.currentTime = sim->currentTime;
			evw->dispatcher.trigger<RenderEvent>(rev);

			RenderGuiEvent gev;
			gev.registry = sim->registry;
			gev.deltaTime = sim->deltaTime;
			gev.currentTime = sim->currentTime;
			evw->dispatcher.trigger<RenderGuiEvent>(gev);
		}

		void updateAll() {
			for (auto sim : sims) {
				update(sim);
			}
		}

	}
}
