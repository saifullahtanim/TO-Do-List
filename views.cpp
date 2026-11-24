#include "views.hpp"
#include "dialogs.hpp"

static std::string countText(size_t n)
{
    return std::to_string(n) + (n == 1 ? " task" : " tasks");
}


void drawSidebar(App& app, bool enabled)
{
    DrawRectangle(0, 0, static_cast<int>(sideW), H, sidebar);
    DrawLine(static_cast<int>(sideW), 0, static_cast<int>(sideW), H, border);

    text("TO Do Flow", 24, 27, 28, ink, true);
    text("Task scheduler", 25, 62, 14, muted);
    divider(24, 96, 172);

    for (int i = 0; i < static_cast<int>(sizeof(menu) / sizeof(menu[0])); ++i)
    {
        Rectangle item{12, i == 4 ? H - 194.0f : 116.0f + i * 46, 196, 40};
        bool hot = enabled && inside(item);
        if (hot) wantHand = true;
        if (enabled && hit(item)) app.goTo(i);

        bool active = app.selected == i;
        if (active)
        {
            rounded(item, 6, raised);
            DrawRectangleRec({item.x, item.y + 8, 3, 24}, accent);
        }
        else if (hot)
            rounded(item, 6, cardHover);

        if (i == 4)
        {
            drawIcon(Icon::History, {29, item.y + 20}, 9, active ? ink : muted);
            text(menu[i], 45, item.y + 10, 18, active ? ink : muted, active);
        }
        else text(menu[i], 26, item.y + 9, 20, active ? ink : muted, active);
    }

    divider(24, H - 140, 172);
    text(localNow("%H:%M:%S"), 24, H - 125, 26, ink, true);
    text(localNow("%d %B %Y"), 25, H - 92, 14, muted);
    text("Saved on this device", 24, H - 40, 18, ink);
}

static bool headerIcon(Rectangle r, Icon icon, bool enabled)
{
    bool hot = enabled && inside(r);
    if (hot) wantHand = true;
    rounded(r, 7, hot ? cardHover : raised);
    drawIcon(icon, {r.x + r.width / 2, r.y + r.height / 2}, 11,
             enabled ? ink : Fade(muted, 0.5f));
    return enabled && hit(r);
}

void drawHeader(App& app, bool enabled, const std::string& subtitle)
{
    text(menu[app.selected], contentX, 22, 34, ink, true);
    clipped(subtitle, contentX, 62, 17, 480, muted);

    Rectangle newBtn{contentR - 148, 26, 148, 42};
    Rectangle themeBtn{newBtn.x - 12 - 44, 26, 44, 42};
    Rectangle developerBtn{themeBtn.x - 10 - 44, 26, 44, 42};

    if (headerIcon(developerBtn, Icon::User, enabled))
    {
        app.developerOpen = true;
        app.searchFocus = false;
    }
    if (headerIcon(themeBtn, darkTheme ? Icon::Sun : Icon::Moon, enabled))
        toggleTheme();
    if (primaryButton(newBtn, "New task", enabled)) app.openForm(0);
}

static void developerLink(Rectangle r, const char* label, const char* address)
{
    bool hot = inside(r);
    if (hot) wantHand = true;
    rounded(r, 7, hot ? raised : field);
    text(label, r.x + 14, r.y + 8, 19, ink, true);
    clipped(address, r.x + 14, r.y + 32, 15, r.width - 58, muted);
    drawIcon(Icon::ExternalLink, {r.x + r.width - 20, r.y + r.height / 2},
             9, hot ? accent : muted);
    if (hit(r)) OpenURL(address);
}

void drawDeveloperInfo(App& app)
{
    DrawRectangle(0, 0, W, H, Color{0, 0, 0, 145});
    // Leave room for the reminder notification on the right.
    Rectangle d{282, 116, 460, 440};
    shadow(d, 14);
    panel(d, 14);
    float x = d.x + 24, y = d.y, w = d.width - 48;

    text("Developer info", x, y + 20, 24, ink, true);
    if (iconButton({d.x + d.width - 52, y + 15, 34, 34}, Icon::Close, muted, true, 8))
    {
        app.developerOpen = false;
        return;
    }
    divider(x, y + 61, w);

    Color tint = darkTheme ? Color{35, 60, 62, 255} : Color{222, 240, 241, 255};
    rounded({x, y + 80, 62, 62}, 12, tint);
    drawIcon(Icon::User, {x + 31, y + 111}, 20,
             darkTheme ? accent : Color{31, 107, 111, 255});
    text("Saifulla Tanim", x + 77, y + 84, 26, ink, true);
    text("ID: 222002014", x + 78, y + 120, 18, muted);

    developerLink({x, y + 160, w, 62}, "Website", "https://www.saifullatanim.me/");
    developerLink({x, y + 230, w, 62}, "GitHub", "https://github.com/saifullahtanim");
    developerLink({x, y + 300, w, 62}, "LinkedIn", "https://www.linkedin.com/in/saifullahtanim/");

    if (softButton({x + w - 100, y + 384, 100, 38}, "Close"))
        app.developerOpen = false;
}

void drawRow(App& app, const Task& task, Rectangle r, bool enabled)
{
    bool hot = enabled && inside(r);
    rounded(r, 12, border);
    rounded({r.x + 1, r.y + 1, r.width - 2, r.height - 2}, 11, hot ? cardHover : card);

    rounded({r.x + 9, r.y + 13, 4, r.height - 26}, 2, task.completed ? muted : taskColors[task.colorIndex]);
    float cy = r.y + r.height / 2;

    Vector2 c{r.x + 38, cy};
    Rectangle boxArea{c.x - 18, c.y - 18, 36, 36};
    bool boxHot = enabled && inside(boxArea);
    if (boxHot) wantHand = true;
    if (task.completed)
    {
        DrawCircleV(c, 13, boxHot ? ColorBrightness(green, 0.12f) : green);
        drawIcon(Icon::Check, c, 8, darkTheme ? bg : WHITE);
    }
    else
    {
        if (boxHot) DrawCircleV(c, 16, Fade(accent, 0.15f));
        DrawRing(c, 11.0f, 13.0f, 0, 360, 64, boxHot ? accent : muted);
        if (boxHot) drawIcon(Icon::Check, c, 7, accent);
    }
    if (enabled && hit(boxArea)) app.toggleCompleted(task.id);

    Rectangle editBtn{r.x + r.width - 92, cy - 17, 34, 34};
    Rectangle delBtn{r.x + r.width - 52, cy - 17, 34, 34};
    Rectangle priority{editBtn.x - 12 - 78, cy - 12, 78, 24};
    float dateX = priority.x - 14 - 152;
    float textX = r.x + 62;
    float titleW = dateX - 16 - textX;

    float ty = task.description.empty() ? cy - 15 : cy - 27;
    std::string title = task.name;
    if (width(title, 24, true) > titleW)
    {
        while (!title.empty() && width(title + "...", 24, true) > titleW)
            title.pop_back();
        title += "...";
    }
    text(title, textX, ty, 24, task.completed ? muted : ink, true);
    if (task.completed)
        line(textX, ty + 15, textX + width(title, 24, true), ty + 15, 1.5f, muted);

    if (!task.description.empty())
        clipped(task.description, textX, cy + 5, 18, titleW, muted);

    bool late = isOverdue(task);
    clipped(relativeDay(task.deadline), dateX, cy - 24, 19,
            150, late ? red : ink, true);
    clipped((task.allDay ? std::string("All day") : displayTime(task.dueTime)) +
            (late ? " overdue" : ""), dateX, cy + 4, 17, 150,
            late ? red : muted);

    bool missed = app.selected == 4 && !task.completed && late;
    centerText(task.completed ? "Done" : missed ? "Missed" : priorityName(task.priority),
               priority, 18, task.completed ? green : missed ? red : priorityColor(task.priority), true);
    if (iconButton(editBtn, Icon::Pencil, ink, enabled, 10)) app.openForm(task.id);
    if (iconButton(delBtn, Icon::Trash, inside(delBtn) && enabled ? red : muted, enabled, 10)) app.deleteId = task.id;
}

void drawEmpty(App& app, Rectangle view, const std::string& title, const std::string& hint,
               bool offerNew, bool enabled)
{
    float y = view.y + 130;
    centerText(title, {view.x, y, view.width, 30}, 21, ink, true);
    centerText(hint, {view.x, y + 34, view.width, 24}, 15, muted);
    if (offerNew && primaryButton({view.x + view.width / 2 - 70, y + 74, 140, 40}, "New task", enabled))
        app.openForm(0);
}

void drawList(App& app, const std::vector<Task>& rows, Rectangle view, bool enabled)
{
    const float rowH = 80, step = 90;
    float maxScroll = std::max(0.0f, rows.size() * step - (step - rowH) - view.height);
    if (enabled && inside(view)) app.scroll -= GetMouseWheelMove() * 60;
    app.scroll = std::clamp(app.scroll, 0.0f, maxScroll);
    bool rowsEnabled = enabled && inside(view);

    BeginScissorMode(static_cast<int>(view.x - 4), static_cast<int>(view.y),
                     static_cast<int>(view.width + 8), static_cast<int>(view.height));
    for (size_t i = 0; i < rows.size(); ++i)
    {
        float y = view.y + i * step - app.scroll;
        if (y + rowH < view.y || y > view.y + view.height) continue;
        drawRow(app, rows[i], {view.x, y, view.width, rowH}, rowsEnabled);
    }
    EndScissorMode();

    if (maxScroll > 0)
    {
        float trackH = view.height;
        float thumbH = std::max(36.0f, trackH * view.height / (view.height + maxScroll));
        float thumbY = view.y + (trackH - thumbH) * (app.scroll / maxScroll);
        rounded({view.x + view.width + 10, view.y, 4, trackH}, 2, Fade(border, 0.6f));
        rounded({view.x + view.width + 10, thumbY, 4, thumbH}, 2, muted);
    }
}

void drawStat(Rectangle r, const std::string& title, int value, Color color)
{
    panel(r);
    text(title, r.x + 16, r.y + 12, 18, muted);
    text(std::to_string(value), r.x + 16, r.y + 37, 34, color, true);
}

void drawDashboard(App& app, const std::vector<Task>& all, const std::vector<Task>& pending, bool enabled)
{
    int overdue = 0;
    for (const Task& t : pending) if (isOverdue(t)) ++overdue;
    int done = static_cast<int>(all.size() - pending.size());
    float cardW = (contentW - 3 * 14) / 4;
    drawStat({contentX, 98, cardW, 88}, "Total tasks", static_cast<int>(all.size()), accent);
    drawStat({contentX + (cardW + 14), 98, cardW, 88}, "Pending", static_cast<int>(pending.size()), yellow);
    drawStat({contentX + 2 * (cardW + 14), 98, cardW, 88}, "Completed", done, green);
    drawStat({contentX + 3 * (cardW + 14), 98, cardW, 88}, "Overdue", overdue, red);

    auto upcoming = upcomingTasks(pending, currentSecond());
    text("Upcoming tasks", contentX, 215, 24, ink, true);
    text(countText(upcoming.size()), contentR - width(countText(upcoming.size()), 17), 220, 17, muted);

    Rectangle view{contentX, 252, contentW, 402};
    if (upcoming.empty())
        drawEmpty(app, view, "No upcoming tasks", "Completed and missed tasks are in Activity History.", true, enabled);
    else drawList(app, upcoming, view, enabled);
}

std::vector<Task> getRows(App& app, const std::vector<Task>& all, const std::vector<Task>& pending)
{
    std::vector<Task> rows;
    if (app.selected == 1) rows = pending;
    else if (app.selected == 2)
    {
        for (const Task& task : all)
            if (!task.completed) rows.push_back(task);
        insertionSort(rows, earlierDeadline);
    }
    else if (app.selected == 3)
    {
        rows = app.scheduler.history();
        if (app.sortByDeadline)
            insertionSort(rows, earlierDeadline);
        else
            insertionSort(rows, [](const Task& a, const Task& b) { return a.id < b.id; });
    }

    else if (app.selected == 4)
    {
        rows = activityTasks(all, currentSecond());
        if (app.filter != 0)
        {
            bool completed = app.filter == 1;
            rows.erase(std::remove_if(rows.begin(), rows.end(), [completed](const Task& task) {
                return task.completed != completed;
            }), rows.end());
        }
    }

    if ((app.selected == 1 || app.selected == 2) && app.filter != 0)
    {
        std::string today = localNow("%Y-%m-%d");
        std::vector<Task> kept;
        for (const Task& task : rows)
            if ((app.filter == 1 && task.deadline == today) ||
                (app.filter == 2 && isOverdue(task)) ||
                (app.filter == 3 && task.priority == 3)) kept.push_back(task);
        rows = std::move(kept);
    }

    if (!app.query.empty())
    {
        std::vector<Task> matches;
        std::string key = lower(app.query);
        for (const Task& task : rows)
            if (lower(task.name).find(key) != std::string::npos ||
                lower(task.description).find(key) != std::string::npos) matches.push_back(task);
        rows = std::move(matches);
    }
    return rows;
}

std::string subtitleFor(const App& app, size_t shown)
{
    switch (app.selected)
    {
    case 0: return localNow("%A, %d %B %Y");
    case 1: return "Highest priority first; earliest deadline breaks ties.";
    case 2: return countText(shown) + " pending";
    case 3: return countText(shown) + " completed";
    case 4: return "Completed and missed tasks";
    default: return "";
    }
}

void drawToolbar(App& app, bool enabled)
{
    Rectangle search{contentX, 98, 300, 42};
    if (enabled && clicked) app.searchFocus = inside(search);
    std::string before = app.query;
    input(search, app.query, enabled && app.searchFocus, "Search tasks...", 45, false, 42);
    drawIcon(Icon::Search, {search.x + 22, search.y + 21}, 7, app.searchFocus ? accent : muted);
    if (before != app.query) app.scroll = 0;
    if (app.searchFocus && IsKeyPressed(KEY_ENTER)) app.searchFocus = false;

    if (app.selected == 1 || app.selected == 2)
    {
        const char* names[] = {"All", "Today", "Overdue", "High"};
        float x = search.x + search.width + 14;
        for (int i = 0; i < 4; ++i)
        {
            float w = width(names[i], 17, true) + 32;
            if (chip({x, 100, w, 38}, names[i], app.filter == i, enabled))
            {
                app.filter = i;
                app.scroll = 0;
            }
            x += w + 8;
        }
    }
    if (app.selected == 4)
    {
        const char* names[] = {"All", "Completed", "Missed"};
        float x = search.x + search.width + 14;
        for (int i = 0; i < 3; ++i)
        {
            float w = width(names[i], 17, true) + 32;
            if (chip({x, 100, w, 38}, names[i], app.filter == i, enabled))
            {
                app.filter = i;
                app.scroll = 0;
            }
            x += w + 8;
        }
    }
    if (app.selected == 3)
        if (softButton({contentR - 150, 98, 150, 42}, app.sortByDeadline ? "Sort: Deadline" : "Sort: ID", enabled))
        {
            app.sortByDeadline = !app.sortByDeadline;
            app.scroll = 0;
        }
}

void drawListPage(App& app, const std::vector<Task>& rows, bool enabled)
{
    drawToolbar(app, enabled);
    Rectangle view{contentX, 158, contentW, 496};
    if (rows.empty())
    {
        bool searching = !app.query.empty() || app.filter != 0;
        if (searching) drawEmpty(app, view, "No matching tasks", "Change the search or filter.", false, enabled);
        else if (app.selected == 4)
            drawEmpty(app, view, "No activity yet", "Completed and missed tasks will appear here.", false, enabled);
        else if (app.selected == 3)
            drawEmpty(app, view, "No completed tasks yet", "Completed tasks will appear here.", false, enabled);
        else drawEmpty(app, view, "No pending tasks", "Select New task to add one.", true, enabled);
    }
    else drawList(app, rows, view, enabled);
}

Rectangle reminderBounds(const App& app)
{
    const float w = 358;
    return {W - w - 20, app.blocked() ? 110.0f : 88.0f, w, 232};
}

void drawReminder(App& app, Rectangle r)
{
    int id = app.activeReminderId;
    if (!id) return;
    Task* task = app.scheduler.find(id);
    if (!task || task->completed || task->reminderSeen) return;

    const float x = r.x + 18, right = r.x + r.width - 18;
    shadow(r, 12);
    panel(r, 12);

    // A small bell and a readable heading, in either theme.
    Color tint = darkTheme ? Color{35, 60, 62, 255} : Color{222, 240, 241, 255};
    Color label = darkTheme ? accent : Color{31, 107, 111, 255};
    rounded({x, r.y + 16, 36, 36}, 9, tint);
    drawIcon(Icon::Bell, {x + 18, r.y + 34}, 9, label);
    bool due = secondNumber(localNow("%Y-%m-%d"), localNow("%H:%M:%S")) >=
               secondNumber(task->deadline, task->dueTime);
    text(due ? "Task is due" : "Upcoming task", x + 46, r.y + 20, 23, ink, true);
    if (iconButton({right - 28, r.y + 17, 28, 32}, Icon::Close, muted))
    {
        app.dismissReminder(id);
        return;
    }

    clipped(task->name, x, r.y + 65, 23, r.width - 36, ink, true);
    clipped(relativeDay(task->deadline) + "  |  " +
            (task->allDay ? string("All day") : displayTime(task->dueTime)),
            x, r.y + 97, 18, r.width - 36, muted);

    float left = static_cast<float>(std::max(0.0, app.alarmEnd - GetTime()));
    float ratio = std::clamp(left / static_cast<float>(alarmSeconds), 0.0f, 1.0f);
    string countdown = std::to_string(static_cast<int>(std::ceil(left))) + "s";
    if (app.reminderIds.size() > 1)
        countdown += "  |  +" + std::to_string(app.reminderIds.size() - 1) + " more";
    text(countdown, x, r.y + 131, 16, muted, true);
    if (app.ringing() && ghostButton({right - 66, r.y + 122, 66, 30}, "Mute"))
        app.alarmMuted = true;
    rounded({x, r.y + 157, r.width - 36, 5}, 2.5f, raised);
    if (ratio > 0)
        rounded({x, r.y + 157, (r.width - 36) * ratio, 5}, 2.5f, accent);

    float bw = (r.width - 36 - 16) / 3;
    float by = r.y + 176;
    if (softButton({x, by, bw, 40}, "Snooze")) app.snoozeReminder(id);
    else if (softButton({x + bw + 8, by, bw, 40}, "Dismiss")) app.dismissReminder(id);
    else
    {
        Color fill = darkTheme ? Color{33, 88, 65, 255} : Color{214, 240, 224, 255};
        Color fg = darkTheme ? Color{235, 250, 241, 255} : Color{25, 94, 62, 255};
        if (buttonBase({x + 2 * (bw + 8), by, bw, 40}, "Done", fill,
                       ColorBrightness(fill, darkTheme ? 0.08f : -0.06f), fg, true, true))
        {
            app.complete(id);
            app.nextCheck = 0;
        }
    }
}

void drawToast(const App& app)
{
    if (GetTime() >= app.toastUntil || app.toast.empty()) return;
    float w = std::min(contentW, width(app.toast, 15) + 32);
    Rectangle r{contentX, H - 38.0f, w, 30};
    rounded(r, 5, raised);
    clipped(app.toast, r.x + 12, r.y + 5, 15, w - 24, ink);
}
