#include "views.hpp"
#include "dialogs.hpp"

void handleEscape(App& app)
{
    if (!IsKeyPressed(KEY_ESCAPE)) return;
    if (app.developerOpen) app.developerOpen = false;
    else if (app.form.picker) { app.form.picker = 0; app.form.focus = 0; }
    else if (app.form.open) app.form.open = false;
    else if (app.deleteId) app.deleteId = 0;
    else app.searchFocus = false;
}

void handleShortcuts(App& app)
{
    bool ctrl = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
    if (!ctrl || app.blocked()) return;
    bool typing = app.searchFocus;
    if (IsKeyPressed(KEY_N)) app.openForm(0);
    else if (IsKeyPressed(KEY_Z) && !typing && app.scheduler.canUndo())
    {
        app.scheduler.undo();
        app.save("Last change undone");
        app.nextCheck = 0;
    }
    else if (IsKeyPressed(KEY_F))
    {
        if (app.selected != 0) app.searchFocus = true;
    }
    else if (IsKeyPressed(KEY_T))
        toggleTheme();
}

int main()
{
    SetConfigFlags(FLAG_WINDOW_ALWAYS_RUN);
    InitWindow(W, H, "TO Do Flow");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);
    loadFonts();

    App app;
    InitAudioDevice();
    if (IsAudioDeviceReady() && FileExists("reminder.wav"))
    {
        app.sound = LoadSound("reminder.wav");
        app.soundReady = app.sound.frameCount > 0;
        if (app.soundReady) SetSoundVolume(app.sound, 0.85f);
    }
    if (FileExists(dataFile.c_str()) && !app.scheduler.load(dataFile))
    {
        if (app.scheduler.load(dataFile + ".bak")) app.say("Tasks recovered from backup", 5);
        else app.say("Could not load tasks.txt. Back up the file before adding tasks.", 8);
    }

    while (!WindowShouldClose())
    {
        app.checkReminders();
        app.updateAlarm();
        clicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
        wantHand = wantText = false;
        handleEscape(app);
        handleShortcuts(app);

        bool enabled = !app.blocked();
        bool hasReminder = !app.reminderIds.empty();
        Rectangle cardRect = reminderBounds(app);
        bool cardClick = hasReminder && clicked && inside(cardRect);
        if (cardClick) clicked = false;

        app.refreshTasks();
        const std::vector<Task>& all = app.allTasks;
        const std::vector<Task>& pending = app.pendingTasks;
        std::vector<Task> rows;
        if (app.selected != 0) rows = getRows(app, all, pending);

        BeginDrawing();
        ClearBackground(bg);
        drawSidebar(app, enabled);
        drawHeader(app, enabled, subtitleFor(app, rows.size()));
        if (app.selected == 0) drawDashboard(app, all, pending, enabled);
        else drawListPage(app, rows, enabled);
        if (app.form.open) drawForm(app);
        else if (app.deleteId) drawDeleteDialog(app);
        else if (app.developerOpen) drawDeveloperInfo(app);
        drawToast(app);
        if (hasReminder)
        {
            if (cardClick) clicked = true;
            drawReminder(app, cardRect);
        }
        SetMouseCursor(wantHand ? MOUSE_CURSOR_POINTING_HAND
                       : wantText ? MOUSE_CURSOR_IBEAM : MOUSE_CURSOR_DEFAULT);
        EndDrawing();
    }

    if (app.soundReady)
    {
        StopSound(app.sound);
        UnloadSound(app.sound);
    }
    if (IsAudioDeviceReady()) CloseAudioDevice();
    unloadFonts();
    CloseWindow();
    return 0;
}
