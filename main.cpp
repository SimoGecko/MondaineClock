#include <SFML/Graphics.hpp>
#include <ctime>
#include <cstring>

#define POW2(x) ((x)*(x))

#if defined (SFML_SYSTEM_WINDOWS)
#include <windows.h>

bool setShapeCircle(HWND hWnd, const sf::Vector2u& size)
{
    HRGN hRegion = CreateRectRgn(0, 0, size.x, size.y);

    // Determine the visible region
    int d2 = POW2(size.x / 2);
    for (int y = 0; y < size.y; y++)
    {
        for (int x = 0; x < size.x; x++)
        {
            int d = POW2(x - size.x / 2) + POW2(y - size.y / 2);
            if (d > d2)
            {
                HRGN hRegionPixel = CreateRectRgn(x, y, x + 1, y + 1);
                CombineRgn(hRegion, hRegion, hRegionPixel, RGN_XOR);
                DeleteObject(hRegionPixel);
            }
        }
    }

    SetWindowRgn(hWnd, hRegion, true);
    DeleteObject(hRegion);
    return true;
}

bool setTransparency(HWND hWnd, unsigned char alpha)
{
    SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
    SetLayeredWindowAttributes(hWnd, 0, alpha, LWA_ALPHA);
    return true;
}

#elif defined (SFML_SYSTEM_LINUX)
#include <X11/Xatom.h>
#include <X11/extensions/shape.h>

bool setShapeCircle(Window wnd, const sf::Vector2i& size)
{
    Display* display = XOpenDisplay(NULL);

    // Try to set the window shape
    int event_base;
    int error_base;
    if (XShapeQueryExtension(display, &event_base, &error_base))
    {
        Pixmap pixmap = XCreatePixmap(display, wnd, size.x, size.y, 1);
        GC gc = XCreateGC(display, pixmap, 0, NULL);

        XSetForeground(display, gc, 1);
        XFillRectangle(display, pixmap, gc, 0, 0, size.x, size.y);
        XSetForeground(display, gc, 0);

        int d2 = pow2(size.x / 2);
        for (unsigned int y = 0; y < size.y; y++)
        {
            for (unsigned int x = 0; x < size.x; x++)
            {
                int d = pow2(x - size.x / 2) + pow2(y - size.y / 2);
                if (d > d2)
                {
                    XFillRectangle(display, pixmap, gc, x, y, 1, 1);
                }
            }
        }

        XShapeCombineMask(display, wnd, ShapeBounding, 0, 0, pixmap, ShapeSet);
        XFreeGC(display, gc);
        XFreePixmap(display, pixmap);
        XFlush(display);
        XCloseDisplay(display);
        return true;
    }

    XCloseDisplay(display);
}

bool setTransparency(Window wnd, unsigned char alpha)
{
    Display* display = XOpenDisplay(NULL);
    unsigned long opacity = (0xffffffff / 0xff) * alpha;
    Atom property = XInternAtom(display, "_NET_WM_WINDOW_OPACITY", false);
    if (property != None)
    {
        XChangeProperty(display, wnd, property, XA_CARDINAL, 32, PropModeReplace, (unsigned char*)&opacity, 1);
        XFlush(display);
        XCloseDisplay(display);
        return true;
    }
    else
    {
        XCloseDisplay(display);
        return false;
    }
}

#undef None // None conflicts with SFML
#elif defined (SFML_SYSTEM_MACOS)
bool setShapeCircle(sf::WindowHandle handle, const sf::Vector2i& size)
{
    return false;
}
bool setTransparency(sf::WindowHandle handle, unsigned char alpha)
{
    return false;
}
#else
bool setShapeCircle(sf::WindowHandle handle, const sf::Vector2i& size)
{
    return false;
}

bool setTransparency(sf::WindowHandle handle, unsigned char alpha)
{
    return false;
}
#endif

// PARAMETERS
int csize = 200;
int posx = -1;
int posy = -1;
bool stop2go = true;
bool dark = false;
unsigned int handcolor = 0;

// state
bool isMouseDragging = false;
int lastDownX = 0, lastDownY = 0;

// CLOCK
#define scaling ((float)csize/360)
#define U *scaling

#define clock_r  (180 U)
#define clock_b  ( 12 U)

#define ticks_r  (160 U)
#define ticks_l1 ( 40 U)
#define ticks_w1 ( 12 U)
#define ticks_l2 ( 12 U)
#define ticks_w2 (  5 U)

#define hour_l1  ( 94 U)
#define hour_l2  ( 39 U)
#define hour_w1  ( 15 U)
#define hour_w2  ( 21 U)

#define min_l1   (152 U)
#define min_l2   ( 40 U)
#define min_w1   ( 12 U)
#define min_w2   ( 18 U)

#define sec_l1   (110 U)
#define sec_l2   ( 52 U)
#define sec_w    (  5 U)
#define sec_r    ( 13 U)

#define col_white 0xfdfdfd
#define col_black 0x221e20
#define col_red   0xec2324

#define colhex_white ((col_white<<8) | 0xff)
#define colhex_black ((col_black<<8) | 0xff)
#define colhex_red   ((col_red  <<8) | 0xff)


sf::CircleShape GetCircle       (sf::Color color, sf::Vector2f origin, sf::Vector2f position, float rotation, float radius, int segments = 32)
{
    sf::CircleShape cs(radius, segments);
    cs.setFillColor(color);
    cs.setOrigin(origin);
    cs.setPosition(position);
    cs.setRotation(sf::degrees(rotation));
    return cs;
}
sf::CircleShape GetCircleOutline(sf::Color color, sf::Vector2f origin, sf::Vector2f position, float rotation, float radius, int segments = 32, sf::Color outlineColor = sf::Color::Transparent, float outlineThickness = 0.f)
{
    sf::CircleShape cs(radius, segments);
    cs.setFillColor(color);
    cs.setOutlineColor(outlineColor);
    cs.setOutlineThickness(outlineThickness);
    cs.setOrigin(origin);
    cs.setPosition(position);
    cs.setRotation(sf::degrees(rotation));
    return cs;
}
sf::RectangleShape GetRectangle(sf::Color color, sf::Vector2f origin, sf::Vector2f position, float rotation, sf::Vector2f size)
{
    sf::RectangleShape rs;
    rs.setFillColor(color);
    rs.setOrigin(origin);
    rs.setPosition(position);
    rs.setRotation(sf::degrees(rotation));
    rs.setSize(size);
    return rs;
}
sf::ConvexShape GetTrapezoid(sf::Color color, sf::Vector2f origin, sf::Vector2f position, float rotation, sf::Vector3f size)
{
    //sf::Vector2f approxSize((size.x + size.y) / 2.f, size.z);
    //return GetRectangle(color, origin, position, rotation, approxSize);

    sf::ConvexShape xs;
    const float d = (size.y - size.x) / 2.f;
    xs.setPointCount(4);
    xs.setPoint(0, sf::Vector2f(0.f, 0.f));
    xs.setPoint(1, sf::Vector2f(size.x, 0.f));
    xs.setPoint(2, sf::Vector2f(size.x + d, size.z));
    xs.setPoint(3, sf::Vector2f(-d, size.z));

    xs.setFillColor(color);
    xs.setOrigin(origin);
    xs.setPosition(position);
    xs.setRotation(sf::degrees(rotation));
    return xs;
}

void DrawClockBackground(sf::RenderTarget& target)
{
    const sf::Color white(!dark ? colhex_white : colhex_black);
    const sf::Color black(!dark ? colhex_black : colhex_white);
    const sf::Vector2f center(clock_r, clock_r);

    // draw background + border
    sf::Vector2f origin(clock_r, clock_r);
    target.draw(GetCircleOutline(white, origin, center, 0.f, clock_r, 120, black, -clock_b));

    // draw ticks
    for (int i = 0; i < 60; ++i)
    {
        bool largeTick = (i % 5 == 0);
        sf::Vector2f size = largeTick ? sf::Vector2f(ticks_w1, ticks_l1) : sf::Vector2f(ticks_w2, ticks_l2);
        float angle = (float)i * 6.f; // / 60.f * 360.f;
        sf::Vector2f origin(size.x / 2.f, ticks_r);
        target.draw(GetRectangle(black, origin, center, angle, size));
    }
}

void CreateClockHands(sf::Shape& hShape, sf::Shape& mShape, sf::Shape& s1Shape, sf::Shape& s2Shape)
{
    const sf::Color black(!dark ? colhex_black : colhex_white);
    const sf::Color red(handcolor == 0 ? colhex_red : handcolor);
    const sf::Vector2f center(clock_r, clock_r);

    const float angle = 0.f;

    // create hours
    {
        sf::Vector3f size(hour_w1, hour_w2, hour_l1 + hour_l2);
        sf::Vector2f origin(size.x / 2.f, hour_l1);
        hShape = GetTrapezoid(black, origin, center, angle, size);
    }

    // create minutes
    {
        sf::Vector3f size(min_w1, min_w2, min_l1 + min_l2);
        sf::Vector2f origin(size.x / 2.f, min_l1);
        mShape = GetTrapezoid(black, origin, center, angle, size);
    }

    // create seconds
    {
        sf::Vector2f size(sec_w, sec_l1 + sec_l2);
        sf::Vector2f origin1(size.x / 2.f, sec_l1);
        s1Shape = GetRectangle(red, origin1, center, angle, size);

        sf::Vector2f origin2(sec_r, sec_r + sec_l1);
        float radius = sec_r;
        s2Shape = GetCircle(red, origin2, center, angle, radius, 32);
    }
}

void DrawClockHands(sf::RenderTarget& target, sf::Shape& hShape, sf::Shape& mShape, sf::Shape& s1Shape, sf::Shape& s2Shape, int h, int m, int s)
{
    // draw hours
    float hAngle = (float)(h % 12) * 30.f + (float)(m % 60) * 0.5f;
    hShape.setRotation(sf::degrees(hAngle));
    target.draw(hShape);

    // draw minutes
    float mAngle = (float)(m % 60) * 6.f;
    mShape.setRotation(sf::degrees(mAngle));
    target.draw(mShape);

    // draw seconds
    float sAngle = (float)(s % 60) * 6.f;
    s1Shape.setRotation(sf::degrees(sAngle));
    s2Shape.setRotation(sf::degrees(sAngle));
    target.draw(s1Shape);
    target.draw(s2Shape);
}

void ReadParams(int argc, char** argv)
{
    for (int i = 1; i < argc; i++)
    {
        if      (_strcmpi(argv[i], "size")        == 0 && (i + 1 < argc)) csize       = std::stoi(argv[++i]);
        else if (_strcmpi(argv[i], "posx")        == 0 && (i + 1 < argc)) posx        = std::stoi(argv[++i]);
        else if (_strcmpi(argv[i], "posy")        == 0 && (i + 1 < argc)) posy        = std::stoi(argv[++i]);
        else if (_strcmpi(argv[i], "stop2go")     == 0 && (i + 1 < argc)) stop2go     = (_strcmpi(argv[++i], "0") != 0);
        else if (_strcmpi(argv[i], "dark")        == 0                  ) dark        = true;
        else if (_strcmpi(argv[i], "handcolor")   == 0 && (i + 1 < argc))
        {
            int hex = std::stoi(argv[++i], 0, 16);
            handcolor = ((hex << 8) | 0xff);
        }
    }
}

void processEvents(sf::Window& window)
{
    while (const std::optional event = window.pollEvent())
    {
        if (event->is<sf::Event::Closed>())// || (event->is<sf::Event::KeyPressed>() && event.key.code == sf::Keyboard::Escape))
        {
            window.close();
        }
        else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
        {
            if (keyPressed->code == sf::Keyboard::Key::Escape)
                window.close();
        }
        // drag window
        else if (const auto* mousePressed = event->getIf<sf::Event::MouseButtonPressed>())
        {
            // TODO: main mouse only
            isMouseDragging = true;
            lastDownX = mousePressed->position.x;
            lastDownY = mousePressed->position.y;
        }
        else if (const auto* mouseMoved = event->getIf<sf::Event::MouseMoved>())
        {
            if (isMouseDragging)
            {
                window.setPosition(window.getPosition() + sf::Vector2<int>(mouseMoved->position.x - lastDownX, mouseMoved->position.y - lastDownY));
            }
        }
        else if (event->is<sf::Event::MouseButtonReleased>())
        {
            isMouseDragging = false;
        }
    }
}

void GetTime(int& Hour, int& Min, int& Sec)
{
    //const std::time_t now = std::time(nullptr);
    //const std::tm calendar_time = *std::localtime(std::addressof(now));

    SYSTEMTIME st;
    GetLocalTime(&st);

    Hour = st.wHour;   //calendar_time.tm_hour;
    Min  = st.wMinute;
    Sec  = st.wSecond;
    if (stop2go)
    {
        int Msec = st.wMilliseconds;
        float Secf = (float)Sec + (float)Msec / 1000.f;
        Secf = Secf * (60.f / 58.f);
        Secf = Secf < 0.f ? 0.f : Secf > 60.f ? 60.f : Secf;
        Sec = (int)Secf; // floor
    }
}

int GetWaitTimeMs()
{
    SYSTEMTIME st;
    GetLocalTime(&st);
    int Msec = st.wMilliseconds;

    if (stop2go)
    {
        int Sec = st.wSecond;
        if (Sec >= 58) return 1000 - Msec;

        int tot = Sec * 1000 + Msec;
        // find next multiple of 966.66=1000*58/60
        int next = ((tot / 967) + 1) * 967;
        return next - tot;
    }
    else
    {
        return 1000 - Msec;
    }
}

int main(int argc, char** argv)
{
    ReadParams(argc, argv);

    sf::Vector2u windowSize(csize, csize);
    const int windowsTaskbarHeight = 48 * 1.5;
    if (posx == -1) posx = (sf::VideoMode::getDesktopMode().size.x - windowSize.x) - 10;
    if (posy == -1) posy = (sf::VideoMode::getDesktopMode().size.y - windowSize.y) - 10 - windowsTaskbarHeight;
    sf::Vector2i windowPosition(posx, posy);


    // Create the window
    sf::ContextSettings contextSettings;
    contextSettings.antiAliasingLevel = 8;

    sf::VideoMode videoMode(windowSize, 32);
    sf::RenderWindow window(videoMode, sf::String("Mondaine Clock"), sf::Style::None, sf::State::Windowed, contextSettings);
    window.setPosition(windowPosition);
    window.setFramerateLimit(5);

    setShapeCircle(window.getNativeHandle(), windowSize);
    //setTransparency(window.getSystemHandle(), 255);
    //ShowWindow(window.getSystemHandle(), SW_HIDE);

    // prepare the background
    sf::RenderTexture renderTexture(windowSize, contextSettings);
    renderTexture.clear(sf::Color(colhex_black));
    DrawClockBackground(renderTexture);
    renderTexture.display();
    const sf::Texture& textureBg = renderTexture.getTexture();
    sf::Sprite spriteBg(textureBg);

    // pre-create the hand shapes
    sf::RectangleShape hShape, mShape, s1Shape; // HACK: it works to assign a sf::ConvexShape with 4 vertices to a sf::RectangleShape
    sf::CircleShape s2Shape;
    CreateClockHands(hShape, mShape, s1Shape, s2Shape);

    while (window.isOpen())
    {
        processEvents(window);

        int Hour, Min, Sec;
        GetTime(Hour, Min, Sec);
        //Hour = 10; Min = 9; Sec = 37; // special stock time

        window.draw(spriteBg); // clear
        DrawClockHands(window, hShape, mShape, s1Shape, s2Shape, Hour, Min, Sec);
        window.display();

        sf::sleep(sf::milliseconds(GetWaitTimeMs()));
    }

    return 0;
}

int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
    return main(__argc, __argv);
}