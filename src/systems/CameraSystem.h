#pragma once
#include "../sundile/EventWrapper.h"
#include "../components/Camera.h"
#include "../components/Input.h"


SYSTEM_DEF_BEGIN(CameraSystem)

inline glm::vec2 cursorpos;
inline glm::vec2 cursorpos_prev;


glm::vec3 rotatexy(glm::vec3 vec, float radians) {
	//note: this is the passive transform, used for rotating axes
	vec.x = vec.x * cosf(radians) + vec.y * sinf(radians);
	vec.y = -vec.x * sinf(radians) + vec.y * cosf(radians);
	return vec;
}

void catchCursorEvent(const TypedWindowEvent<double>& ev) { //TODO - move this to InputSystem
	GLFWwindow* win = ev.window;
	
	int* width = new int;
	int* height = new int;
	glfwGetFramebufferSize(win, width, height);

	cursorpos = glm::vec2((ev.vals[0])/(*width), (ev.vals[1])/(*height)); //normalized :)
	delete width;
	delete height;
}

void catchStepEvent(const SimStepEvent& ev) {
	ev.registry->view<camera, input>().each([&](auto entity, camera& cam, input& Input) {

		//
		//Identity and inverse.
		glm::mat4 T = glm::mat4(1.f);
		glm::mat4 Mi = glm::inverse(cam.model);
		glm::vec3& dir = cam.dir;
		glm::vec3& pos = cam.pos;
		glm::vec3& spd = cam.spd;
		constexpr float pi = glm::pi<float>();

		//
		//Rotation
		glm::vec3 newdir = glm::vec3(0.f);
		if (cursorpos != cursorpos_prev) {
			float scale = pi;
			//glm::vec3 axis = glm::vec3(0, 1, 0);

			if (Input.held[btn::mb_left]) {
				float xdif = (cursorpos.x - cursorpos_prev.x);
				float radians = xdif * scale;
				newdir.y += radians;
				//axis = glm::vec3(0, 1, 0);
			}
			if (Input.held[btn::mb_right]) {
				float ydif = (cursorpos.y - cursorpos_prev.y);
				float radians = ydif * scale;
				newdir.x += radians;
				//axis = glm::vec3(1, 0, 0);
			}

			/**
			cam.model = glm::translate(
				glm::rotate(
					glm::translate(cam.model, -cam.pos),
					radians, axis),
				cam.pos);
			/**/
		}
		dir += newdir;

		T = glm::rotate(T, dir.x, glm::vec3(1, 0, 0));
		T = glm::rotate(T, dir.y, glm::vec3(0, 1, 0));
		T = glm::rotate(T, dir.z, glm::vec3(0, 0, 1));

		//
		//Translation
		float movspd = -.1f;
		glm::vec3 newspd = glm::vec3(0.f);
		if (Input.held[btn::left]) {
			newspd.x -= movspd;
		}
		if (Input.held[btn::right]) {
			newspd.x += movspd;
		}
		if (Input.held[btn::up]) {
			newspd.z -= movspd;
		}
		if (Input.held[btn::down]) {
			newspd.z += movspd;
		}
		if (Input.held[btn::trigger_left]) {
			newspd.y -= movspd;
		}
		if (Input.held[btn::trigger_right]) {
			newspd.y += movspd;
		}

		//Clamp speed, do friction (probably a better way to do this)
		float absx = glm::abs(newspd.x);
		newspd.x -= glm::sign(newspd.x) * cam.fric;
		if (absx < glm::abs(cam.fric)) newspd.x = 0.f;
		if (absx > cam.maxspd) newspd.x = absx * cam.maxspd;

		float absy = glm::abs(newspd.y);
		newspd.y -= glm::sign(newspd.y) * cam.fric;
		if (glm::abs(newspd.y) < glm::abs(cam.fric)) newspd.y = 0.f;
		if (absy > cam.maxspd) newspd.y = absy * cam.maxspd;

		float absz = glm::abs(newspd.z);
		newspd.z -= glm::sign(newspd.z) * cam.fric;
		if (glm::abs(newspd.z) < glm::abs(cam.fric)) newspd.z = 0.f;
		if (absz > cam.maxspd) newspd.z = absz * cam.maxspd;

		//Do the translation
		newspd = glm::rotateY(newspd, -dir.y);
		cam.spd += newspd;
		T = glm::translate(T, cam.spd);

		//set cursor position
		cursorpos_prev = cursorpos;

		//Set MVP
		cam.model *= Mi * T;
		cam.mvp = cam.model;

		//Log
		/**
		std::string Spos = "pos = (%f, %f, %f)\n";
		std::string Sspd = "spd = (%f,%f,%f)\n";
		std::string Sfront = "front = (%f,%f,%f)\n";
		std::string Sdir = "dir = (%f, %f, %f)\n";
		std::string Smvp = "mvp = (%f, %f, %f, %f)\n";
		std::string ST = "T = (%f, %f, %f, %f)\n";
		std::string SMi = "Mi = (%f, %f, %f, %f)\n";
		std::string Sfinal = Spos + Sspd + Sfront + Sdir + Smvp + Smvp + Smvp + Smvp + ST + ST + ST + ST + SMi + SMi + SMi + SMi + '\n';
		printf(Sfinal.c_str(),
			cam.pos.x, cam.pos.y, cam.pos.z,
			newspd.x, newspd.y, newspd.z,
			cam.front.x, cam.front.y, cam.front.z,
			cam.dir.x, cam.dir.y, cam.dir.z,
			cam.mvp[0][0], cam.mvp[0][1], cam.mvp[0][2], cam.mvp[0][3],
			cam.mvp[1][0], cam.mvp[1][1], cam.mvp[1][2], cam.mvp[1][3],
			cam.mvp[2][0], cam.mvp[2][1], cam.mvp[2][2], cam.mvp[2][3],
			cam.mvp[3][0], cam.mvp[3][1], cam.mvp[3][2], cam.mvp[3][3],
			T[0][0], T[0][1], T[0][2], T[0][3],
			T[1][0], T[1][1], T[1][2], T[1][3],
			T[2][0], T[2][1], T[2][2], T[2][3],
			T[3][0], T[3][1], T[3][2], T[3][3],
			Mi[0][0], Mi[0][1], Mi[0][2], Mi[0][3],
			Mi[1][0], Mi[1][1], Mi[1][2], Mi[1][3],
			Mi[2][0], Mi[2][1], Mi[2][2], Mi[2][3],
			Mi[3][0], Mi[3][1], Mi[3][2], Mi[3][3]
		);
		/**/
	});
}

void init(const SimInitEvent& ev) {
	ev.evw->dispatcher.sink<SimStepEvent>().connect<&catchStepEvent>();
	ev.evw->dispatcher.sink< TypedWindowEvent<double>>().connect<&catchCursorEvent>();

	//dependencies
	ev.registry->on_construct<camera>().connect<&entt::registry::emplace_or_replace<velocity>>();
}

SYSTEM_DEF_END