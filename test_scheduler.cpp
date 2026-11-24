#include "taskflow.hpp"
#include <cassert>
#include <iostream>

void testScheduling()
{
    Scheduler scheduler;
    assert(!scheduler.add("Invalid", 2, "2026-02-30"));
    assert(validDate("2028-02-29"));
    assert(!validDate("2026-02-29"));
    assert(validTime("00:00") && validTime("23:59"));
    assert(!validTime("24:00") && !validTime("12:60") && !validTime("9:30"));
    assert(!scheduler.add("Invalid time", 2, "2026-11-25", "25:00"));
    assert(scheduler.add("Low", 1, "2026-11-25", "08:00"));
    assert(scheduler.add("High later", 3, "2026-11-26", "15:00"));
    assert(scheduler.add("High first", 3, "2026-11-26", "09:00"));
    assert(scheduler.add("Tie", 3, "2026-11-26", "09:00"));
    auto ranked = scheduler.scheduled();
    assert(ranked.size() == 4);
    assert(ranked[0].id == 3 && ranked[1].id == 4 && ranked[2].id == 2 && ranked[3].id == 1);
    assert(scheduler.complete(3) && scheduler.complete(2));
    auto history = scheduler.history();
    assert(history.size() == 2 && history[0].id == 3 && history[1].id == 2);
    assert(scheduler.undo() && !scheduler.find(2)->completed);
    assert(scheduler.edit(1, "Updated", 3, "2026-11-24", "10:30"));
    assert(scheduler.scheduled().front().id == 1);
    assert(scheduler.remove(1) && !scheduler.find(1));
    assert(scheduler.undo() && scheduler.find(1));
    assert(scheduler.undo() && scheduler.find(1)->priority == 1);
    assert(scheduler.find(1)->dueTime == "08:00");
}

void testReminders()
{
    Scheduler scheduler;
    assert(scheduler.add("First", 2, "2026-11-25", "09:00"));
    assert(scheduler.add("Second", 3, "2026-11-25", "09:05"));
    assert(scheduler.dueReminder("2026-11-25", "08:59") == 0);
    assert(scheduler.dueReminder("2026-11-25", "09:00") == 1);
    assert(scheduler.acknowledgeReminder(1));
    assert(!scheduler.acknowledgeReminder(1));
    assert(scheduler.dueReminder("2026-11-25", "09:04") == 0);
    assert(scheduler.dueReminder("2026-11-26", "00:00") == 2);
    assert(scheduler.complete(2));
    assert(scheduler.dueReminder("2026-11-26", "00:00") == 0);
    assert(scheduler.edit(1, "Rescheduled", 2, "2026-11-26", "10:00"));
    assert(!scheduler.find(1)->reminderSeen);
    assert(scheduler.dueReminder("2026-11-26", "10:00") == 1);
    assert(scheduler.acknowledgeReminder(1));
    assert(scheduler.save("test_tasks.txt"));
    Scheduler restored;
    assert(restored.load("test_tasks.txt"));
    assert(restored.find(1)->dueTime == "10:00" && restored.find(1)->reminderSeen);
    assert(restored.history().size() == 1);
    assert(restored.dueReminder("2026-11-27", "12:00") == 0);
    assert(!restored.canUndo());
    std::remove("test_tasks.txt");
    std::remove("test_tasks.txt.bak");
}

void testLegacyFile()
{
    std::ofstream file("legacy_test.txt");
    file << "TASKFLOW1 2 1\n1 2 0 0 \"2026-11-28\" \"Old task\"\n";
    file.close();
    Scheduler scheduler;
    assert(scheduler.load("legacy_test.txt"));
    assert(scheduler.find(1)->dueTime == "23:59");
    assert(scheduler.save("legacy_test.txt"));
    Scheduler migrated;
    assert(migrated.load("legacy_test.txt"));
    assert(migrated.find(1)->name == "Old task");
    std::remove("legacy_test.txt");
    std::remove("legacy_test.txt.bak");
}

void testMaterialOptions()
{
    Scheduler scheduler;
    assert(scheduler.add("Midnight", 3, "2028-03-01", "00:05", "Read chapter", false, 10, 4));
    assert(scheduler.dueReminder("2028-02-29", "23:54") == 0);
    assert(scheduler.dueReminder("2028-02-29", "23:55") == 1);
    assert(scheduler.acknowledgeReminder(1));
    assert(scheduler.add("All day", 2, "2028-03-01", "15:30", "Notes", true, 30, 1));
    assert(scheduler.find(2)->dueTime == "23:59");
    assert(scheduler.dueReminder("2028-03-01", "08:29") == 0);
    assert(scheduler.dueReminder("2028-03-01", "08:30") == 2);
    assert(scheduler.acknowledgeReminder(2));
    assert(scheduler.add("Silent", 1, "2028-03-01", "10:00", "", false, -1, 2));
    assert(scheduler.dueReminder("2028-03-02", "10:00") == 0);
    assert(scheduler.edit(1, "Midnight", 3, "2028-03-01", "00:05", "New notes", false, 0, 5));
    assert(scheduler.dueReminder("2028-03-01", "00:04") == 0);
    assert(scheduler.dueReminder("2028-03-01", "00:05") == 1);
    assert(scheduler.undo());
    assert(scheduler.find(1)->description == "Read chapter");
    assert(scheduler.find(1)->reminderSeen);
    assert(!scheduler.add("Bad color", 2, "2028-03-01", "00:00", "", false, 0, 6));
    assert(scheduler.save("material_test.txt"));
    Scheduler restored;
    assert(restored.load("material_test.txt"));
    assert(restored.find(1)->description == "Read chapter");
    assert(restored.find(1)->colorIndex == 4 && restored.find(1)->reminderMinutes == 10);
    assert(restored.find(2)->allDay && restored.find(2)->reminderSeen);
    assert(restored.find(3)->reminderMinutes == -1);
    std::remove("material_test.txt");
    std::remove("material_test.txt.bak");

    std::ofstream old("version2_test.txt");
    old << "TASKFLOW2 2 1\n1 2 0 0 \"2028-03-01\" \"Old task\" \"10:00\" 0\n";
    old.close();
    Scheduler migrated;
    assert(migrated.load("version2_test.txt"));
    assert(migrated.find(1)->dueTime == "10:00");
    assert(migrated.find(1)->description.empty());
    assert(migrated.find(1)->reminderMinutes == 0);
    assert(migrated.dueReminder("2028-03-01", "10:00") == 1);
    std::remove("version2_test.txt");
}

void testDailyScheduleAndMultipleReminders()
{
    Scheduler scheduler;
    assert(scheduler.add("Late", 3, "2026-11-26", "16:00", "Class", false, 0));
    assert(scheduler.add("Early", 1, "2026-11-26", "09:00", "Study", false, 0));
    assert(scheduler.add("All day", 2, "2026-11-26", "10:00", "Notes", true, -1));
    assert(scheduler.add("Another day", 3, "2026-11-27", "12:00", "", false, 0));
    auto day = scheduler.tasksOnDate("2026-11-26");
    assert(day.size() == 3 && day[0].id == 3 && day[1].id == 2 && day[2].id == 1);
    assert(scheduler.tasksOnDate("2026-11-26", "CLASS").front().id == 1);
    assert(scheduler.tasksOnDate("2026-11-28").empty());
    assert(scheduler.dueReminders("2026-11-26", "08:59").empty());
    assert(scheduler.dueReminders("2026-11-26", "09:00") == std::vector<int>{2});
    auto due = scheduler.dueReminders("2026-11-26", "16:00");
    assert((due == std::vector<int>{2, 1}));
    // An older pending notification must not stop a later task becoming due.
    assert(scheduler.acknowledgeReminder(1));
    assert(scheduler.dueReminders("2026-11-26", "16:01") == std::vector<int>{2});
    assert(scheduler.complete(2));
    assert(scheduler.dueReminders("2026-11-26", "16:02").empty());
    assert(scheduler.edit(1, "Rescheduled", 3, "2026-11-26", "17:00"));
    assert(scheduler.dueReminders("2026-11-26", "16:59").empty());
    assert(scheduler.dueReminders("2026-11-26", "17:00") == std::vector<int>{1});
}

int main()
{
    testScheduling();
    testReminders();
    testLegacyFile();
    testMaterialOptions();
    testDailyScheduleAndMultipleReminders();
    std::cout << "All scheduler, reminder and file tests passed\n";
}
