#ifndef GUI_FILETREE
#define GUI_FILETREE
SYSTEM(GuiSystem)

namespace Filetree {

	struct filetreeNode {
		fs::path path;
		std::vector<filetreeNode> children;
	};

	static filetreeNode root;


	filetreeNode scanFiletree(fs::path rootPath) {
		filetreeNode root;
		root.path = rootPath;
		for (const auto& path : fs::directory_iterator(root.path)) {
			filetreeNode child;
			child.path = path;
			if (fs::is_directory(path)) {
				child = scanFiletree(path);
			}
			root.children.push_back(child);
		}

		return root;
	}

	void renderNode(filetreeNode node, int id) {
		for (auto& child : node.children) {
			std::stringstream ss;
			ss << child.path;
			std::string str = ss.str();
			findAndReplaceAll(str, "\\", "/");
			findAndReplaceAll(str, "\"", "");
			auto pos = str.rfind("/");
			str.replace(str.begin(), str.begin() + pos + 1, "");

			if (ImGui::TreeNode(str.c_str())) {
				renderNode(child,id);
				ImGui::TreePop();
			}
		}
	}

	void render(guiContainer& container) {
		renderNode(root, 0);
	}

	void destroy(TerminateEvent<SmartEVW>& ev) {

	}
	void init() {

		guiContainer fileDialog("file dialog", Filetree::render);
		primaryGuiContainer.state["file dialog"] = true;
		primaryGuiContainer.children.push_back(fileDialog);

		root = scanFiletree(ProjectSystem::currentProject->projectRoot);
		EventSystem::currentEVW->dispatcher.sink<TerminateEvent<SmartEVW>>().connect<&destroy>();
	}
}

END_SYSTEM
#endif