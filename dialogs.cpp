#include "dialogs.hpp"

void openCalendar(TaskForm& form)
{
    form.draftDate = validDate(form.date) ? form.date : localNow("%Y-%m-%d");
    form.year = std::stoi(form.draftDate.substr(0, 4));
    form.month = std::stoi(form.draftDate.substr(5, 2));
    form.picker = 1;
    form.focus = 0;
}

void drawCalendar(TaskForm& form)
{
    DrawRectangle(0, 0, W, H, Color{0, 0, 0, 170});
    const float ox = 375, oy = 64;
    Rectangle dialog{ox, oy, 370, 510};
    shadow(dialog, 18);
    panel(dialog, 18);
    rounded({ox, oy, 370, 108}, 18, accent);
    DrawRectangle(static_cast<int>(ox), static_cast<int>(oy + 80), 370, 28, accent);
    text(form.draftDate.substr(0, 4), ox + 26, oy + 16, 17, accentDark);
    text(readableDate(form.draftDate, true), ox + 26, oy + 42, 32, accentDark, true);

    if (iconButton({ox + 20, oy + 124, 38, 34}, Icon::Left, ink, form.year > 1900 || form.month > 1))
        if (--form.month == 0) { form.month = 12; --form.year; }
    if (iconButton({ox + 312, oy + 124, 38, 34}, Icon::Right, ink, form.year < 9999 || form.month < 12))
        if (++form.month == 13) { form.month = 1; ++form.year; }
    centerText(std::string(months[form.month - 1]) + " " + std::to_string(form.year),
               {ox + 61, oy + 124, 248, 34}, 20, ink, true);

    const char* weekdays[] = {"M", "T", "W", "T", "F", "S", "S"};
    for (int i = 0; i < 7; ++i)
        centerText(weekdays[i], {ox + 25 + i * 46.0f, oy + 170, 44, 30}, 14, muted, true);

    int first = (weekday(form.year, form.month, 1) + 6) % 7;
    std::string today = localNow("%Y-%m-%d");
    for (int day = 1; day <= monthDays(form.year, form.month); ++day)
    {
        int index = first + day - 1;
        Rectangle cell{ox + 25 + (index % 7) * 46.0f, oy + 202 + (index / 7) * 43.0f, 44, 42};
        std::string date = formatDate(form.year, form.month, day);
        Vector2 mid{cell.x + 22, cell.y + 21};
        if (inside(cell)) wantHand = true;
        bool picked = date == form.draftDate;
        if (picked) DrawCircleV(mid, 20, accent);
        else if (inside(cell)) DrawCircleV(mid, 20, raised);
        else if (date == today) DrawRing(mid, 18.5f, 20, 0, 360, 32, accent);
        centerText(std::to_string(day), cell, 19, picked ? accentDark : ink, picked || date == today);
        if (hit(cell))
        {
            form.draftDate = date;
            form.date = date;
            form.error.clear();
            form.picker = 0;
            return;
        }
    }
    text("Click a date to select it.", ox + 26, oy + 478, 16, muted);
}

static bool replaceTimeDigits = true;

void openTimePicker(TaskForm& form)
{
    std::string value = normalizeTime(validTime(form.time) ? form.time : "09:00:00");
    form.hour = value.substr(0, 2);
    form.minute = value.substr(3, 2);
    form.second = value.substr(6, 2);
    form.picker = 2;
    form.focus = 3;
    replaceTimeDigits = true;
    form.pickerError.clear();
}

static int clampPart(const std::string& s, int upper)
{
    if (s.empty()) return 0;
    int v = std::stoi(s);
    if (v < 0) v = 0;
    if (v > upper) v = upper;
    return v;
}

static Vector2 dialPoint(Vector2 center, int value, int divisions, float radius)
{
    constexpr float pi = 3.14159265358979323846f;
    float angle = 2 * pi * value / divisions;
    return {center.x + radius * std::sin(angle), center.y - radius * std::cos(angle)};
}

static int dialValue(Vector2 center, Vector2 mouse, int divisions)
{
    constexpr float pi = 3.14159265358979323846f;
    float angle = std::atan2(mouse.x - center.x, center.y - mouse.y);
    return (static_cast<int>(std::round(angle * divisions / (2 * pi))) + divisions) % divisions;
}

static void drawTimeBox(Rectangle box, const std::string& value,
                        const char* label, bool active)
{
    Color fill = active ? (darkTheme ? Color{39, 65, 71, 255}
                                    : Color{219, 239, 241, 255}) : field;
    rounded(box, 12, active ? accent : border);
    rounded({box.x + 2, box.y + 2, box.width - 4, box.height - 4}, 10, fill);
    centerText(label, {box.x, box.y + 7, box.width, 18}, 15, muted, true);
    centerText(value.empty() ? "--" : value,
               {box.x, box.y + 25, box.width, box.height - 29}, 38, ink, true);
    if (inside(box)) wantText = true;
    if (active && !replaceTimeDigits && static_cast<int>(GetTime() * 2) % 2 == 0)
    {
        float x = box.x + (box.width + width(value, 38, true)) / 2 + 3;
        line(x, box.y + 34, x, box.y + box.height - 11, 1.5f, accent);
    }
}

void drawTimePicker(TaskForm& form)
{
    DrawRectangle(0, 0, W, H, Color{0, 0, 0, 170});
    const float ox = 320, oy = 56;
    Rectangle dialog{ox, oy, 480, 568};
    shadow(dialog, 18);
    panel(dialog, 18);
    text("Select time", ox + 27, oy + 20, 26, ink, true);

    Rectangle boxes[] = {
        {ox + 42, oy + 66, 108, 86},
        {ox + 186, oy + 66, 108, 86},
        {ox + 330, oy + 66, 108, 86}
    };
    std::string* parts[] = {&form.hour, &form.minute, &form.second};
    const char* labels[] = {"Hour", "Minute", "Second"};
    for (int i = 0; i < 3; ++i)
    {
        if (hit(boxes[i]))
        {
            form.focus = i + 3;
            replaceTimeDigits = true;
        }
    }
    if (IsKeyPressed(KEY_TAB))
    {
        form.focus = form.focus == 5 ? 3 : form.focus + 1;
        replaceTimeDigits = true;
    }
    int mode = std::clamp(form.focus - 3, 0, 2);
    std::string& value = *parts[mode];
    for (int ch = GetCharPressed(); ch > 0; ch = GetCharPressed())
    {
        if (ch < '0' || ch > '9') continue;
        if (replaceTimeDigits || value.size() >= 2) value.clear();
        replaceTimeDigits = false;
        value.push_back(static_cast<char>(ch));
        form.pickerError.clear();
    }
    if (IsKeyPressed(KEY_BACKSPACE))
    {
        if (replaceTimeDigits) value.clear();
        else if (!value.empty()) value.pop_back();
        replaceTimeDigits = false;
    }

    Vector2 center{ox + 240, oy + 322};
    const float outer = 118, inner = 76;
    Vector2 mouse = GetMousePosition();
    float dx = mouse.x - center.x, dy = mouse.y - center.y;
    float distance = std::sqrt(dx * dx + dy * dy);
    bool dragZone = distance >= 48 && distance <= 148;
    if (dragZone) wantHand = true;
    if (dragZone && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
    {
        int picked = mode == 0 ? dialValue(center, mouse, 12) : dialValue(center, mouse, 60);
        if (mode == 0 && distance < 96) picked += 12;
        value = twoDigits(picked);
        replaceTimeDigits = true;
        form.pickerError.clear();
        clicked = false;
    }

    // Draw after input so both the dial and the boxes show the current value.
    for (int i = 0; i < 3; ++i)
        drawTimeBox(boxes[i], *parts[i], labels[i], mode == i);
    centerText(":", {ox + 150, oy + 88, 36, 60}, 38, ink, true);
    centerText(":", {ox + 294, oy + 88, 36, 60}, 38, ink, true);

    DrawCircleV(center, 148, raised);
    DrawRing(center, 147, 148, 0, 360, 96, border);
    int selected = clampPart(value, mode == 0 ? 23 : 59);
    float radius = mode == 0 && selected >= 12 ? inner : outer;
    Vector2 chosen = dialPoint(center, mode == 0 ? selected % 12 : selected,
                               mode == 0 ? 12 : 60, radius);
    DrawLineEx(center, chosen, 2.5f, accent);
    DrawCircleV(center, 4, accent);
    DrawCircleV(chosen, 19, Fade(accent, 0.40f));

    if (mode == 0)
    {
        for (int i = 0; i < 24; ++i)
        {
            Vector2 point = dialPoint(center, i % 12, 12, i < 12 ? outer : inner);
            centerText(twoDigits(i), {point.x - 18, point.y - 18, 36, 36},
                       20, ink, true);
        }
    }
    else
    {
        for (int i = 0; i < 60; ++i)
        {
            if (i % 5 == 0)
            {
                Vector2 point = dialPoint(center, i, 60, outer);
                centerText(twoDigits(i), {point.x - 19, point.y - 19, 38, 38},
                           21, ink, true);
            }
            else DrawCircleV(dialPoint(center, i, 60, 136), 1.5f, muted);
        }
    }

    text("Type in a box or drag the clock hand.", ox + 27, oy + 478, 16, ink);
    if (!form.pickerError.empty()) clipped(form.pickerError, ox + 27, oy + 503, 15, 420, red);
    if (ghostButton({ox + 280, oy + 522, 88, 38}, "Cancel")) form.picker = 0;
    if (primaryButton({ox + 378, oy + 522, 74, 38}, "OK") || IsKeyPressed(KEY_ENTER))
    {
        if (form.hour.empty() || form.minute.empty() || form.second.empty())
        {
            form.pickerError = "Enter hours, minutes and seconds.";
            return;
        }
        int h = std::stoi(form.hour), m = std::stoi(form.minute), s = std::stoi(form.second);
        if (h > 23 || m > 59 || s > 59)
        {
            form.pickerError = "Use 00-23 hours and 00-59 minutes/seconds.";
            return;
        }
        form.time = twoDigits(h) + ":" + twoDigits(m) + ":" + twoDigits(s);
        form.picker = 0;
        form.focus = 0;
        form.error.clear();
    }
}

static void saveForm(App& app)
{
    TaskForm& form = app.form;
    size_t start = form.name.find_first_not_of(' ');
    if (start == std::string::npos) { form.error = "Please enter a task title."; form.focus = 1; return; }
    if (!validDate(form.date) || !validTime(form.time))
    {
        form.error = "Pick a valid date and time.";
        return;
    }

    std::string title = form.name.substr(start, form.name.find_last_not_of(' ') - start + 1);
    std::string time = normalizeTime(form.time);
    bool ok = form.editId
        ? app.scheduler.edit(form.editId, title, form.priority, form.date, time,
                             form.description, false, 0, form.colorIndex)
        : app.scheduler.add(title, form.priority, form.date, time,
                            form.description, false, 0, form.colorIndex);
    if (!ok) { form.error = "Could not save the task."; return; }
    if (form.editId) app.forget(form.editId);
    app.save(form.editId ? "Task updated" : "Task created");
    form.open = false;
    app.scroll = 0;
    app.nextCheck = 0;
}

void drawForm(App& app)
{
    TaskForm& form = app.form;
    DrawRectangle(0, 0, W, H, Color{0, 0, 0, 170});

    Rectangle d{280, 135, 560, 410};
    shadow(d, 16);
    panel(d, 16);

    const float x = d.x + 28;
    const float w = d.width - 56;
    const float y = d.y;
    bool on = form.picker == 0;

    text(form.editId ? "Edit task" : "New task", x, y + 22, 28, ink, true);
    if (iconButton({d.x + d.width - 64, y + 18, 38, 38}, Icon::Close, muted, on))
    {
        form.open = false;
        return;
    }
    divider(x, y + 66, w);

    // Task title
    Rectangle name{x, y + 101, w, 46};
    if (on && clicked && inside(name)) form.focus = 1;
    if (on && IsKeyPressed(KEY_TAB)) form.focus = 1;
    text("Title", x, y + 78, 17, ink, true);
    input(name, form.name, on && form.focus == 1, "Task title", 65);

    // Scheduled date and time
    text("Date", x, y + 165, 17, ink, true);
    text("Time", x + 288, y + 165, 17, ink, true);
    if (fieldButton({x, y + 188, 272, 46}, readableDate(form.date), Icon::Calendar, on))
        openCalendar(form);
    if (fieldButton({x + 288, y + 188, w - 288, 46}, displayTime(form.time), Icon::Clock, on))
        openTimePicker(form);

    // Task priority
    text("Priority", x, y + 250, 17, ink, true);
    float segW = (w - 16) / 3;
    for (int p = 1; p <= 3; ++p)
    {
        Rectangle r{x + (p - 1) * (segW + 8), y + 274, segW, 42};
        bool chosen = form.priority == p;
        bool hot = on && inside(r);
        if (hot) wantHand = true;
        if (on && hit(r))
        {
            form.priority = p;
            chosen = true;
        }
        Color tone = priorityColor(p);
        Color fill = hot ? raised : field;
        if (chosen)
        {
            const Color light[] = {{224, 244, 235, 255}, {251, 241, 217, 255}, {250, 230, 235, 255}};
            const Color dark[] = {{28, 58, 46, 255}, {61, 50, 28, 255}, {62, 39, 45, 255}};
            fill = darkTheme ? dark[p - 1] : light[p - 1];
        }
        rounded(r, 9, chosen ? tone : border);
        rounded({r.x + 2, r.y + 2, r.width - 4, r.height - 4}, 7, fill);
        DrawCircleV({r.x + 17, r.y + r.height / 2}, 5, tone);
        centerText(priorityName(p), {r.x + 15, r.y, r.width - 15, r.height}, 20, ink, true);
    }

    if (!form.error.empty()) clipped(form.error, x, y + 324, 15, w, red);
    if (ghostButton({x + w - 230, y + 350, 90, 40}, "Cancel", on))
    {
        form.open = false;
        return;
    }
    if (primaryButton({x + w - 130, y + 350, 130, 40}, "Save task", on) ||
        (form.picker == 0 && on && IsKeyPressed(KEY_ENTER)))
    {
        saveForm(app);
    }

    if (form.picker == 1) drawCalendar(form);
    else if (form.picker == 2) drawTimePicker(form);
}

void drawDeleteDialog(App& app)
{
    DrawRectangle(0, 0, W, H, Color{0, 0, 0, 185});
    Rectangle d{340, 210, 440, 240};
    shadow(d, 20);
    panel(d, 20);
    text("Delete this task?", d.x + 28, d.y + 26, 24, ink, true);
    if (Task* task = app.scheduler.find(app.deleteId)) clipped(task->name, d.x + 28, d.y + 70, 18, 384, red, true);
    text("Undo is available after deletion.", d.x + 28, d.y + 106, 14, muted);
    if (softButton({d.x + 28, d.y + 168, 180, 42}, "Cancel")) app.deleteId = 0;
    if (dangerButton({d.x + 232, d.y + 168, 180, 42}, "Delete"))
    {
        if (app.scheduler.remove(app.deleteId))
        {
            app.forget(app.deleteId);
            app.save("Task deleted");
        }
        app.deleteId = 0;
    }
}
