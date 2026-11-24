#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include <utility>
#include <cstdio>

struct Task
{
    int id = 0;
    std::string name, deadline;
    int priority = 2;
    bool completed = false;
    int completionOrder = 0;
    std::string dueTime = "23:59";
    bool reminderSeen = false;
    std::string description;
    bool allDay = false;
    int reminderMinutes = 0;
    int colorIndex = 0;
};

inline bool validDate(const std::string& s)
{
    if (s.size() != 10 || s[4] != '-' || s[7] != '-') return false;
    for (int i = 0; i < 10; ++i)
        if (i != 4 && i != 7 && !std::isdigit(static_cast<unsigned char>(s[i]))) return false;
    int y = std::stoi(s.substr(0, 4));
    int m = std::stoi(s.substr(5, 2));
    int d = std::stoi(s.substr(8, 2));
    if (y < 1900 || m < 1 || m > 12) return false;
    int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (m == 2 && y % 4 == 0 && (y % 100 != 0 || y % 400 == 0)) ++days[1];
    return d >= 1 && d <= days[m - 1];
}

inline bool validTime(const std::string& time)
{
    if (time.size() == 5)
    {
        if (time[2] != ':') return false;
        for (int i : {0, 1, 3, 4}) if (!std::isdigit(static_cast<unsigned char>(time[i]))) return false;
        return std::stoi(time.substr(0, 2)) < 24 && std::stoi(time.substr(3, 2)) < 60;
    }
    if (time.size() == 8)
    {
        if (time[2] != ':' || time[5] != ':') return false;
        for (int i : {0, 1, 3, 4, 6, 7}) if (!std::isdigit(static_cast<unsigned char>(time[i]))) return false;
        return std::stoi(time.substr(0, 2)) < 24 && std::stoi(time.substr(3, 2)) < 60 &&
               std::stoi(time.substr(6, 2)) < 60;
    }
    return false;
}

inline std::string normalizeTime(const std::string& time)
{
    if (!validTime(time)) return "00:00:00";
    return time.size() == 5 ? time + ":00" : time;
}

inline int timeToSeconds(const std::string& time)
{
    std::string value = normalizeTime(time);
    return std::stoi(value.substr(0, 2)) * 3600 + std::stoi(value.substr(3, 2)) * 60 + std::stoi(value.substr(6, 2));
}

inline bool validReminder(int minutes)
{
    return minutes >= -1 && minutes <= 1440;
}

inline long long secondNumber(const std::string& date, const std::string& time)
{
    int year = std::stoi(date.substr(0, 4));
    int month = std::stoi(date.substr(5, 2));
    int day = std::stoi(date.substr(8, 2));
    long long y = year - 1;
    long long days = 365 * y + y / 4 - y / 100 + y / 400;
    const int monthLengths[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    bool leap = year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
    for (int m = 1; m < month; ++m)
        days += monthLengths[m - 1] + (m == 2 && leap ? 1 : 0);
    days += day - 1;
    return days * 86400 + timeToSeconds(time);
}

inline long long minuteNumber(const std::string& date, const std::string& time)
{
    return secondNumber(date, time) / 60;
}

inline long long reminderMoment(const Task& task)
{
    return secondNumber(task.deadline, task.allDay ? "09:00:00" : task.dueTime) - 60LL * task.reminderMinutes;
}

inline bool earlierDeadline(const Task& a, const Task& b)
{
    if (a.deadline != b.deadline) return a.deadline < b.deadline;
    int ta = timeToSeconds(a.dueTime), tb = timeToSeconds(b.dueTime);
    if (ta != tb) return ta < tb;
    return a.id < b.id;
}

inline bool higher(const Task& a, const Task& b)
{
    if (a.priority != b.priority) return a.priority > b.priority;
    return earlierDeadline(a, b);
}

inline std::string lower(std::string s)
{
    for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

class TaskList
{
    struct Node { Task task; Node* next; };
    Node* head = nullptr;
public:
    TaskList() = default;
    TaskList(const TaskList&) = delete;
    TaskList& operator=(const TaskList&) = delete;
    ~TaskList() { clear(); }

    void clear()
    {
        while (head)
        {
            Node* n = head;
            head = head->next;
            delete n;
        }
    }

    Task* find(int id)
    {
        for (Node* n = head; n; n = n->next)
            if (n->task.id == id) return &n->task;
        return nullptr;
    }

    void put(const Task& t)
    {
        if (Task* p = find(t.id)) { *p = t; return; }
        Node** p = &head;
        while (*p && (*p)->task.id < t.id) p = &(*p)->next;
        *p = new Node{t, *p};
    }

    bool erase(int id)
    {
        Node** p = &head;
        while (*p && (*p)->task.id != id) p = &(*p)->next;
        if (!*p) return false;
        Node* n = *p;
        *p = n->next;
        delete n;
        return true;
    }

    std::vector<Task> all() const
    {
        std::vector<Task> v;
        for (Node* n = head; n; n = n->next) v.push_back(n->task);
        return v;
    }
};

class TaskHeap
{
    std::vector<Task> a;
public:
    bool empty() const { return a.empty(); }
    const Task& top() const { return a.front(); }

    void push(const Task& t)
    {
        a.push_back(t);
        size_t i = a.size() - 1;
        while (i)
        {
            size_t p = (i - 1) / 2;
            if (!higher(a[i], a[p])) break;
            std::swap(a[i], a[p]);
            i = p;
        }
    }

    Task pop()
    {
        Task t = a.front();
        a[0] = a.back();
        a.pop_back();
        size_t i = 0;
        while (i < a.size())
        {
            size_t best = i, l = 2 * i + 1, r = l + 1;
            if (l < a.size() && higher(a[l], a[best])) best = l;
            if (r < a.size() && higher(a[r], a[best])) best = r;
            if (best == i) break;
            std::swap(a[i], a[best]);
            i = best;
        }
        return t;
    }
};

struct Change
{
    int id;
    bool existed;
    Task previous;
};

class UndoStack
{
    struct Node { Change change; Node* next; };
    Node* topNode = nullptr;
public:
    UndoStack() = default;
    UndoStack(const UndoStack&) = delete;
    UndoStack& operator=(const UndoStack&) = delete;
    ~UndoStack() { while (topNode) pop(); }
    bool empty() const { return !topNode; }
    void push(Change c) { topNode = new Node{c, topNode}; }
    Change pop()
    {
        Node* n = topNode;
        Change c = n->change;
        topNode = n->next;
        delete n;
        return c;
    }
};

class CompletedQueue
{
    struct Node { Task task; Node* next; };
    Node* front = nullptr;
    Node* rear = nullptr;
public:
    CompletedQueue() = default;
    CompletedQueue(const CompletedQueue&) = delete;
    CompletedQueue& operator=(const CompletedQueue&) = delete;
    ~CompletedQueue() { while (front) pop(); }
    bool empty() const { return !front; }
    void push(Task t)
    {
        Node* n = new Node{t, nullptr};
        if (rear) rear->next = n; else front = n;
        rear = n;
    }
    Task pop()
    {
        Node* n = front;
        Task t = n->task;
        front = n->next;
        if (!front) rear = nullptr;
        delete n;
        return t;
    }
};

template <class Compare>
inline void insertionSort(std::vector<Task>& v, Compare before)
{
    for (size_t i = 1; i < v.size(); ++i)
    {
        Task key = v[i];
        size_t j = i;
        while (j > 0 && before(key, v[j - 1]))
        {
            v[j] = v[j - 1];
            --j;
        }
        v[j] = key;
    }
}

class Scheduler
{
    TaskList tasks;
    UndoStack undoStack;
    int nextId = 1, nextCompletion = 1;
public:
    std::vector<Task> all() const { return tasks.all(); }
    Task* find(int id) { return tasks.find(id); }
    bool canUndo() const { return !undoStack.empty(); }

    bool add(std::string name, int p, std::string date, std::string time = "23:59",
             std::string description = "", bool allDay = false,
             int reminderMinutes = 0, int colorIndex = 0)
    {
        if (name.empty() || !validDate(date) || !validTime(time) || p < 1 || p > 3 ||
            !validReminder(reminderMinutes) || colorIndex < 0 || colorIndex > 5) return false;
        Task task;
        task.id = nextId++;
        task.name = name;
        task.deadline = date;
        task.priority = p;
        task.dueTime = allDay ? "23:59" : time;
        task.description = description;
        task.allDay = allDay;
        task.reminderMinutes = reminderMinutes;
        task.colorIndex = colorIndex;
        tasks.put(task);
        undoStack.push({task.id, false, {}});
        return true;
    }

    bool edit(int id, std::string name, int p, std::string date, std::string time = "23:59",
              std::string description = "", bool allDay = false,
              int reminderMinutes = 0, int colorIndex = 0)
    {
        Task* task = find(id);
        if (!task || name.empty() || !validDate(date) || !validTime(time) || p < 1 || p > 3 ||
            !validReminder(reminderMinutes) || colorIndex < 0 || colorIndex > 5) return false;
        undoStack.push({id, true, *task});
        if (allDay) time = "23:59";
        if (task->deadline != date || normalizeTime(task->dueTime) != normalizeTime(time) ||
            task->allDay != allDay || task->reminderMinutes != reminderMinutes)
            task->reminderSeen = false;
        task->name = name;
        task->priority = p;
        task->deadline = date;
        task->dueTime = time;
        task->description = description;
        task->allDay = allDay;
        task->reminderMinutes = reminderMinutes;
        task->colorIndex = colorIndex;
        return true;
    }

    bool remove(int id)
    {
        Task* t = find(id);
        if (!t) return false;
        undoStack.push({id, true, *t});
        return tasks.erase(id);
    }

    bool complete(int id)
    {
        Task* t = find(id);
        if (!t || t->completed) return false;
        undoStack.push({id, true, *t});
        t->completed = true;
        t->completionOrder = nextCompletion++;
        return true;
    }

    bool reopen(int id)
    {
        Task* task = find(id);
        if (!task || !task->completed) return false;
        undoStack.push({id, true, *task});
        task->completed = false;
        task->completionOrder = 0;
        return true;
    }

    bool undo()
    {
        if (!canUndo()) return false;
        Change c = undoStack.pop();
        if (c.existed) tasks.put(c.previous);
        else tasks.erase(c.id);
        return true;
    }

    std::vector<int> dueReminders(const std::string& date, const std::string& time) const
    {
        if (!validDate(date) || !validTime(time)) return {};
        long long now = secondNumber(date, time);
        std::vector<Task> due;
        for (const Task& task : all())
            if (!task.completed && !task.reminderSeen && task.reminderMinutes >= 0 && reminderMoment(task) <= now)
                due.push_back(task);
        insertionSort(due, [](const Task& a, const Task& b) {
            long long x = reminderMoment(a), y = reminderMoment(b);
            return x == y ? a.id < b.id : x < y;
        });
        std::vector<int> ids;
        for (const Task& task : due) ids.push_back(task.id);
        return ids;
    }

    int dueReminder(const std::string& date, const std::string& time) const
    {
        auto due = dueReminders(date, time);
        return due.empty() ? 0 : due.front();
    }

    std::vector<Task> tasksOnDate(const std::string& date, const std::string& query = "") const
    {
        std::vector<Task> rows;
        std::string key = lower(query);
        for (const Task& task : all())
            if (task.deadline == date && (lower(task.name).find(key) != std::string::npos ||
                lower(task.description).find(key) != std::string::npos)) rows.push_back(task);
        insertionSort(rows, [](const Task& a, const Task& b) {
            if (a.allDay != b.allDay) return a.allDay;
            return earlierDeadline(a, b);
        });
        return rows;
    }

    bool acknowledgeReminder(int id)
    {
        Task* task = find(id);
        if (!task || task->completed || task->reminderSeen) return false;
        task->reminderSeen = true;
        return true;
    }

    std::vector<Task> scheduled() const
    {
        TaskHeap heap;
        for (const Task& t : all()) if (!t.completed) heap.push(t);
        std::vector<Task> v;
        while (!heap.empty()) v.push_back(heap.pop());
        return v;
    }

    std::vector<Task> history() const
    {
        auto v = all();
        insertionSort(v, [](const Task& a, const Task& b) { return a.completionOrder < b.completionOrder; });
        CompletedQueue q;
        for (const Task& t : v) if (t.completed) q.push(t);
        v.clear();
        while (!q.empty()) v.push_back(q.pop());
        return v;
    }

    bool save(const std::string& path) const
    {
        std::ofstream f(path + ".tmp", std::ios::trunc);
        if (!f) return false;
        f << "TASKFLOW3 " << nextId << ' ' << nextCompletion << '\n';
        for (const Task& t : all())
            f << t.id << ' ' << t.priority << ' ' << t.completed << ' '
              << t.completionOrder << ' ' << std::quoted(t.deadline) << ' '
              << std::quoted(t.name) << ' ' << std::quoted(t.dueTime) << ' '
              << t.reminderSeen << ' ' << std::quoted(t.description) << ' '
              << t.allDay << ' ' << t.reminderMinutes << ' ' << t.colorIndex << '\n';
        f.close();
        if (!f) return false;
        std::string bak = path + ".bak";
        std::ifstream old(path);
        bool exists = old.good();
        old.close();
        if (exists)
        {
            std::remove(bak.c_str());
            if (std::rename(path.c_str(), bak.c_str()) != 0) return false;
        }
        if (std::rename((path + ".tmp").c_str(), path.c_str()) != 0)
        {
            if (exists) std::rename(bak.c_str(), path.c_str());
            return false;
        }
        return true;
    }

    bool load(const std::string& path)
    {
        std::ifstream f(path);
        if (!f) return false;
        std::string magic;
        int ni, nc;
        if (!(f >> magic >> ni >> nc)) return false;
        if (magic != "TASKFLOW1" && magic != "TASKFLOW2" && magic != "TASKFLOW3") return false;
        if (ni < 1 || nc < 1) return false;

        std::vector<Task> v;
        while (f >> std::ws && f.peek() != EOF)
        {
            Task t;
            if (!(f >> t.id >> t.priority >> t.completed >> t.completionOrder
                    >> std::quoted(t.deadline) >> std::quoted(t.name))) return false;
            if (t.id <= 0 || t.id >= ni || t.name.empty() || !validDate(t.deadline) || t.priority < 1 || t.priority > 3)
                return false;
            bool badOrder = t.completed ? (t.completionOrder <= 0 || t.completionOrder >= nc) : t.completionOrder != 0;
            if (badOrder) return false;
            if (magic != "TASKFLOW1")
            {
                if (!(f >> std::quoted(t.dueTime) >> t.reminderSeen) || !validTime(t.dueTime)) return false;
            }
            if (magic == "TASKFLOW3")
            {
                if (!(f >> std::quoted(t.description) >> t.allDay >> t.reminderMinutes >> t.colorIndex) ||
                    !validReminder(t.reminderMinutes) || t.colorIndex < 0 || t.colorIndex > 5 ||
                    (t.allDay && normalizeTime(t.dueTime).substr(0, 5) != "23:59"))
                    return false;
            }
            for (const Task& a : v) if (a.id == t.id) return false;
            v.push_back(t);
        }
        tasks.clear();
        while (!undoStack.empty()) undoStack.pop();
        for (const Task& t : v) tasks.put(t);
        nextId = ni;
        nextCompletion = nc;
        return true;
    }
};
