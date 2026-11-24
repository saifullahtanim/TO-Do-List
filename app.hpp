#pragma once

#include "ui.hpp"
#include <cmath>
#include <map>

using namespace ui;
using std::string;
using std::vector;

constexpr int W = 1120;
constexpr int H = 680;
constexpr float sideW = 220;
constexpr float contentX = 252;
constexpr float contentW = 836;
constexpr float contentR = contentX + contentW;
constexpr double alarmSeconds = 10;
constexpr double beepGap = 1.5;
inline const string dataFile = "tasks.txt";

inline const char* const menu[] = {"Dashboard", "Priority Queue", "Pending", "Completed", "Activity History"};

inline int monthDays(int year, int month)
{
    const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    bool leap = year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
    return days[month - 1] + (month == 2 && leap ? 1 : 0);
}

inline int weekday(int year, int month, int day)
{
    const int offsets[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    if (month < 3) --year;
    return (year + year / 4 - year / 100 + year / 400 + offsets[month - 1] + day) % 7;
}

inline const char* const months[] = {"January", "February", "March", "April", "May", "June",
                                     "July", "August", "September", "October", "November", "December"};
inline const char* const shortDays[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

inline string readableDate(const string& date, bool shortForm = false)
{
    if (!validDate(date)) return "Select date";
    int y = std::stoi(date.substr(0, 4));
    int m = std::stoi(date.substr(5, 2));
    int d = std::stoi(date.substr(8, 2));
    string day = shortDays[weekday(y, m, d)];
    if (shortForm) return day + ", " + std::to_string(d) + " " + string(months[m - 1]).substr(0, 3);
    return day + ", " + std::to_string(d) + " " + months[m - 1] + " " + std::to_string(y);
}

inline string twoDigits(int number)
{
    return (number < 10 ? "0" : "") + std::to_string(number);
}

inline string displayTime(const string& time)
{
    return normalizeTime(time);
}

inline int dayDiff(const string& date)
{
    long long a = minuteNumber(date, "00:00:00");
    long long b = minuteNumber(localNow("%Y-%m-%d"), "00:00:00");
    return static_cast<int>((a - b) / 1440);
}

inline string relativeDay(const string& date)
{
    int diff = dayDiff(date);
    if (diff == 0) return "Today";
    if (diff == 1) return "Tomorrow";
    if (diff == -1) return "Yesterday";
    return readableDate(date, true);
}

inline bool isOverdueAt(const Task& task, long long now)
{
    return !task.completed && secondNumber(task.deadline, task.dueTime) < now;
}

inline long long currentSecond()
{
    return secondNumber(localNow("%Y-%m-%d"), localNow("%H:%M:%S"));
}

inline bool isOverdue(const Task& task)
{
    return isOverdueAt(task, currentSecond());
}

inline vector<Task> upcomingTasks(const vector<Task>& pending, long long now)
{
    vector<Task> rows;
    for (const Task& task : pending)
        if (!task.completed && !isOverdueAt(task, now)) rows.push_back(task);
    insertionSort(rows, earlierDeadline);
    return rows;
}

inline vector<Task> activityTasks(const vector<Task>& all, long long now)
{
    vector<Task> rows;
    for (const Task& task : all)
        if (task.completed || isOverdueAt(task, now)) rows.push_back(task);
    insertionSort(rows, [](const Task& a, const Task& b) {
        return earlierDeadline(b, a);
    });
    return rows;
}

inline string reminderLabel(int minutes)
{
    if (minutes == -1) return "No reminder";
    if (minutes == 0) return "At scheduled time";
    if (minutes == 60) return "1 hour before";
    if (minutes == 1440) return "1 day before";
    return std::to_string(minutes) + " minutes before";
}

struct TaskForm
{
    bool open = false;
    int picker = 0; // 1 calendar, 2 time, 3 reminder
    int editId = 0;
    int priority = 2;
    int focus = 0;  // 1 title, 2 description, 3 hour, 4 minute, 5 second
    int year = 2026;
    int month = 1;
    string name, description, date, time, error;
    bool allDay = false;
    int reminderMinutes = 0;
    int colorIndex = 0;
    string draftDate, hour, minute, second, pickerError;
};

struct App
{
    Scheduler scheduler;
    vector<Task> allTasks;
    vector<Task> pendingTasks;
    bool tasksChanged = true;
    TaskForm form;
    int selected = 0;
    bool developerOpen = false;
    int deleteId = 0;
    int filter = 0;
    float scroll = 0;
    vector<int> reminderIds;
    int activeReminderId = 0;
    std::map<int, long long> snoozed;
    Sound sound{};
    bool soundReady = false;
    double alarmEnd = 0, nextBeep = 0, nextCheck = 0;
    bool alarmMuted = false;
    bool searchFocus = false, sortByDeadline = false;
    string query, toast;
    double toastUntil = 0;

    bool blocked() const { return form.open || deleteId != 0 || developerOpen; }

    void say(const string& message, double seconds = 2.6)
    {
        toast = message;
        toastUntil = GetTime() + seconds;
    }

    void save(const string& message)
    {
        tasksChanged = true;
        if (scheduler.save(dataFile)) say(message);
        else say("Could not save tasks.txt", 5);
    }

    void refreshTasks()
    {
        if (!tasksChanged) return;
        allTasks = scheduler.all();
        pendingTasks = scheduler.scheduled();
        tasksChanged = false;
    }

    void goTo(int page)
    {
        selected = page;
        query.clear();
        filter = 0;
        scroll = 0;
        searchFocus = false;
        sortByDeadline = false;
    }

    void openForm(int id)
    {
        form = TaskForm{};
        form.open = true;
        form.editId = id;
        form.focus = 1;
        form.date = localNow("%Y-%m-%d", 3600);
        form.time = localNow("%H:%M:%S", 3600);
        if (Task* task = scheduler.find(id))
        {
            form.name = task->name;
            form.date = task->deadline;
            form.time = displayTime(task->dueTime);
            form.priority = task->priority;
            form.description = task->description;
            form.allDay = task->allDay;
            form.reminderMinutes = task->reminderMinutes;
            form.colorIndex = task->colorIndex;
        }
        searchFocus = false;
    }

    void forget(int id)
    {
        reminderIds.erase(std::remove(reminderIds.begin(), reminderIds.end(), id), reminderIds.end());
        snoozed.erase(id);
        if (activeReminderId == id) activeReminderId = 0;
    }

    void complete(int id)
    {
        if (scheduler.complete(id))
        {
            forget(id);
            save("Task completed");
            nextCheck = 0;
        }
    }

    void toggleCompleted(int id)
    {
        Task* task = scheduler.find(id);
        if (!task) return;
        if (!task->completed)
        {
            complete(id);
            return;
        }
        if (scheduler.reopen(id))
        {
            forget(id);
            save("Task moved to pending");
            nextCheck = 0;
        }
    }

    void checkReminders()
    {
        double now = GetTime();
        if (now < nextCheck) return;
        nextCheck = now + 0.35;
        string date = localNow("%Y-%m-%d");
        string clock = localNow("%H:%M:%S");
        long long secondNow = secondNumber(date, clock);
        vector<int> due;
        for (int id : scheduler.dueReminders(date, clock))
        {
            auto wait = snoozed.find(id);
            if (wait != snoozed.end() && secondNow < wait->second) continue;
            due.push_back(id);
        }
        reminderIds = std::move(due);
        // Keep the current notification visible for its full duration.
        auto current = std::find(reminderIds.begin(), reminderIds.end(), activeReminderId);
        if (current == reminderIds.end())
        {
            activeReminderId = reminderIds.empty() ? 0 : reminderIds.front();
            if (activeReminderId)
            {
                alarmEnd = now + alarmSeconds;
                nextBeep = 0;
                alarmMuted = false;
            }
        }
    }

    bool ringing() const
    {
        return soundReady && activeReminderId != 0 && !alarmMuted && GetTime() < alarmEnd;
    }

    void updateAlarm()
    {
        if (activeReminderId && GetTime() >= alarmEnd)
        {
            int id = activeReminderId;
            // Expiring an alert does not complete its task.
            if (scheduler.acknowledgeReminder(id))
            {
                tasksChanged = true;
                if (!scheduler.save(dataFile)) say("Could not save tasks.txt", 5);
            }
            forget(id);
            nextCheck = 0;
        }
        if (ringing())
        {
            if (GetTime() >= nextBeep)
            {
                PlaySound(sound);
                nextBeep = GetTime() + beepGap;
            }
        }
        else if (soundReady && IsSoundPlaying(sound))
            StopSound(sound);
    }

    void dismissReminder(int id)
    {
        if (scheduler.acknowledgeReminder(id)) save("Reminder dismissed");
        forget(id);
        nextCheck = 0;
    }

    void snoozeReminder(int id)
    {
        long long now = secondNumber(localNow("%Y-%m-%d"), localNow("%H:%M:%S"));
        snoozed[id] = now + 300;
        reminderIds.erase(std::remove(reminderIds.begin(), reminderIds.end(), id), reminderIds.end());
        if (activeReminderId == id) activeReminderId = 0;
        nextCheck = 0;
        say("Snoozed for 5 minutes");
    }
};
