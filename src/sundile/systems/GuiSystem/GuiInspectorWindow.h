#ifndef GUI_INSPECTOR_H
#define GUI_INSPECTOR_H

#include "GuiTypes.h"
#include "GuiUtility.h"

/*
\\TODO:
	Implement drag'n'drop reordering and component shuffling/exchange
	Would Be Nice:
	shift selection
	drag selection
*/

SYSTEM(GuiSystem)

// [SECTION] - Structs & namespace globals

//static std::vector<sceneTab> sceneTabs;
	static guiTreeContainer<SmartScene> sceneTree;
	static std::vector<entityTab> entityTabs;
	static std::vector<componentTab> componentTabs;
	static std::vector<dataTab> dataTabs;
	static float guiInspectorHeight = 300.f;

	// [SECTION] - Editor windows
	static const nodeEventCallback<listComponent> componentTab_LeftClick = [&](listNodeRef<listComponent> p_node, guiTreeContainer<listComponent>& tree) {
		auto io = ImGui::GetIO();
		auto clipboard = getClipboard<listComponent>();
		auto& selected = clipboard->selected;
		auto p_component = p_node->content;

		if (io.KeyCtrl) {
			p_node->state["selected"] = !p_node->state["selected"];
			if (p_node->state["selected"]) {
				addOrReplace(selected, p_node);
			}
		}
		else {
			ClearGuiTreeSelectionState(tree);
			selected.clear();
			selected.push_back(p_node);
			p_node->state["selected"] = true;

			bool found = false;
			for (auto i : dataTabs) {
				if (i.name == p_component->name) {
					found = true; break;
				}
			}
			if (!found) {
				dataTab tab(p_component->name, p_component->index.f, p_component->meta);
				dataTabs.emplace_back(tab);
			}
		}
	};
	static const nodeEventCallback<listComponent> componentTab_RightClick = [&](listNodeRef<listComponent> p_node, guiTreeContainer<listComponent>& tree) {
		auto& io = ImGui::GetIO();
		if (!io.KeyCtrl) {
			ClearGuiTreeSelectionState(tree);
		}
		p_node->state["selected"] = true;
	};
	componentTab createComponentTab(std::string name, guiComponentList& components) {
		//create component tab
		componentTab tab(name, [](guiContainer&, guiTreeContainer<listComponent> tree) {
			RenderGuiTree(tree);
			ClipboardContextMenu(getClipboard<listComponent>(), tree.root, "Component", "Components");
			}, &components);
		tab.treeContainer.callbacks[leftClick] = componentTab_LeftClick;
		tab.treeContainer.callbacks[rightClick] = componentTab_RightClick;
		return componentTabs.emplace_back<componentTab>(std::move(tab));
	};
	
	static const nodeEventCallback<listEntity> entityTab_LeftClick = [&](listNodeRef<listEntity> p_node, guiTreeContainer<listEntity>& tree) {
		auto& io = ImGui::GetIO();
		auto clipboard = getClipboard<listEntity>();
		auto& selected = clipboard->selected;
		auto p_entity = p_node->content;

		if (io.KeyCtrl) {
			p_node->state["selected"] = !p_node->state["selected"];
			if (p_node->state["selected"]) {
				addOrReplace(selected, p_node);
			}
		}
		else {
			ClearGuiTreeSelectionState(tree);
			selected.clear();
			selected.push_back(p_node);
			p_node->state["selected"] = true;

			bool found = false;
			for (auto& i : componentTabs) {
				if (i.name == p_entity->name) {
					found = true; break;
				}
			}
			if (!found) {
				createComponentTab(p_entity->name, p_entity->componentList);
			}
		}
	};
	static const nodeEventCallback<listEntity> entityTab_RightClick = [&](listNodeRef<listEntity> p_node, guiTreeContainer<listEntity>& tree) {
		auto& io = ImGui::GetIO();
		if (!io.KeyCtrl) {
			ClearGuiTreeSelectionState(tree);
		}
		p_node->state["selected"] = true;
	};
	entityTab& createEntityTab(std::string name, guiEntityList& entities) {
		entityTab tab(name, [](guiContainer&, guiTreeContainer<listEntity>& tree) {
			RenderGuiTree(tree);
			ClipboardContextMenu(getClipboard<listEntity>(), tree.root, "Entity", "Entities");
			}, &entities);
		tab.container.state["open"] = true;
		tab.treeContainer.callbacks[leftClick] = entityTab_LeftClick;
		tab.treeContainer.callbacks[rightClick] = entityTab_RightClick;

		return entityTabs.emplace_back<entityTab>(std::move(tab));
	}

	static const nodeEventCallback<SmartScene> sceneTree_LeftClick = [&](listNodeRef<SmartScene> p_node, guiTreeContainer<SmartScene>& tree) {
		auto& io = ImGui::GetIO();
		auto clipboard = getClipboard<SmartScene>();
		auto& selected = clipboard->selected;
		auto scene = *(p_node->content);

		if (io.KeyCtrl) {
			p_node->state["selected"] = !p_node->state["selected"];
			if (p_node->state["selected"]) {
				addOrReplace(selected, p_node);
			}
		}
		else {
			ClearGuiTreeSelectionState(tree);
			selected.clear();
			selected.push_back(p_node);
			p_node->state["selected"] = true;

			bool found = false;
			for (auto& i : entityTabs) {
				if (i.name == scene->name) {
					found = true; break;
				}
			}
			if (!found) {
				createEntityTab(scene->name, entityList);
			}
		}
	};
	static const nodeEventCallback<SmartScene> sceneTree_DoubleClick = [&](listNodeRef<SmartScene> p_node, guiTreeContainer<SmartScene>& tree) {
		auto scene = *(p_node->content);
		SceneActivateEvent ev;
		ev.currentTime = scene->currentTime;
		ev.deltaTime = scene->deltaTime;
		ev.id = scene->id;
		ev.registry = scene->registry;
		EventSystem::currentEVW->dispatcher.trigger<SceneActivateEvent>(ev);
	};
	static const nodeEventCallback<SmartScene> sceneTree_RightClick = [&](listNodeRef<SmartScene> p_node, guiTreeContainer<SmartScene>& tree) {

	};

	void updateScenes() { //cf GuiFrontEnd.h : 132
		sceneTree.root->children.clear(); //no memory leak with smart pointer
		for (SmartScene scene : SceneSystem::scenes) {
			listNode<SmartScene>* node = new listNode<SmartScene>(scene);
			node->name = scene->name;
			sceneTree.root->children.emplace_back(std::move(node));
		}
	}

	// To be called on GuiInit, which should be called when GUI is loaded from ProjectSystem
	void initInspector(guiContainer& container) {
		sceneTree.callbacks[leftClick] = sceneTree_LeftClick;
		sceneTree.callbacks[rightClick] = sceneTree_RightClick;

		//Initialize Tabs (should be scene instead of entities, but this is fine for now)
		createEntityTab("[scene name]", entityList);
	}
	void Inspector(guiContainer& container) {
		auto ctx = checkContext();
		auto& io = ImGui::GetIO();

		auto& primary = guiRegistry.get<guiContainer>(primaryGuiEntity);
		auto maxSize = primary.size;

		float numChildren = 4.f;
		auto width = ImGui::GetWindowWidth() / numChildren;

		//need some storage for selected entities, components, &c.
		//figure out how to use omittable variadic template parameters like entt::emplace
		ImGuiTabBarFlags tabBarFlags = ImGuiTabBarFlags_AutoSelectNewTabs | ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_TabListPopupButton;

		ImGui::SetWindowSize(ImVec2(0.f, guiInspectorHeight));
		ImGui::SetWindowPos(ImVec2(0.f, maxSize.y - ImGui::GetWindowHeight()));

		if (ImGui::BeginChild("Scene Selector", ImVec2(width, 0), true)) {
			RenderGuiTree(sceneTree);
		}
		ImGui::EndChild();

		ImGui::SameLine();
		if (ImGui::BeginChild("Entity Selector", ImVec2(width, 0), true)) {
			if (ImGui::BeginTabBar("Entity Selector Tabs", tabBarFlags)) {
				RenderTabs<entityTab>(entityTabs);
				ImGui::EndTabBar();
			}
		}
		ImGui::EndChild();

		ImGui::SameLine();
		if (ImGui::BeginChild("Component Selector", ImVec2(width, 0), true)) {
			if (ImGui::BeginTabBar("Component Selector Tabs", tabBarFlags)) {
				RenderTabs<componentTab>(componentTabs);
				ImGui::EndTabBar();
			}
		}
		ImGui::EndChild();

		ImGui::SameLine();
		if (ImGui::BeginChild("Data Editor", ImVec2(width, 0), true)) {
			if (ImGui::BeginTabBar("Data Editor Tabs", tabBarFlags)) {
				RenderTabs<dataTab>(dataTabs);
				ImGui::EndTabBar();
			}
		}
		ImGui::EndChild();

	}

END_SYSTEM

#endif