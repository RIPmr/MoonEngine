#pragma once
#include <unordered_map>
#include <iostream>
#include <vector>

#include "Matrix.h"
#include "imgui/ImPlot.h"

namespace MOON {
	#define MOON_PlotManager	PlotManager
	#define MOON_PlotType		ImGui::PlotItem::Type
	#define MOON_PlotList		PlotManager::plotList

	extern class Plotter;
	class PlotManager {
	public:
		static unsigned int cnt;
		static std::vector<Plotter*> plotList;

		static unsigned int GetID() {
			return cnt++;
		}

		static void Release() {
			for (auto &plt : plotList) delete plt;
			plotList.clear();
		}
	};

	class Plotter {
	public:
		bool open;
		std::string wndName;
		unsigned int id;
		ImGui::PlotInterface plot;
		std::vector<ImGui::PlotItem> items;

		Plotter(const std::string &title, const std::string &x_axis_label = "X-Axis",
			const std::string &y_axis_label = "Y-Axis") : open(false), wndName(""), 
			id(PlotManager::GetID()) {
			plot.title = title;
			plot.x_axis.label = x_axis_label;
			plot.y_axis.label = y_axis_label;
			plot.x_axis.minimum = 0;
			plot.x_axis.maximum = 1;
			PlotManager::plotList.push_back(this);
		}

		Plotter() : open(false), wndName(""), id(PlotManager::GetID()) {
			plot.title = "New Plot_" + std::to_string(id);
			plot.x_axis.label = "X-Axis";
			plot.y_axis.label = "Y-Axis";
			plot.x_axis.minimum = 0;
			plot.x_axis.maximum = 1;
			PlotManager::plotList.push_back(this);
		}

		void SetRange(const float& x_start, const float& x_end,
			const float& y_start, const float& y_end) {
			plot.x_axis.minimum = x_start;
			plot.x_axis.maximum = x_end;

			plot.y_axis.minimum = y_start;
			plot.y_axis.maximum = y_end;
		}

		void SetRangeY(const float& start, const float& end) {
			plot.y_axis.minimum = start;
			plot.y_axis.maximum = end;
		}

		void SetRangeX(const float& start, const float& end) {
			plot.x_axis.minimum = start;
			plot.x_axis.maximum = end;
		}

		void AddItem(const std::string &label, const unsigned int &type,
			const float &size, const ImVec4 &color, const std::vector<ImVec2> &data) {
			items.push_back(ImGui::PlotItem());
			items.back().label = label;
			items.back().type = (ImGui::PlotItem::Type)type;
			items.back().size = size;
			items.back().color = color;
			items.back().data = data;
		}

		void UpdateData(const int& id, const std::vector<ImVec2> &data) {
			items[id].data = data;
		}

		// TODO
		/*void AddItem(const std::string &label, const ImGui::PlotItem::Type &type,
			const float &size, const ImVec4 &color, const Matrix &data) {
			items.push_back(ImGui::PlotItem());
			int iid = items.size() - 1;
			items[iid].label = label;
			items[iid].type = type;
			items[iid].size = size;
			items[iid].color = color;

			std::vector<ImVec2> vecData;

			items[iid].data = data;
		}*/

		// draw plot to any imgui UI context
		void Draw(const ImVec2 size = ImVec2(-1, -1)) {
			ImGui::Plot(plot.title.c_str(), plot, items, size);
		}

		// *NOTE: call 'SpecifyWindow' to specify a window before draw plot!
		void DrawInWindow() {
			ImGui::Begin(wndName.c_str(), &open);
			Draw();
			ImGui::End();
		}

		void SpecifyWindow(const std::string &wndName) {
			ImGui::SetNextWindowSize(ImVec2(400, 300));
			this->wndName = wndName;
			this->open = true;
		}

		// close plot window
		void Close() {
			open = false;
		}
	};

}