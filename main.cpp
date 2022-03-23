#include <SFML/Graphics.hpp>
#include <ctime>

// TODO:
// - parameters
// - make it tray only (?)
// - improve border aliasing (?)

// DONE:
// - statically link
// - add icon
// - remove console
// - reduce resources usage
// - make it draggable

#define pow2(x) ((x)*(x))

#if defined (SFML_SYSTEM_WINDOWS)
#include <windows.h>

bool setShapeCircle(HWND hWnd, const sf::Vector2i& size)
{
    HRGN hRegion = CreateRectRgn(0, 0, size.x, size.y);

    // Determine the visible region
    int d2 = pow2(size.x / 2);
    for (int y = 0; y < size.y; y++)
    {
        for (int x = 0; x < size.x; x++)
        {
            int d = pow2(x - size.x / 2) + pow2(y - size.y / 2);
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
bool setShapeCircle(sf::WindowHandle handle, const sf::Vector2i& size);
bool setTransparency(sf::WindowHandle handle, unsigned char alpha);
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
// parameters -size=200 -posx=X -posy=X -stop2go
int wsize = 200;
bool stop2go = true;
int posx = 0;
int posy = 0;
int frameLimit = 30;

// parameters
#define scaling ((float)wsize/360.f)
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

#define sec_l1   (122 U)
#define sec_l2   ( 50 U)
#define sec_w    (  5 U)
#define sec_r    ( 13 U)

#define col_white 0xfdfdfd
#define col_black 0x221e20
#define col_red   0xec2324

#define colhex_white ((col_white<<8) | 0xff)
#define colhex_black ((col_black<<8) | 0xff)
#define colhex_red   ((col_red  <<8) | 0xff)


sf::CircleShape GetCircle(sf::Color color, sf::Vector2f origin, sf::Vector2f position, float rotation, float radius)
{
    sf::CircleShape cs(radius, 64);
    cs.setFillColor(color);
    cs.setOrigin(origin);
    cs.setPosition(position);
    cs.setRotation(rotation);
    return cs;
}
sf::CircleShape GetCircle(sf::Color color, sf::Vector2f origin, sf::Vector2f position, float rotation, float radius, sf::Color outlineColor, float outlineThickness)
{
    sf::CircleShape cs(radius, 64);
    cs.setFillColor(color);
    cs.setOutlineColor(outlineColor);
    cs.setOutlineThickness(outlineThickness);
    cs.setOrigin(origin);
    cs.setPosition(position);
    cs.setRotation(rotation);
    return cs;
}
sf::RectangleShape GetRectangle(sf::Color color, sf::Vector2f origin, sf::Vector2f position, float rotation, sf::Vector2f size)
{
    sf::RectangleShape rs;
    rs.setFillColor(color);
    rs.setOrigin(origin);
    rs.setPosition(position);
    rs.setRotation(rotation);
    rs.setSize(size);
    return rs;
}
sf::ConvexShape GetTrapezoid(sf::Color color, sf::Vector2f origin, sf::Vector2f position, float rotation, sf::Vector3f size)
{
    //sf::Vector2f approxSize((size.x + size.y) / 2.f, size.z);
    //return GetRectangle(color, origin, position, rotation, approxSize);

    sf::ConvexShape xs;
    xs.setPointCount(4);
    xs.setPoint(0, sf::Vector2f(0.f, 0.f));
    xs.setPoint(1, sf::Vector2f(size.x, 0.f));
    xs.setPoint(2, sf::Vector2f(size.x + (size.y-size.x)/2.f, size.z));
    xs.setPoint(3, sf::Vector2f(-(size.y - size.x) / 2.f, size.z));

    xs.setFillColor(color);
    xs.setOrigin(origin);
    xs.setPosition(position);
    xs.setRotation(rotation);
    return xs;
}


void DrawClockBackground(sf::RenderTarget& target)
{
    sf::Color white(colhex_white);
    sf::Color black(colhex_black);
    sf::Vector2f center(clock_r, clock_r);

    // draw background + border
    {
        sf::Vector2f origin(clock_r, clock_r);
        target.draw(GetCircle(white, origin, center, 0.f, clock_r, black, -clock_b));
    }

    // draw ticks
    {
        for (int i = 0; i < 60; ++i)
        {
            bool largeTick = (i % 5 == 0);
            sf::Vector2f size = largeTick ? sf::Vector2f(ticks_w1, ticks_l1) : sf::Vector2f(ticks_w2, ticks_l2);
            float angle = (float)i * 6.f; // / 60.f * 360.f;
            sf::Vector2f origin(size.x / 2.f, ticks_r);
            target.draw(GetRectangle(black, origin, center, angle, size));
        }
    }
}

/*
void DrawClockHands(sf::RenderTarget& target, int h, int m, int s)
{
	sf::Color black(colhex_black);
	sf::Color red(colhex_red);
	sf::Vector2f center(clock_r, clock_r);

	// draw hours
	{
		float angle = (float)(h % 12) * 30.f + (float)(m % 60) * 0.5f;
		sf::Vector3f size(hour_w1, hour_w2, hour_l1 + hour_l2);
        sf::Vector2f origin(size.x / 2.f, hour_l1);
        target.draw(GetTrapezoid(black, origin, center, angle, size));
	}

	// draw minutes
	{
		float angle = (float)(m % 60) * 6.f;
		sf::Vector3f size(min_w1, min_w2, min_l1 + min_l2);
        sf::Vector2f origin(size.x / 2.f, min_l1);
        target.draw(GetTrapezoid(black, origin, center, angle, size));
	}

	// draw seconds
	{
		float angle = (float)(s % 60) * 6.f;
		sf::Vector2f size(sec_w, sec_l1 + sec_l2);
        sf::Vector2f origin1(size.x / 2.f, sec_l1);
        target.draw(GetRectangle(red, origin1, center, angle, size));

        sf::Vector2f origin2(sec_r, sec_l1);
        float radius = sec_r;
        target.draw(GetCircle(red, origin2, center, angle, radius));
	}
}
*/

void CreateClockHands(sf::Shape& hShape, sf::Shape& mShape, sf::Shape& s1Shape, sf::Shape& s2Shape)
{
    sf::Color black(colhex_black);
    sf::Color red(colhex_red);
    sf::Vector2f center(clock_r, clock_r);

    float angle = 0.f;
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

        sf::Vector2f origin2(sec_r, sec_l1);
        float radius = sec_r;
        s2Shape = GetCircle(red, origin2, center, angle, radius);
    }
}

void DrawClockHands(sf::RenderTarget& target, sf::Shape& hShape, sf::Shape& mShape, sf::Shape& s1Shape, sf::Shape& s2Shape, int h, int m, int s)
{
	// draw hours
	float hAngle = (float)(h % 12) * 30.f + (float)(m % 60) * 0.5f;
	hShape.setRotation(hAngle);
	target.draw(hShape);

	// draw minutes
	float mAngle = (float)(m % 60) * 6.f;
	mShape.setRotation(mAngle);
	target.draw(mShape);

	// draw seconds
	float sAngle = (float)(s % 60) * 6.f;
	s1Shape.setRotation(sAngle);
	s2Shape.setRotation(sAngle);
	target.draw(s1Shape);
	target.draw(s2Shape);
}


void ReadParams(int argc, char** argv)
{

}

int main();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    main();
}


void GetTime(int& Hour, int& Min, int& Sec)
{
    //const std::time_t now = std::time(nullptr);
    //const std::tm calendar_time = *std::localtime(std::addressof(now));

    SYSTEMTIME st;
    GetLocalTime(&st);

    Hour = st.wHour;   //calendar_time.tm_hour;
    Min  = st.wMinute; //calendar_time.tm_min;
    Sec  = st.wSecond; //calendar_time.tm_sec;
    if (stop2go)
    {
        int Msec = st.wMilliseconds;
        float Secf = (float)Sec + (float)Msec / 1000.f;
        //Secf = ((Secf + 1.f) - 30.f) * (60.f / 58.f) + 30.f;
        Secf = Secf * (60.f / 58.f);
        Secf = Secf < 0.f ? 0.f : Secf > 60.f ? 60.f : Secf;
        Sec = (int)Secf;
    }
}


int main()
{
    //ReadParams(argc, argv);
    sf::Vector2i windowSize(wsize, wsize);
    sf::Vector2i windowPosition(-wsize-10, 2160-wsize-10-60);
        //(sf::VideoMode::getDesktopMode().width  - windowSize.x) / 2,
        //(sf::VideoMode::getDesktopMode().height - windowSize.y) / 2);


    // Create the window
    sf::ContextSettings contextSettings;
    contextSettings.antialiasingLevel = 8;
    sf::RenderWindow window;
    window.create(sf::VideoMode(windowSize.x, windowSize.y, 32), "Mondaine Clock", sf::Style::None, contextSettings);
    window.setPosition(windowPosition);
    window.setFramerateLimit(frameLimit);

    // prepare the background
    sf::RenderTexture renderTextureBg;
    renderTextureBg.create(windowSize.x, windowSize.y, contextSettings);
    renderTextureBg.clear(sf::Color(colhex_black)); // sf::Color(255, 0, 255, 255)
    DrawClockBackground(renderTextureBg);
    renderTextureBg.display();
    const sf::Texture& textureBg = renderTextureBg.getTexture();
    sf::Sprite spriteBg(textureBg);


    //ShowWindow(window.getSystemHandle(), SW_HIDE);

    // set icon
    //sf::Image icon;
    //icon.loadFromFile("icon.png"); // File/Image/Pixel
    //window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());

    setShapeCircle(window.getSystemHandle(), windowSize);
    //setTransparency(window.getSystemHandle(), 255);

    bool isMouseDragging = false;
    int lastDownX = 0, lastDownY = 0;

    sf::RectangleShape hShape, mShape, s1Shape; // HACK: it works to assign a sf::ConvexShape with 4 vertices to a sf::RectangleShape
    sf::CircleShape s2Shape;

    CreateClockHands(hShape, mShape, s1Shape, s2Shape);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed || (event.key.code == sf::Keyboard::Escape && event.type == sf::Event::KeyPressed))
            {
                window.close();
            }

            // drag window
            switch (event.type)
            {
            case sf::Event::MouseMoved:
                if (isMouseDragging)
                {
                    window.setPosition(window.getPosition() + sf::Vector2<int>(event.mouseMove.x - lastDownX, event.mouseMove.y - lastDownY));
                }
                break;
            case sf::Event::MouseButtonPressed:
                lastDownX = event.mouseButton.x;
                lastDownY = event.mouseButton.y;
                isMouseDragging = true;
                //window.setFramerateLimit(30);
                break;
            case sf::Event::MouseButtonReleased:
                isMouseDragging = false;
                //window.setFramerateLimit(frameLimit);
                break;
            }
        }

        window.draw(spriteBg); // clear
        int Hour, Min, Sec;
        GetTime(Hour, Min, Sec);
        //Hour = 10; Min = 9; Sec = 37;
        //DrawClockHands(window, Hour, Min, Sec);
        DrawClockHands(window, hShape, mShape, s1Shape, s2Shape, Hour, Min, Sec);

        window.display();
        sf::sleep(sf::milliseconds(480)); // ~(58/60)*1000 /2
    }

    return 0;
}
