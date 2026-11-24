# TO Do Flow

A clean Raylib-based task manager for a Data Structures project.

## Build

```powershell
g++ -std=c++17 main.cpp views.cpp dialogs.cpp -o TO_Do_Flow.exe -lraylib -lopengl32 -lgdi32 -lwinmm
```

## Main changes in this version

- Dashboard now shows all tasks directly.
- Removed the separate **All Tasks** and **Daily Schedule** pages.
- Added seconds to the clock display and task time picker.
- Added a **Day mode / Dark mode** toggle button.
- App branding updated everywhere to **TO Do Flow**.
- Calendar picker now selects the date immediately on click.
- Time picker supports typing directly into the hour, minute and second boxes.
- The dial updates the top time boxes live while dragging.
- Removed the keyboard toggle from the time picker.
- Removed the **Next task** and **Progress** cards from the dashboard.
