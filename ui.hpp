#pragma once
#include <raylib.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <ctime>
#include <string>
#include "taskflow.hpp"

namespace ui
{
    inline Color bg, sidebar, card, cardHover, raised, border, field;
    inline Color accent, accentDark, ink, muted, green, yellow, red;
    inline Color taskColors[] = {
        {96, 165, 250, 255}, {45, 212, 191, 255}, {251, 191, 36, 255},
        {244, 114, 182, 255}, {167, 139, 250, 255}, {224, 125, 125, 255}
    };
    inline bool darkTheme = true;

    inline void applyTheme(bool dark)
    {
        darkTheme = dark;
        if (dark)
        {
            bg = {13, 18, 22, 255};
            sidebar = {17, 24, 29, 255};
            card = {24, 33, 39, 255};
            cardHover = {30, 41, 48, 255};
            raised = {37, 49, 58, 255};
            border = {42, 56, 65, 255};
            field = {16, 23, 28, 255};
            accent = {93, 178, 183, 255};
            accentDark = {7, 36, 38, 255};
            ink = {235, 243, 245, 255};
            muted = {175, 190, 200, 255};
            green = {107, 190, 145, 255};
            yellow = {216, 179, 96, 255};
            red = {224, 125, 125, 255};
        }
        else
        {
            bg = {241, 245, 249, 255};
            sidebar = {226, 232, 240, 255};
            card = {255, 255, 255, 255};
            cardHover = {244, 247, 250, 255};
            raised = {229, 236, 243, 255};
            border = {202, 213, 225, 255};
            field = {250, 252, 254, 255};
            accent = {84, 169, 175, 255};
            accentDark = {255, 255, 255, 255};
            ink = {28, 37, 44, 255};
            muted = {94, 110, 126, 255};
            green = {35, 142, 103, 255};
            yellow = {184, 137, 37, 255};
            red = {189, 87, 106, 255};
        }
    }

    inline void toggleTheme() { applyTheme(!darkTheme); }

    inline Font regular, bold;
    inline bool clicked = false;
    inline bool wantHand = false;
    inline bool wantText = false;

    enum class Icon { Plus, Check, Close, Left, Right, Trash, Pencil, Search, Calendar, Clock, Bell, History, Sun, Moon, User, ExternalLink };

    inline void loadFonts()
    {
        applyTheme(true);
        regular = FileExists("C:/Windows/Fonts/segoeui.ttf")
            ? LoadFontEx("C:/Windows/Fonts/segoeui.ttf", 48, nullptr, 0)
            : GetFontDefault();
        bold = FileExists("C:/Windows/Fonts/segoeuib.ttf")
            ? LoadFontEx("C:/Windows/Fonts/segoeuib.ttf", 48, nullptr, 0)
            : regular;
        SetTextureFilter(regular.texture, TEXTURE_FILTER_BILINEAR);
        SetTextureFilter(bold.texture, TEXTURE_FILTER_BILINEAR);
    }

    inline void unloadFonts()
    {
        unsigned int defaultId = GetFontDefault().texture.id;
        if (bold.texture.id != regular.texture.id && bold.texture.id != defaultId) UnloadFont(bold);
        if (regular.texture.id != defaultId) UnloadFont(regular);
    }

    inline void text(const std::string& value, float x, float y, float size,
                     Color color = WHITE, bool strong = false)
    {
        DrawTextEx(strong ? bold : regular, value.c_str(), {x, y}, size, 0.2f, color);
    }

    inline float width(const std::string& value, float size, bool strong = false)
    {
        return MeasureTextEx(strong ? bold : regular, value.c_str(), size, 0.2f).x;
    }

    inline void clipped(std::string value, float x, float y, float size,
                        float maxWidth, Color color = WHITE, bool strong = false)
    {
        if (width(value, size, strong) > maxWidth)
        {
            while (!value.empty() && width(value + "...", size, strong) > maxWidth) value.pop_back();
            value += "...";
        }
        text(value, x, y, size, color, strong);
    }

    inline void centerText(const std::string& label, Rectangle r, float size,
                           Color color = WHITE, bool strong = false)
    {
        text(label, r.x + (r.width - width(label, size, strong)) / 2,
             r.y + (r.height - size) / 2 - 1, size, color, strong);
    }

    inline void rounded(Rectangle r, float radius, Color color)
    {
        float m = std::min(r.width, r.height);
        if (m <= 0) return;
        DrawRectangleRounded(r, std::min(1.0f, radius * 2 / m), 10, color);
    }

    inline void shadow(Rectangle r, float radius)
    {
        rounded({r.x + 2, r.y + 4, r.width, r.height}, radius, Color{0, 0, 0, 48});
    }

    inline void panel(Rectangle r, float radius = 8)
    {
        rounded(r, radius, border);
        rounded({r.x + 1, r.y + 1, r.width - 2, r.height - 2}, radius - 1, card);
    }

    inline void divider(float x, float y, float w)
    {
        DrawLine(static_cast<int>(x), static_cast<int>(y),
                 static_cast<int>(x + w), static_cast<int>(y), border);
    }

    inline void line(float x1, float y1, float x2, float y2, float thick, Color c)
    {
        DrawLineEx({x1, y1}, {x2, y2}, thick, c);
    }

    inline void drawIcon(Icon icon, Vector2 c, float s, Color col)
    {
        switch (icon)
        {
        case Icon::Plus:
            line(c.x - s, c.y, c.x + s, c.y, 2.2f, col);
            line(c.x, c.y - s, c.x, c.y + s, 2.2f, col);
            break;
        case Icon::Check:
            line(c.x - s * 0.9f, c.y + s * 0.05f, c.x - s * 0.3f, c.y + s * 0.65f, 2.4f, col);
            line(c.x - s * 0.3f, c.y + s * 0.65f, c.x + s * 0.95f, c.y - s * 0.6f, 2.4f, col);
            break;
        case Icon::Close:
            line(c.x - s * 0.8f, c.y - s * 0.8f, c.x + s * 0.8f, c.y + s * 0.8f, 2.2f, col);
            line(c.x + s * 0.8f, c.y - s * 0.8f, c.x - s * 0.8f, c.y + s * 0.8f, 2.2f, col);
            break;
        case Icon::Left:
            line(c.x + s * 0.4f, c.y - s, c.x - s * 0.5f, c.y, 2.4f, col);
            line(c.x - s * 0.5f, c.y, c.x + s * 0.4f, c.y + s, 2.4f, col);
            break;
        case Icon::Right:
            line(c.x - s * 0.4f, c.y - s, c.x + s * 0.5f, c.y, 2.4f, col);
            line(c.x + s * 0.5f, c.y, c.x - s * 0.4f, c.y + s, 2.4f, col);
            break;
        case Icon::Trash:
        {
            // Outline trash can with a lid and two interior lines.
            float t = 1.8f;
            line(c.x - s * .8f, c.y - s * .55f, c.x + s * .8f, c.y - s * .55f, t, col);
            line(c.x - s * .32f, c.y - s * .55f, c.x - s * .32f, c.y - s * .9f, t, col);
            line(c.x - s * .32f, c.y - s * .9f, c.x + s * .32f, c.y - s * .9f, t, col);
            line(c.x + s * .32f, c.y - s * .9f, c.x + s * .32f, c.y - s * .55f, t, col);
            line(c.x - s * .58f, c.y - s * .55f, c.x - s * .48f, c.y + s * .85f, t, col);
            line(c.x - s * .48f, c.y + s * .85f, c.x + s * .48f, c.y + s * .85f, t, col);
            line(c.x + s * .48f, c.y + s * .85f, c.x + s * .58f, c.y - s * .55f, t, col);
            for (float dx : {-0.19f, 0.19f})
                line(c.x + dx * s, c.y - s * .2f, c.x + dx * s, c.y + s * .5f, t, col);
            break;
        }
        case Icon::Pencil:
        {
            // Pencil body, nib and end cap.
            float t = 1.8f;
            Vector2 a{c.x - s * .8f, c.y + s * .8f};
            Vector2 b{c.x - s * .65f, c.y + s * .25f};
            Vector2 d{c.x + s * .25f, c.y - s * .65f};
            Vector2 e{c.x + s * .65f, c.y - s * .25f};
            Vector2 f{c.x - s * .25f, c.y + s * .65f};
            DrawLineEx(a, b, t, col);
            DrawLineEx(b, d, t, col);
            DrawLineEx(d, {c.x + s * .48f, c.y - s * .88f}, t, col);
            DrawLineEx({c.x + s * .48f, c.y - s * .88f}, {c.x + s * .88f, c.y - s * .48f}, t, col);
            DrawLineEx({c.x + s * .88f, c.y - s * .48f}, e, t, col);
            DrawLineEx(d, e, t, col);
            DrawLineEx(e, f, t, col);
            DrawLineEx(f, a, t, col);
            break;
        }
        case Icon::Sun:
        {
            float t = std::max(1.5f, s * 0.16f);
            DrawRing(c, s * 0.45f - t / 2, s * 0.45f + t / 2,
                     0, 360, 48, col);
            for (int i = 0; i < 8; ++i)
            {
                float a = i * 3.14159265f / 4;
                Vector2 start{c.x + std::cos(a) * s * 0.72f,
                              c.y + std::sin(a) * s * 0.72f};
                Vector2 end{c.x + std::cos(a) * s,
                            c.y + std::sin(a) * s};
                DrawLineEx(start, end, t, col);
                DrawCircleV(start, t / 2, col);
                DrawCircleV(end, t / 2, col);
            }
            break;
        }
        case Icon::Moon:
        {
            float t = std::max(1.5f, s * 0.16f);
            auto point = [&](float angle)
            {
                float a = angle * 3.14159265f / 180;
                return Vector2{c.x + s * 0.9f * std::cos(a),
                               c.y + s * 0.9f * std::sin(a)};
            };
            Vector2 start = point(-70), previous = start;
            for (int i = 1; i <= 36; ++i)
            {
                Vector2 next = point(-70.0f - 270.0f * i / 36);
                DrawLineEx(previous, next, t, col);
                DrawCircleV(next, t / 2, col);
                previous = next;
            }
            Vector2 end = previous;
            Vector2 control{c.x - s * 0.70f, c.y + s * 0.70f};
            for (int i = 1; i <= 24; ++i)
            {
                float u = i / 24.0f, v = 1 - u;
                Vector2 next{v * v * end.x + 2 * v * u * control.x + u * u * start.x,
                             v * v * end.y + 2 * v * u * control.y + u * u * start.y};
                DrawLineEx(previous, next, t, col);
                DrawCircleV(next, t / 2, col);
                previous = next;
            }
            break;
        }
        case Icon::User:
        {
            float t = std::max(1.5f, s * 0.16f);
            Vector2 head{c.x, c.y - s * 0.45f};
            DrawRing(head, s * 0.33f - t / 2, s * 0.33f + t / 2,
                     0, 360, 48, col);
            Vector2 previous{c.x - s * 0.82f, c.y + s * 0.87f};
            for (int i = 1; i <= 32; ++i)
            {
                float a = 3.14159265f + i * 3.14159265f / 32;
                Vector2 next{c.x + s * 0.82f * std::cos(a),
                             c.y + s * 0.87f + s * 0.72f * std::sin(a)};
                DrawLineEx(previous, next, t, col);
                DrawCircleV(next, t / 2, col);
                previous = next;
            }
            break;
        }
        case Icon::ExternalLink:
        {
            float t = std::max(1.4f, s * 0.17f);
            line(c.x - s * 0.65f, c.y + s * 0.65f, c.x + s * 0.65f, c.y - s * 0.65f, t, col);
            line(c.x - s * 0.15f, c.y - s * 0.65f, c.x + s * 0.65f, c.y - s * 0.65f, t, col);
            line(c.x + s * 0.65f, c.y - s * 0.65f, c.x + s * 0.65f, c.y + s * 0.15f, t, col);
            break;
        }
        case Icon::History:
        {
            const float t = std::max(1.5f, s * 0.19f);
            const float r = s * 0.87f;
            const float pi = 3.14159265f;

            auto point = [&](float degrees)
            {
                float angle = degrees * pi / 180.0f;
                return Vector2{c.x + r * std::cos(angle),
                               c.y + r * std::sin(angle)};
            };

            auto stroke = [&](Vector2 a, Vector2 b)
            {
                DrawLineEx(a, b, t, col);
                DrawCircleV(a, t / 2, col);
                DrawCircleV(b, t / 2, col);
            };

            // Clock outline with a rounded return arrow.
            Vector2 start = point(205.0f);
            Vector2 previous = start;
            for (int i = 1; i <= 32; ++i)
            {
                Vector2 next = point(205.0f + 295.0f * i / 32.0f);
                stroke(previous, next);
                previous = next;
            }

            stroke(start, {start.x, start.y - s * 0.52f});
            stroke(start, {start.x + s * 0.52f, start.y});

            // Clock hands.
            stroke(c, {c.x, c.y - s * 0.46f});
            stroke(c, {c.x + s * 0.34f, c.y + s * 0.22f});
            break;
        }
        case Icon::Search:
            DrawRing({c.x - s * 0.2f, c.y - s * 0.2f}, s * 0.6f, s * 0.6f + 2, 0, 360, 28, col);
            line(c.x + s * 0.25f, c.y + s * 0.25f, c.x + s, c.y + s, 2.2f, col);
            break;
        case Icon::Calendar:
            DrawRectangleLinesEx({c.x - s, c.y - s * 0.8f, s * 2, s * 1.8f}, 1.8f, col);
            line(c.x - s, c.y - s * 0.2f, c.x + s, c.y - s * 0.2f, 1.8f, col);
            line(c.x - s * 0.45f, c.y - s * 1.2f, c.x - s * 0.45f, c.y - s * 0.55f, 1.8f, col);
            line(c.x + s * 0.45f, c.y - s * 1.2f, c.x + s * 0.45f, c.y - s * 0.55f, 1.8f, col);
            break;
        case Icon::Clock:
            DrawRing(c, s, s + 1.8f, 0, 360, 32, col);
            line(c.x, c.y, c.x, c.y - s * 0.6f, 1.8f, col);
            line(c.x, c.y, c.x + s * 0.45f, c.y + s * 0.25f, 1.8f, col);
            break;
        case Icon::Bell:
            DrawCircleV({c.x, c.y - s * 0.2f}, s * 0.7f, col);
            DrawRectangleRec({c.x - s * 0.7f, c.y - s * 0.2f, s * 1.4f, s * 0.7f}, col);
            DrawRectangleRec({c.x - s, c.y + s * 0.4f, s * 2, s * 0.25f}, col);
            DrawCircleV({c.x, c.y + s * 0.85f}, s * 0.25f, col);
            break;
        }
    }

    inline bool inside(Rectangle r)
    {
        return CheckCollisionPointRec(GetMousePosition(), r);
    }

    inline bool hit(Rectangle r)
    {
        if (!clicked || !inside(r)) return false;
        clicked = false;
        return true;
    }

    inline bool buttonBase(Rectangle r, const std::string& label, Color fill, Color fillHot,
                           Color fg, bool enabled, bool strong)
    {
        bool hot = enabled && inside(r);
        if (hot) wantHand = true;
        rounded(r, 6, enabled ? (hot ? fillHot : fill) : field);
        centerText(label, r, 17, enabled ? fg : Fade(muted, 0.55f), strong);
        return enabled && hit(r);
    }

    inline bool primaryButton(Rectangle r, const std::string& label, bool enabled = true)
    {
        return buttonBase(r, label, accent, ColorBrightness(accent, 0.15f), accentDark, enabled, true);
    }

    inline bool softButton(Rectangle r, const std::string& label, bool enabled = true)
    {
        return buttonBase(r, label, raised, ColorBrightness(raised, 0.08f), ink, enabled, false);
    }

    inline bool ghostButton(Rectangle r, const std::string& label, bool enabled = true)
    {
        return buttonBase(r, label, Color{0, 0, 0, 0}, raised, muted, enabled, false);
    }

    inline bool dangerButton(Rectangle r, const std::string& label, bool enabled = true)
    {
        return buttonBase(r, label, Color{66, 33, 43, 255}, Color{90, 42, 56, 255}, red, enabled, true);
    }

    inline bool successButton(Rectangle r, const std::string& label, bool enabled = true)
    {
        return buttonBase(r, label, Color{20, 62, 50, 255}, Color{27, 84, 67, 255}, green, enabled, true);
    }

    inline bool iconButton(Rectangle r, Icon icon, Color col, bool enabled = true, float s = 7)
    {
        bool hot = enabled && inside(r);
        if (hot)
        {
            wantHand = true;
            rounded(r, 6, raised);
        }
        drawIcon(icon, {r.x + r.width / 2, r.y + r.height / 2}, s,
                 enabled ? (hot ? ink : col) : Fade(col, 0.35f));
        return enabled && hit(r);
    }

    inline bool fieldButton(Rectangle r, const std::string& label, Icon icon, bool enabled = true)
    {
        bool hot = enabled && inside(r);
        if (hot) wantHand = true;
        rounded(r, 6, hot ? accent : border);
        rounded({r.x + 1.5f, r.y + 1.5f, r.width - 3, r.height - 3}, 5, hot ? cardHover : field);
        drawIcon(icon, {r.x + 24, r.y + r.height / 2}, 7, enabled ? accent : muted);
        clipped(label, r.x + 46, r.y + (r.height - 17) / 2 - 1, 17, r.width - 58, enabled ? ink : muted);
        return enabled && hit(r);
    }

    inline void pill(Rectangle r, const std::string& label, Color color)
    {
        rounded(r, r.height / 2, Fade(color, 0.16f));
        centerText(label, r, 12, color, true);
    }

    inline bool chip(Rectangle r, const std::string& label, bool active, bool enabled = true)
    {
        bool hot = enabled && inside(r);
        if (hot) wantHand = true;

        Color fill = active ? accent : (hot ? raised : bg);
        Color labelColor = active ? Color{7, 36, 38, 255} : ink;

        rounded(r, 6, active ? accent : border);
        rounded({r.x + 1, r.y + 1, r.width - 2, r.height - 2}, 5, fill);
        centerText(label, r, 17, labelColor, active);
        return enabled && hit(r);
    }

    inline void input(Rectangle r, std::string& value, bool active,
                      const std::string& placeholder, size_t limit,
                      bool digitsOnly = false, float pad = 14)
    {
        static double nextDelete = 0;
        if (active || inside(r)) wantText = true;
        rounded(r, 6, active ? accent : border);
        rounded({r.x + 1.5f, r.y + 1.5f, r.width - 3, r.height - 3}, 5, field);
        auto allowed = [&](int ch)
        {
            if (ch < 32 || ch > 126 || value.size() >= limit) return false;
            return !digitsOnly || (ch >= '0' && ch <= '9');
        };
        if (active)
        {
            for (int ch = GetCharPressed(); ch > 0; ch = GetCharPressed()) if (allowed(ch)) value.push_back(static_cast<char>(ch));
            bool ctrl = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
            if (ctrl && IsKeyPressed(KEY_V))
            {
                const char* clip = GetClipboardText();
                for (const char* p = clip; p && *p; ++p) if (allowed(*p)) value.push_back(*p);
            }
            if (ctrl && IsKeyPressed(KEY_BACKSPACE)) value.clear();
            else if (IsKeyPressed(KEY_BACKSPACE))
            {
                if (!value.empty()) value.pop_back();
                nextDelete = GetTime() + 0.4;
            }
            else if (IsKeyDown(KEY_BACKSPACE) && GetTime() >= nextDelete)
            {
                if (!value.empty()) value.pop_back();
                nextDelete = GetTime() + 0.035;
            }
        }
        float maxW = r.width - pad * 2;
        float ty = r.y + (r.height - 18) / 2 - 1;
        if (value.empty()) text(placeholder, r.x + pad, ty, 18, muted);
        else if (active)
        {
            std::string shown = value;
            while (shown.size() > 1 && width(shown, 18) > maxW - 4) shown.erase(0, 1);
            text(shown, r.x + pad, ty, 18, ink);
            if (static_cast<int>(GetTime() * 2) % 2 == 0)
            {
                int x = static_cast<int>(r.x + pad + width(shown, 18) + 2);
                DrawLine(x, static_cast<int>(r.y + 11), x, static_cast<int>(r.y + r.height - 11), accent);
            }
        }
        else clipped(value, r.x + pad, ty, 18, maxW, ink);
        if (value.empty() && active && static_cast<int>(GetTime() * 2) % 2 == 0)
        {
            int x = static_cast<int>(r.x + pad);
            DrawLine(x, static_cast<int>(r.y + 11), x, static_cast<int>(r.y + r.height - 11), accent);
        }
    }

    inline Color priorityColor(int p) { return p == 3 ? red : p == 2 ? yellow : green; }
    inline const char* priorityName(int p) { return p == 3 ? "High" : p == 2 ? "Medium" : "Low"; }

    inline std::string localNow(const char* format, int secondsAhead = 0)
    {
        std::time_t now = std::time(nullptr) + secondsAhead;
        std::tm* local = std::localtime(&now);
        char buffer[64]{};
        if (local) std::strftime(buffer, sizeof(buffer), format, local);
        return buffer;
    }

    inline std::string formatDate(int y, int m, int d)
    {
        char buffer[16];
        std::snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d", y, m, d);
        return buffer;
    }
}
