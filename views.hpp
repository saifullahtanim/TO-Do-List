#pragma once

#include "app.hpp"

void drawSidebar(App& app, bool enabled);
void drawHeader(App& app, bool enabled, const std::string& subtitle);
void drawDashboard(App& app, const std::vector<Task>& all, const std::vector<Task>& pending, bool enabled);
std::vector<Task> getRows(App& app, const std::vector<Task>& all, const std::vector<Task>& pending);
std::string subtitleFor(const App& app, size_t shown);
void drawListPage(App& app, const std::vector<Task>& rows, bool enabled);
Rectangle reminderBounds(const App& app);
void drawReminder(App& app, Rectangle bounds);
void drawToast(const App& app);

void drawDeveloperInfo(App& app);
